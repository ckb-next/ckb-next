#include <limits.h>
#include "device.h"
#include "input.h"
#include "notify.h"

int macromask(const uchar* key1, const uchar* key2){
    // Scan a macro against key input. Return 0 if any of them don't match
    for(int i = 0; i < N_KEYBYTES_INPUT; i++){
        // if((key1[i] & key2[i]) != key2[i])
        if(key1[i] != key2[i])  // Changed to detect G-keys + modifiers
            return 0;
    }
    return 1;
}

///
/// \brief pt_head is the head pointer for the single linked thread list managed by macro_pt_en/dequeue().
static ptlist_t* pt_head = 0;
/// \brief pt_tail is the tail pointer for the single linked thread list managed by macro_pt_en/dequeue().
static ptlist_t* pt_tail = 0;

///
/// \brief macro_pt_enqueue Save the new thread in the single linked list (FIFO).
/// \attention Becuase multiple threads may use this function in parallel,
/// save the critical section with a mutex.
///
static void macro_pt_enqueue() {
    ptlist_t* new_elem = malloc(sizeof(ptlist_t));
    if (!new_elem) {
        perror("macro_pt_enqueue: ");
        exit (-1);  ///< exit on critical situation; \todo find a better exit strategy if no more mem available.
    }
    new_elem->next = 0;
    new_elem->thread_id = pthread_self();   ///< The element knows its ID byself
    if (pt_head == 0) {
        pt_head = pt_tail = new_elem;       ///< new list, first element
    } else {
        pt_tail->next = new_elem;           ///< existing list, append on last element (FIFO)
        pt_tail = new_elem;
    }
}

///
/// \brief macro_pt_dequeue gets the first thread id of the list and returns the thread_id stored in it.
/// \return the ptread_id of the first element. If list is empty, return 0.
/// \attention Becuase multiple threads may use this function in parallel,
/// save the critical section with a mutex.
///
static pthread_t macro_pt_dequeue() {
    pthread_t retval = 0;
    ptlist_t* elem = 0;
    if (pt_head == 0 && pt_tail == 0) {
        ckb_err("macro_pt_dequeue: called on empty list.\n");
        return 0;       ///< why are we called?
    }
    elem = pt_head;
    pt_head = pt_head->next;
    if (pt_head == 0) pt_tail = 0;      ///< Was last element in the list, so clear tail.
    retval = elem->thread_id;           ///< save the return value before deleting element
    free(elem);
    return retval;
}

///
/// \brief macro_pt_first returns the first pthread_id but does not remove the first entry.
/// \return the pthread_id of the first element in the list or 0 if list is empty.
/// \attention Becuase multiple threads may use this function in parallel,
/// save the critical section with a mutex (avoid NIL-ptr)
///
static pthread_t macro_pt_first() {
    return pt_head? pt_head->thread_id : 0;
}

///
/// \brief play_macro is the code for all threads started to play a macro.
/// \param param \a parameter_t to store Kb-ptr and macro-ptr (thread may get only one user-parameter)
/// \return 0 on success, -1 else (no one is interested in it except the kernel...)
///
static void* play_macro(void* param) {
    parameter_t* ptr = (parameter_t*) param;
    usbdevice* kb = ptr->kb;
    keymacro* macro = ptr->macro;

    /// First have a look if we are the first and only macro-thread to run. If not, wait.
    /// So enqueue our thread first, so it is remembered for us and can be seen by all others.
    pthread_mutex_lock(mmutex2(kb));
    macro_pt_enqueue();
    // ckb_info("Entering critical section with 0x%lx. Queue head is 0x%lx\n",  (unsigned long int)pthread_self(), (unsigned long int)macro_pt_first());
    while (macro_pt_first() != pthread_self()) {    ///< If the first thread in the list is not our, another one is running
        // ckb_info("Now waiting with 0x%lx because of 0x%lx\n", (unsigned long int)pthread_self(), (unsigned long int)macro_pt_first());
        pthread_cond_wait(mvar(kb), mmutex2(kb));
        // ckb_info("Waking up with 0x%lx\n", (unsigned long int)pthread_self());
    }
    pthread_mutex_unlock(mmutex2(kb));       ///< Give all new threads the chance to enter the block.

    /// Send events for each keypress in the macro
    pthread_mutex_lock(mmutex(kb)); ///< Synchonization between macro output and color information
    for (int a = 0; a < macro->actioncount; a++) {
        macroaction* action = macro->actions + a;
        if (action->rel_x != 0 || action->rel_y != 0)
            os_mousemove(kb, action->rel_x, action->rel_y);
        else {
            os_keypress(kb, action->scan, action->down);
            pthread_mutex_unlock(mmutex(kb));           ///< use this unlock / relock for enablling the parallel running colorization
            if (action->delay != UINT_MAX && action->delay) {    ///< local delay set
                clock_nanosleep(CLOCK_MONOTONIC, 0, &(struct timespec) {.tv_nsec = action->delay * 1000}, NULL);
            } else if (kb->delay != UINT_MAX && kb->delay) {     ///< use default global delay
                clock_nanosleep(CLOCK_MONOTONIC, 0, &(struct timespec) {.tv_nsec = kb->delay * 1000}, NULL);
            } else if (a < (macro->actioncount - 1)) {  ///< use delays depending on macro length
                if (a > 200) {
                    clock_nanosleep(CLOCK_MONOTONIC, 0, &(struct timespec) {.tv_nsec = action->delay * 100000}, NULL);
                } else if (a > 20) {
                    clock_nanosleep(CLOCK_MONOTONIC, 0, &(struct timespec) {.tv_nsec = 30000}, NULL);
                }
            }
            pthread_mutex_lock(mmutex(kb));
        }
    }

    pthread_mutex_lock(mmutex2(kb));    ///< protect the linked list and the mvar
    // ckb_info("Now leaving 0x%lx and waking up all others\n", (unsigned long int)pthread_self());
    macro_pt_dequeue();
    pthread_cond_broadcast(mvar(kb));   ///< Wake up all waiting threads
    pthread_mutex_unlock(mmutex2(kb));  ///< for the linked list and the mvar

    pthread_mutex_unlock(mmutex(kb));   ///< Sync keyboard input/output and colorization
    return 0;
}

///
/// \brief inputupdate_keys Handle input from Keyboard or mouse; start Macrof if detected.
/// \param kb
///
static void inputupdate_keys(usbdevice* kb){
    usbmode* mode = kb->profile->currentmode;
    binding* bind = &mode->bind;
    usbinput* input = &kb->input;

    // Don't do anything if the state hasn't changed
    if(!memcmp(input->prevkeys, input->keys, N_KEYBYTES_INPUT))
        return;
    // Look for macros matching the current state
    if (kb->active) {
        for (int i = 0; i < bind->macrocount; i++) {
            keymacro* macro = &bind->macros[i];
            if (macromask(input->keys, macro->combo)) {
                if (!macro->triggered) {
                    parameter_t* params = malloc(sizeof(parameter_t));
                    if (params == 0) {
                        perror("inputupdate_keys got no more mem:");
                    } else {
                        pthread_t thread = 0;
                        params->kb = kb;
                        params->macro = macro;
                        int retval = pthread_create(&thread, 0, play_macro, (void*)params);
                        if (retval) {
                            perror("inputupdate_keys: Creating thread returned not null");
                        } else {
                            macro->triggered = 1;
                        }
                    }
                }
            } else macro->triggered = 0;
        }
    }
    // Make a list of keycodes to send. Rearrange them so that modifier keydowns always come first
    // and modifier keyups always come last. This ensures that shortcut keys will register properly
    // even if both keydown events happen at once.
    // N_KEYS + 4 is used because the volume wheel generates keydowns and keyups at the same time
    // (it's currently impossible to press all four at once, but safety first)
    int events[N_KEYS_INPUT + 4];
    int modcount = 0, keycount = 0, rmodcount = 0;
    for(int byte = 0; byte < N_KEYBYTES_INPUT; byte++){
        char oldb = input->prevkeys[byte], newb = input->keys[byte];
        if(oldb == newb)
            continue;
        for(int bit = 0; bit < 8; bit++){
            int keyindex = byte * 8 + bit;
            if(keyindex >= N_KEYS_INPUT)
                break;
            const key* map = keymap + keyindex;
            int scancode = (kb->active) ? bind->base[keyindex] : map->scan;
            char mask = 1 << bit;
            char old = oldb & mask, new = newb & mask;
            // If the key state changed, send it to the input device
            if(old != new){
                // Don't echo a key press if there's no scancode associated
                if(!(scancode & SCAN_SILENT)){
                    if(IS_MOD(scancode)){
                        if(new){
                            // Modifier down: Add to the end of modifier keys
                            for(int i = keycount + rmodcount; i > 0; i--)
                                events[modcount + i] = events[modcount + i - 1];
                            // Add 1 to the scancode because A is zero on OSX
                            // Positive code = keydown, negative code = keyup
                            events[modcount++] = scancode + 1;
                        } else {
                            // Modifier up: Add to the end of everything
                            events[modcount + keycount + rmodcount++] = -(scancode + 1);
                        }
                    } else {
                        // Regular keypress: add to the end of regular keys
                        for(int i = rmodcount; i > 0; i--)
                            events[modcount + keycount + i] = events[modcount + keycount + i - 1];
                        events[modcount + keycount++] = new ? (scancode + 1) : -(scancode + 1);
                        // The volume wheel and the mouse wheel don't generate keyups, so create them automatically
#define IS_WHEEL(scan, kb)  (((scan) == KEY_VOLUMEUP || (scan) == KEY_VOLUMEDOWN || (scan) == BTN_WHEELUP || (scan) == BTN_WHEELDOWN) && (!IS_K65(kb) && !IS_K63(kb)))
                        if(new && IS_WHEEL(map->scan, kb)){
                            for(int i = rmodcount; i > 0; i--)
                                events[modcount + keycount + i] = events[modcount + keycount + i - 1];
                            events[modcount + keycount++] = -(scancode + 1);
                            input->keys[byte] &= ~mask;
                        }
                    }
                }
                // Print notifications if desired
                if(kb->active){
                    for(int notify = 0; notify < OUTFIFO_MAX; notify++){
                        if(mode->notify[notify][byte] & mask){
                            nprintkey(kb, notify, keyindex, new);
                            // Wheels doesn't generate keyups
                            if(new && IS_WHEEL(map->scan, kb))
                                nprintkey(kb, notify, keyindex, 0);
                        }
                    }
                }
            }
        }
    }
    /// Process all queued keypresses if no macro is running yet.
    /// \todo If we want to get all keys typed while a macro is played, add the code for it here.
    if (!macro_pt_first()) {
        int totalkeys = modcount + keycount + rmodcount;
        for(int i = 0; i < totalkeys; i++){
            int scancode = events[i];
            os_keypress(kb, (scancode < 0 ? -scancode : scancode) - 1, scancode > 0);
        }
    }
}

void inputupdate(usbdevice* kb){
#ifdef OS_LINUX
    if((!kb->uinput_kb || !kb->uinput_mouse)
#else
    if(!kb->event
#endif
            || !kb->profile)
        return;
    // Process key/button input
    inputupdate_keys(kb);
    // Process mouse movement
    usbinput* input = &kb->input;
    if(input->rel_x != 0 || input->rel_y != 0){
        os_mousemove(kb, input->rel_x, input->rel_y);
        input->rel_x = input->rel_y = 0;
    }
    // Finish up
    memcpy(input->prevkeys, input->keys, N_KEYBYTES_INPUT);
}

void updateindicators_kb(usbdevice* kb, int force){
    // Read current hardware indicator state (set externally)
    uchar old = kb->ileds, hw_old = kb->hw_ileds_old;
    uchar new = kb->hw_ileds, hw_new = new;
    // Update them if needed
    if(kb->active){
        usbmode* mode = kb->profile->currentmode;
        new = (new & ~mode->ioff) | mode->ion;
    }
    kb->ileds = new;
    kb->hw_ileds_old = hw_new;
    if(old != new || force){
        DELAY_SHORT(kb);
        os_sendindicators(kb);
    }
    // Print notifications if desired
    if(!kb->active)
        return;
    usbmode* mode = kb->profile->currentmode;
    uchar indicators[] = { I_NUM, I_CAPS, I_SCROLL };
    for(unsigned i = 0; i < sizeof(indicators) / sizeof(uchar); i++){
        uchar mask = indicators[i];
        if((hw_old & mask) == (hw_new & mask))
            continue;
        for(int notify = 0; notify < OUTFIFO_MAX; notify++){
            if(mode->inotify[notify] & mask)
                nprintind(kb, notify, mask, hw_new & mask);
        }
    }
}

void initbind(binding* bind){
    for(int i = 0; i < N_KEYS_INPUT; i++)
        bind->base[i] = keymap[i].scan;
    bind->macros = calloc(32, sizeof(keymacro));
    bind->macrocap = 32;
    bind->macrocount = 0;
}

void freebind(binding* bind){
    for(int i = 0; i < bind->macrocount; i++)
        free(bind->macros[i].actions);
    free(bind->macros);
    memset(bind, 0, sizeof(*bind));
}

void cmd_bind(usbdevice* kb, usbmode* mode, int dummy, int keyindex, const char* to){
    (void)dummy;

    if(keyindex >= N_KEYS_INPUT)
        return;
    // Find the key to bind to
    int tocode = 0;
    if(sscanf(to, "#x%ux", &tocode) != 1 && sscanf(to, "#%u", &tocode) == 1 && tocode < N_KEYS_INPUT){
        pthread_mutex_lock(imutex(kb));
        mode->bind.base[keyindex] = tocode;
        pthread_mutex_unlock(imutex(kb));
        return;
    }
    // If not numeric, look it up
    for(int i = 0; i < N_KEYS_INPUT; i++){
        if(keymap[i].name && !strcmp(to, keymap[i].name)){
            pthread_mutex_lock(imutex(kb));
            mode->bind.base[keyindex] = keymap[i].scan;
            pthread_mutex_unlock(imutex(kb));
            return;
        }
    }
}

void cmd_unbind(usbdevice* kb, usbmode* mode, int dummy, int keyindex, const char* to){
    (void)dummy;
    (void)to;

    if(keyindex >= N_KEYS_INPUT)
        return;
    pthread_mutex_lock(imutex(kb));
    mode->bind.base[keyindex] = KEY_UNBOUND;
    pthread_mutex_unlock(imutex(kb));
}

void cmd_rebind(usbdevice* kb, usbmode* mode, int dummy, int keyindex, const char* to){
    (void)dummy;
    (void)to;

    if(keyindex >= N_KEYS_INPUT)
        return;
    pthread_mutex_lock(imutex(kb));
    mode->bind.base[keyindex] = keymap[keyindex].scan;
    pthread_mutex_unlock(imutex(kb));
}

static void _cmd_macro(usbmode* mode, const char* keys, const char* assignment){
    binding* bind = &mode->bind;
    if(!keys && !assignment){
        // Null strings = "macro clear" -> erase the whole thing
        for(int i = 0; i < bind->macrocount; i++)
            free(bind->macros[i].actions);
        bind->macrocount = 0;
        return;
    }
    if(bind->macrocount >= MACRO_MAX)
        return;
    // Create a key macro
    keymacro macro;
    memset(&macro, 0, sizeof(macro));
    // Scan the left side for key names, separated by +
    int empty = 1;
    int left = strlen(keys), right = strlen(assignment);
    int position = 0, field = 0;
    char keyname[24];
    while(position < left && sscanf(keys + position, "%10[^+]%n", keyname, &field) == 1){
        int keycode;
        if((sscanf(keyname, "#%d", &keycode) && keycode >= 0 && keycode < N_KEYS_INPUT)
                  || (sscanf(keyname, "#x%x", &keycode) && keycode >= 0 && keycode < N_KEYS_INPUT)){
            // Set a key numerically
            SET_KEYBIT(macro.combo, keycode);
            empty = 0;
        } else {
            // Find this key in the keymap
            for(unsigned i = 0; i < N_KEYS_INPUT; i++){
                if(keymap[i].name && !strcmp(keyname, keymap[i].name)){
                    macro.combo[i / 8] |= 1 << (i % 8);
                    empty = 0;
                    break;
                }
            }
        }
        if(keys[position += field] == '+')
            position++;
    }
    if(empty)
        return;
    // Count the number of actions (comma separated)
    int count = 1;
    for(const char* c = assignment; *c != 0; c++){
        if(*c == ',')
            count++;
    }
    // Allocate a buffer for them
    macro.actions = calloc(count, sizeof(macroaction));
    macro.actioncount = 0;
    // Scan the actions
    position = 0;
    field = 0;
    // max action = old 11 chars plus 12 chars which is the max 32-bit int 4294967295 size
    while(position < right && sscanf(assignment + position, "%23[^,]%n", keyname, &field) == 1){
        if(!strcmp(keyname, "clear"))
            break;

        // Check for local key delay of the form '[+-]<key>=<delay>'
        long int long_delay;    // scanned delay value, used to keep delay in range.
        unsigned int delay = UINT_MAX; // computed delay value. UINT_MAX means use global delay value.
        char real_keyname[12];  // temp to hold the left side (key) of the <key>=<delay>
        int scan_matches = sscanf(keyname, "%11[^=]=%ld", real_keyname, &long_delay);
        if (scan_matches == 2) {
            if (0 <= long_delay && long_delay < UINT_MAX) {
                delay = (unsigned int)long_delay;
                strcpy(keyname, real_keyname); // keyname[24], real_keyname[12]
            }
        }

        int down = (keyname[0] == '+');
        if(down || keyname[0] == '-'){
            int keycode;
            if((sscanf(keyname + 1, "#%d", &keycode) && keycode >= 0 && keycode < N_KEYS_INPUT)
                      || (sscanf(keyname + 1, "#x%x", &keycode) && keycode >= 0 && keycode < N_KEYS_INPUT)){
                // Set a key numerically
                macro.actions[macro.actioncount].scan = keymap[keycode].scan;
                macro.actions[macro.actioncount].down = down;
                macro.actions[macro.actioncount].delay = delay;
                macro.actioncount++;
            } else {
                // Find this key in the keymap
                for(unsigned i = 0; i < N_KEYS_INPUT; i++){
                    if(keymap[i].name && !strcmp(keyname + 1, keymap[i].name)){
                        macro.actions[macro.actioncount].scan = keymap[i].scan;
                        macro.actions[macro.actioncount].down = down;
                        macro.actions[macro.actioncount].delay = delay;
                        macro.actioncount++;
                        break;
                    }
                }
            }
        }
        if(assignment[position += field] == ',')
            position++;
    }

    // See if there's already a macro with this trigger
    keymacro* macros = bind->macros;
    for(int i = 0; i < bind->macrocount; i++){
        if(!memcmp(macros[i].combo, macro.combo, N_KEYBYTES_INPUT)){
            free(macros[i].actions);
            // If the new macro has no actions, erase the existing one
            if(!macro.actioncount){
                for(int j = i + 1; j < bind->macrocount; j++)
                    memcpy(macros + j - 1, macros + j, sizeof(keymacro));
                bind->macrocount--;
            } else
                // If there are actions, replace the existing with the new
                memcpy(macros + i, &macro, sizeof(keymacro));
            return;
        }
    }

    // Add the macro to the device settings if not empty
    if(macro.actioncount < 1)
        return;
    memcpy(bind->macros + (bind->macrocount++), &macro, sizeof(keymacro));
    if(bind->macrocount >= bind->macrocap)
        bind->macros = realloc(bind->macros, (bind->macrocap += 16) * sizeof(keymacro));
}

void cmd_macro(usbdevice* kb, usbmode* mode, const int notifynumber, const char* keys, const char* assignment){
    (void)notifynumber;

    pthread_mutex_lock(imutex(kb));
    _cmd_macro(mode, keys, assignment);
    pthread_mutex_unlock(imutex(kb));
}

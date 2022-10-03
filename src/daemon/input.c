#include "os.h"

#include <limits.h>
#include "device.h"
#include "input.h"
#include "notify.h"
#include <assert.h>

#define IS_SCROLLWHEEL_V(scan)  ((scan) == BTN_WHEELUP   || (scan) == BTN_WHEELDOWN)
#define IS_SCROLLWHEEL_H(scan)  ((scan) == BTN_WHEELLEFT || (scan) == BTN_WHEELRIGHT)
#define IS_VOLWHEEL(scan)      (((scan) == KEY_VOLUMEUP  || (scan) == KEY_VOLUMEDOWN) && DEV_HAS_VOLWHEEL(kb))

static int macromask(const uchar* keys, const uchar* macro){
    // Scan a macro against key input. Return 0 if any of them don't match
    for(int i = 0; i < N_KEYBYTES_INPUT; i++){
        if((keys[i] & macro[i]) != macro[i])
            return 0;
    }
    return 1;
}

///
/// \brief pt_head is the head pointer for the single linked thread list managed by macro_pt_en/dequeue().
static ptlist_t* pt_head = 0;
/// \brief pt_tail is the tail pointer for the single linked thread list managed by macro_pt_en/dequeue().
static ptlist_t* pt_tail = 0;

// FIXME: Most of the following macro queueing code is wrong.
// We can't assume pthread_t is an arithmetic type, so assigning it to 0 is incorrect.

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
        ckb_err("macro_pt_dequeue: called on empty list.");
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

// Default macro keystroke delay
const struct timespec macrodelay = { .tv_nsec = 1000000 };
// Initial repeat delay
#define DELAY_REPEAT_INITIAL 500000000
// Delay for every subsequent repeat
#define DELAY_REPEAT_CATCHUP 500000000

static inline void clock_microsleep(uint32_t s) {
    const struct timespec ts = {
        .tv_sec = s / 1000000,
        .tv_nsec = (s % 1000000) * 1000,
    };
    clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
}

///
/// \brief play_macro is the code for all threads started to play a macro.
/// \param param \a macro_param to store Kb-ptr and macro-ptr (thread may get only one user-parameter)
/// \return 0 on success, -1 else (no one is interested in it except the kernel...)
///
static void* play_macro(void* param) {
    macro_param* ptr = param;
    usbdevice* kb = ptr->kb;
    keymacro* macro = ptr->macro;

#ifdef OS_MAC
    pthread_setname_np("ckb macro");
#endif // OS_MAC

    /// First have a look if we are the first and only macro-thread to run. If not, wait.
    /// So enqueue our thread first, so it is remembered for us and can be seen by all others.
    pthread_mutex_lock(mmutex2(kb));
    macro_pt_enqueue();
    // ckb_info("Entering critical section with 0x%lx. Queue head is 0x%lx",  (unsigned long int)pthread_self(), (unsigned long int)macro_pt_first());
    while (macro_pt_first() != pthread_self()) {    ///< If the first thread in the list is not our, another one is running
        // ckb_info("Now waiting with 0x%lx because of 0x%lx", (unsigned long int)pthread_self(), (unsigned long int)macro_pt_first());
        pthread_cond_wait(mvar(kb), mmutex2(kb));
        // ckb_info("Waking up with 0x%lx", (unsigned long int)pthread_self());
    }
    pthread_mutex_unlock(mmutex2(kb));       ///< Give all new threads the chance to enter the block.

    /*
     * Is this the first time the macro is being repeated due to the key being
     * held down continuously?
     */
    int first_keydownloop = 1;
    while(1){
        /// Send events for each keypress in the macro
        queued_mutex_lock(mmutex(kb)); ///< Synchonization between macro output and color information
        for (int a = 0; a < ptr->actioncount; a++) {
            macroaction* action = ptr->actions + a;
            if (action->rel_x != 0 || action->rel_y != 0){
                os_mousemove(kb, action->rel_x, action->rel_y);
                os_inputsync(kb, 0, 1);
            } else {
                if(IS_SCROLLWHEEL_V(action->scan)) {
                    if(action->down)
                        os_mousescroll(kb, 0, (action->scan == BTN_WHEELUP ? 1 : -1));
                } else if(IS_SCROLLWHEEL_H(action->scan)) {
                    if(action->down)
                        os_mousescroll(kb, (action->scan == BTN_WHEELLEFT ? -1 : 1), 0);
                } else {
                    os_keypress(kb, action->scan, action->down);
                }
                int sync_mouse = action->scan & SCAN_MOUSE;
                os_inputsync(kb, !sync_mouse, sync_mouse);

                queued_mutex_unlock(mmutex(kb));           ///< use this unlock / relock for enablling the parallel running colorization
                if (action->delay != UINT32_MAX && action->delay) {    ///< local delay set
                    if(action->delay_max) {
                        // Generate a random delay in the range requested
                        // First, we need a random number that can be as large as uint32_t
                        // RAND_MAX can be as low as 0x7fff
                        uint16_t second = rand() & 0x7fff;
                        uint16_t first = rand() & 0x7fff;
                        uint32_t final = ((uint32_t)first << 17);
                        final |= (second << 1);
                        // Now that we have a sufficently large random number, range it appropriately
                        final = ((uint64_t)action->delay + (uint64_t)final) % (action->delay_max - action->delay);
                        clock_microsleep(final);
                    } else {
                        clock_microsleep(action->delay);
                    }
                } else {
                    clock_nanosleep(CLOCK_MONOTONIC, 0, &macrodelay, NULL);
                }
                queued_mutex_lock(mmutex(kb));
            }
        }

        queued_mutex_unlock(mmutex(kb));

        int delay_ns = first_keydownloop ? DELAY_REPEAT_INITIAL : DELAY_REPEAT_CATCHUP;

        queued_mutex_lock(imutex(kb));
        // detect if the macro playback has been aborted
        if (ptr->abort) {
            // we are responsible for free-ing actions.
            free(ptr->actions);
            break;
        }

        if (macro->triggered > 1) {
            macro->triggered -= 2;
            delay_ns = 0;
            first_keydownloop = 1;
        }

        if (macro->triggered == 0)
            break;

        if (!delay_ns) {
            queued_mutex_unlock(imutex(kb));
            continue;
        }

        queued_cond_nanosleep(mintvar(kb), imutex(kb), delay_ns);

        if (macro->triggered == 2) {
            // the key was released, but not pressed again, during the sleep
            macro->triggered = 0;
            break;
        }

        // if the key released and pressed during the sleep, reset to use DELAY_REPEAT_INITIAL
        if (macro->triggered == 1)
            first_keydownloop = 0;

        queued_mutex_unlock(imutex(kb));
    }
    if (!ptr->abort)
        macro->param = NULL;
    queued_mutex_unlock(imutex(kb));

    queued_mutex_lock(mmutex(kb));

    pthread_mutex_lock(mmutex2(kb));    ///< protect the linked list and the mvar
    // ckb_info("Now leaving 0x%lx and waking up all others", (unsigned long int)pthread_self());
    macro_pt_dequeue();
    pthread_cond_broadcast(mvar(kb));   ///< Wake up all waiting threads
    pthread_mutex_unlock(mmutex2(kb));  ///< for the linked list and the mvar

    queued_mutex_unlock(mmutex(kb));   ///< Sync keyboard input/output and colorization
    free(param);
    return 0;
}

// Checks if the macro mask contains any wheels to prevent it from looping endlessly
static inline int is_wheel_keybit(const usbdevice* kb, const uchar* macro){
    for(int i = 0; i < N_KEYBYTES_INPUT; i++){
        // Most entries are probably going to be 0, so skip over them
        if(!macro[i])
            continue;
        // Go through each bit
        for(int j = 0; j < 8; j++){
            if(!((macro[i] >> j) & 1))
                continue;
            // Get the index of the item and look it up in the keymap
            const key* ckey = kb->keymap + i * 8 + j;
            // If there's at least a single wheel, return true
            if(IS_VOLWHEEL(ckey->scan) || IS_SCROLLWHEEL_V(ckey->scan) || IS_SCROLLWHEEL_H(ckey->scan))
                return 1;
        }
    }
    return 0;
}

///
/// \brief inputupdate_keys Handle input from Keyboard or mouse; start Macro if detected.
/// \param kb
///
static inline void inputupdate_keys(usbdevice* kb, int* sync_kb, int* sync_mouse){
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
            // see the definition of keymacro.triggered in structures.h
            if (macromask(input->keys, macro->combo)) {
                if(!(macro->triggered & 1)){
                    macro->triggered++;
                    // Because there are no keyup events for wheels, pretend we received them
                    // to prevent endless looping
                    if(is_wheel_keybit(kb, macro->combo))
                        macro->triggered++;
                } else
                    continue;
            } else {
                if((macro->triggered & 1)) {
                    macro->triggered++;
                    pthread_cond_broadcast(mintvar(kb));
                }
                continue;
            }

            if (macro->triggered <= 3) {
                // start up a thread if there isn't already one running
                if (!macro->param) {
                    // assert(macro->triggered == 1)
                    macro_param* params = malloc(sizeof(macro_param));
                    if (params == NULL) {
                        perror("inputupdate_keys got no more mem:");
                    } else {
                        params->kb = kb;
                        params->macro = macro;
                        params->actions = macro->actions;
                        params->actioncount = macro->actioncount;
                        params->abort = 0;
                        macro->param = params;
                        pthread_t thread;
                        int retval = pthread_create(&thread, 0, play_macro, params);
                        if (retval) {
                            macro->triggered = 0;
                            macro->param = NULL;
                            perror("inputupdate_keys: Creating thread returned not null");
                        } else {
                            pthread_detach(thread);

#ifndef OS_MAC
                            // name thread externally if it was created on non-mac systems
                            // ignore the result
                            pthread_setname_np(thread, "ckb macro");
#endif // OS_MAC
                        }
                    }
                } else {
                    // if there is already a thread running, it may be waiting for DELAY_REPEAT_*
                    // it does not need to wait anymore
                    pthread_cond_broadcast(mintvar(kb));
                }
            }
        }
    }
    // Make a list of keycodes to send. Rearrange them so that modifier keydowns always come first
    // and modifier keyups always come last. This ensures that shortcut keys will register properly
    // even if both keydown events happen at once.
    // + 2 is used because the volume wheel generates keydowns and keyups at the same time
    // + (-SCHAR_MIN) * 2 * 2 is used because a mouse wheel can generate up to 128*2 events at once, one for x, and one for y to be safe
    static_assert(sizeof(input->whl_rel_x) == sizeof(char) && sizeof(input->whl_rel_y) == sizeof(char),
        "Please change SCHAR_MIN below to the one for the appropriate type used by the scroll wheel.");
    int events[N_KEYS_INPUT + 2 + (-SCHAR_MIN) * 2 * 2];
    int modcount = 0, keycount = 0, rmodcount = 0;
    for(int byte = 0; byte < N_KEYBYTES_INPUT; byte++){
        char oldb = input->prevkeys[byte], newb = input->keys[byte];
        if(oldb == newb)
            continue;
        for(int bit = 0; bit < 8; bit++){
            int keyindex = byte * 8 + bit;
            if(keyindex >= N_KEYS_INPUT)
                break;
            const key* map = kb->keymap + keyindex;
            int scancode = (kb->active) ? bind->base[keyindex] : map->scan;
            char mask = 1 << bit;
            char old = oldb & mask, new = newb & mask;
            // If the key state changed, send it to the input device
            if(old != new){
                // Don't echo a key press if there's no scancode associated
                if(!(scancode & SCAN_SILENT)){
                    if(IS_MOD(scancode)){
                        // Only keyboards have modifiers, so ask explicitly for a sync if we only have a modifier in the queue
                        *sync_kb = 1;
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
                    } else if(IS_SCROLLWHEEL_V(scancode)) {
                        // Simulate scrolling if this came from a macro or a device without an axis (keyboard)
                        if(!input->whl_rel_y)
                            input->whl_rel_y = (scancode == BTN_WHEELUP ? 1 : -1);
                        os_mousescroll(kb, 0, input->whl_rel_y);
                        *sync_mouse = 1;
                    } else if(IS_SCROLLWHEEL_H(scancode)) {
                        // Simulate horizontal scrolling as no device supports this yet
                        // The only exception is binding the vertical scroll wheel to the horizontal one
                        if(IS_SCROLLWHEEL_V(map->scan))
                            input->whl_rel_x = input->whl_rel_y;
                        else
                            input->whl_rel_x = (scancode == BTN_WHEELLEFT ? -1 : 1);
                        os_mousescroll(kb, input->whl_rel_x, 0);
                        *sync_mouse = 1;
                    } else {
                        // Regular keypress: add to the end of regular keys
                        if(scancode & SCAN_MOUSE)
                            *sync_mouse = 1;
                        else
                            *sync_kb = 1;
                        memmove(events + modcount + keycount + 1, events + modcount + keycount, sizeof(int) * rmodcount);
                        events[modcount + keycount++] = new ? (scancode + 1) : -(scancode + 1);
                    }
                }

                // The volume/scroll wheels don't generate keyups, so create them automatically when necessary
                if(new){
                    if(scancode != KEY_UNBOUND){
                        // If it's a volume wheel, just add a single keyup event to the FIFO
                        // Else if the event originated from a scroll wheel, and is bound to something other than a scroll wheel,
                        // multiply the last few events by the amount of events received from the hid report
                        if(IS_VOLWHEEL(map->scan)){
                            // Make room for it
                            memmove(events + modcount + keycount + 1, events + modcount + keycount, sizeof(int) * rmodcount);
                            // Duplicate the last event, but with keyup
                            events[modcount + keycount++] = -(scancode + 1);
                        } else if (IS_SCROLLWHEEL_V(map->scan)){
                            if(!IS_SCROLLWHEEL_V(scancode)){
                                if(!input->whl_rel_y){
                                    ckb_err("whl_rel_y is 0");
                                    input->whl_rel_y = 1;
                                }
                                const int rel_y = abs(input->whl_rel_y);
                                // Make room for the extra events. We already have one, thus - 1.
                                memmove(events + modcount + keycount + rel_y * 2 - 1, events + modcount + keycount, sizeof(int) * rmodcount);
                                // Duplicate the last event, but with keyup
                                events[modcount + keycount++] = -(scancode + 1);

                                // Copy the last two events as many times as needed
                                for(int i = 0; i < rel_y - 1; i++){
                                    memcpy(events + modcount + keycount, events + modcount + keycount - 2, sizeof(int) * 2);
                                    keycount += 2;
                                }
                            }
                        }
                    }
                    // Clear the key bit so that we can receive events of this type in the next run
                    if(IS_VOLWHEEL(map->scan) || IS_SCROLLWHEEL_V(map->scan))
                        input->keys[byte] &= ~mask;
                }

                // Print notifications if desired
                if(kb->active){
                    for(int notify = 0; notify < OUTFIFO_MAX; notify++){
                        if(mode->notify[notify][byte] & mask){
                            // Wheels don't generate keyups and additionally can produce multiple events
                            // with a single report. No device so far supports scrolling horizontally,
                            // so only vertical notifications are implemented.
                            if(IS_SCROLLWHEEL_V(map->scan)){
                                for(int i = 0; i < abs(input->whl_rel_y); i++){
                                    nprintkey(kb, notify, keyindex, 1);
                                    nprintkey(kb, notify, keyindex, 0);
                                }
                            } else if(IS_VOLWHEEL(map->scan)){
                                    nprintkey(kb, notify, keyindex, 1);
                                    nprintkey(kb, notify, keyindex, 0);
                            } else {
                                nprintkey(kb, notify, keyindex, new);
                            }
                        }
                    }
                }
            }
        }
    }
    // Process all queued keypresses
    int totalkeys = modcount + keycount + rmodcount;
    int prev_scan = KEY_UNBOUND;
    for(int i = 0; i < totalkeys; i++){
        const int scancode = events[i];
        const int absscan = abs(scancode);
        // Force a keyboard sync if the event queue contains two of the same scancode before the second keypress is sent
        // This can't happen in mice because the wheel is an axis
        if(prev_scan == absscan){
#ifdef DEBUG_INPUT_SYNC
            ckb_info("Pre duplicate event sync");
#endif
            os_inputsync(kb, 1, 0);
#ifdef DEBUG_INPUT_SYNC
            ckb_info("Post duplicate event sync");
#endif
        }
        os_keypress(kb, absscan - 1, scancode > 0);
        prev_scan = absscan;
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

    usbinput* input = &kb->input;
    int sync_kb = 0;
    int sync_mouse = 0;

    // Let inputupdate_keys handle the wheel, while also keeping
    // the data reported by the mouse for the axis.
    // This is done to allow rebinding

    // Wheel up/down
    if(input->whl_rel_y > 0)
        SET_KEYBIT(input->keys, MOUSE_EXTRA_FIRST);
    else
        CLEAR_KEYBIT(input->keys, MOUSE_EXTRA_FIRST);

    if(input->whl_rel_y < 0)
        SET_KEYBIT(input->keys, MOUSE_EXTRA_FIRST + 1);
    else
        CLEAR_KEYBIT(input->keys, MOUSE_EXTRA_FIRST + 1);

    // Process key/button input
    inputupdate_keys(kb, &sync_kb, &sync_mouse);
    // Process mouse movement
    if(input->rel_x != 0 || input->rel_y != 0){
        os_mousemove(kb, input->rel_x, input->rel_y);
        input->rel_x = input->rel_y = 0;
        sync_mouse = 1;
    }
    // Finish up
    memcpy(input->prevkeys, input->keys, N_KEYBYTES_INPUT);
    input->whl_rel_x = input->whl_rel_y = 0;
    os_inputsync(kb, sync_kb, sync_mouse);
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
        kb->vtable.delay(kb, DELAY_INDICATORS);

        ushort leds = kb->ileds;
        int len = 1;
        if(kb->fwversion >= 0x300 || IS_V3_OVERRIDE(kb) || kb->protocol == PROTO_BRAGI) {
            leds = (kb->ileds << 8) | 0x0001;
            len = 2;
        }
        queued_mutex_unlock(dmutex(kb));
        ctrltransfer transfer = { .bRequestType = 0x21, .bRequest = 0x09, .wValue = 0x0200, .wIndex = 0, .wLength = len, .timeout = 5000, .data = &leds };
        if(kb->protocol == PROTO_BRAGI)
            transfer.wValue++;
        os_usb_control(kb, &transfer, __FILE_NOPATH__, __LINE__);
        queued_mutex_lock(dmutex(kb));
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

///
/// \brief destroymacro free actions if macro is not currently running
/// \param macro
///
static inline void destroymacro(keymacro* macro) {
    if (macro->param)
        macro->param->abort = 1;
    else
        free(macro->actions);
}

void initbind(binding* bind, usbdevice* kb){
    for(int i = 0; i < N_KEYS_INPUT; i++)
        bind->base[i] = kb->keymap[i].scan;
    bind->macros = calloc(32, sizeof(keymacro));
    bind->macrocap = 32;
    bind->macrocount = 0;
}

void freebind(binding* bind){
    for(int i = 0; i < bind->macrocount; i++)
        destroymacro(&bind->macros[i]);
    free(bind->macros);
    memset(bind, 0, sizeof(*bind));
}

void cmd_bind(usbdevice* kb, usbmode* mode, int dummy, int keyindex, const char* to){
    (void)dummy;

    if(keyindex >= N_KEYS_INPUT)
        return;
    // Find the key to bind to
    uint tocode = 0;
    if(sscanf(to, "#x%ux", &tocode) != 1 && sscanf(to, "#%u", &tocode) == 1 && tocode < N_KEYS_INPUT){
        queued_mutex_lock(imutex(kb));
        mode->bind.base[keyindex] = tocode;
        queued_mutex_unlock(imutex(kb));
        return;
    }
    // If not numeric, look it up
    for(int i = 0; i < N_KEYS_INPUT; i++){
        if(kb->keymap[i].name && !strcmp(to, kb->keymap[i].name)){
            queued_mutex_lock(imutex(kb));
            mode->bind.base[keyindex] = kb->keymap[i].scan;
            queued_mutex_unlock(imutex(kb));
            return;
        }
    }
}

void cmd_unbind(usbdevice* kb, usbmode* mode, int dummy, int keyindex, const char* to){
    (void)dummy;
    (void)to;

    if(keyindex >= N_KEYS_INPUT)
        return;
    queued_mutex_lock(imutex(kb));
    mode->bind.base[keyindex] = KEY_UNBOUND;
    queued_mutex_unlock(imutex(kb));
}

void cmd_rebind(usbdevice* kb, usbmode* mode, int dummy, int keyindex, const char* to){
    (void)dummy;
    (void)to;

    if(keyindex >= N_KEYS_INPUT)
        return;
    queued_mutex_lock(imutex(kb));
    mode->bind.base[keyindex] = kb->keymap[keyindex].scan;
    queued_mutex_unlock(imutex(kb));
}

static void _cmd_macro(usbmode* mode, const char* keys, const char* assignment, usbdevice* kb){
    binding* bind = &mode->bind;
    if(!keys && !assignment){
        // Null strings = "macro clear" -> erase the whole thing
        for(int i = 0; i < bind->macrocount; i++)
            destroymacro(&bind->macros[i]);
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
    char keyname[40];
    while(position < left && sscanf(keys + position, "%10[^+]%n", keyname, &field) == 1){
        // Find this key in the keymap
        for(unsigned i = 0; i < N_KEYS_INPUT; i++){
            if(kb->keymap[i].name && !strcmp(keyname, kb->keymap[i].name)){
                macro.combo[i / 8] |= 1 << (i % 8);
                empty = 0;
                break;
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
    // max action = old 11 chars plus 12 chars which is the max 32-bit unsigned int 4294967295 size * 2 + 1 for range underscore
    while(position < right && sscanf(assignment + position, "%36[^,]%n", keyname, &field) == 1){
        if(!strcmp(keyname, "clear"))
            break;

        // Check for local key delay of the form '[+-]<key>=<delay>'
        int64_t long_delay;    // scanned delay value, used to keep delay in range.
        int64_t long_delay_range = 0;
        uint32_t delay = UINT32_MAX; // computed delay value. UINT32_MAX means use the default value.
        uint32_t delay_range = 0;
        char real_keyname[12];  // temp to hold the left side (key) of the <key>=<delay>
        int scan_matches = sscanf(keyname, "%11[^=]=%"SCNd64"_%"SCNd64, real_keyname, &long_delay, &long_delay_range);
        if (scan_matches == 2 || scan_matches == 3) {
            if (0 <= long_delay && long_delay < UINT32_MAX) {
                delay = (uint32_t)long_delay;
                strcpy(keyname, real_keyname); // keyname[40], real_keyname[12]
                if(0 < long_delay_range && long_delay_range < UINT32_MAX
                        && long_delay_range > long_delay) {
                    delay_range = (uint32_t)long_delay_range;
                }
            }
        }

        int down = (keyname[0] == '+');
        if(down || keyname[0] == '-'){
            // Find this key in the keymap
            for(unsigned i = 0; i < N_KEYS_INPUT; i++){
                if(kb->keymap[i].name && !strcmp(keyname + 1, kb->keymap[i].name)){
                    macro.actions[macro.actioncount].scan = kb->keymap[i].scan;
                    macro.actions[macro.actioncount].down = down;
                    macro.actions[macro.actioncount].delay = delay;
                    macro.actions[macro.actioncount].delay_max = delay_range;
                    macro.actioncount++;
                    break;
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
            destroymacro(&macros[i]);
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

    queued_mutex_lock(imutex(kb));
    _cmd_macro(mode, keys, assignment, kb);
    queued_mutex_unlock(imutex(kb));
}

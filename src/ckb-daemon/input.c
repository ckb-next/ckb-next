#include "device.h"
#include "input.h"
#include "notify.h"

int macromask(const uchar* key1, const uchar* key2){
    // Scan a macro against key input. Return 0 if any of them don't match
    for(int i = 0; i < N_KEYBYTES_INPUT; i++){
        if((key1[i] & key2[i]) != key2[i])
            return 0;
    }
    return 1;
}

static void inputupdate_keys(usbdevice* kb){
    usbmode* mode = kb->profile->currentmode;
    binding* bind = &mode->bind;
    usbinput* input = &kb->input;
    // Don't do anything if the state hasn't changed
    if(!memcmp(input->prevkeys, input->keys, N_KEYBYTES_INPUT))
        return;
    // Look for macros matching the current state
    int macrotrigger = 0;
    if(kb->active){
        for(int i = 0; i < bind->macrocount; i++){
            keymacro* macro = &bind->macros[i];
            if(macromask(input->keys, macro->combo)){
                if(!macro->triggered){
                    macrotrigger = 1;
                    macro->triggered = 1;
                    // Send events for each keypress in the macro
                    for(int a = 0; a < macro->actioncount; a++){
                        macroaction* action = macro->actions + a;
                        if(action->rel_x != 0 || action->rel_y != 0)
                            os_mousemove(kb, action->rel_x, action->rel_y);
                        else
                            os_keypress(kb, action->scan, action->down);
                    }
                }
            } else {
                macro->triggered = 0;
            }
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
                // Don't echo a key press if a macro was triggered or if there's no scancode associated
                if(!macrotrigger && !(scancode & SCAN_SILENT)){
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
#define IS_WHEEL(scan, kb)  (((scan) == KEY_VOLUMEUP || (scan) == KEY_VOLUMEDOWN || (scan) == BTN_WHEELUP || (scan) == BTN_WHEELDOWN) && !IS_K65(kb))
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
    // Process all queued keypresses
    int totalkeys = modcount + keycount + rmodcount;
    for(int i = 0; i < totalkeys; i++){
        int scancode = events[i];
        os_keypress(kb, (scancode < 0 ? -scancode : scancode) - 1, scancode > 0);
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
    os_isync(kb);
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
    if(keyindex >= N_KEYS_INPUT)
        return;
    pthread_mutex_lock(imutex(kb));
    mode->bind.base[keyindex] = KEY_UNBOUND;
    pthread_mutex_unlock(imutex(kb));
}

void cmd_rebind(usbdevice* kb, usbmode* mode, int dummy, int keyindex, const char* to){
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
    char keyname[12];
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
    while(position < right && sscanf(assignment + position, "%11[^,]%n", keyname, &field) == 1){
        if(!strcmp(keyname, "clear"))
            break;
        int down = (keyname[0] == '+');
        if(down || keyname[0] == '-'){
            int keycode;
            if((sscanf(keyname + 1, "#%d", &keycode) && keycode >= 0 && keycode < N_KEYS_INPUT)
                      || (sscanf(keyname + 1, "#x%x", &keycode) && keycode >= 0 && keycode < N_KEYS_INPUT)){
                // Set a key numerically
                macro.actions[macro.actioncount].scan = keymap[keycode].scan;
                macro.actions[macro.actioncount].down = down;
                macro.actioncount++;
            } else {
                // Find this key in the keymap
                for(unsigned i = 0; i < N_KEYS_INPUT; i++){
                    if(keymap[i].name && !strcmp(keyname + 1, keymap[i].name)){
                        macro.actions[macro.actioncount].scan = keymap[i].scan;
                        macro.actions[macro.actioncount].down = down;
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
    pthread_mutex_lock(imutex(kb));
    _cmd_macro(mode, keys, assignment);
    pthread_mutex_unlock(imutex(kb));
}

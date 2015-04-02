#include "device.h"
#include "input.h"

#ifdef OS_MAC

// Numpad keys have an extra flag
#define IS_NUMPAD(scancode) ((scancode) >= kVK_ANSI_KeypadDecimal && (scancode) <= kVK_ANSI_Keypad9 && (scancode) != kVK_ANSI_KeypadClear && (scancode) != kVK_ANSI_KeypadEnter)

// Event helper
void postevent(io_connect_t event, int kbflags, int scancode, int down, int is_flags, int is_repeat){
    NXEventData kp;
    memset(&kp, 0, sizeof(kp));
    UInt32 type;
    IOOptionBits flags = kbflags;
    IOOptionBits options = 0;
    if(IS_MEDIA(scancode)){
        kp.compound.subType = NX_SUBTYPE_AUX_CONTROL_BUTTONS;
        kp.compound.misc.L[0] = (scancode - KEY_MEDIA) << 16 | (down ? 0x0a00 : 0x0b00) | is_repeat;
        type = NX_SYSDEFINED;
    } else if(is_flags){
        type = NX_FLAGSCHANGED;
        options = kIOHIDSetGlobalEventFlags;
    } else {
        // Standard key
        kp.key.repeat = is_repeat;
        kp.key.keyCode = scancode;
        kp.key.origCharSet = kp.key.charSet = NX_ASCIISET;
        type = down ? NX_KEYDOWN : NX_KEYUP;
        if(IS_NUMPAD(scancode))
            flags |= NX_NUMERICPADMASK;
        else if(scancode == kVK_Help)
            flags |= NX_HELPMASK;
    }

    // HACK: IOHIDPostEvent will fail with kIOReturnNotPrivileged if the event doesn't originate from the UID that owns /dev/console
    // You'd think being root would be good enough. You'd be wrong. ckb-daemon needs to run as root for other reasons though
    // (namely, being able to seize the physical IOHIDDevices) so what we do instead is change our EUID to the appropriate owner,
    // post the event, and then change it right back.
    // Yeah...
    uid_t uid = 0;
    gid_t gid = 0;
    struct stat file;
    if(!stat("/dev/console", &file)){
        uid = file.st_uid;
        gid = file.st_gid;
        if(uid != 0)
            seteuid(uid);
        if(gid != 0)
            setegid(gid);
    }

    kern_return_t res = IOHIDPostEvent(event, type, ((IOGPoint[]){{0, 0}})[0], &kp, kNXEventDataVersion, flags | NX_NONCOALSESCEDMASK, options);
    if(res != KERN_SUCCESS)
        printf("Post event failed: %x\n", res);

    if(uid != 0)
        seteuid(0);
    if(gid != 0)
        setegid(0);
}

// Key repeat delay helper (result in ns)
long repeattime(io_connect_t event, int first){
    long delay = 0;
    IOByteCount actualSize = 0;
    if(IOHIDGetParameter(event, first ? CFSTR(kIOHIDInitialKeyRepeatKey) : CFSTR(kIOHIDKeyRepeatKey), sizeof(long), &delay, &actualSize) != KERN_SUCCESS || actualSize == 0)
        return -1;
    return delay;
}

void clearkeys(usbdevice* kb){
    // Reset modifiers
    postevent(kb->event, 0, 0, 0, 1, 0);
    // Send keyup events for every scancode in the keymap
    for(int key = 0; key < N_KEYS; key++){
        int scan = keymap_system[key].scan;
        if(scan < 0 || IS_MEDIA(scan) || IS_MOD(scan))
            continue;
        postevent(kb->event, 0, scan, 0, 0, 0);
    }
}

int inputopen(usbdevice* kb){
    // Create a keyboard driver
    static mach_port_t master = 0;
    kern_return_t res;
    if(!master && (res = IOMasterPort(bootstrap_port, &master)) != KERN_SUCCESS){
        printf("Unable to open master port: 0x%08x\n", res);
        return 0;
    }
    io_iterator_t iter;
    if((res = IOServiceGetMatchingServices(master, IOServiceMatching(kIOHIDSystemClass), &iter)) != KERN_SUCCESS){
        printf("Unable to get input service iterator: 0x%08x\n", res);
        return 0;
    }
    if((res = IOServiceOpen(IOIteratorNext(iter), mach_task_self(), kIOHIDParamConnectType, &kb->event)) != KERN_SUCCESS){
        IOObjectRelease(iter);
        printf("Unable to open IO service: 0x%08x\n", res);
        kb->event = 0;
        return 0;
    }
    IOObjectRelease(iter);
    clearkeys(kb);
    return 1;
}

void inputclose(usbdevice* kb){
    if(kb->event){
        clearkeys(kb);
        IOServiceClose(kb->event);
        kb->event = 0;
    }
}

void os_keypress(usbdevice* kb, int scancode, int down){
    // Check for modifier keys and update flags
    int flags = 0;
    // Keep a separate list of left/right side modifiers - combine them below
    IOOptionBits* side = (scancode == KEY_RIGHTSHIFT || scancode == KEY_RIGHTCTRL || scancode == KEY_RIGHTMETA || scancode == KEY_RIGHTALT) ? &kb->rflags : &kb->lflags;
    CGEventFlags mod = 0;
    if(scancode == KEY_CAPSLOCK){
        if(down)
            kb->lflags ^= NX_ALPHASHIFTMASK;
        flags = 1;
    } else if(scancode == KEY_LEFTSHIFT || scancode == KEY_RIGHTSHIFT){
        mod = NX_SHIFTMASK;
        mod |= (scancode == KEY_LEFTSHIFT) ? NX_DEVICELSHIFTKEYMASK : NX_DEVICERSHIFTKEYMASK;
        flags = 1;
    } else if(scancode == KEY_LEFTCTRL || scancode == KEY_RIGHTCTRL){
        mod = NX_CONTROLMASK;
        mod |= (scancode == KEY_LEFTCTRL) ? NX_DEVICELCTLKEYMASK : NX_DEVICERCTLKEYMASK;
        flags = 1;
    } else if(scancode == KEY_LEFTMETA || scancode == KEY_RIGHTMETA){
        mod = NX_COMMANDMASK;
        mod |= (scancode == KEY_LEFTMETA) ? NX_DEVICELCMDKEYMASK : NX_DEVICERCMDKEYMASK;
        flags = 1;
    } else if(scancode == KEY_LEFTALT || scancode == KEY_RIGHTALT){
        mod = NX_ALTERNATEMASK;
        mod |= (scancode == KEY_LEFTALT) ? NX_DEVICELALTKEYMASK : NX_DEVICERALTKEYMASK;
        flags = 1;
    }

    if(flags){
        // Update flags when a modifier key is pressed
        if(down)
            *side |= mod;
        else
            *side &= ~mod;
        kb->eventflags = kb->lflags | kb->rflags;
        kb->lastkeypress = KEY_NONE;
    } else {
        // For any other key, trigger key repeat
        if(down){
            long repeat = repeattime(kb->event, 1);
            if(repeat > 0){
                kb->lastkeypress = scancode;
                clock_gettime(CLOCK_MONOTONIC, &kb->keyrepeat);
                timespec_add(&kb->keyrepeat, repeat);
            } else
                kb->lastkeypress = KEY_NONE;
        } else
            kb->lastkeypress = KEY_NONE;
    }

    postevent(kb->event, kb->eventflags, scancode, down, flags, 0);
}

void keyretrigger(usbdevice* kb){
    int scancode = kb->lastkeypress;
    if(scancode < 0)
        return;
    // Retrigger the last-pressed key
    postevent(kb->event, kb->eventflags, scancode, 1, 0, 1);
    // Set next key repeat time
    long repeat = repeattime(kb->event, 0);
    if(repeat > 0)
        timespec_add(&kb->keyrepeat, repeat);
    else
        kb->lastkeypress = KEY_NONE;
}

// Key repeat thread
void* krthread(void* context){
    while(1){
        // Re-scan every 1ms
        usleep(1000);
        struct timespec time;
        clock_gettime(CLOCK_MONOTONIC, &time);
        for(int i = 1; i < DEV_MAX; i++){
            if(IS_CONNECTED(keyboard + i)){
                // Scan all connected devices
                pthread_mutex_lock(&keyboard[i].keymutex);
                // Repeat the key as many times as needed to catch up
                while(keyboard[i].lastkeypress != KEY_NONE && timespec_ge(time, keyboard[i].keyrepeat))
                    keyretrigger(keyboard + i);
                pthread_mutex_unlock(&keyboard[i].keymutex);
            }
        }
    }
    return 0;
}

void os_kpsync(usbdevice* kb){
    // OSX doesn't have any equivalent to the SYN_ events
}

void os_updateindicators(usbdevice* kb, int force){
    if(!IS_CONNECTED(kb) || NEEDS_FW_UPDATE(kb))
        return;
    // Set NumLock on permanently
    char ileds = 1;
    // Set Caps Lock if enabled. Unlike Linux, OSX keyboards have independent caps lock states, so
    // we use the last-assigned value rather than fetching it from the system
    if(kb->eventflags & kCGEventFlagMaskAlphaShift)
        ileds |= 2;
    usbmode* mode = kb->profile.currentmode;
    if(mode && kb->active)
        ileds = (ileds & ~mode->ioff) | mode->ion;
    if(force || ileds != kb->ileds){
        kb->ileds = ileds;
        // Set the LEDs
        CFArrayRef leds = IOHIDDeviceCopyMatchingElements(kb->handles[0], 0, kIOHIDOptionsTypeNone);
        CFIndex count = CFArrayGetCount(leds);
        for(CFIndex i = 0; i < count; i++){
            IOHIDElementRef led = (void*)CFArrayGetValueAtIndex(leds, i);
            uint32_t page = IOHIDElementGetUsagePage(led);
            if(page != kHIDPage_LEDs)
                continue;
            uint32_t usage = IOHIDElementGetUsage(led);
            IOHIDValueRef value = IOHIDValueCreateWithIntegerValue(kCFAllocatorDefault, led, 0, !!(ileds & (1 << (usage - 1))));
            IOHIDDeviceSetValue(kb->handles[0], led, value);
            CFRelease(value);
        }
        CFRelease(leds);
    }
}

#endif

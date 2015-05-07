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
        kp.compound.subType = (scancode != KEY_POWER ? NX_SUBTYPE_AUX_CONTROL_BUTTONS : NX_SUBTYPE_POWER_KEY);
        kp.compound.misc.L[0] = (scancode - KEY_MEDIA) << 16 | (down ? 0x0a00 : 0x0b00) | is_repeat;
        type = NX_SYSDEFINED;
    } else {
        if(is_flags){
            // Modifier (shift, ctrl, etc)
            type = NX_FLAGSCHANGED;
            options = kIOHIDSetGlobalEventFlags;
        } else
            // Standard key
            type = down ? NX_KEYDOWN : NX_KEYUP;
        kp.key.repeat = is_repeat;
        kp.key.keyCode = scancode;
        kp.key.origCharSet = kp.key.charSet = NX_ASCIISET;
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

    kern_return_t res = IOHIDPostEvent(event, type, *(IOGPoint[]){{0, 0}}, &kp, kNXEventDataVersion, flags | NX_NONCOALSESCEDMASK, options);
    if(res != KERN_SUCCESS)
        ckb_warn("Post event failed: %x\n", res);

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
    // Send keyup events for every scancode in the keymap
    for(int key = 0; key < N_KEYS_INPUT; key++){
        int scan = keymap[key].scan;
        if((scan & SCAN_SILENT) || scan == BTN_WHEELUP || scan == BTN_WHEELDOWN || IS_MEDIA(scan))
            continue;
        postevent(kb->event, 0, scan, 0, 0, 0);
    }
}

int inputopen(usbdevice* kb){
    // Open master port (if not done yet)
    static mach_port_t master = 0;
    kern_return_t res;
    if(!master&& (res = IOMasterPort(bootstrap_port, &master)) != KERN_SUCCESS){
        master = 0;
        ckb_err("Unable to open master port: 0x%08x\n", res);
        return 0;
    }
    // Open an HID service
    io_iterator_t iter;
    if((res = IOServiceGetMatchingServices(master, IOServiceMatching(kIOHIDSystemClass), &iter)) != KERN_SUCCESS){
        ckb_err("Unable to get input service iterator: 0x%08x\n", res);
        return 0;
    }
    if((res = IOServiceOpen(IOIteratorNext(iter), mach_task_self(), kIOHIDParamConnectType, &kb->event)) != KERN_SUCCESS){
        IOObjectRelease(iter);
        ckb_err("Unable to open IO service: 0x%08x\n", res);
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
    if(scancode & SCAN_MOUSE){
        // TODO: implement
        return;
    }
    // Some boneheaded Apple engineers decided to reverse kVK_ANSI_Grave and kVK_ISO_Section on the 105-key layouts...
    if(!HAS_ANY_FEATURE(kb, FEAT_LMASK)){
        // If the layout hasn't been set yet, it can be auto-detected from certain keys
        if(scancode == KEY_BACKSLASH_ISO || scancode == KEY_102ND)
            kb->features |= FEAT_ISO;
        else if(scancode == KEY_BACKSLASH)
            kb->features |= FEAT_ANSI;
    }
    if(scancode == KEY_BACKSLASH_ISO)
        scancode = KEY_BACKSLASH;
    if(HAS_FEATURES(kb, FEAT_ISO)){
        // Compensate for key reversal
        if(scancode == KEY_GRAVE)
            scancode = KEY_102ND;
        else if(scancode == KEY_102ND)
            scancode = KEY_GRAVE;
    }
    // Check for modifier keys and update flags
    int isMod = 0;
    IOOptionBits mod = 0;
    if(scancode == KEY_CAPSLOCK){
        if(down)
            kb->modifiers ^= NX_ALPHASHIFTMASK;
        isMod = 1;
    }
    else if(scancode == KEY_LEFTSHIFT) mod = NX_DEVICELSHIFTKEYMASK;
    else if(scancode == KEY_RIGHTSHIFT) mod = NX_DEVICERSHIFTKEYMASK;
    else if(scancode == KEY_LEFTCTRL) mod = NX_DEVICELCTLKEYMASK;
    else if(scancode == KEY_RIGHTCTRL) mod = NX_DEVICERCTLKEYMASK;
    else if(scancode == KEY_LEFTMETA) mod = NX_DEVICELCMDKEYMASK;
    else if(scancode == KEY_RIGHTMETA) mod = NX_DEVICERCMDKEYMASK;
    else if(scancode == KEY_LEFTALT) mod = NX_DEVICELALTKEYMASK;
    else if(scancode == KEY_RIGHTALT) mod = NX_DEVICERALTKEYMASK;
    if(mod){
        // Update global modifiers
        if(down)
            mod |= kb->modifiers;
        else
            mod = kb->modifiers & ~mod;
        if((mod & NX_DEVICELSHIFTKEYMASK) || (mod & NX_DEVICERSHIFTKEYMASK)) mod |= NX_SHIFTMASK; else mod &= ~NX_SHIFTMASK;
        if((mod & NX_DEVICELCTLKEYMASK) || (mod & NX_DEVICERCTLKEYMASK)) mod |= NX_CONTROLMASK; else mod &= ~NX_CONTROLMASK;
        if((mod & NX_DEVICELCMDKEYMASK) || (mod & NX_DEVICERCMDKEYMASK)) mod |= NX_COMMANDMASK; else mod &= ~NX_COMMANDMASK;
        if((mod & NX_DEVICELALTKEYMASK) || (mod & NX_DEVICERALTKEYMASK)) mod |= NX_ALTERNATEMASK; else mod &= ~NX_ALTERNATEMASK;
        kb->modifiers = mod;
        kb->lastkeypress = KEY_NONE;
        isMod = 1;
    } else if(!isMod){
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

    postevent(kb->event, kb->modifiers, scancode, down, isMod, 0);
}

// Retrigger the last-pressed key
void keyretrigger(usbdevice* kb){
    int scancode = kb->lastkeypress;
    postevent(kb->event, kb->modifiers, scancode, 1, 0, 1);
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
            // Scan all connected devices
            pthread_mutex_lock(inputmutex + i);
            if(IS_CONNECTED(keyboard + i)){
                // Repeat the key as many times as needed to catch up
                while(keyboard[i].lastkeypress >= 0 && timespec_ge(time, keyboard[i].keyrepeat))
                    keyretrigger(keyboard + i);
            }
            pthread_mutex_unlock(inputmutex + i);
        }
    }
    return 0;
}

void os_mousemove(usbdevice* kb, int x, int y){
    // TODO: stub
}

void os_isync(usbdevice* kb){
    // OSX doesn't have any equivalent to the SYN_ events
}

void os_updateindicators(usbdevice* kb, int force){
    // Set NumLock on permanently
    char ileds = 1;
    // Set Caps Lock if enabled. Unlike Linux, OSX keyboards have independent caps lock states, so
    // we use the last-assigned value rather than fetching it from the system
    if(kb->modifiers & kCGEventFlagMaskAlphaShift)
        ileds |= 2;
    if(kb->active){
        usbmode* mode = kb->profile->currentmode;
        ileds = (ileds & ~mode->ioff) | mode->ion;
    }
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

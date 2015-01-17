#include "device.h"
#include "input.h"

#ifdef OS_MAC

int inputopen(usbdevice* kb){
    CGEventSourceRef event = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
    if(!event){
        kb->event = 0;
        return 0;
    }
    kb->event = event;
    return 1;
}

void inputclose(usbdevice* kb){
    if(kb->event){
        CFRelease(kb->event);
        kb->event = 0;
    }
}

// Helper function to create CGEvents
CGEventRef eventcreate(CGEventSourceRef source, int scancode, int down){
    if(scancode < KEY_MEDIA)
        return CGEventCreateKeyboardEvent(source, scancode, down);
    else {
        // Media keys need a special event type
        CGEventRef kp = CGEventCreate(source);
        CGEventSetType(kp, NX_SYSDEFINED);
        CGEventSetIntegerValueField(kp, 83, NX_SUBTYPE_AUX_CONTROL_BUTTONS);
        // This is a terrible hack, but I have no idea how to get the correct data in here without setting the bytes manually...
        CFDataRef data = CGEventCreateData(kCFAllocatorDefault, kp);
        CFRelease(kp);
        CFMutableDataRef mutdata = CFDataCreateMutableCopy(kCFAllocatorDefault, 0, data);
        CFRelease(data);
        int evdata = (scancode - KEY_MEDIA) << 8 | (down ? 0x0a : 0x0b) << 16;
        CFRange range = { 31 * 4, 4 };
        CFDataReplaceBytes(mutdata, range, (const uchar*)&evdata, 4);
        kp = CGEventCreateFromData(kCFAllocatorDefault, mutdata);
        CFRelease(mutdata);
        return kp;
    }
}

void os_keypress(usbdevice* kb, int scancode, int down){
    // Check for modifier keys and update flags
    int flags = 0;
    if(scancode == KEY_CAPSLOCK){
        if(down)
            kb->lflags ^= kCGEventFlagMaskAlphaShift;
        flags = 1;
    } else {
        // Keep a separate list of left/right side modifiers - combine them below
        // Apple's HID driver seems to include a lot of odd bits that aren't documented. Those are included here as well for completeness.
        CGEventFlags* side = (scancode == KEY_RIGHTSHIFT || scancode == KEY_RIGHTCTRL || scancode == KEY_RIGHTMETA || scancode == KEY_RIGHTALT) ? &kb->rflags : &kb->lflags;
        CGEventFlags mod = 0, mod2 = 0;
        if(scancode == KEY_LEFTSHIFT || scancode == KEY_RIGHTSHIFT){
            mod = kCGEventFlagMaskShift;
            mod2 = 0x2;
            flags = 1;
        } else if(scancode == KEY_LEFTCTRL || scancode == KEY_RIGHTCTRL){
            mod = kCGEventFlagMaskControl;
            if(side == &kb->lflags)
                mod2 = 0x1;
            else
                mod2 = 0x100;
            flags = 1;
        } else if(scancode == KEY_LEFTMETA || scancode == KEY_RIGHTMETA){
            mod = kCGEventFlagMaskCommand;
            mod2 = 0x8;
            flags = 1;
        } else if(scancode == KEY_LEFTALT || scancode == KEY_RIGHTALT){
            mod = kCGEventFlagMaskAlternate;
            mod2 = 0x20;
            flags = 1;
        }
        if(side == &kb->rflags)
            mod2 <<= 1;
        mod |= mod2;
        if(flags){
            if(down)
                *side |= mod;
            else
                *side &= ~mod;
        }
    }

    CGEventRef kp;
    if(flags){
        // Create flag change event
        kb->eventflags = kb->lflags | kb->rflags | 0x100;
        kp = CGEventCreate(kb->event);
        CGEventSetType(kp, kCGEventFlagsChanged);
        CGEventSetIntegerValueField(kp, kCGKeyboardEventKeycode, scancode);
        kb->lastkeypress = -1;
    } else {
        // Create keypress event
        kp = eventcreate(kb->event, scancode, down);
        kb->lastkeypress = (down ? scancode : -1);
    }
    kb->keypresstime = 0;
    CGEventSetFlags(kp, kb->eventflags);
    CGEventPost(kCGHIDEventTap, kp);
    CFRelease(kp);
}

void keyretrigger(usbdevice* kb, int scancode){
    CGEventRef kp = eventcreate(kb->event, scancode, 1);
    CGEventSetIntegerValueField(kp, kCGKeyboardEventAutorepeat, 1);
    CGEventSetFlags(kp, kb->eventflags);
    CGEventPost(kCGHIDEventTap, kp);
    CFRelease(kp);
}

void os_kpsync(usbdevice* kb){
    // OSX doesn't have any equivalent to the SYN_ events, but we do need to prevent idle sleep
    // This stops the system from sleeping or wakes it up if it was asleep already
    IOPMAssertionID assertionID;
    IOPMAssertionDeclareUserActivity(CFSTR(""), kIOPMUserActiveLocal, &assertionID);
    IOPMAssertionRelease(assertionID);
}

void os_updateindicators(usbdevice* kb, int force){
    // Set NumLock on permanently
    char ileds = 1;
    // Set Caps Lock if enabled. Unlike Linux, OSX keyboards have independent caps lock states, so
    // we use the last-assigned value rather than fetching it from the system
    if(kb->eventflags & kCGEventFlagMaskAlphaShift)
        ileds |= 2;
    usbmode* mode = kb->profile.currentmode;
    if(mode)
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

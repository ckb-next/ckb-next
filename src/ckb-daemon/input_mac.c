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

// extra_mac.m
extern long keyrepeatdelay();
extern CGEventRef media_event(uint data1, uint modifiers);
#define MEDIA_FLAGS(scancode, down, is_repeat) (((scancode) - KEY_MEDIA) << 16 | ((down) ? 0x0a00 : 0x0b00) | !!(is_repeat))

void os_keypress(usbdevice* kb, int scancode, int down){
    // Check for modifier keys and update flags
    int flags = 0;
    // Keep a separate list of left/right side modifiers - combine them below
    // Apple's HID driver seems to include a lot of odd bits that aren't documented. Those are included here as well for completeness.
    CGEventFlags* side = (scancode == KEY_RIGHTSHIFT || scancode == KEY_RIGHTCTRL || scancode == KEY_RIGHTMETA || scancode == KEY_RIGHTALT) ? &kb->rflags : &kb->lflags;
    CGEventFlags mod = 0;
    if(scancode == KEY_CAPSLOCK){
        if(down)
            kb->lflags ^= kCGEventFlagMaskAlphaShift;
        flags = 1;
    } else if(scancode == KEY_LEFTSHIFT || scancode == KEY_RIGHTSHIFT){
        mod = kCGEventFlagMaskShift;
        mod |= (scancode == KEY_LEFTSHIFT) ? 0x2 : 0x4;
        flags = 1;
    } else if(scancode == KEY_LEFTCTRL || scancode == KEY_RIGHTCTRL){
        mod = kCGEventFlagMaskControl;
        mod |= (scancode == KEY_LEFTCTRL) ? 0x1 : 0x200;
        flags = 1;
    } else if(scancode == KEY_LEFTMETA || scancode == KEY_RIGHTMETA){
        mod = kCGEventFlagMaskCommand;
        mod |= (scancode == KEY_LEFTMETA) ? 0x8 : 0x10;
        flags = 1;
    } else if(scancode == KEY_LEFTALT || scancode == KEY_RIGHTALT){
        mod = kCGEventFlagMaskAlternate;
        mod |= (scancode == KEY_LEFTALT) ? 0x20 : 0x40;
        flags = 1;
    }

    if(flags){
        // Update flags when a modifier key is pressed
        if(down)
            *side |= mod;
        else
            *side &= ~mod;
        kb->eventflags = kb->lflags | kb->rflags | 0x100;
        kb->lastkeypress = KEY_NONE;
    } else {
        // For any other key, trigger key repeat
        long repeat = keyrepeatdelay();
        if(down && repeat > 0){
            kb->lastkeypress = scancode;
            clock_gettime(CLOCK_MONOTONIC, &kb->keyrepeat);
            timespec_add(&kb->keyrepeat, repeat);
        } else
            kb->lastkeypress = KEY_NONE;
    }
    if(scancode >= KEY_MEDIA){
        // Media keys get a separate event
        CGEventRef mevent = media_event(MEDIA_FLAGS(scancode, down, 0), kb->eventflags);
        CGEventPost(kCGHIDEventTap, mevent);
        return;
    }
    // Create keypress event
    CGEventRef kevent = CGEventCreateKeyboardEvent(kb->event, scancode, down);
    CGEventSetFlags(kevent, kb->eventflags);
    CGEventPost(kCGHIDEventTap, kevent);
    CFRelease(kevent);
}

void keyretrigger(usbdevice* kb, int scancode){
    if(scancode >= KEY_MEDIA){
        // Media keys get a separate event
        CGEventRef mevent = media_event(MEDIA_FLAGS(scancode, 1, 1), kb->eventflags);
        CGEventPost(kCGHIDEventTap, mevent);
        return;
    }
    CGEventRef kp = CGEventCreateKeyboardEvent(kb->event, scancode, 1);
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

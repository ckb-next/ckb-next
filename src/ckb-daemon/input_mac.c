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

void os_keypress(usbdevice* kb, int scancode, int down){
    // Check for modifier keys and update flags
    int flags = 0;
    if(scancode == KEY_CAPSLOCK){
        if(down)
            kb->lflags ^= kCGEventFlagMaskAlphaShift;
        flags = 1;
    } else {
        // Keep a separate list of left/right side modifiers - combine them below
        CGEventFlags* side = (scancode == KEY_RIGHTSHIFT || scancode == KEY_RIGHTCTRL || scancode == KEY_RIGHTMETA || scancode == KEY_RIGHTALT) ? &kb->rflags : &kb->lflags;
        CGEventFlags mod;
        if(scancode == KEY_LEFTSHIFT || scancode == KEY_RIGHTSHIFT){
            mod = kCGEventFlagMaskShift;
            flags = 1;
        } else if(scancode == KEY_LEFTCTRL || scancode == KEY_RIGHTCTRL){
            mod = kCGEventFlagMaskControl;
            flags = 1;
        } else if(scancode == KEY_LEFTMETA || scancode == KEY_RIGHTMETA){
            mod = kCGEventFlagMaskCommand;
            flags = 1;
        } else if(scancode == KEY_LEFTALT || scancode == KEY_RIGHTALT){
            mod = kCGEventFlagMaskAlternate;
            flags = 1;
        }
        if(flags){
            if(down)
                *side |= mod;
            else
                *side &= ~mod;
        }
    }

    CGEventRef kp;
    if(flags){
        kb->eventflags = kb->lflags | kb->rflags;
        kp = CGEventCreate(kb->event);
        CGEventSetType(kp, kCGEventFlagsChanged);
        CGEventSetIntegerValueField(kp, kCGKeyboardEventKeycode, scancode);
        kb->lastkeypress = -1;
    } else {
        kp = CGEventCreateKeyboardEvent(kb->event, scancode, down);
        kb->lastkeypress = (down ? scancode : -1);
    }
    kb->keypresstime = 0;
    CGEventSetFlags(kp, kb->eventflags);
    CGEventPost(kCGHIDEventTap, kp);
    CFRelease(kp);
}

void keyretrigger(usbdevice* kb, int scancode){
    CGEventRef kp = CGEventCreateKeyboardEvent(kb->event, scancode, 1);
    CGEventSetIntegerValueField(kp, kCGKeyboardEventAutorepeat, 1);
    CGEventSetFlags(kp, kb->eventflags);
    CGEventPost(kCGHIDEventTap, kp);
    CFRelease(kp);
}

void os_kpsync(usbdevice* kb){
    // OSX doesn't have any equivalent to the SYN_ events
}

void updateindicators(usbdevice* kb, int force){
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
        IOHIDDeviceSetReport(kb->handles[1], kIOHIDReportTypeOutput, 0, &kb->ileds, 1);
    }
}

#endif

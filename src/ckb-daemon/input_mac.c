#include "usb.h"
#include "input.h"
#include "device.h"

#ifdef OS_MAC

int inputopen(int index){
    CGEventSourceRef event = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
    if(!event){
        keyboard[index].event = 0;
        return 0;
    }
    keyboard[index].event = event;
    return 1;
}

void inputclose(int index){
    if(keyboard[index].event){
        CFRelease(keyboard[index].event);
        keyboard[index].event = 0;
    }
}

void os_keypress(usbdevice* kb, int scancode, int down){
    // Check for modifier keys and update flags
    int flags = 0;
    if(scancode == KEY_CAPSLOCK){
        if(down){
            if(!(kb->eflags & kCGEventFlagMaskAlphaShift))
                kb->eflags |= kCGEventFlagMaskAlphaShift;
            else
                kb->eflags &= ~kCGEventFlagMaskAlphaShift;
        }
        flags = 1;
    } else if(scancode == KEY_LEFTSHIFT || scancode == KEY_RIGHTSHIFT){
        if(down)
            kb->eflags |= kCGEventFlagMaskShift;
        else
            kb->eflags &= ~kCGEventFlagMaskShift;
        flags = 1;
    } else if(scancode == KEY_LEFTCTRL || scancode == KEY_RIGHTCTRL){
        if(down)
            kb->eflags |= kCGEventFlagMaskControl;
        else
            kb->eflags &= ~kCGEventFlagMaskControl;
        flags = 1;
    } else if(scancode == KEY_LEFTMETA || scancode == KEY_RIGHTMETA){
        if(down)
            kb->eflags |= kCGEventFlagMaskCommand;
        else
            kb->eflags &= ~kCGEventFlagMaskCommand;
        flags = 1;
    } else if(scancode == KEY_LEFTALT || scancode == KEY_RIGHTALT){
        if(down)
            kb->eflags |= kCGEventFlagMaskAlternate;
        else
            kb->eflags &= ~kCGEventFlagMaskAlternate;
        flags = 1;
    }

    CGEventRef kp;
    if(flags){
        kp = CGEventCreate(kb->event);
        CGEventSetType(kp, kCGEventFlagsChanged);
        CGEventSetIntegerValueField(kp, kCGKeyboardEventKeycode, scancode);
        kb->lastkeypress = -1;
    } else {
        kp = CGEventCreateKeyboardEvent(kb->event, scancode, down);
        kb->lastkeypress = (down ? scancode : -1);
    }
    kb->keypresstime = 0;
    CGEventSetFlags(kp, kb->eflags);
    CGEventPost(kCGHIDEventTap, kp);
    CFRelease(kp);
}

void keyretrigger(usbdevice* kb, int scancode){
    CGEventRef kp = CGEventCreateKeyboardEvent(kb->event, scancode, 1);
    CGEventSetIntegerValueField(kp, kCGKeyboardEventAutorepeat, 1);
    CGEventSetFlags(kp, kb->eflags);
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
    if(kb->eflags & kCGEventFlagMaskAlphaShift)
        ileds |= 2;
    usbmode* mode = kb->setting.profile.currentmode;
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

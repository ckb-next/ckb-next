#include "usb.h"
#include "input.h"

#ifdef OS_MAC

int inputopen(int index, const struct libusb_device_descriptor* descriptor){
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
    printf("%d %d\n", scancode, down);
    if(scancode == KEY_CAPSLOCK){
      if(down)
          kb->eflags |= kCGEventFlagMaskAlphaShift;
      else
          kb->eflags &= ~kCGEventFlagMaskAlphaShift;
    } else if(scancode == KEY_LEFTSHIFT || scancode == KEY_RIGHTSHIFT){
        if(down)
            kb->eflags |= kCGEventFlagMaskShift;
        else
            kb->eflags &= ~kCGEventFlagMaskShift;
    } else if(scancode == KEY_LEFTCTRL || scancode == KEY_RIGHTCTRL){
        if(down)
            kb->eflags |= kCGEventFlagMaskControl;
        else
            kb->eflags &= ~kCGEventFlagMaskControl;
    } else if(scancode == KEY_LEFTMETA || scancode == KEY_RIGHTMETA){
        if(down)
            kb->eflags |= kCGEventFlagMaskCommand;
        else
            kb->eflags &= ~kCGEventFlagMaskCommand;
    } else if(scancode == KEY_LEFTALT || scancode == KEY_RIGHTALT){
        if(down)
            kb->eflags |= kCGEventFlagMaskAlternate;
        else
            kb->eflags &= ~kCGEventFlagMaskAlternate;
    }

    CGEventRef kp = CGEventCreateKeyboardEvent(kb->event, scancode, down);
    CGEventSetFlags(kp, kb->eflags);
    CGEventPost(kCGHIDEventTap, kp);
    CFRelease(kp);
}

void os_kpsync(usbdevice* kb){
    // OSX doesn't have any equivalent to the SYN_ events
}

int os_readind(usbdevice* kb){
    // Set NumLock on permanently
    char ileds = 1;
    // Set Caps Lock if enabled. Unlike Linux, OSX keyboards have independent caps lock states, so
    // we use the last-assigned value rather than fetching it from the system
    if(kb->eflags & kCGEventFlagMaskAlphaShift)
        ileds |= 2;
    if(ileds != kb->ileds){
        kb->ileds = ileds;
        return 1;
    }
    return 0;
}

#endif

// This is a stub file to aid porting the software to other operating systems

#include "command.h"
#include "device.h"
#include "input.h"

#ifdef OS_WINDOWS

int os_inputopen(usbdevice* kb){
    return 0;
}

void os_inputclose(usbdevice* kb){

}

void os_keypress(usbdevice* kb, int scancode, int down){

}

void os_mousemove(usbdevice* kb, int x, int y){

}

int os_setupindicators(usbdevice* kb){
    // Initialize LEDs to all off
    kb->hw_ileds = kb->hw_ileds_old = kb->ileds = 0;
    return 0;
}

#endif

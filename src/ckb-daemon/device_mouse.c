#include "command.h"
#include "device.h"
#include "devnode.h"
#include "firmware.h"
#include "input.h"
#include "profile.h"
#include "usb.h"

int setactive_mouse(usbdevice* kb, int active){
    if(NEEDS_FW_UPDATE(kb))
        return 0;
    uchar msg[MSG_SIZE] = { 0x07, 0x04, 0 };    // Disables or enables HW control for DPI and Sniper button
    if(active)
        // Put the mouse into SW mode
        msg[2] = 2;
    else
        // Restore HW mode
        msg[2] = 1;
    pthread_mutex_lock(imutex(kb));
    kb->active = !!active;
    // Clear input
    memset(&kb->input.keys, 0, sizeof(kb->input.keys));
    inputupdate(kb);
    pthread_mutex_unlock(imutex(kb));
    if(!usbsend(kb, msg, 1))
        return -1;
    return 0;
}

int cmd_active_mouse(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4){
    return setactive_mouse(kb, 1);
}

int cmd_idle_mouse(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4){
    return setactive_mouse(kb, 0);
}

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
    uchar msg[2][MSG_SIZE] = {
        { 0x07, 0x04, 0 },                                                          // Disables or enables HW control for DPI and Sniper button
        { 0x07, 0x40, 0x08, 0,
          1, 0x80, 2, 0x80, 3, 0x80, 4, 0x80, 5, 0x80, 6, 0x80, 7, 0x80, 8, 0x80 }, // Enable button input. This is similar to the keyboard input selection, but we want everything to come through the HID reports.
    };
    if(active)
        // Put the mouse into SW mode
        msg[0][2] = 2;
    else
        // Restore HW mode
        msg[0][2] = 1;
    pthread_mutex_lock(imutex(kb));
    kb->active = !!active;
    kb->profile->lastlight.forceupdate = 1;
    // Clear input
    memset(&kb->input.keys, 0, sizeof(kb->input.keys));
    inputupdate(kb);
    pthread_mutex_unlock(imutex(kb));
    if(!usbsend(kb, msg[0], 1))
        return -1;
    if(active && !usbsend(kb, msg[1], 1))
        return -1;
    return 0;
}

int cmd_active_mouse(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4){
    return setactive_mouse(kb, 1);
}

int cmd_idle_mouse(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4){
    return setactive_mouse(kb, 0);
}

int cmd_pollrate(usbdevice* kb, usbmode* dummy1, int dummy2, int rate, const char* dummy3){
    uchar msg[MSG_SIZE] = {
        0x07, 0x0a, 0, 0, (uchar)rate
    };
    if(!usbsend(kb, msg, 1))
        return -1;
    // Device should disconnect+reconnect, but update the poll rate field in case it doesn't
    kb->pollrate = rate;
    return 0;
}

#include "command.h"
#include "device.h"
#include "devnode.h"
#include "firmware.h"
#include "input.h"
#include "profile.h"
#include "usb.h"

int start_mouse_legacy(usbdevice* kb, int makeactive){
    (void)makeactive;

    // Put the legacy M95 into software mode. Nothing else needs to be done hardware wise
    
    // Fill out RGB features for consistency, even though the mouse doesn't have them
    kb->active = 1;
    kb->pollrate = -1;
    return 0;
}

int setactive_mouse(usbdevice* kb, int active){
    if(NEEDS_FW_UPDATE(kb))
        return 0;
    const int keycount = 20;
    uchar msg[2][MSG_SIZE] = {
        { CMD_SET, FIELD_SPECIAL, 0 },            // Disables or enables HW control for DPI and Sniper button
        { CMD_SET, FIELD_KEYINPUT, keycount, 0 }, // Select button input (simlilar to the packet sent to keyboards, but lacks a commit packet)
    };
    if(active)
        // Put the mouse into SW mode
        msg[0][2] = MODE_SOFTWARE;
    else
        // Restore HW mode
        msg[0][2] = MODE_HARDWARE;
    pthread_mutex_lock(imutex(kb));
    kb->active = !!active;
    kb->profile->lastlight.forceupdate = 1;
    // Clear input
    memset(&kb->input.keys, 0, sizeof(kb->input.keys));
    inputupdate(kb);
    pthread_mutex_unlock(imutex(kb));
    if(!usbsend(kb, msg[0], 1))
        return -1;
    if(active){
        // Set up key input
        if(!usbsend(kb, msg[1], 1))
            return -1;
        for(int i = 0; i < keycount; i++){
            msg[1][i * 2 + 4] = i + 1;
            msg[1][i * 2 + 5] = (i < 6 ? IN_HID : IN_CORSAIR);
        }
    }
    return 0;
}

int cmd_active_mouse(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4){
    (void)dummy1;
    (void)dummy2;
    (void)dummy3;
    (void)dummy4;

    return setactive_mouse(kb, 1);
}

int cmd_idle_mouse(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4){
    (void)dummy1;
    (void)dummy2;
    (void)dummy3;
    (void)dummy4;

    return setactive_mouse(kb, 0);
}

int cmd_pollrate(usbdevice* kb, usbmode* dummy1, int dummy2, int rate, const char* dummy3){
    (void)dummy1;
    (void)dummy2;
    (void)dummy3;

    uchar msg[MSG_SIZE] = {
        CMD_SET, FIELD_POLLRATE, 0, 0, (uchar)rate
    };
    if(!usbsend(kb, msg, 1))
        return -1;
    // Device should disconnect+reconnect, but update the poll rate field in case it doesn't
    kb->pollrate = rate;
    return 0;
}

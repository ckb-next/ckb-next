#include "command.h"
#include "device.h"
#include "devnode.h"
#include "firmware.h"
#include "input.h"
#include "profile.h"
#include "usb.h"

int start_mouse_legacy(usbdevice* kb, int makeactive){
    (void)makeactive;

    // kb, ptr, len, bRequest, wValue, wIndex
    // 0x00002 == HW Playback off
    // 0x00001 == HW Playback on
    usbsend_control(kb, NULL, 0, 2, 0x0002, 0);
    // Send initial DPI settings
    unsigned char pkt1[] = {0x04, 0x02, 0x10, 0x10, 0x30, 0x30, 0x78, 0x78, 0x08, 0x08};
    usbsend_control(kb, pkt1, 10, 174, 0x0000, 0);
    // ????
    unsigned char pkt2[] = {0x02, 0x02, 0x03, 0x02, 0xff, 0x3c, 0x00, 0x00, 0x00, 0x06};
    usbsend_control(kb, pkt2, 10, 3, 0x0000, 0);
    // Angle snap ON
    unsigned char pkt3[] = {0x01};
    usbsend_control(kb, pkt3, 1, 100, 0x0000, 0);

    kb->active = 1;
    kb->pollrate = -1;
    return 0;
}

int cmd_pollrate_legacy(usbdevice* kb, usbmode* dummy1, int dummy2, int rate, const char* dummy3){
    (void)dummy1;
    (void)dummy2;
    (void)dummy3;

    usbsend_control(kb, NULL, 0, 10, rate, 0);

    // Device should disconnect+reconnect, but update the poll rate field in case it doesn't
    kb->pollrate = rate;
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

    clear_input_and_rgb(kb, active);

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

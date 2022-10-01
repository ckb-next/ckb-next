#include "command.h"
#include "device.h"
#include "devnode.h"
#include "firmware.h"
#include "input.h"
#include "profile.h"
#include "usb.h"
#include "nxp_proto.h"

int start_mouse_legacy(usbdevice* kb, int makeactive){
    (void)makeactive;
    // 0x00002 == HW Playback off
    // 0x00001 == HW Playback on
    usbsend(kb, (&(ctrltransfer) { .bRequestType = 0x40, .bRequest = 2, .wValue = 0x0002, .wIndex = 0, .wLength = 0, .timeout = 5000, .data = NULL }), 0, 1);
    // Send initial DPI settings
    unsigned char pkt1[] = {0x04, 0x02, 0x10, 0x10, 0x30, 0x30, 0x78, 0x78, 0x08, 0x08};
    usbsend(kb, (&(ctrltransfer) { .bRequestType = 0x40, .bRequest = 174, .wValue = 0x0000, .wIndex = 0, .wLength = sizeof(pkt1), .timeout = 5000, .data = pkt1 }), 0, 1);
    // ????
    unsigned char pkt2[] = {0x02, 0x02, 0x03, 0x02, 0xff, 0x3c, 0x00, 0x00, 0x00, 0x06};
    usbsend(kb, (&(ctrltransfer) { .bRequestType = 0x40, .bRequest = 3, .wValue = 0x0000, .wIndex = 0, .wLength = sizeof(pkt2), .timeout = 5000, .data = pkt2 }), 0, 1);
    // Angle snap ON
    unsigned char pkt3[] = {0x01};
    usbsend(kb, (&(ctrltransfer) { .bRequestType = 0x40, .bRequest = 100, .wValue = 0x0000, .wIndex = 0, .wLength = sizeof(pkt3), .timeout = 5000, .data = pkt3 }), 0, 1);

    // Maybe in the future someone can add the command to get the pollrate
    // Until then, pretend it's all 1ms
    kb->active = 1;
    kb->maxpollrate = kb->pollrate = POLLRATE_1MS;
    return 0;
}

static const uint16_t nxp_and_legacy_pollrate[POLLRATE_COUNT] = {
    [POLLRATE_8MS] = 8,
    [POLLRATE_4MS] = 4,
    [POLLRATE_2MS] = 2,
    [POLLRATE_1MS] = 1,
};


int cmd_pollrate_legacy(usbdevice* kb, pollrate_t rate){
    usbsend(kb, (&(ctrltransfer) { .bRequestType = 0x40, .bRequest = 10, .wValue = nxp_and_legacy_pollrate[rate], .wIndex = 0, .wLength = 0, .timeout = 5000, .data = NULL }), 0, 1);

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

    if(!usbsend(kb, msg[0], MSG_SIZE, 1))
        return -1;
    if(active){
        // Set up key input
        if(!usbsend(kb, msg[1], MSG_SIZE, 1))
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

int cmd_pollrate(usbdevice* kb, pollrate_t rate){
    uchar msg[MSG_SIZE] = {
        CMD_SET, FIELD_POLLRATE, 0, 0, nxp_and_legacy_pollrate[rate]
    };
    if(!usbsend(kb, msg, sizeof(msg), 1))
        return -1;
    // Device should disconnect+reconnect, but update the poll rate field in case it doesn't
    kb->pollrate = rate;
    return 0;
}

void nxp_mouse_setfps(usbdevice* kb, int fps){
    // Assumes 2 packets at most
    if(fps > 35)
        kb->usbdelay_ns = 6000000L;
    else
        kb->usbdelay_ns = 10000000L;
}

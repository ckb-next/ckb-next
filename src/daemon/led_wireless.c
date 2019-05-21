#include "led.h"
#include "usb.h"

int updatergb_wireless(usbdevice* kb, lighting* lastlight, lighting* newlight){
    // Dark Core zone bits:
    // 1: Front
    // 2: Thumb
    // 4: Rear

    // Dark Core commands:
    // 00: Colour Shift
    // 01: Colour Pulse
    // 03: Rainbow
    // 07: Static Colour
    // FF: No animation (black)


    if(IS_DARK_CORE(kb)) {
        for(int zone = 0; zone < 3; zone++) {
            uchar data_pkt[MSG_SIZE] = {
                CMD_SET, 0xaa, 0
            };

            data_pkt[4]  = 1 << zone;        // Bitmask.
            data_pkt[5]  = 7;                // Command (static colour).
            data_pkt[8]  = 100;              // Opacity (100%).
            data_pkt[9]  = newlight->r[zone];
            data_pkt[10] = newlight->g[zone];
            data_pkt[11] = newlight->b[zone];
            // Colour gets sent twice for some reason.
            data_pkt[12] = newlight->r[zone];
            data_pkt[13] = newlight->g[zone];
            data_pkt[14] = newlight->b[zone];
            data_pkt[15] = 5 - zone;         // Profile byte?

            if(!usbsend(kb, data_pkt, 1))
                return -1;
        }
    } else if (IS_K63_WL(kb)) {
        uchar data_pkt[MSG_SIZE] = {
            CMD_SET, 0xaa, 0
        };

        data_pkt[4]  = 0;
        data_pkt[5]  = 7;
        data_pkt[8]  = 100;
        data_pkt[9]  = newlight->r[0];
        data_pkt[10] = newlight->g[0];
        data_pkt[11] = newlight->b[0];
        // Colour gets sent twice for some reason.
        data_pkt[12] = newlight->r[0];
        data_pkt[13] = newlight->g[0];
        data_pkt[14] = newlight->b[0];
        data_pkt[15] = 0;

        if(!usbsend(kb, data_pkt, 1))
            return -1;
    }

    memcpy(lastlight, newlight, sizeof(lighting));
    return 0;
}

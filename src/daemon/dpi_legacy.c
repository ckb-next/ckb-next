#include "dpi.h"
#include "usb.h"

void cmd_lift_legacy(usbdevice* kb, usbmode* mode, int dummy1, int dummy2, const char* height){
    (void)kb;
    (void)dummy1;
    (void)dummy2;

    uchar heightnum;
    if(sscanf(height, "%hhu", &heightnum) != 1)
        return;
    if(heightnum > LIFT_MAX || heightnum < LIFT_MIN)
        return;
    mode->dpi.lift = heightnum;

    usbsend(kb, (&(ctrltransfer) { .bRequestType = 0x40, .bRequest = 13, .wValue = heightnum - 1, .wIndex = 0, .wLength = 0, .timeout = 5000, .data = NULL }), 0, 1);
}

void cmd_snap_legacy(usbdevice* kb, usbmode* mode, int dummy1, int dummy2, const char* enable){
    (void)kb;
    (void)dummy1;
    (void)dummy2;

    if(!strcmp(enable, "on")) {
        mode->dpi.snap = 1;
        usbsend(kb, (&(ctrltransfer) { .bRequestType = 0x40, .bRequest = 100, .wValue = 0x0000, .wIndex = 0, .wLength = 1, .timeout = 5000, .data = &mode->dpi.snap }), 0, 1);
    } else if(!strcmp(enable, "off")) {
        mode->dpi.snap = 0;
        usbsend(kb, (&(ctrltransfer) { .bRequestType = 0x40, .bRequest = 100, .wValue = 0x0000, .wIndex = 0, .wLength = 0, .timeout = 5000, .data = NULL }), 0, 1);
    }
}

int updatedpi_legacy(usbdevice* kb, int force){
    if(!kb->active)
        return 0;
    dpiset* lastdpi = &kb->profile->lastdpi;
    dpiset* newdpi = &kb->profile->currentmode->dpi;
    // Don't do anything if the settings haven't changed
    if(!force && !lastdpi->forceupdate && !newdpi->forceupdate
            && !memcmp(lastdpi, newdpi, sizeof(dpiset)))
        return 0;
    lastdpi->forceupdate = newdpi->forceupdate = 0;

    // Build DPI packet
    // 0x04 0x02 Y1 X1 Y2 X2 Y3 X3 YS XS
    uchar dpi_pkt[10] = { 0x04, 0x02, 0 };

    // M95 has 3 stages + sniper
    int pos = 2;
    for(int i = 1; i < 4; i++){
        // Offset to skip the header, Y is first
        // Round to the next DPI
        dpi_pkt[i - 1 + pos] = newdpi->y[i] / 50;
        pos++;
        dpi_pkt[i - 1 + pos] = newdpi->x[i] / 50;
    }

    // Add sniper
    dpi_pkt[8] = newdpi->y[0] / 50;
    dpi_pkt[9] = newdpi->x[0] / 50;
    
    usbsend(kb, (&(ctrltransfer) { .bRequestType = 0x40, .bRequest = 174, .wValue = 0x0000, .wIndex = 0, .wLength = sizeof(dpi_pkt), .timeout = 5000, .data = dpi_pkt }), 0, 1);
    
    memcpy(lastdpi, newdpi, sizeof(dpiset));
    return 0;
}


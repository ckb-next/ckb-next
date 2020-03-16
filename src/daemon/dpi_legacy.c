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
    if(mode->dpi.lift == heightnum)
        return;
    mode->dpi.lift = heightnum;

    usbsend_control(kb, NULL, 0, 13, heightnum - 1, 0);
}

void cmd_snap_legacy(usbdevice* kb, usbmode* mode, int dummy1, int dummy2, const char* enable){
    (void)kb;
    (void)dummy1;
    (void)dummy2;

    if(!strcmp(enable, "on")) {
        if(mode->dpi.snap)
            return;
        mode->dpi.snap = 1;
        usbsend_control(kb, &mode->dpi.snap, 1, 100, 0, 0);
    }
    if(!strcmp(enable, "off")) {
        if(!mode->dpi.snap)
            return;
        mode->dpi.snap = 0;
        usbsend_control(kb, NULL, 0, 100, 0, 0);
    }
}

int updatedpi_legacy(usbdevice* kb, int force){
    if(!kb->active)
        return 0;
    dpiset* lastdpi = &kb->profile->lastdpi;
    dpiset* newdpi = &kb->profile->currentmode->dpi;
    // Don't do anything if the settings haven't changed
    if(force || lastdpi->forceupdate || newdpi->forceupdate
            || memcmp(lastdpi->x, newdpi->x, sizeof(ushort) * DPI_COUNT)
            || memcmp(lastdpi->y, newdpi->y, sizeof(ushort) * DPI_COUNT))
    {
        lastdpi->forceupdate = newdpi->forceupdate = 0;

        // Build DPI packet
        // 0x04 0x02 Y1 X1 Y2 X2 Y3 X3 YS XS
        uchar dpi_pkt[10] = { 0x04, 0x02, 0 };

        // M95 has 3 stages + sniper
        int pos = 2;
        for(int i = 1; i < 4; i++){
            // Offset to skip the header, Y is first
            // Round to the next DPI
            dpi_pkt[pos++] = newdpi->y[i] / 50;
            dpi_pkt[pos++] = newdpi->x[i] / 50;
        }

        // Add sniper
        dpi_pkt[8] = newdpi->y[0] / 50;
        dpi_pkt[9] = newdpi->x[0] / 50;

        usbsend_control(kb, dpi_pkt, 10, 174, 0x0000, 0);
        DELAY_CUSTOM(400);

        // Maybe these are needed for the next command not to fail
        usbrecv_control(kb, dpi_pkt, 2, 5, 0, 0);
        usbrecv_control(kb, dpi_pkt, 10, 4, 0, 0);
    }

    // If the DPI stage differs, set the new one
    if(newdpi->current != lastdpi->current)
    {
        // Sniper is 4 instead of 0. Everything else remains the same.
        uchar dpistage = (newdpi->current && newdpi->current < 5 ? newdpi->current : 0x04);
        uchar dpisetpkt[] = {0x01, dpistage, 0x04, 0x03, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01};
        usbsend_control(kb, dpisetpkt, 10, 3, 0x0000, 0);
    }
    
    memcpy(lastdpi, newdpi, sizeof(dpiset));
    return 0;
}


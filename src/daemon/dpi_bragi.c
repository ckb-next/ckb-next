#include "dpi.h"
#include "usb.h"

void cmd_lift_bragi(usbdevice* kb, usbmode* mode, int dummy1, int dummy2, const char* height){
    (void)kb;
    (void)dummy1;
    (void)dummy2;

    uchar heightnum;
    if(sscanf(height, "%hhu", &heightnum) != 1)
        return;
    if(heightnum > LIFT_MAX || heightnum < LIFT_MIN)
        return;
    mode->dpi.lift = heightnum;

#warning "FIXME"
}

void cmd_snap_bragi(usbdevice* kb, usbmode* mode, int dummy1, int dummy2, const char* enable){
    (void)kb;
    (void)dummy1;
    (void)dummy2;

    if(!strcmp(enable, "on")) {
        mode->dpi.snap = 1;
#warning "FIXME"
    }
    if(!strcmp(enable, "off")) {
        mode->dpi.snap = 0;
#warning "FIXME"
    }
}

int updatedpi_bragi(usbdevice* kb, int force){
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
#warning "FIXME"
    
    memcpy(lastdpi, newdpi, sizeof(dpiset));
    return 0;
}


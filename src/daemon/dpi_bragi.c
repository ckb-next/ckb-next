#include "dpi.h"
#include "usb.h"
#include "bragi_proto.h"
#include "bragi_common.h"

void cmd_snap_bragi(usbdevice* kb, usbmode* mode, int dummy1, int dummy2, const char* enable){
    (void)kb;
    (void)dummy1;
    (void)dummy2;

    uchar orig_snap = mode->dpi.snap;
    if(!strcmp(enable, "on"))
        mode->dpi.snap = 1;

    if(!strcmp(enable, "off"))
        mode->dpi.snap = 0;

    if(mode->dpi.snap != orig_snap){
        uchar pkt[BRAGI_JUMBO_SIZE] = {BRAGI_MAGIC, BRAGI_SET, BRAGI_ANGLE_SNAP, 0, mode->dpi.snap};
        uchar response[BRAGI_JUMBO_SIZE] = {0};
        if(!usbrecv(kb, pkt, sizeof(pkt), response) || bragi_check_success(pkt, response))
            return;
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

    // Set the current DPI requested.
    uchar response[BRAGI_JUMBO_SIZE] = {0};
    uchar pkt[BRAGI_JUMBO_SIZE] = {BRAGI_MAGIC, BRAGI_SET, BRAGI_DPI_X, 0};
    pkt[4] = newdpi->x[newdpi->current] & 0xFF;
    pkt[5] = (newdpi->x[newdpi->current] >> 8) & 0xFF;
    if(!usbrecv(kb, pkt, sizeof(pkt), response))
        return -2;
#warning "FIXME: This fails initially for some reason, causing a reset"
    /*
    if(bragi_check_success(pkt, response))
        return -1;
    */

    pkt[2] = BRAGI_DPI_Y;
    pkt[4] = newdpi->y[newdpi->current] & 0xFF;
    pkt[5] = (newdpi->y[newdpi->current] >> 8) & 0xFF;
    if(!usbrecv(kb, pkt, sizeof(pkt), response))
        return -2;

    /*
    if(bragi_check_success(pkt, response))
        return -1;
    */

    // M55 need this to properly update DPI LED
    kb->vtable.updatergb(kb, 1);

    memcpy(lastdpi, newdpi, sizeof(dpiset));
    return 0;
}


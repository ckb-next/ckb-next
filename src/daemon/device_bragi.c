#include "command.h"
#include "device.h"
#include "devnode.h"
#include "firmware.h"
#include "input.h"
#include "profile.h"
#include "usb.h"
#include "bragi_proto.h"

int setactive_bragi(usbdevice* kb, int active){
    uchar pkt[64] = {BRAGI_MAGIC, BRAGI_SET, BRAGI_MODE, 0, active};
    uchar response[64] = {0};
    if(!usbrecv(kb, pkt, response))
        return 1;
    kb->active = active - 1;
    return 0;
}

int start_mouse_bragi(usbdevice* kb, int makeactive){
    (void)makeactive;

#warning "FIXME"
    // Check if we're in software mode, and if so, force back to hardware until we explicitly want SW.
    uchar get_mode[64] = {BRAGI_MAGIC, BRAGI_GET, BRAGI_MODE, 0};
    uchar response[64] = {0};
    if(!usbrecv(kb, get_mode, response))
        return 1;

    if(response[3] == BRAGI_MODE_SOFTWARE){
        ckb_info("ckb%d Device is software mode during init. Switching to hardware\n", INDEX_OF(kb, keyboard));
        if(setactive_bragi(kb, BRAGI_MODE_HARDWARE))
            return 1;
    }

    uchar pollrateLUT[5] = {-1};
    pollrateLUT[BRAGI_POLLRATE_1MS] = 1;
    pollrateLUT[BRAGI_POLLRATE_2MS] = 2;
    pollrateLUT[BRAGI_POLLRATE_4MS] = 4;
    pollrateLUT[BRAGI_POLLRATE_8MS] = 8;
    // Get pollrate
    uchar poll_pkt[64] = {BRAGI_MAGIC, BRAGI_GET, BRAGI_POLLRATE, 0};
    if(!usbrecv(kb, poll_pkt, response))
        return 1;
    
    uchar pollrate = response[3];
    if(pollrate > 4)
        return 1;
    
    kb->pollrate = pollrateLUT[pollrate];
    
    kb->features |= FEAT_ADJRATE;
    kb->features &= ~FEAT_HWLOAD;
    
    // The daemon always sends RGB data through handle 0, so go ahead and open it
    uchar light_init[64] = {BRAGI_MAGIC, BRAGI_OPEN_HANDLE, BRAGI_LIGHTING_HANDLE, BRAGI_RES_LIGHTING};
    memset(response, 0, 64);
    if(!usbrecv(kb, light_init, response))
        return 1;

    // Check if the device returned an error
    // Non fatal for now. Should first figure out what the error codes mean.
    // Device returns 0x03 if we haven't opened the handle.
    if(response[2] != 0x00)
        ckb_err("ckb%d Bragi light init returned error 0x%hhx\n", INDEX_OF(kb, keyboard), response[2]);

    return 0;
}

int cmd_pollrate_bragi(usbdevice* kb, usbmode* dummy1, int dummy2, int rate, const char* dummy3){
    (void)dummy1;
    (void)dummy2;
    (void)dummy3;
    
    if(rate > 8 || rate < 0)
        return 0;
    
    uchar pollrateLUT[9] = {
        1,
        BRAGI_POLLRATE_1MS,
        BRAGI_POLLRATE_2MS,
        1,
        BRAGI_POLLRATE_4MS,
        1,
        1,
        1,
        BRAGI_POLLRATE_8MS,
    };

    uchar pkt[64] = {BRAGI_MAGIC, BRAGI_SET, BRAGI_POLLRATE, 0, pollrateLUT[rate]};
    uchar response[64] = {0};
    if(!usbrecv(kb, pkt, response))
        return 1;
    
    kb->pollrate = rate;
    return 0;
}

int cmd_active_bragi(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4){
    (void)dummy1;
    (void)dummy2;
    (void)dummy3;
    (void)dummy4;

    return setactive_bragi(kb, BRAGI_MODE_SOFTWARE);
}

int cmd_idle_bragi(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4){
    (void)dummy1;
    (void)dummy2;
    (void)dummy3;
    (void)dummy4;

    return setactive_bragi(kb, BRAGI_MODE_HARDWARE);
}


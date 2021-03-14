#include "command.h"
#include "device.h"
#include "devnode.h"
#include "firmware.h"
#include "input.h"
#include "profile.h"
#include "usb.h"
#include "bragi_proto.h"

// CUE polls devices every 52 seconds
const struct timespec bragi_poll_delay = { .tv_sec = 50 };
const uchar poll_pkt[64] = {0x08, 0x12};

void* bragi_poll_thread(void* ctx){
    usbdevice* kb = ctx;
    int ret;
    while(!(ret = clock_nanosleep(CLOCK_MONOTONIC, 0, &bragi_poll_delay, NULL))){
        queued_mutex_lock(dmutex(kb));
        if(kb->active){
            uchar poll_response[64] = {0};
            if(usbrecv(kb, poll_pkt, poll_response)){
                if(poll_response[1] != 0x12)
                    ckb_err("ckb%d: Invalid bragi poll response (0x%hhx)", INDEX_OF(kb, keyboard), poll_response[1]);
            } else {
                ckb_err("ckb%d: Error sending bragi poll packet", INDEX_OF(kb, keyboard));
            }
        }
        queued_mutex_unlock(dmutex(kb));
    }
    ckb_info("ckb%d: Bragi poll thread shutting down due to %d (%s)", INDEX_OF(kb, keyboard), ret, strerror(ret));
    return NULL;
}

static int setactive_bragi(usbdevice* kb, int active){
    const int ckb_id = INDEX_OF(kb, keyboard);
    uchar pkt[64] = {BRAGI_MAGIC, BRAGI_SET, BRAGI_MODE, 0, active};
    uchar response[64] = {0};
    if(!usbrecv(kb, pkt, response))
        return 1;
    if(response[2] != 0x00){
        ckb_err("ckb%d: Failed to set device to %s mode 0x%hhx", ckb_id, (active == BRAGI_MODE_SOFTWARE ? "SW" : "HW"), response[2]);
    }
    kb->active = active - 1;
    // We don't need to do anything else if we're going back to hardware mode
    if(active == BRAGI_MODE_HARDWARE)
        return 0;

    // Start poll thread for this device if it's not running already
    if(!kb->pollthread){
        kb->pollthread = malloc(sizeof(pthread_t));
        if(kb->pollthread){
            int err = pthread_create(kb->pollthread, 0, bragi_poll_thread, kb);
            if(err != 0){
                ckb_err("ckb%d: Failed to create bragi poll thread", ckb_id);
                free(kb->pollthread);
                kb->pollthread = NULL;
            } else {
                //pthread_detach(*kb->pollthread);
            }
        } else {
            ckb_err("ckb%d: Failed to allocate memory for bragi poll thread", ckb_id);
        }
    }

    // The daemon always sends RGB data through handle 0, so go ahead and open it
    uchar light_init[64] = {BRAGI_MAGIC, BRAGI_OPEN_HANDLE, BRAGI_LIGHTING_HANDLE, BRAGI_RES_LIGHTING};
    memset(response, 0, 64);
    if(!usbrecv(kb, light_init, response))
        return 1;

    // Check if the device returned an error
    // Non fatal for now. Should first figure out what the error codes mean.
    // Device returns 0x03 on writes if we haven't opened the handle.
    if(response[2] != 0x00){
        ckb_err("ckb%d: Bragi light init returned error 0x%hhx", ckb_id, response[2]);
        // CUE seems to attempt to close and reopen the handle if it gets 0x03 on open
        if(response[2] == 0x03){
            uchar light_deinit[64] = {BRAGI_MAGIC, BRAGI_CLOSE_HANDLE, 0x01, BRAGI_LIGHTING_HANDLE};
            if(!usbrecv(kb, light_deinit, response))
                return 1;
            if(response[2] != 0x00){
                ckb_err("ckb%d: Close lighting handle failed with 0x%hhx", ckb_id, response[2]);
            }
            // Try to reopen it
            if(!usbrecv(kb, light_init, response))
                return 1;
            if(response[2] != 0x00){
                ckb_err("ckb%d: Bragi light init (attempt 2) returned error 0x%hhx", ckb_id, response[2]);
            }
        }
    }

    return 0;
}

static int start_bragi_common(usbdevice* kb){
    kb->usbdelay = 10; // This might not be needed, but won't harm
#warning "FIXME. Read more properties, such as fw version and pairing id"
    // Check if we're in software mode, and if so, force back to hardware until we explicitly want SW.
    uchar pkt[64] = {BRAGI_MAGIC, BRAGI_GET, BRAGI_MODE, 0};
    uchar response[64] = {0};
    if(!usbrecv(kb, pkt, response))
        return 1;

    if(response[3] == BRAGI_MODE_SOFTWARE){
        ckb_info("ckb%d: Device is in software mode during init. Switching to hardware", INDEX_OF(kb, keyboard));
        if(setactive_bragi(kb, BRAGI_MODE_HARDWARE))
            return 1;
    }

#warning "Add error messages in case of failure"

    uchar pollrateLUT[5] = {-1};
    pollrateLUT[BRAGI_POLLRATE_1MS] = 1;
    pollrateLUT[BRAGI_POLLRATE_2MS] = 2;
    pollrateLUT[BRAGI_POLLRATE_4MS] = 4;
    pollrateLUT[BRAGI_POLLRATE_8MS] = 8;
    // Get pollrate
    pkt[2] = BRAGI_POLLRATE;
    if(!usbrecv(kb, pkt, response))
        return 1;
    
    uchar pollrate = response[3];
    if(pollrate > 4)
        return 1;
    
    kb->pollrate = pollrateLUT[pollrate];
    
    kb->features |= FEAT_ADJRATE;
    kb->features &= ~FEAT_HWLOAD;

    kb->usbdelay = USB_DELAY_DEFAULT;

    return 0;
}

int start_mouse_bragi(usbdevice* kb, int makeactive){
    if(start_bragi_common(kb))
        return 1;

    if(makeactive)
        setactive_bragi(kb, BRAGI_MODE_SOFTWARE);
    return 0;
}

int start_keyboard_bragi(usbdevice* kb, int makeactive){
    if(start_bragi_common(kb))
        return 1;

    if(makeactive)
        if(setactive_bragi(kb, BRAGI_MODE_SOFTWARE))
            return 1;
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

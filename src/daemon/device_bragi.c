#include "command.h"
#include "device.h"
#include "devnode.h"
#include "firmware.h"
#include "input.h"
#include "profile.h"
#include "usb.h"
#include "bragi_proto.h"
#include "bragi_common.h"

// CUE polls devices every 52 seconds
const struct timespec bragi_poll_delay = { .tv_sec = 50 };

void* bragi_poll_thread(void* ctx){
    uchar poll_pkt[BRAGI_JUMBO_SIZE] = {0x08, 0x12};
    usbdevice* kb = ctx;
    int ret;
    while(!(ret = clock_nanosleep(CLOCK_MONOTONIC, 0, &bragi_poll_delay, NULL))){
        queued_mutex_lock(dmutex(kb));
        if(kb->active){
            uchar poll_response[BRAGI_JUMBO_SIZE] = {0};
            if(usbrecv(kb, poll_pkt, sizeof(poll_response), poll_response)){
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
    if(bragi_set_property(kb, BRAGI_MODE, active)){
        ckb_err("ckb%d: Failed to set device to %s mode", ckb_id, (active == BRAGI_MODE_SOFTWARE ? "SW" : "HW"));
        return 1;
    }

    clear_input_and_rgb(kb, active - 1);
    // Propagate the active status to the parent
#warning "FIXME: On idle we should only propagate it if we're the last device left active"
    if(kb->parent){
        usbdevice* p = kb->parent;
        queued_mutex_lock(dmutex(p));
        // FIXME: Should we use setactive from the vtable?
        p->active = kb->active;
        queued_mutex_unlock(dmutex(p));
    }

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
    uchar light_init[BRAGI_JUMBO_SIZE] = {BRAGI_MAGIC, BRAGI_OPEN_HANDLE, BRAGI_LIGHTING_HANDLE, BRAGI_RES_LIGHTING};
    uchar response[BRAGI_JUMBO_SIZE] = {0};
    if(!usbrecv(kb, light_init, sizeof(light_init), response))
        return 1;

    // Check if the device returned an error
    // Non fatal for now. Should first figure out what the error codes mean.
    // Device returns 0x03 on writes if we haven't opened the handle.
    if(response[2] != 0x00){
        ckb_err("ckb%d: Bragi light init returned error 0x%hhx", ckb_id, response[2]);
        // CUE seems to attempt to close and reopen the handle if it gets 0x03 on open
        if(response[2] == 0x03){
            uchar light_deinit[BRAGI_JUMBO_SIZE] = {BRAGI_MAGIC, BRAGI_CLOSE_HANDLE, 0x01, BRAGI_LIGHTING_HANDLE};
            if(!usbrecv(kb, light_deinit, sizeof(light_deinit), response))
                return 1;
            if(response[2] != 0x00){
                ckb_err("ckb%d: Close lighting handle failed with 0x%hhx", ckb_id, response[2]);
            }
            // Try to reopen it
            if(!usbrecv(kb, light_init, sizeof(light_init), response))
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
    int prop = bragi_get_property(kb, BRAGI_MODE);
    if(prop < 0){
        ckb_fatal("ckb%d: Couldn't get bragi device mode. Aborting", INDEX_OF(kb, keyboard));
        return 1;
    }

    if(prop == BRAGI_MODE_SOFTWARE){
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
    prop = bragi_get_property(kb, BRAGI_POLLRATE);

    uchar pollrate = prop;
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
        if(setactive_bragi(kb, BRAGI_MODE_SOFTWARE))
            return 1;
    return 0;
}

int start_keyboard_bragi(usbdevice* kb, int makeactive){
    if(start_bragi_common(kb))
        return 1;

    int prop = bragi_get_property(kb, BRAGI_HWLAYOUT);
    // Physical layout detection.
    kb->layout = prop;
    // So far ISO and ANSI are known and match.
    if (kb->layout != LAYOUT_ANSI && kb->layout != LAYOUT_ISO) {
        ckb_warn("Got unknown physical layout byte value %d, please file a bug report mentioning your keyboard's physical layout", prop);
        kb->layout = LAYOUT_UNKNOWN;
    }

    if(makeactive)
        if(setactive_bragi(kb, BRAGI_MODE_SOFTWARE))
            return 1;
    return 0;
}

static inline int bragi_dongle_probe(usbdevice* kb){
    // Ask the device for the mapping
    int prop = bragi_get_property(kb, BRAGI_SUBDEVICE_BITFIELD);
    // Go through every device and add it
    for(int i = 1; i < 8; i++){
        if(!((prop >> i) & 1))
            continue;
        ckb_info("Found bragi subdevice %d", i);
        // Find a free device slot
        for(int index = 1; index < DEV_MAX; index++){
            usbdevice* subkb = keyboard + index;
            if(queued_mutex_trylock(dmutex(subkb))){
                // If the mutex is locked then the device is obviously in use, so keep going
                continue;
            }

            // Ignore it if it has already been initialised
            if(subkb->status > DEV_STATUS_DISCONNECTED){
                queued_mutex_unlock(dmutex(subkb));
                continue;
            }

            subkb->status = DEV_STATUS_CONNECTING;
            subkb->fwversion = 1234; // invalid
            subkb->parent = kb;

            subkb->out_ep_packet_size = kb->out_ep_packet_size;

            // Assign a mouse vtable for now, can be changed later after we get vid/pid
            memcpy(&subkb->vtable, &vtable_bragi_mouse, sizeof(devcmd));

            subkb->bragi_child_id = i;

            // Add the device to our children array
            pthread_mutex_lock(cmutex(kb));
            kb->children[i-1] = subkb;
            pthread_mutex_unlock(cmutex(kb));

            // Fill dev information
            ushort vid = bragi_get_property(subkb, BRAGI_VID);
            ushort pid = bragi_get_property(subkb, BRAGI_PID);

            ckb_info("Subkb vendor: 0x%hx, product: 0x%hx", vid, pid);
            subkb->vendor = vid;
            subkb->product = pid;

            // Unlock mutex here for now
            //queued_mutex_unlock(dmutex(subkb));
            setupusb(subkb);
            break;
        }
    }
    return 0;
}

int start_dongle_bragi(usbdevice* kb, int makeactive){
    // Probe for devices
    bragi_dongle_probe(kb);
    return 0;
}

static const uchar daemon_pollrate_to_bragi[9] = {
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

int cmd_pollrate_bragi(usbdevice* kb, usbmode* dummy1, int dummy2, int rate, const char* dummy3){
    (void)dummy1;
    (void)dummy2;
    (void)dummy3;

    if(rate > 8 || rate < 0)
        return 0;

    if(bragi_set_property(kb, BRAGI_POLLRATE, daemon_pollrate_to_bragi[rate]))
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

void bragi_get_battery_info(usbdevice* kb){
    long int stat = bragi_get_property(kb, BRAGI_BATTERY_STATUS);
    long int chg = bragi_get_property(kb, BRAGI_BATTERY_LEVEL);
    if(stat < 0 || chg < 0){
        ckb_err("ckb%d: Failed to get bragi battery properties", INDEX_OF(kb, keyboard));
        return;
    }
    kb->battery_level = chg / 10;
    kb->battery_status = stat;
}

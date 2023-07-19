#include <stdbool.h>
#include "command.h"
#include "device.h"
#include "devnode.h"
#include "firmware.h"
#include "input.h"
#include "profile.h"
#include "usb.h"
#include "bragi_proto.h"
#include "bragi_common.h"
#include "utils.h"
#include "led.h"

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
    if(active == BRAGI_MODE_HARDWARE){
        bragi_close_handle(kb, BRAGI_LIGHTING_HANDLE);
        bragi_close_handle(kb, BRAGI_2ND_LIGHTING_HANDLE);
    }

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

    // The daemon will always send RGB data through handle 0 (), so go ahead and open it
    uchar res = BRAGI_RES_LIGHTING;
    if(IS_MONOCHROME_DEV(kb))
        res = BRAGI_RES_LIGHTING_MONOCHROME;
    int light = bragi_open_handle(kb, BRAGI_LIGHTING_HANDLE, res);
    if(light < 0)
        return light;

    // Check if the device returned an error
    // Non fatal for now. Should first figure out what the error codes mean.
    // Device returns 0x03 on writes if we haven't opened the handle.
    if(light == 0x01 || light == 0x06) {
        // Some K100s have been observed to return either 0x01 or 0x06 which probably means "not supported"
        // If we get either response, we instead try to open the alt rgb lighting resource
        ckb_warn("ckb%d: Bragi light init returned not supported", ckb_id);
        light = bragi_open_handle(kb, BRAGI_LIGHTING_HANDLE, BRAGI_RES_ALT_LIGHTING);
        if(light < 0)
            return light;

        if(light) {
            ckb_err("ckb%d: Bragi alt light init returned error 0x%hhx", ckb_id, (uchar)light);
        } else {
            // Swap the RGB function if we're using alt lighting
            kb->vtable.updatergb = updatergb_keyboard_bragi_alt;

            // Open the second lighting handle
            // We don't yet know if this is K100 specific or if it has to be opened along with the alt one
            light = bragi_open_handle(kb, BRAGI_2ND_LIGHTING_HANDLE, BRAGI_RES_LIGHTING_EXTRA);
            if(light < 0)
                return light;

            if(light){
                ckb_err("ckb%d: Bragi extra light init returned error 0x%hhx", ckb_id, (uchar)light);
            } else {
                // FIXME: Figure out what this is. It is definitely lighting related.
                // It has been observed to break lighting on specific keys (such has the '3' key)
                // but only on a specific K100 (fw 0.32.6, bld 0.10.45)
                // It remains commented out for now
                /*uchar pkt2[BRAGI_JUMBO_SIZE] = {
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // header
                    0x1b, 0x00, 0x20, 0xe7, 0xca, 0x2f, 0x88, 0x9f, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00,
                    0x00, 0x01, 0x00, 0x0d, 0x00, 0x01, 0x01, 0x00, 0x04, 0x00, 0x00, 0x08, 0x06, 0x02, 0x48, 0x00,
                    0x00, 0x00, 0x2b, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x01, 0x00, 0x00, 0x00, 0xff, 0x02, 0x00,
                    0x00, 0x00, 0xff, 0x72, 0x00, 0x00, 0x00, 0xff, 0x80, 0x00, 0x00, 0x00, 0xff, 0x89, 0x00, 0x00,
                    0x00, 0xff, 0xb6, 0x00, 0x00, 0x00, 0xff, 0xb7, 0x00, 0x00, 0x00, 0xff, 0xb8, 0x00, 0x00, 0x00,
                    0xff, 0xb9, 0x00, 0x00, 0x00, 0xff, 0xba, 0x00, 0x00, 0x00, 0xff, 0xbb, 0x00, 0x00, 0x00, 0xff,
                    0xbc, 0x00, 0x00, 0x00, 0xff, 0xbd, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                };

                if(bragi_write_to_handle(kb, pkt2, BRAGI_2ND_LIGHTING_HANDLE, sizeof(pkt2), 0x48))
                    return 1;*/
            }
        }
    } else if(light) {
        ckb_err("ckb%d: Bragi light init returned error 0x%hhx", ckb_id, (uchar)light);
    }
    return 0;
}

// Dear compiler, please emit a bswap and an shr. Thank you!
static inline uint32_t bragi_fwver_bswap(uint32_t fwv){
    return ((((fwv) & 0xff000000) >> 24) | (((fwv) & 0x00ff0000) >>  8) |
        (((fwv) & 0x0000ff00) <<  8) | (((fwv) & 0x000000ff) << 24)) >> 8;
}

static int start_bragi_common(usbdevice* kb){
    kb->pollrate = POLLRATE_UNKNOWN;
    // Assume 1 ms unless told otherwise
    kb->maxpollrate = POLLRATE_1MS;

    // Check if we're in HW mode, and if so, switch to software in order to read the properties/handles
    int64_t prop = bragi_get_property(kb, BRAGI_MODE);
    if(prop < 0){
        ckb_fatal("ckb%d: Couldn't get bragi device mode. Aborting", INDEX_OF(kb, keyboard));
        return 1;
    }

    if(prop == BRAGI_MODE_HARDWARE){
        // We set this directly to avoid spawning the poll thread
        bragi_set_property(kb, BRAGI_MODE, BRAGI_MODE_SOFTWARE);
    }

    // Read FW versions
    kb->fwversion = kb->bldversion = kb->radioappversion = kb->radiobldversion = UINT32_MAX;

    prop = bragi_get_property(kb, BRAGI_APP_VER);
    if(prop >= 0)
        kb->fwversion = bragi_fwver_bswap(prop);

    prop = bragi_get_property(kb, BRAGI_BLD_VER);
    if(prop >= 0)
        kb->bldversion = bragi_fwver_bswap(prop);

    prop = bragi_get_property(kb, BRAGI_RADIO_APP_VER);
    if(prop >= 0)
        kb->radioappversion = bragi_fwver_bswap(prop);

    prop = bragi_get_property(kb, BRAGI_RADIO_BLD_VER);
    if(prop >= 0)
        kb->radiobldversion = bragi_fwver_bswap(prop);

    // Get pollrate
    prop = bragi_get_property(kb, BRAGI_POLLRATE);
    if(prop >= 0 && prop - 1 < POLLRATE_COUNT)
        kb->pollrate = prop - 1;

    // Get max pollrate
    prop = bragi_get_property(kb, BRAGI_MAX_POLLRATE);
    if(prop >= 0 && prop - 1 < POLLRATE_COUNT)
        kb->maxpollrate = prop - 1;

    kb->features |= FEAT_ADJRATE;
    kb->features &= ~FEAT_HWLOAD;

    // Check if the device supports fine or coarse brightness
    if(bragi_get_property(kb, BRAGI_BRIGHTNESS) >= 0)
        kb->brightness_mode = BRIGHTNESS_HARDWARE_FINE;
    else if(bragi_get_property(kb, BRAGI_BRIGHTNESS_COARSE) >= 0)
        kb->brightness_mode = BRIGHTNESS_HARDWARE_COARSE;

    // Read pairing ID
    if(IS_WIRELESS_DEV(kb)){
        int pair = bragi_open_handle(kb, BRAGI_GENERIC_HANDLE, BRAGI_RES_PAIRINGID);
        if(pair < 0)
            return pair;

        if(!pair){
            uchar* pairid;
            uint32_t dlen = bragi_read_from_handle(kb, BRAGI_GENERIC_HANDLE, &pairid);
            if(dlen == PAIR_ID_SIZE){
                memcpy(kb->wl_pairing_id, pairid, dlen);
            } else {
                // FIXME: Clean this up
                printf("Invalid pairing ID length (%"PRIu32"). Data: ", dlen);
                for(uint32_t i = 0; i < dlen; i++)
                    printf("%02hhx ", pairid[i]);

                putchar('\n');
            }

            free(pairid);

            bragi_close_handle(kb, BRAGI_GENERIC_HANDLE);
        }

        char str[PAIR_ID_SIZE*3+1] = {0};
        for(int i = 0; i < PAIR_ID_SIZE; i++)
            snprintf(str + i * 3, sizeof(str) - i * 3, "%02hhx ", kb->wl_pairing_id[i]);

        ckb_info("ckb%d: Pairing id: %s", INDEX_OF(kb, keyboard), str);
    }

    // Switch back to HW mode
    bragi_set_property(kb, BRAGI_MODE, BRAGI_MODE_HARDWARE);

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

    int64_t prop = bragi_get_property(kb, BRAGI_HWLAYOUT);
    // Physical layout detection.
    kb->layout = prop;
    // So far ISO and ANSI are known and match.
    if (kb->layout != LAYOUT_ANSI && kb->layout != LAYOUT_ISO) {
        ckb_warn("Got unknown physical layout byte value %" PRId64 ", please file a bug report mentioning your keyboard's physical layout", prop);
        kb->layout = LAYOUT_UNKNOWN;
    }

    if(makeactive)
        if(setactive_bragi(kb, BRAGI_MODE_SOFTWARE))
            return 1;
    return 0;
}

static inline int bragi_dongle_probe(usbdevice* kb){
    // Ask the device for the mapping
    int64_t prop = bragi_get_property(kb, BRAGI_SUBDEVICE_BITFIELD);
    if(prop < 0)
        return -1;
    bragi_update_dongle_subdevs(kb, prop);
    return 0;
}

int start_dongle_bragi(usbdevice* kb, int makeactive){
    start_bragi_common(kb);
    // Force back to SW mode
    // FIXME: Does this make a difference?
    bragi_set_property(kb, BRAGI_MODE, BRAGI_MODE_SOFTWARE);
    kb->active = 1;
    // Probe for devices
    // FIXME: Do something about this failing
    bragi_dongle_probe(kb);
    return 0;
}

int cmd_pollrate_bragi(usbdevice* kb, pollrate_t rate){
    if(bragi_set_property(kb, BRAGI_POLLRATE, rate + 1))
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
    int64_t stat = bragi_get_property(kb, BRAGI_BATTERY_STATUS);
    int64_t chg = bragi_get_property(kb, BRAGI_BATTERY_LEVEL);
    if(stat < 0 || chg < 0){
        ckb_err("ckb%d: Failed to get bragi battery properties", INDEX_OF(kb, keyboard));
        return;
    }
    kb->battery_level = chg / 10;
    kb->battery_status = stat;
}

void bragi_delay(usbdevice* kb, delay_type_t type){
    // Don't bother with delays in bragi.
    // Since we use usbrecv for everything, the devices tell us when they are ready to handle another packet.
    return;
}

static void bragi_set_device_keys(usbdevice* kb, unsigned char* pairing_id, unsigned char* encryption_key) {
    uchar pkt[BRAGI_JUMBO_SIZE] = {0};

    // Write the PAIRING_ID to the device first
    bragi_open_handle(kb, BRAGI_GENERIC_HANDLE, BRAGI_RES_PAIRINGID);
    memcpy(pkt + 7, pairing_id, sizeof(uchar) * 8);
    bragi_write_to_handle(kb, pkt, BRAGI_GENERIC_HANDLE, sizeof(pkt), sizeof(uchar) * 8);
    bragi_close_handle(kb, BRAGI_GENERIC_HANDLE);

    // The ENCRYPTION_KEY must be written directly after the PAIRING_ID
    bragi_open_handle(kb, BRAGI_GENERIC_HANDLE, BRAGI_RES_ENCRYPTIONKEY);
    memcpy(pkt + 7, encryption_key, sizeof(uchar) * 16);
    bragi_write_to_handle(kb, pkt, BRAGI_GENERIC_HANDLE, sizeof(pkt), sizeof(uchar) * 16);

    bragi_close_handle(kb, BRAGI_GENERIC_HANDLE);
}

void cmd_pair_bragi(usbdevice* kb, usbmode* mode, int dummy1, int dummy2, const char* to) {
    (void)mode;
    (void)dummy1;
    (void)dummy2;

    if(kb->protocol != PROTO_BRAGI) {
        ckb_err("Dongle device doesn't use bragi protocol");
        return;
    }

    uchar encryption_key[16];
    if(ckb_generate_random(16, encryption_key) < 16) {
        ckb_err("Couldn't generate encryption key");
        return;
    }

    uchar pairing_id[8];
    if(ckb_generate_random(8, pairing_id) < 8) {
        ckb_err("Couldn't generate pairing ID");
        return;
    }

    // Set the dongle device pairing_id and encryption_key
    bragi_set_device_keys(kb, pairing_id, encryption_key);

    char* devices = strdup(to);
    char* token_ptr;
    char* device_name = strtok_r(devices, ",", &token_ptr);
    while(device_name != NULL) {

        int ckb_id = 0;
        if(!sscanf(device_name, "ckb%d", &ckb_id)) {
            ckb_err("Failed to parse device name: %s", device_name);
            continue;
        }

        usbdevice* device = keyboard + ckb_id;
        queued_mutex_lock(dmutex(device));

        if(device->protocol == PROTO_BRAGI) {
            if(device->status == DEV_STATUS_CONNECTED) {
                bragi_set_device_keys(device, pairing_id, encryption_key);
                ckb_info("Synchronized %s to %s", device->name, kb->name);
            } else {
                ckb_err("Device ckb%d not found or connected", ckb_id);
            }
        } else {
            ckb_err("ckb%d doesn't use bragi protocol", ckb_id);
        }

        device_name = strtok_r(NULL, ",", &token_ptr);
        queued_mutex_unlock(dmutex(device));
    }
    free(devices);
}


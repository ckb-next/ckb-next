#include "profile.h"
#include "usb.h"
#include "led.h"

#define PROFILE_SIZE     268
#define LIGHTCOUNT_SIZE    4
#define LIGHTRGB_SIZE   1024

#define LAYER_COUNT        1

static int k95p_get_file(usbdevice* kb, const char* filename, int size, int profile, uchar* data){
    ckb_info("Receiving %s\n", filename);
    // Prepare to load the file.
    uchar switch_pkt[2][MSG_SIZE] = {
        { 0x07, 0x17, 0x0c, profile, 0 }, // Switch profile.
        { 0x07, 0x17, 0x07, 0x00, 0 }     // Load filename.
    };
    memcpy(switch_pkt[1] + 4, filename, 16);
    ckb_info("Syncing\n");
    if(!usbsend(kb, switch_pkt[0], 2))
        return -1;
    // Synchronisation.
    uchar sync_pkt[2][MSG_SIZE] = {
        { 0x0e, 0x17, 0x0d, 0x00, 0 }, // Sync packet.
        { 0x0e, 0x17, 0x03, 0x01, 0 }  // ???
    };
    uchar dummy[MSG_SIZE];
    for(int i = 0; i <= 1; i++){
        if(!usbrecv(kb, sync_pkt[i], dummy))
            return -1;
    }
    ckb_info("Beginning read...\n");
    // Read file.
    while(size > 0){
        for(int pkt = 1; pkt <= 5 && size > 0; pkt++, size -= 60){
            int s = (size > 60) ? 60 : size;
            uchar read_pkt[MSG_SIZE] = { 0xff, pkt, s, 0x00, 0 };
            uchar out_pkt[MSG_SIZE];
            if(!usbrecv(kb, read_pkt, out_pkt))
                return -1;
            memcpy(data, out_pkt + 4, s);
            *data = *data + s;
        }
        // Synchronise between bursts.
        if(!usbrecv(kb, sync_pkt[0], dummy))
            return -1;
    }
    ckb_info("Finishing read...\n");
    // Finish up.
    uchar eof_pkt[MSG_SIZE] = { 0x07, 0x17, 0x08, 0x00, 0 };
    if(!usbsend(kb, eof_pkt, 1))
        return -1;
    ckb_info("Done reading %s!\n", filename);
    return 1;
}

static int k95p_send_file(usbdevice* kb, const char* filename, int size, int profile, uchar* data){
    ckb_info("Sending %s\n", filename);
    // Prepare to send the file.
    uchar switch_pkt[2][MSG_SIZE] = {
        { 0x07, 0x17, 0x0c, profile, 0 }, // Switch profile.
        { 0x07, 0x17, 0x05, 0x00, 0 }     // Write to filename.
    };
    memcpy(switch_pkt[1] + 4, filename, 16);
    ckb_info("Syncing\n");
    if(!usbsend(kb, switch_pkt[0], 2))
        return -1;
    // Synchronisation.
    uchar sync_pkt[2][MSG_SIZE] = {
        { 0x0e, 0x17, 0x0d, 0x00, 0 }, // Sync packet.
        { 0x0e, 0x17, 0x03, 0x00, 0 }  // ???
    };
    uchar dummy[MSG_SIZE];
    for(int i = 0; i <= 1; i++){
        if(!usbrecv(kb, sync_pkt[i], dummy))
            return -1;
    }
    ckb_info("Beginning write...\n");
    // Write file.
    while(size > 0){
        for(int pkt = 1; pkt <= 5 && size > 0; pkt++, size -= 60){
            int s = (size > 60) ? 60 : size;
            uchar out_pkt[MSG_SIZE] = { 0x7f, pkt, s, 0x00, 0 };
            memcpy(out_pkt + 4, data, s);
            if(!usbsend(kb, out_pkt, 1))
                return -1;
            *data = *data + s;
        }
        // Write burst.
        uchar out_pkt[MSG_SIZE] = { 0x07, 0x17, 0x09, 0 };
        if(!usbsend(kb, out_pkt, 1))
            return -1;
        // Synchronise between bursts.
        if(!usbrecv(kb, sync_pkt[0], dummy))
            return -1;
    }
    ckb_info("Finishing write...\n");
    // Finish up.
    uchar eof_pkt[MSG_SIZE] = { 0x07, 0x17, 0x08, 0x00, 0 };
    if(!usbsend(kb, eof_pkt, 1))
        return -1;
    ckb_info("Done writing %s!", filename);
    return 1;
}

static int loadrgb_k95p(usbdevice* kb, lighting* light, int mode){
    uchar data[18*MSG_SIZE];
    // Profile RGB count.
    if(!k95p_get_file(kb, "lightcnt.cnt", LIGHTCOUNT_SIZE, mode, data))
        return -1;
    memset(light, 0, sizeof(lighting));
    uchar lightcount = data[0];
    if(lightcount == 0)
        // No lighting profiles; keyboard is black.
        return 0;
    else if(lightcount > LAYER_COUNT){
        ckb_warn("Profile %d appears to have multiple lighting layers, but this is currently unsupported.", mode);
        ckb_warn("Will use the first lighting layer.");
        lightcount = 1;
    }
    for(int layer = 0; layer < lightcount; layer++){
        char filename[16] = { 0 };
        snprintf(filename, 16, "light_%02hhx.r", layer);
        if(!k95p_get_file(kb, filename, LIGHTRGB_SIZE, mode, data))
            return -1;
        for(int key = 0; key < N_KEYS_HW; key++){
            // The hardware profiles represent colours as RGBA,
            // but we represent them as RGB, so we need to multiply
            // by alpha.
            int r = data[4*key + 0];
            int g = data[4*key + 1];
            int b = data[4*key + 2];
            int a = data[4*key + 3] + 1; // Round up.

            r = (r * a) >> 8;
            g = (g * a) >> 8;
            b = (b * a) >> 8;

            light->r[key] = r;
            light->g[key] = g;
            light->b[key] = b;
        }
    }
    return 1;
}

static int savergb_k95p(usbdevice* kb, lighting* light, int mode){
    // Lighting count (currently one supported).
    uchar data[LIGHTRGB_SIZE] = { LAYER_COUNT, 0 };
    if(!k95p_send_file(kb, "lightcnt.cnt", LIGHTCOUNT_SIZE, mode, data))
        return -1;
    // Zero out the buffer.
    data[0] = 0;
    char filename[16] = { 0 };
    for(int layer = 0; layer < LAYER_COUNT; layer++){
        // light_XX.d - ???
        // Will be added if needed.

        // light_XX.k - ???
        // Possibly a binary blob.
        // Will be added if needed.

        // light_XX.r - RGBA data.
        memset(data, 0, LIGHTRGB_SIZE);
        memset(filename, 0, 16);
        snprintf(filename, 16, "light_%02hhx.r", layer);
        for(int key = 0; key < N_KEYS_HW; key++){
            // The hardware profiles represent colours as RGBA,
            // but we represent them as RGB, so we just use a constant
            // alpha channel.
            data[4*key + 0] = light->r[key];
            data[4*key + 1] = light->g[key];
            data[4*key + 2] = light->b[key];
            data[4*key + 3] = 0xFF;
        }
       
        if(!k95p_send_file(kb, filename, LIGHTRGB_SIZE, mode, data))
            return -1;
    }
    return 1;
}

int cmd_hwload_k95p(usbdevice* kb, usbmode* dummy1, int dummy2, int apply, const char* dummy3){
    (void)dummy1;
    (void)dummy2;
    (void)dummy3;

    DELAY_LONG(kb);
    hwprofile* hw = calloc(1, sizeof(hwprofile));
    // Ask for profile metadata.
    uchar in_data[5*MSG_SIZE];
    int modes = (IS_K95(kb) ? HWMODE_K95 : HWMODE_K70); 
    for(int mode = 0; mode <= modes; mode++){
        // Profile metadata.
        if(!k95p_get_file(kb, "PROFILE.I", PROFILE_SIZE, mode, in_data))
            return -1;
        // This file always seems to start with 0x49 0x00. 
        // Maybe a magic number? A size?

        // Profile GUID.
        memcpy(hw->id + mode, in_data + 2, sizeof(usbid));
        // Profile name.
        memcpy(hw->name + mode, in_data + 22, PR_NAME_LEN * 2);
        // Load RGB.
        if(!loadrgb_k95p(kb, hw->light + mode, mode))
            return -1;
    }
    // Make the profile active (if requested).
    if(apply)
        hwtonative(kb->profile, hw, modes);
    // Free the existing profile (if any).
    free(kb->hw);
    kb->hw = hw;
    DELAY_LONG(kb);
    return 1;
},

int cmd_hwsave_k95p(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4){
    (void)dummy1;
    (void)dummy2;
    (void)dummy3;
    (void)dummy4;

    DELAY_LONG(kb);
    hwprofile* hw = kb->hw;
    if(!hw)
        hw = kb->hw = calloc(1, sizeof(hwprofile));
    int modes = (IS_K95(kb) ? HWMODE_K95 : HWMODE_K70);
    nativetohw(kb->profile, hw, modes);
    // Save the profile metadata.
    for(int mode = 0; mode <= modes; mode++){
        uchar data[5*MSG_SIZE] = { 0x49, 0x00, 0 };
        // Profile GUID.
        memcpy(data + 2, hw->id + mode, sizeof(usbid));
        // Profile name.
        memcpy(data + 22, hw->name + mode, MD_NAME_LEN * 2);
        // Send to the keyboard.
        if(!k95p_send_file(kb, "PROFILE.I", PROFILE_SIZE, mode, data))
            return -1;
        // Save RGB.
        if(!savergb_k95p(kb, hw->light + mode, mode))
            return -1;
    }
    DELAY_LONG(kb);
    return 0;
}

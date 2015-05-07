#include "led.h"
#include "notify.h"
#include "profile.h"
#include "usb.h"

static void makergb_512(const keylight* light, uchar data_pkt[5][MSG_SIZE]){
    uchar r[N_KEYS_KB / 2], g[N_KEYS_KB / 2], b[N_KEYS_KB / 2];
    // Compress RGB values to a 512-color palette
    for(int i = 0; i < N_KEYS_KB; i += 2){
        char r1 = light->r[i], r2 = light->r[i + 1];
        char g1 = light->g[i], g2 = light->g[i + 1];
        char b1 = light->b[i], b2 = light->b[i + 1];
        r[i / 2] = (7 - (r2 >> 5)) << 4 | (7 - (r1 >> 5));
        g[i / 2] = (7 - (g2 >> 5)) << 4 | (7 - (g1 >> 5));
        b[i / 2] = (7 - (b2 >> 5)) << 4 | (7 - (b1 >> 5));
    }
    memcpy(data_pkt[0] + 4, r, 60);
    memcpy(data_pkt[1] + 4, r + 60, 12);
    memcpy(data_pkt[1] + 16, g, 48);
    memcpy(data_pkt[2] + 4, g + 48, 24);
    memcpy(data_pkt[2] + 28, b, 36);
    memcpy(data_pkt[3] + 4, b + 36, 36);
}

static void makergb_full(const keylight* light, uchar data_pkt[12][MSG_SIZE]){
    const uchar* r = light->r, *g = light->g, *b = light->b;
    // Red
    memcpy(data_pkt[0] + 4, r, 60);
    memcpy(data_pkt[1] + 4, r + 60, 60);
    memcpy(data_pkt[2] + 4, r + 120, 24);
    // Green (final R packet is blank)
    memcpy(data_pkt[4] + 4, g, 60);
    memcpy(data_pkt[5] + 4, g + 60, 60);
    memcpy(data_pkt[6] + 4, g + 120, 24);
    // Blue (final G packet is blank)
    memcpy(data_pkt[8] + 4, b, 60);
    memcpy(data_pkt[9] + 4, b + 60, 60);
    memcpy(data_pkt[10] + 4, b + 120, 24);
}

static int rgbcmp(const keylight* lhs, const keylight* rhs){
    // Compare two light structures, ignore mouse zones
    return memcmp(lhs->r, rhs->r, N_KEYS_KB) + memcmp(lhs->g, rhs->g, N_KEYS_KB) + memcmp(lhs->b, rhs->b, N_KEYS_KB);
}

int updatergb_kb(usbdevice* kb, int force){
    if(!kb->active)
        return 0;
    keylight* lastlight = &kb->profile->lastlight;
    keylight* newlight = &kb->profile->currentmode->light;
    // Don't do anything if the lighting hasn't changed
    if(!force && !rgbcmp(lastlight, newlight))
        return 0;

    /*if(kb->fwversion >= 0x0120){
        uchar data_pkt[12][MSG_SIZE] = {
            // Red
            { 0x7f, 0x01, 60, 0 },
            { 0x7f, 0x02, 60, 0 },
            { 0x7f, 0x03, 24, 0 },
            { 0x07, 0x28, 0x01, 0x00, 0x01, 0x01},
            // Green
            { 0x7f, 0x01, 60, 0 },
            { 0x7f, 0x02, 60, 0 },
            { 0x7f, 0x03, 24, 0 },
            { 0x07, 0x28, 0x02, 0x00, 0x01, 0x01},
            // Blue
            { 0x7f, 0x01, 60, 0 },
            { 0x7f, 0x02, 60, 0 },
            { 0x7f, 0x03, 24, 0 },
            { 0x07, 0x28, 0x03, 0x00, 0x02, 0x01}
        };
        makergb_full(newlight, data_pkt);
        if(!usbsend(kb, data_pkt[0], 12))
            return -1;
    } else {*/
    // 16.8M color lighting causes flickering and color glitches. Don't use it for this.
    // Maybe in a future version this can be re-added as an advanced feature.
        uchar data_pkt[5][MSG_SIZE] = {
            { 0x7f, 0x01, 60, 0 },
            { 0x7f, 0x02, 60, 0 },
            { 0x7f, 0x03, 60, 0 },
            { 0x7f, 0x04, 36, 0 },
            { 0x07, 0x27, 0x00, 0x00, 0xD8 }
        };
        makergb_512(newlight, data_pkt);
        if(!usbsend(kb, data_pkt[0], 5))
            return -1;
    //}

    memcpy(lastlight, newlight, sizeof(keylight));
    return 0;
}

int savergb_kb(usbdevice* kb, int mode){
    if(kb->fwversion >= 0x0120){
        uchar data_pkt[12][MSG_SIZE] = {
            // Red
            { 0x7f, 0x01, 60, 0 },
            { 0x7f, 0x02, 60, 0 },
            { 0x7f, 0x03, 24, 0 },
            { 0x07, 0x14, 0x03, 0x01, 0x01, mode + 1, 0x01 },
            // Green
            { 0x7f, 0x01, 60, 0 },
            { 0x7f, 0x02, 60, 0 },
            { 0x7f, 0x03, 24, 0 },
            { 0x07, 0x14, 0x03, 0x01, 0x01, mode + 1, 0x02 },
            // Blue
            { 0x7f, 0x01, 60, 0 },
            { 0x7f, 0x02, 60, 0 },
            { 0x7f, 0x03, 24, 0 },
            { 0x07, 0x14, 0x03, 0x01, 0x01, mode + 1, 0x03 }
        };
        makergb_full(&kb->profile->mode[mode].light, data_pkt);
        if(!usbsend(kb, data_pkt[0], 12))
            return -1;
    } else {
        uchar data_pkt[5][MSG_SIZE] = {
            { 0x7f, 0x01, 60, 0 },
            { 0x7f, 0x02, 60, 0 },
            { 0x7f, 0x03, 60, 0 },
            { 0x7f, 0x04, 36, 0 },
            { 0x07, 0x14, 0x02, 0x00, 0x01, mode + 1 }
        };
        makergb_512(&kb->profile->mode[mode].light, data_pkt);
        if(!usbsend(kb, data_pkt[0], 5))
            return -1;
    }
    return 0;
}

int loadrgb_kb(usbdevice* kb, keylight* light, int mode){
    if(kb->fwversion >= 0x0120){
        uchar data_pkt[12][MSG_SIZE] = {
            { 0x0e, 0x14, 0x03, 0x01, 0x01, mode + 1, 0x01 },
            { 0xff, 0x01, 60, 0 },
            { 0xff, 0x02, 60, 0 },
            { 0xff, 0x03, 24, 0 },
            { 0x0e, 0x14, 0x03, 0x01, 0x01, mode + 1, 0x02 },
            { 0xff, 0x01, 60, 0 },
            { 0xff, 0x02, 60, 0 },
            { 0xff, 0x03, 24, 0 },
            { 0x0e, 0x14, 0x03, 0x01, 0x01, mode + 1, 0x03 },
            { 0xff, 0x01, 60, 0 },
            { 0xff, 0x02, 60, 0 },
            { 0xff, 0x03, 24, 0 },
        };
        uchar in_pkt[4][MSG_SIZE] = {
            { 0x0e, 0x14, 0x03, 0x01 },
            { 0xff, 0x01, 60, 0 },
            { 0xff, 0x02, 60, 0 },
            { 0xff, 0x03, 24, 0 },
        };
        // Read colors
        uchar* colors[3] = { light->r, light->g, light->b };
        for(int clr = 0; clr < 3; clr++){
            for(int i = 0; i < 4; i++){
                if(!usbsend(kb, data_pkt[i + clr * 4], 1))
                    return -1;
                // Wait for the response. Make sure the first four bytes match
                if(!usbrecv(kb, in_pkt[i]))
                    return -1;
                if(memcmp(in_pkt[i], data_pkt[i], 4)){
                    ckb_err("Bad input header\n");
                    return -1;
                }
            }
            // Copy colors to lighting. in_pkt[0] is irrelevant.
            memcpy(colors[clr], in_pkt[1] + 4, 60);
            memcpy(colors[clr] + 60, in_pkt[2] + 4, 60);
            memcpy(colors[clr] + 120, in_pkt[3] + 4, 24);
        }
    } else {
        uchar data_pkt[5][MSG_SIZE] = {
            { 0x0e, 0x14, 0x02, 0x01, 0x01, mode + 1, 0 },
            { 0xff, 0x01, 60, 0 },
            { 0xff, 0x02, 60, 0 },
            { 0xff, 0x03, 60, 0 },
            { 0xff, 0x04, 36, 0 },
        };
        uchar in_pkt[4][MSG_SIZE] = {
            { 0xff, 0x01, 60, 0 },
            { 0xff, 0x02, 60, 0 },
            { 0xff, 0x03, 60, 0 },
            { 0xff, 0x04, 36, 0 },
        };
        // Write initial packet
        if(!usbsend(kb, data_pkt[0], 1))
            return -1;
        // Read colors
        for(int i = 1; i < 5; i++){
            if(!usbsend(kb, data_pkt[i], 1))
                return -1;
            // Wait for the response. Make sure the first four bytes match
            if(!usbrecv(kb, in_pkt[i - 1]))
                return -1;
            if(memcmp(in_pkt[i - 1], data_pkt[i], 4)){
                ckb_err("Bad input header\n");
                return -1;
            }
        }
        // Copy the data back to the mode
        uchar mr[N_KEYS_KB / 2], mg[N_KEYS_KB / 2], mb[N_KEYS_KB / 2];
        memcpy(mr, in_pkt[0] + 4, 60);
        memcpy(mr + 60, in_pkt[1] + 4, 12);
        memcpy(mg, in_pkt[1] + 16, 48);
        memcpy(mg + 48, in_pkt[2] + 4, 24);
        memcpy(mb, in_pkt[2] + 28, 36);
        memcpy(mb + 36, in_pkt[3] + 4, 36);
        // Unpack LED data to 8bpc format
        for(int i = 0; i < N_KEYS_KB; i++){
            uchar r, g, b;
            if(i & 1){
                r = (7 - ((mr[i / 2] & 0xF0) >> 4)) << 5;
                g = (7 - ((mg[i / 2] & 0xF0) >> 4)) << 5;
                b = (7 - ((mb[i / 2] & 0xF0) >> 4)) << 5;
            } else {
                r = (7 - (mr[i / 2] & 0x0F)) << 5;
                g = (7 - (mg[i / 2] & 0x0F)) << 5;
                b = (7 - (mb[i / 2] & 0x0F)) << 5;
            }
            // Convert 0xe0 to 0xff (white color)
            if(r == 0xe0) r = 0xff;
            if(g == 0xe0) g = 0xff;
            if(b == 0xe0) b = 0xff;
            light->r[i] = r;
            light->g[i] = g;
            light->b[i] = b;
        }
    }
    return 0;
}

// Does a key exist in the current LED layout?
int has_key(const char* name, const usbdevice* kb){
    if(!name
            // Only K95 has G keys and M keys (G1 - G18, MR, M1 - M3)
            || (!IS_K95(kb) && ((name[0] == 'g' && name[1] >= '1' && name[1] <= '9') || (name[0] == 'm' && (name[1] == 'r' || name[1] == '1' || name[1] == '2' || name[1] == '3'))))
            // Only K65 has lights on VolUp/VolDn
            || (!IS_K65(kb) && (!strcmp(name, "volup") || !strcmp(name, "voldn")))
            // K65 lacks numpad and media buttons
            || (IS_K65(kb) && (strstr(name, "num") == name || !strcmp(name, "stop") || !strcmp(name, "prev") || !strcmp(name, "play") || !strcmp(name, "next"))))
        return 0;
    return 1;
}

char* printrgb(const keylight* light, const usbdevice* kb){
    int length = 0;
    uchar r[N_KEYS_KB], g[N_KEYS_KB], b[N_KEYS_KB];
    const uchar* mr = light->r;
    const uchar* mg = light->g;
    const uchar* mb = light->b;
    for(int i = 0; i < N_KEYS_KB; i++){
        // Translate the key index to an RGB index using the key map
        int k = keymap[i].led;
        if(k < 0)
            continue;
        r[i] = mr[k];
        g[i] = mg[k];
        b[i] = mb[k];
    }
    // Make a buffer to track key names and to filter out duplicates
    char names[N_KEYS_KB][11];
    for(int i = 0; i < N_KEYS_KB; i++){
        const char* name = keymap[i].name;
        if(keymap[i].led < 0 || !has_key(name, kb))
            names[i][0] = 0;
        else
            strncpy(names[i], name, 11);
    }
    // Check to make sure these aren't all the same color
    int same = 1;
    for(int i = 1; i < N_KEYS_KB; i++){
        if(!names[i][0])
            continue;
        if(r[i] != r[0] || g[i] != g[0] || b[i] != b[0]){
            same = 0;
            break;
        }
    }
    // If they are, just output that color
    if(same){
        char* buffer = malloc(7);
        snprintf(buffer, 7, "%02x%02x%02x", r[0], g[0], b[0]);
        return buffer;
    }
    const int BUFFER_LEN = 4096;
    char* buffer = malloc(BUFFER_LEN);
    for(int i = 0; i < N_KEYS_KB; i++){
        if(!names[i][0])
            continue;
        // Print the key name
        int newlen = 0;
        snprintf(buffer + length, BUFFER_LEN - length, length == 0 ? "%s%n" : " %s%n", names[i], &newlen);
        length += newlen;
        // Look ahead to see if any other keys have this color. If so, print them here as well.
        uchar kr = r[i], kg = g[i], kb = b[i];
        for(int j = i + 1; j < N_KEYS_KB; j++){
            if(!names[j][0])
                continue;
            if(r[j] != kr || g[j] != kg || b[j] != kb)
                continue;
            snprintf(buffer + length, BUFFER_LEN - length, ",%s%n", names[j], &newlen);
            length += newlen;
            // Erase the key's name so it won't get printed later
            names[j][0] = 0;
        }
        // Print the color
        snprintf(buffer + length, BUFFER_LEN - length, ":%02x%02x%02x%n", kr, kg, kb, &newlen);
        length += newlen;
    }

    return buffer;
}

#include "led.h"
#include "device.h"
#include "notify.h"

void initrgb(keylight* light){
    // Allocate colors. Default to all white.
    light->enabled = 1;
    memset(light->r, 0, sizeof(light->r));
    memset(light->g, 0, sizeof(light->g));
    memset(light->b, 0, sizeof(light->b));
}

void makergb(const keylight* light, uchar data_pkt[5][MSG_SIZE], int forceon){
    if(forceon || light->enabled){
        const char* r = light->r, *g = light->g, *b = light->b;
        memcpy(data_pkt[0] + 4, r, 60);
        memcpy(data_pkt[1] + 4, r + 60, 12);
        memcpy(data_pkt[1] + 16, g, 48);
        memcpy(data_pkt[2] + 4, g + 48, 24);
        memcpy(data_pkt[2] + 28, b, 36);
        memcpy(data_pkt[3] + 4, b + 36, 36);
    } else {
        memset(data_pkt[0] + 4, 0x77, 60);
        memset(data_pkt[1] + 4, 0x77, 60);
        memset(data_pkt[2] + 4, 0x77, 60);
        memset(data_pkt[3] + 4, 0x77, 36);
    }
}

void updatergb(usbdevice* kb, int force){
    if(!IS_ACTIVE(kb) || !HAS_FEATURES(kb, FEAT_RGB))
        return;
    // Don't do anything if the lighting hasn't changed
    keylight* lastlight = &kb->lastlight;
    keylight* newlight = &kb->profile.currentmode->light;
    if(!force && ((!lastlight->enabled && !newlight->enabled) || !memcmp(lastlight, newlight, sizeof(keylight))))
        return;
    memcpy(lastlight, newlight, sizeof(keylight));

    uchar data_pkt[5][MSG_SIZE] = {
        { 0x7f, 0x01, 60, 0 },
        { 0x7f, 0x02, 60, 0 },
        { 0x7f, 0x03, 60, 0 },
        { 0x7f, 0x04, 36, 0 },
        { 0x07, 0x27, 0x00, 0x00, 0xD8 }
    };

    makergb(newlight, data_pkt, 0);
    usbqueue(kb, data_pkt[0], 5);
}

void savergb(usbdevice* kb, int mode){
    uchar data_pkt[5][MSG_SIZE] = {
        { 0x7f, 0x01, 60, 0 },
        { 0x7f, 0x02, 60, 0 },
        { 0x7f, 0x03, 60, 0 },
        { 0x7f, 0x04, 36, 0 },
        { 0x07, 0x14, 0x02, 0x00, 0x01, mode + 1 }
    };

    makergb(&kb->profile.mode[mode].light, data_pkt, 1);
    usbqueue(kb, data_pkt[0], 5);
}

int loadrgb(usbdevice* kb, keylight* light, int mode){
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
    usbqueue(kb, data_pkt[0], 1);
    DELAY_SHORT;
    if(!usbdequeue(kb))
        return -1;
    for(int i = 1; i < 5; i++){
        usbqueue(kb, data_pkt[i], 1);
        DELAY_SHORT;
        if(!usbdequeue(kb))
            return -1;
        // Wait for the response. Make sure the first four bytes match
        DELAY_SHORT;
        if(!usbinput(kb, in_pkt[i - 1]) || memcmp(in_pkt[i - 1], data_pkt[i], 4))
            return -1;
    }
    // Copy the data back to the mode
    char* r = light->r, *g = light->g, *b = light->b;
    memcpy(r, in_pkt[0] + 4, 60);
    memcpy(r + 60, in_pkt[1] + 4, 12);
    memcpy(g, in_pkt[1] + 16, 48);
    memcpy(g + 48, in_pkt[2] + 4, 24);
    memcpy(b, in_pkt[2] + 28, 36);
    memcpy(b + 36, in_pkt[3] + 4, 36);
    light->enabled = 1;
    return 0;
}

char* printrgb(keylight* light, const key* keymap){
    int length = 0;
    // Unpack LED data back to 8bpc format.
    uchar r[N_KEYS], g[N_KEYS], b[N_KEYS];
    char* mr = light->r;
    char* mg = light->g;
    char* mb = light->b;
    for(int i = 0; i < N_KEYS; i++){
        // Translate the RGB index to a key index using the key map
        int k = keymap[i].led;
        if(k < 0)
            continue;
        if(k & 1){
            r[i] = (7 - ((mr[k / 2] & 0xF0) >> 4)) << 5;
            g[i] = (7 - ((mg[k / 2] & 0xF0) >> 4)) << 5;
            b[i] = (7 - ((mb[k / 2] & 0xF0) >> 4)) << 5;
        } else {
            r[i] = (7 - (mr[k / 2] & 0x0F)) << 5;
            g[i] = (7 - (mg[k / 2] & 0x0F)) << 5;
            b[i] = (7 - (mb[k / 2] & 0x0F)) << 5;
        }

        // Convert 0xe0 to 0xff (white color)
        if(r[i] == 0xe0)
            r[i] = 0xff;
        if(g[i] == 0xe0)
            g[i] = 0xff;
        if(b[i] == 0xe0)
            b[i] = 0xff;
    }
    // Make a buffer to track key names and to filter out duplicates
    char names[N_KEYS][11];
    for(int i = 0; i < N_KEYS; i++){
        const char* name = keymap[i].name;
        if(!name || keymap[i].led < 0)
            names[i][0] = 0;
        else
            strncpy(names[i], name, 11);
    }
    // Check to make sure these aren't all the same color
    int same = 1;
    for(int i = 1; i < N_KEYS; i++){
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
    for(int i = 0; i < N_KEYS; i++){
        if(!names[i][0])
            continue;
        // Print the key name
        int newlen = 0;
        snprintf(buffer + length, BUFFER_LEN - length, length == 0 ? "%s%n" : " %s%n", names[i], &newlen);
        length += newlen;
        // Look ahead to see if any other keys have this color. If so, print them here as well.
        uchar kr = r[i], kg = g[i], kb = b[i];
        for(int j = i + 1; j < N_KEYS; j++){
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

void cmd_rgboff(usbmode* mode){
    mode->light.enabled = 0;
}

void cmd_rgbon(usbmode* mode){
    mode->light.enabled = 1;
}

void cmd_rgb(usbmode* mode, const key* keymap, int dummy, int keyindex, const char* code){
    int index = keymap[keyindex].led;
    if(index < 0)
        return;
    unsigned int r, g, b;
    if(sscanf(code, "%2x%2x%2x", &r, &g, &b) == 3){
        if(r > 255)
            r = 255;
        if(g > 255)
            g = 255;
        if(b > 255)
            b = 255;
        char* mr = mode->light.r;
        char* mg = mode->light.g;
        char* mb = mode->light.b;
        if(index & 1){
            mr[index / 2] = (mr[index / 2] & 0x0F) | ((7 - (r >> 5)) << 4);
            mg[index / 2] = (mg[index / 2] & 0x0F) | ((7 - (g >> 5)) << 4);
            mb[index / 2] = (mb[index / 2] & 0x0F) | ((7 - (b >> 5)) << 4);
        } else {
            mr[index / 2] = (mr[index / 2] & 0xF0) | (7 - (r >> 5));
            mg[index / 2] = (mg[index / 2] & 0xF0) | (7 - (g >> 5));
            mb[index / 2] = (mb[index / 2] & 0xF0) | (7 - (b >> 5));
        }
    }
}

static int iselect(const char* led){
    if(!strcmp(led, "num"))
        return 1;
    if(!strcmp(led, "caps"))
        return 2;
    if(!strcmp(led, "scroll"))
        return 3;
    return 0;
}

void cmd_ioff(usbmode* mode, const key* keymap, int dummy1, int dummy2, const char* led){
    int bit = iselect(led);
    if(!bit)
        return;
    // Add the bit to ioff, remove it from ion
    bit = 1 << (bit - 1);
    mode->ioff |= bit;
    mode->ion &= ~bit;
}

void cmd_ion(usbmode* mode, const key* keymap, int dummy1, int dummy2, const char* led){
    int bit = iselect(led);
    if(!bit)
        return;
    // Remove the bit from ioff, add it to ion
    bit = 1 << (bit - 1);
    mode->ioff &= ~bit;
    mode->ion |= bit;
}

void cmd_iauto(usbmode* mode, const key* keymap, int dummy1, int dummy2, const char* led){
    int bit = iselect(led);
    if(!bit)
        return;
    // Remove the bit from both ioff and ion
    bit = 1 << (bit - 1);
    mode->ioff &= ~bit;
    mode->ion &= ~bit;
}

volatile unsigned fps = 0;

void setfps(unsigned newfps){
    if(newfps > 60 || newfps == 0){
        // There's no point running higher than 60FPS.
        // The LED controller is locked to 60Hz so it will only cause tearing and/or device freezes.
        printf("Warning: Refusing request for %d FPS\n", newfps);
        return;
    }
    if(newfps != fps){
        printf("Setting FPS to %u\n", newfps);
        fps = newfps;
    }
}

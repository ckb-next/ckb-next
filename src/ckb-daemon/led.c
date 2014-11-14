#include "led.h"
#include "device.h"

void initrgb(keylight* light){
    // Allocate colors. Default to all white.
    light->enabled = 1;
    memset(light->r, 0, sizeof(light->r));
    memset(light->g, 0, sizeof(light->g));
    memset(light->b, 0, sizeof(light->b));
}

void makergb(const keylight* light, uchar data_pkt[5][MSG_SIZE]){
    if(light->enabled){
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

void updatergb(usbdevice* kb){
    if(!IS_ACTIVE(kb))
        return;

    uchar data_pkt[5][MSG_SIZE] = {
        { 0x7f, 0x01, 0x3c, 0 },
        { 0x7f, 0x02, 0x3c, 0 },
        { 0x7f, 0x03, 0x3c, 0 },
        { 0x7f, 0x04, 0x24, 0 },
        { 0x07, 0x27, 0x00, 0x00, 0xD8 }
    };

    makergb(&kb->profile.currentmode->light, data_pkt);
    usbqueue(kb, data_pkt[0], 5);
}

void savergb(usbdevice* kb, int mode){
    uchar data_pkt[5][MSG_SIZE] = {
        { 0x7f, 0x01, 0x3c, 0 },
        { 0x7f, 0x02, 0x3c, 0 },
        { 0x7f, 0x03, 0x3c, 0 },
        { 0x7f, 0x04, 0x24, 0 },
        { 0x07, 0x14, 0x02, 0x00, 0x01, mode + 1 }
    };

    makergb(&kb->profile.mode[mode].light, data_pkt);
    usbqueue(kb, data_pkt[0], 5);
}

int loadrgb(usbdevice* kb, keylight* light, int mode){
    uchar data_pkt[5][MSG_SIZE] = {
        { 0x0e, 0x14, 0x02, 0x01, 0x01, mode + 1, 0 },
        { 0xff, 0x01, 0x3c, 0 },
        { 0xff, 0x02, 0x3c, 0 },
        { 0xff, 0x03, 0x3c, 0 },
        { 0xff, 0x04, 0x24, 0 },
    };
    usbqueue(kb, data_pkt[0], 1);
    usleep(3000);
    if(!usbdequeue(kb))
        return -1;
    for(int i = 1; i < 5; i++){
        usbqueue(kb, data_pkt[i], 1);
        usleep(3000);
        if(!usbdequeue(kb))
            return -1;
        // Wait for the response
        if(!usbinput(kb, data_pkt[i]))
            return -1;
    }
    // Copy the data back to the mode
    char* r = light->r, *g = light->g, *b = light->b;
    memcpy(r, data_pkt[1] + 4, 60);
    memcpy(r + 60, data_pkt[2] + 4, 12);
    memcpy(g, data_pkt[2] + 16, 48);
    memcpy(g + 48, data_pkt[3] + 4, 24);
    memcpy(b, data_pkt[3] + 28, 36);
    memcpy(b + 36, data_pkt[4] + 4, 36);
    light->enabled = 1;
    return 0;
}

#define MAX_WORDS 3

void cmd_rgboff(usbmode* mode){
    mode->light.enabled = 0;
}

void cmd_rgbon(usbmode* mode){
    mode->light.enabled = 1;
}

void cmd_rgb(usbmode* mode, const key* keymap, int keyindex, const char* code){
    unsigned int r, g, b;
    if(sscanf(code, "%2x%2x%2x", &r, &g, &b) == 3){
        if(r > 255)
            r = 255;
        if(g > 255)
            g = 255;
        if(b > 255)
            b = 255;
        int index = keymap[keyindex].led;
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

void cmd_ioff(usbmode* mode, const key* keymap, int zero, const char* led){
    int bit = iselect(led);
    if(!bit)
        return;
    // Add the bit to ioff, remove it from ion
    bit = 1 << (bit - 1);
    mode->ioff |= bit;
    mode->ion &= ~bit;
}

void cmd_ion(usbmode* mode, const key* keymap, int zero, const char* led){
    int bit = iselect(led);
    if(!bit)
        return;
    // Remove the bit from ioff, add it to ion
    bit = 1 << (bit - 1);
    mode->ioff &= ~bit;
    mode->ion |= bit;
}

void cmd_iauto(usbmode* mode, const key* keymap, int zero, const char* led){
    int bit = iselect(led);
    if(!bit)
        return;
    // Remove the bit from both ioff and ion
    bit = 1 << (bit - 1);
    mode->ioff &= ~bit;
    mode->ion &= ~bit;
}

volatile unsigned fps = 60;

void setfps(unsigned newfps){
    if(newfps > 60 || newfps == 0){
        // There's no point running higher than 60FPS.
        // The LED controller is locked to 60Hz so it will only cause tearing and/or device freezes.
        printf("Warning: Requested %d FPS but setting to 60\n", newfps);
        fps = 60;
    } else {
        fps = newfps;
        printf("Setting FPS to %u\n", fps);
    }
}

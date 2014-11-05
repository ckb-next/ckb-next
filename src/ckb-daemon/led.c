#include "led.h"

void initrgb(keylight* light){
    // Allocate colors. Default to all white.
    light->enabled = 1;
    memset(light->r, 0, sizeof(light->r));
    memset(light->g, 0, sizeof(light->g));
    memset(light->b, 0, sizeof(light->b));
}

void makergb(const keylight* light, char data_pkt[5][MSG_SIZE]){
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

void updateleds(usbdevice* kb){
    if(!kb)
        return;

    char data_pkt[5][MSG_SIZE] = {
        { 0x7f, 0x01, 0x3c, 0 },
        { 0x7f, 0x02, 0x3c, 0 },
        { 0x7f, 0x03, 0x3c, 0 },
        { 0x7f, 0x04, 0x24, 0 },
        { 0x07, 0x27, 0x00, 0x00, 0xD8 }
    };

    makergb(&kb->setting.profile.currentmode->light, data_pkt);
    usbqueue(kb, data_pkt[0], 5);
}

void saveleds(usbdevice* kb, int mode){
    char data_pkt[5][MSG_SIZE] = {
        { 0x7f, 0x01, 0x3c, 0 },
        { 0x7f, 0x02, 0x3c, 0 },
        { 0x7f, 0x03, 0x3c, 0 },
        { 0x7f, 0x04, 0x24, 0 },
        { 0x07, 0x14, 0x02, 0x00, 0x01, mode + 1 }
    };

    makergb(&kb->setting.profile.mode[mode].light, data_pkt);
    usbqueue(kb, data_pkt[0], 5);
}

void loadleds(usbdevice* kb, int mode){
    char data_pkt[5][MSG_SIZE] = {
        { 0x0e, 0x14, 0x02, 0x01, 0x01, mode + 1, 0 },
        { 0xff, 0x01, 0x3c, 0 },
        { 0xff, 0x02, 0x3c, 0 },
        { 0xff, 0x03, 0x3c, 0 },
        { 0xff, 0x04, 0x24, 0 },
    };
    usbqueue(kb, data_pkt[0], 1);
    usleep(3333);
    usbdequeue(kb);
    for(int i = 1; i < 5; i++){
        usbqueue(kb, data_pkt[i], 1);
        usleep(3333);
        usbdequeue(kb);
        // Wait for the response
        libusb_control_transfer(kb->handle, 0xa1, 1, 0x0300, 0x03, data_pkt[i], MSG_SIZE, 500);
    }
    // Copy the data back to the mode
    keylight* light = &kb->setting.profile.mode[mode].light;
    char* r = light->r, *g = light->g, *b = light->b;
    memcpy(r, data_pkt[1] + 4, 60);
    memcpy(r + 60, data_pkt[2] + 4, 12);
    memcpy(g, data_pkt[2] + 16, 48);
    memcpy(g + 48, data_pkt[3] + 4, 24);
    memcpy(b, data_pkt[3] + 28, 36);
    memcpy(b + 36, data_pkt[4] + 4, 36);
}

#define MAX_WORDS 3

void cmd_ledoff(usbmode* mode){
    mode->light.enabled = 0;
}

void cmd_ledon(usbmode* mode){
    mode->light.enabled = 1;
}

void cmd_ledrgb(usbmode* mode, int keyindex, const char* code){
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

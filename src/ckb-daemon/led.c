#include "led.h"

void initrgb(keylight* light){
    // Allocate colors. Default to all white.
    light->enabled = 1;
    memset(light->rgb, 0xFF, sizeof(light->rgb));
}

#define red(i) (7 - (rgb[i] >> 6 & 0x7))
#define green(i) (7 - (rgb[i] >> 3 & 0x7))
#define blue(i) (7 - (rgb[i] & 0x7))

void updateleds(usbdevice* kb){
    if(!kb)
        return;
    usbmode* mode = kb->setting.profile.currentmode;

    char data_pkt[5][MSG_SIZE] = { { 0 } };
    data_pkt[0][0] = 0x7F;
    data_pkt[0][1] = 0x01;
    data_pkt[0][2] = 0x3C;

    data_pkt[1][0] = 0x7F;
    data_pkt[1][1] = 0x02;
    data_pkt[1][2] = 0x3C;

    data_pkt[2][0] = 0x7F;
    data_pkt[2][1] = 0x03;
    data_pkt[2][2] = 0x3C;

    data_pkt[3][0] = 0x7F;
    data_pkt[3][1] = 0x04;
    data_pkt[3][2] = 0x24;

    data_pkt[4][0] = 0x07;
    data_pkt[4][1] = 0x27;
    data_pkt[4][4] = 0xD8;

    if(mode->light.enabled){
        const short* rgb = mode->light.rgb;
        for(int i = 0; i < 60; i++)
            data_pkt[0][i+4] = red(i*2+1) << 4 | red(i*2);
        for(int i = 0; i < 12; i++)
            data_pkt[1][i+4] = red(i*2+121) << 4 | red(i*2+120);
        for(int i = 0; i < 48; i++)
            data_pkt[1][i+16] = green(i*2+1) << 4 | green(i*2);
        for(int i = 0; i < 24; i++)
            data_pkt[2][i+4] = green(i*2+97) << 4 | green(i*2+96);
        for(int i = 0; i < 36; i++)
            data_pkt[2][i+28] = blue(i*2+1) << 4 | blue(i*2);
        for(int i = 0; i < 36; i++)
            data_pkt[3][i+4] = blue(i*2+73) << 4 | blue(i*2+72);
    } else {
        for(int i = 0; i < 60; i++)
            data_pkt[0][i+4] = 0x77;
        for(int i = 0; i < 12; i++)
            data_pkt[1][i+4] = 0x77;
        for(int i = 0; i < 48; i++)
            data_pkt[1][i+16] = 0x77;
        for(int i = 0; i < 24; i++)
            data_pkt[2][i+4] = 0x77;
        for(int i = 0; i < 36; i++)
            data_pkt[2][i+28] = 0x77;
        for(int i = 0; i < 36; i++)
            data_pkt[3][i+4] = 0x77;
    }

    usbqueue(kb, data_pkt[0], 5);
}

#define MAX_WORDS 3
#define MAKERGB(r, g, b) (((((r) > 255 ? 255 : (r)) >> 5) << 6) | ((((g) > 255 ? 255 : (g)) >> 5) << 3) | (((b) > 255 ? 255 : (b)) >> 5))

void cmd_ledoff(usbmode* mode){
    mode->light.enabled = 0;
}

void cmd_ledon(usbmode* mode){
    mode->light.enabled = 1;
}

void cmd_ledrgb(usbmode* mode, int keyindex, const char* code){
    unsigned int r, g, b;
    if(sscanf(code, "%2x%2x%2x", &r, &g, &b) == 3)
        mode->light.rgb[keymap[keyindex].led] = MAKERGB(r, g, b);
}

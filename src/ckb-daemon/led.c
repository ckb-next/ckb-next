#include "led.h"

void initrgb(short** rgb, int* rgbon){
    if(!*rgb){
        // Allocate colors if they're not set already. Default to all white.
        *rgb = malloc(RGB_SIZE);
        *rgbon = 1;
        memset(*rgb, 0xFF, RGB_SIZE);
    }
}

#define red(i) (7 - (rgb[i] >> 6 & 0x7))
#define green(i) (7 - (rgb[i] >> 3 & 0x7))
#define blue(i) (7 - (rgb[i] & 0x7))

void updateleds(usbdevice* kb, short* rgb){
    if(!kb)
        return;
    int freemem = 0;
    if(!rgb){
        rgb = malloc(RGB_SIZE);
        memset(rgb, 0, RGB_SIZE);
        freemem = 1;
    }
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

    usbqueue(kb, data_pkt[0], 5);

    if(freemem)
        free(rgb);
}

#define MAX_WORDS 3
#define MAKERGB(r, g, b) (((((r) > 255 ? 255 : (r)) >> 5) << 6) | ((((g) > 255 ? 255 : (g)) >> 5) << 3) | (((b) > 255 ? 255 : (b)) >> 5))

void readled(usbdevice* kb, char* line){
    short* colors = kb->rgb;
    int* enable = &kb->rgbon;

    // Split the message up into words
    char* word[MAX_WORDS] = { line };
    int wpos = 0;
    for(int i = 1; i < MAX_WORDS; i++){
        for(; line[wpos] != '\0'; wpos++){
            if(line[wpos] == ' '){
                line[wpos] = '\0';
                wpos++;
                break;
            }
        }
        word[i] = line + wpos;
    }
    // Remove the newline at the end (or cut the string after the third word)
    line[wpos - 1] = '\0';
    // See if there's a serial number here. If so, switch keyboards.
    if(strlen(word[0]) == SERIAL_LEN - 1){
        int found = 0;
        for(int i = 1; i < DEV_MAX; i++){
            if(keyboard[i].ledfifo && !strcmp(word[0], keyboard[i].serial)){
                kb = keyboard + i;
                found = 1;
                initrgb(&kb->rgb, &kb->rgbon);
                colors = kb->rgb;
                enable = &kb->rgbon;
                break;
            }
        }
        // If it wasn't found in the device list, put it in the store.
        if(!found){
            usbstore* stored = addstore(word[0]);
            initrgb(&stored->rgb, &stored->rgbon);
            colors = stored->rgb;
            enable = &stored->rgbon;
            kb = 0;
        }
        // Parse the rest of the message
        for(int i = 0; i < MAX_WORDS - 1; i++)
            word[i] = word[i + 1];
    } else if(kb->model == -1)
        // Root controller *must* be given a serial number
        return;
    // Parse commands
    if(!strcmp(word[0], "off") && word[1][0] == '\0'){
        // Turn all keys off. Set LEDs to black.
        *enable = 0;
        updateleds(kb, 0);
    } else if(!strcmp(word[0], "on") && word[1][0] == '\0'){
        // Turn all keys on. Restore previous color settings.
        *enable = 1;
        updateleds(kb, colors);
    } else if(!strcmp(word[0], "rgb")){
        // Set the RGB color of key(s).
        *enable = 1;
        unsigned int r, g, b;
        if(sscanf(word[1], "%2x%2x%2x", &r, &g, &b) == 3){
            // Single color for all keys
            short rgb = MAKERGB(r, g, b);
            for(int i = 0; i < N_KEYS; i++)
                colors[i] = rgb;
        } else {
            // Scan a list of key/color pairs
            int length = strlen(word[1]);
            int position = 0, fieldleft = 0, field = 0;
            char* keynames = malloc(length);
            while(position < length && sscanf(word[1] + position, "%[^:]%n:%2x%2x%2x%n", keynames, &fieldleft, &r, &g, &b, &field) == 4){
                short rgb = MAKERGB(r, g, b);
                // Scan keys
                fieldleft += position;
                int position2 = position, field2 = 0;
                char keyname[11];
                while(position2 < fieldleft && sscanf(word[1] + position2, "%10[^:,]%n", keyname, &field2) == 1){
                    int keycode;
                    if(!strcmp(keyname, "all")){
                        // Set all keys
                        for(int i = 0; i < N_KEYS; i++)
                            colors[i] = rgb;
                    } else if(sscanf(keyname, "#%d", &keycode) && keycode >= 0 && keycode < N_KEYS){
                        // Set a key numerically
                        colors[keycode] = rgb;
                    } else {
                        // Find this key in the keymap
                        for(unsigned i = 0; i < N_KEYMAP; i++){
                            if(!strcmp(keyname, keymap[i].name)){
                                colors[keymap[i].code] = rgb;
                                break;
                            }
                        }
                    }
                    if(word[1][position2 += field2] == ',')
                        position2++;
                }
                if(word[1][position += field] == ',')
                    position++;
            }
            free(keynames);
        }
        updateleds(kb, colors);
    }
}

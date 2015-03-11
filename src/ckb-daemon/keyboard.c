#include "includes.h"
#include "keyboard.h"

const key* keymap_system = 0;

const key* getkeymap(const char* name){
    if(!strcmp(name, "de"))
        return keymap_de;
    if(!strcmp(name, "fr"))
        return keymap_fr;
    if(!strcmp(name, "gb"))
        return keymap_gb;
    if(!strcmp(name, "se"))
        return keymap_se;
    if(!strcmp(name, "us"))
        return keymap_us;
    return 0;
}

const char* getmapname(const key* layout){
    if(layout == keymap_de)
        return "de";
    if(layout == keymap_fr)
        return "fr";
    if(layout == keymap_gb)
        return "gb";
    if(layout == keymap_se)
        return "se";
    if(layout == keymap_us)
        return "us";
    return "";
}

void hid_translate(unsigned char* kbinput, int endpoint, int length, const unsigned char* urbinput){
    // LUT for HID -> Corsair scancodes (-1 for no scan code, -2 for currently unsupported)
    // Modified from Linux drivers/hid/usbhid/usbkbd.c, key codes replaced with array indices and K95 keys added
    static const short hid_codes[256] = {
        -1,  -1,  -1,  -1,  37,  54,  52,  39,  27,  40,  41,  42,  32,  43,  44,  45,
        56,  55,  33,  34,  25,  28,  38,  29,  31,  53,  26,  51,  30,  50,  13,  14,
        15,  16,  17,  18,  19,  20,  21,  22,  82,   0,  86,  24,  64,  23,  84,  35,
        79,  80,  81,  46,  47,  12,  57,  58,  59,  36,   1,   2,   3,   4,   5,   6,
         7,   8,   9,  10,  11,  72,  73,  74,  75,  76,  77,  78,  87,  88,  89,  95,
        93,  94,  92, 102, 103, 104, 105, 106, 107, 115, 116, 117, 112, 113, 114, 108,
       109, 110, 118, 119,  49,  69,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
        -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  98,  -2,  -2,  -2,  -2,  -2,  -2,  97,
       130, 131,  -1,  -1,  -1,  -2,  -1,  -2,  -2,  -2,  -2,  -2,  -2,  -1,  -1,  -1,
        -2,  -2,  -2,  -2,  -2,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -3,  -1,  -1,  -1,  // <- -3 = non-RGB program key
       120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 136, 137, 138, 139, 140, 141,
        60,  48,  62,  61,  91,  90,  67,  68, 142, 143,  99, 101,  -2, 130, 131,  97,
        -2, 133, 134, 135,  -2,  96,  -2, 132,  -2,  -2,  71,  71,  71,  71,  -1,  -1,
    };
    if(endpoint == 1 || endpoint == -1){
        // EP 1: 6KRO input
        // Clear previous input
        for(int i = 0; i < 256; i++){
            if(hid_codes[i] >= 0)
                CLEAR_KEYBIT(kbinput, hid_codes[i]);
        }
        // Set new input
        for(int i = 0; i < 8; i++){
            if((urbinput[0] >> i) & 1)
                SET_KEYBIT(kbinput, hid_codes[i + 224]);
        }
        for(int i = 2; i < length; i++){
            if(urbinput[i] > 3){
                int scan = hid_codes[urbinput[i]];
                if(scan >= 0)
                    SET_KEYBIT(kbinput, scan);
                else
                    printf("Got unknown key press %d on EP 1\n", urbinput[i]);
            }
        }
    } else if(endpoint == -2){
        // EP 2 RGB: NKRO input
        if(length != 21 || urbinput[0] != 1)
            return;
        for(int bit = 0; bit < 8; bit++){
            if((urbinput[1] >> bit) & 1)
                SET_KEYBIT(kbinput, hid_codes[bit + 224]);
            else
                CLEAR_KEYBIT(kbinput, hid_codes[bit + 224]);
        }
        for(int byte = 0; byte < 19; byte++){
            char input = urbinput[byte + 2];
            for(int bit = 0; bit < 8; bit++){
                int keybit = byte * 8 + bit;
                int scan = hid_codes[keybit];
                if((input >> bit) & 1){
                    if(scan >= 0)
                        SET_KEYBIT(kbinput, hid_codes[keybit]);
                    else
                        printf("Got unknown key press %d on EP 2\n", keybit);
                } else if(scan >= 0)
                    CLEAR_KEYBIT(kbinput, hid_codes[keybit]);
            }
        }
    } else if(endpoint == 3){
        // EP 3 non-RGB: NKRO input
        if(length != 15)
            return;
        for(int bit = 0; bit < 8; bit++){
            if((urbinput[0] >> bit) & 1)
                SET_KEYBIT(kbinput, hid_codes[bit + 224]);
            else
                CLEAR_KEYBIT(kbinput, hid_codes[bit + 224]);
        }
        for(int byte = 0; byte < 14; byte++){
            char input = urbinput[byte + 1];
            for(int bit = 0; bit < 8; bit++){
                int keybit = byte * 8 + bit;
                int scan = hid_codes[keybit];
                if((input >> bit) & 1){
                    if(scan >= 0)
                        SET_KEYBIT(kbinput, hid_codes[keybit]);
                    else
                        printf("Got unknown key press %d on EP 3\n", keybit);
                } else if(scan >= 0)
                    CLEAR_KEYBIT(kbinput, hid_codes[keybit]);
            }
        }
    } else if(endpoint == 2){
        // EP 2 Non-RGB: media keys
        CLEAR_KEYBIT(kbinput, 97);          // mute
        CLEAR_KEYBIT(kbinput, 98);          // stop
        CLEAR_KEYBIT(kbinput, 99);          // prev
        CLEAR_KEYBIT(kbinput, 100);         // play
        CLEAR_KEYBIT(kbinput, 101);         // next
        CLEAR_KEYBIT(kbinput, 130);         // volup
        CLEAR_KEYBIT(kbinput, 131);         // voldn
        for(int i = 0; i < length; i++){
            switch(urbinput[i]){
            case 181:
                SET_KEYBIT(kbinput, 101);   // next
                break;
            case 182:
                SET_KEYBIT(kbinput, 99);    // prev
                break;
            case 183:
                SET_KEYBIT(kbinput, 98);    // stop
                break;
            case 205:
                SET_KEYBIT(kbinput, 100);   // play
                break;
            case 226:
                SET_KEYBIT(kbinput, 97);    // mute
                break;
            case 233:
                SET_KEYBIT(kbinput, 130);   // volup
                break;
            case 234:
                SET_KEYBIT(kbinput, 131);   // voldn
                break;
            }
        }
    }
}

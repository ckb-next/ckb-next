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
    // Test keycodes
    printf("EP %d:", endpoint);
    for(int i = 0; i < length; i++)
        printf(" %02hhx", urbinput[i]);
    printf("\n");
    // LUT for HID -> Corsair scancodes (-1 for no scan code, -2 for currently unsupported)
    // Modified from Linux drivers/hid/usbhid/usbkbd.c, key codes replaced with array indices
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
        -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        60,  48,  62,  61,  91,  90,  67,  68, 100,  -2,  99, 101,  -2, 130, 131,  97,
        -2,  -2,  -2,  98,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -1,  -1,  -1,  -1,
    };
    if(endpoint == 1){
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
                int bit = hid_codes[urbinput[i]];
                if(bit >= 0)
                    SET_KEYBIT(kbinput, bit);
                //else
                //    printf("Got unknown key press %d on EP %d\n", urbinput[i], endpoint);
            }
        }
    } else {
        // TO-DO: handle other endpoints
        for(int i = 0; i < length; i++){
            if(urbinput[i] != 0){
                //printf("Got unknown key press %d on EP %d\n", urbinput[i], endpoint);
            }
        }
    }
}

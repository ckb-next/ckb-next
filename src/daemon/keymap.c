#include "device.h"
#include "includes.h"
#include "keymap.h"

const key keymap[N_KEYS_EXTENDED] = {
    // Keyboard keys
    { "esc",        0x00, KEY_ESC },
    { "f1",         0x0c, KEY_F1 },
    { "f2",         0x18, KEY_F2 },
    { "f3",         0x24, KEY_F3 },
    { "f4",         0x30, KEY_F4 },
    { "f5",         0x3c, KEY_F5 },
    { "f6",         0x48, KEY_F6 },
    { "f7",         0x54, KEY_F7 },
    { "f8",         0x60, KEY_F8 },
    { "f9",         0x6c, KEY_F9 },
    { "f10",        0x78, KEY_F10 },
    { "f11",        0x84, KEY_F11 },
    { "grave",      0x01, KEY_GRAVE },
    { "1",          0x0d, KEY_1 },
    { "2",          0x19, KEY_2 },
    { "3",          0x25, KEY_3 },
    { "4",          0x31, KEY_4 },
    { "5",          0x3d, KEY_5 },
    { "6",          0x49, KEY_6 },
    { "7",          0x55, KEY_7 },
    { "8",          0x61, KEY_8 },
    { "9",          0x6d, KEY_9 },
    { "0",          0x79, KEY_0 },
    { "minus",      0x85, KEY_MINUS },
    { "tab",        0x02, KEY_TAB },
    { "q",          0x0e, KEY_Q },
    { "w",          0x1a, KEY_W },
    { "e",          0x26, KEY_E },
    { "r",          0x32, KEY_R },
    { "t",          0x3e, KEY_T },
    { "y",          0x4a, KEY_Y },
    { "u",          0x56, KEY_U },
    { "i",          0x62, KEY_I },
    { "o",          0x6e, KEY_O },
    { "p",          0x7a, KEY_P },
    { "lbrace",     0x86, KEY_LEFTBRACE },
    { "caps",       0x03, KEY_CAPSLOCK },
    { "a",          0x0f, KEY_A },
    { "s",          0x1b, KEY_S },
    { "d",          0x27, KEY_D },
    { "f",          0x33, KEY_F },
    { "g",          0x3f, KEY_G },
    { "h",          0x4b, KEY_H },
    { "j",          0x57, KEY_J },
    { "k",          0x63, KEY_K },
    { "l",          0x6f, KEY_L },
    { "colon",      0x7b, KEY_SEMICOLON },
    { "quote",      0x87, KEY_APOSTROPHE },
    { "lshift",     0x04, KEY_LEFTSHIFT },
    { "bslash_iso", 0x10, KEY_102ND },
    { "z",          0x1c, KEY_Z },
    { "x",          0x28, KEY_X },
    { "c",          0x34, KEY_C },
    { "v",          0x40, KEY_V },
    { "b",          0x4c, KEY_B },
    { "n",          0x58, KEY_N },
    { "m",          0x64, KEY_M },
    { "comma",      0x70, KEY_COMMA },
    { "dot",        0x7c, KEY_DOT },
    { "slash",      0x88, KEY_SLASH },
    { "lctrl",      0x05, KEY_LEFTCTRL },
    { "lwin",       0x11, KEY_LEFTMETA },
    { "lalt",       0x1d, KEY_LEFTALT },
    { 0,            -1,   KEY_NONE },
    { "space",      0x35, KEY_SPACE },
    { 0,            -1,   KEY_NONE },
    { 0,            -1,   KEY_NONE },
    { "ralt",       0x59, KEY_RIGHTALT },
    { "rwin",       0x65, KEY_RIGHTMETA },
    { "rmenu",      0x71, KEY_COMPOSE },
    { 0,            -1,   KEY_NONE },
    { "light",      0x89, KEY_CORSAIR },
    { "f12",        0x06, KEY_F12 },
    { "prtscn",     0x12, KEY_SYSRQ },
    { "scroll",     0x1e, KEY_SCROLLLOCK },
    { "pause",      0x2a, KEY_PAUSE },
    { "ins",        0x36, KEY_INSERT },
    { "home",       0x42, KEY_HOME },
    { "pgup",       0x4e, KEY_PAGEUP },
    { "rbrace",     0x5a, KEY_RIGHTBRACE },
    { "bslash",     0x66, KEY_BACKSLASH },
    { "hash",       0x72, KEY_BACKSLASH_ISO },
    { "enter",      0x7e, KEY_ENTER },
    { 0,            -1,   KEY_NONE },
    { "equal",      0x07, KEY_EQUAL },
    { 0,            -1,   KEY_NONE },
    { "bspace",     0x1f, KEY_BACKSPACE },
    { "del",        0x2b, KEY_DELETE },
    { "end",        0x37, KEY_END },
    { "pgdn",       0x43, KEY_PAGEDOWN },
    { "rshift",     0x4f, KEY_RIGHTSHIFT },
    { "rctrl",      0x5b, KEY_RIGHTCTRL },
    { "up",         0x67, KEY_UP },
    { "left",       0x73, KEY_LEFT },
    { "down",       0x7f, KEY_DOWN },
    { "right",      0x8b, KEY_RIGHT },
    { "lock",       0x08, KEY_CORSAIR },
    { "mute",       0x14, KEY_MUTE },
    { "stop",       0x20, KEY_STOPCD },
    { "prev",       0x2c, KEY_PREVIOUSSONG },
    { "play",       0x38, KEY_PLAYPAUSE },
    { "next",       0x44, KEY_NEXTSONG },
    { "numlock",    0x50, KEY_NUMLOCK },
    { "numslash",   0x5c, KEY_KPSLASH },
    { "numstar",    0x68, KEY_KPASTERISK },
    { "numminus",   0x74, KEY_KPMINUS },
    { "numplus",    0x80, KEY_KPPLUS },
    { "numenter",   0x8c, KEY_KPENTER },
    { "num7",       0x09, KEY_KP7 },
    { "num8",       0x15, KEY_KP8 },
    { "num9",       0x21, KEY_KP9 },
    { 0,            -1,   KEY_NONE },
    { "num4",       0x39, KEY_KP4 },
    { "num5",       0x45, KEY_KP5 },
    { "num6",       0x51, KEY_KP6 },
    { "num1",       0x5d, KEY_KP1 },
    { "num2",       0x69, KEY_KP2 },
    { "num3",       0x75, KEY_KP3 },
    { "num0",       0x81, KEY_KP0 },
    { "numdot",     0x8d, KEY_KPDOT },
    { "g1",         0x0a, KEY_CORSAIR },
    { "g2",         0x16, KEY_CORSAIR },
    { "g3",         0x22, KEY_CORSAIR },
    { "g4",         0x2e, KEY_CORSAIR },
    { "g5",         0x3a, KEY_CORSAIR },
    { "g6",         0x46, KEY_CORSAIR },
    { "g7",         0x52, KEY_CORSAIR },
    { "g8",         0x5e, KEY_CORSAIR },
    { "g9",         0x6a, KEY_CORSAIR },
    { "g10",        0x76, KEY_CORSAIR },
    { "volup",      0x20, KEY_VOLUMEUP },
    { "voldn",      0x2c, KEY_VOLUMEDOWN },
    { "mr",         0x0b, KEY_CORSAIR },
    { "m1",         0x17, KEY_CORSAIR },
    { "m2",         0x23, KEY_CORSAIR },
    { "m3",         0x2f, KEY_CORSAIR },
    { "g11",        0x3b, KEY_CORSAIR },
    { "g12",        0x47, KEY_CORSAIR },
    { "g13",        0x53, KEY_CORSAIR },
    { "g14",        0x5f, KEY_CORSAIR },
    { "g15",        0x6b, KEY_CORSAIR },
    { "g16",        0x77, KEY_CORSAIR },
    { "g17",        0x83, KEY_CORSAIR },
    { "g18",        0x8f, KEY_CORSAIR },
    { 0,            -1,   KEY_NONE },
    { 0,            -1,   KEY_NONE },
    { "fn",         0x65, KEY_FN },         // STRAFE Fn - rwin on other keyboards
    { 0,            -1,   KEY_NONE },
    { 0,            -1,   KEY_NONE },
    { 0,            -1,   KEY_NONE },
    { 0,            -1,   KEY_NONE },
    { 0,            -1,   KEY_NONE },

    // Strafe specific side leds, that are set via a special command
    { "lsidel",      -2, KEY_CORSAIR },
    { "rsidel",      -2, KEY_CORSAIR },
    // Strafe logo backlight
    { "logo",      0x7d, KEY_CORSAIR },

    // Keys not present on any device
    { "lightup",    -1, KEY_BRIGHTNESSUP },
    { "lightdn",    -1, KEY_BRIGHTNESSDOWN },
    { "eject",      -1, KEY_EJECTCD },
    { "power",      -1, KEY_POWER },
    { "f13",        -1, KEY_F13 },
    { "f14",        -1, KEY_F14 },
    { "f15",        -1, KEY_F15 },
    { "f16",        -1, KEY_F16 },
    { "f17",        -1, KEY_F17 },
    { "f18",        -1, KEY_F18 },
    { "f19",        -1, KEY_F19 },
    { "f20",        -1, KEY_F20 },

    // Mouse buttons
    { "mouse1",     -1, SCAN_MOUSE | BTN_LEFT },
    { "mouse2",     -1, SCAN_MOUSE | BTN_RIGHT },
    { "mouse3",     -1, SCAN_MOUSE | BTN_MIDDLE },
    { "mouse4",     -1, SCAN_MOUSE | BTN_SIDE },
    { "mouse5",     -1, SCAN_MOUSE | BTN_EXTRA },
    { "dpiup",      -1, KEY_CORSAIR },
    { "dpidn",      -1, KEY_CORSAIR },
    { "sniper",     -1, KEY_CORSAIR },
    { "thumb1",     -1, KEY_CORSAIR },
    { "thumb2",     -1, KEY_CORSAIR },
    { "thumb3",     -1, KEY_CORSAIR },
    { "thumb4",     -1, KEY_CORSAIR },
    { "thumb5",     -1, KEY_CORSAIR },
    { "thumb6",     -1, KEY_CORSAIR },
    { "thumb7",     -1, KEY_CORSAIR },
    { "thumb8",     -1, KEY_CORSAIR },
    { "thumb9",     -1, KEY_CORSAIR },
    { "thumb10",    -1, KEY_CORSAIR },
    { "thumb11",    -1, KEY_CORSAIR },
    { "thumb12",    -1, KEY_CORSAIR },

    // Extended mouse buttons (wheel is an axis in HW, 6-8 are recognized by the OS but not present in HW)
    { "wheelup",    -1, SCAN_MOUSE | BTN_WHEELUP },
    { "wheeldn",    -1, SCAN_MOUSE | BTN_WHEELDOWN },
    { "mouse6",     -1, SCAN_MOUSE | BTN_FORWARD },
    { "mouse7",     -1, SCAN_MOUSE | BTN_BACK },
    { "mouse8",     -1, SCAN_MOUSE | BTN_TASK },

    // RGB mouse zones
    { "front",      LED_MOUSE, KEY_NONE },
    { "back",       LED_MOUSE + 1, KEY_NONE },
    { "dpi",        LED_MOUSE + 2, KEY_NONE },  // SW DPI light
    { "wheel",      LED_MOUSE + 3, KEY_NONE },
    { "thumb",      LED_MOUSE + 4, KEY_NONE },
    { "dpi0",       LED_MOUSE + 5, KEY_NONE },  // HW DPI light levels (0 = sniper, 1..5 = modes)
    { "dpi1",       LED_MOUSE + 6, KEY_NONE },
    { "dpi2",       LED_MOUSE + 7, KEY_NONE },
    { "dpi3",       LED_MOUSE + 8, KEY_NONE },
    { "dpi4",       LED_MOUSE + 9, KEY_NONE },
    { "dpi5",       LED_MOUSE + 10, KEY_NONE },
};

void hid_kb_translate(unsigned char* kbinput, int endpoint, int length, const unsigned char* urbinput){
    if(length < 1)
        return;
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
    switch(endpoint){
    case 1:
    case -1:
        // EP 1: 6KRO input (RGB and non-RGB)
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
                    ckb_warn("Got unknown key press %d on EP 1\n", urbinput[i]);
            }
        }
        break;
    case -2:
        // EP 2 RGB: NKRO input
        if(urbinput[0] == 1){
            // Type 1: standard key
            if(length != 21)
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
                            ckb_warn("Got unknown key press %d on EP 2\n", keybit);
                    } else if(scan >= 0)
                        CLEAR_KEYBIT(kbinput, hid_codes[keybit]);
                }
            }
            break;
        } else if(urbinput[0] == 2)
            ;       // Type 2: media key (fall through)
        else
            break;  // No other known types
    case 2:
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
        break;
    case 3:
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
                        ckb_warn("Got unknown key press %d on EP 3\n", keybit);
                } else if(scan >= 0)
                    CLEAR_KEYBIT(kbinput, hid_codes[keybit]);
            }
        }
        break;
    }
}

#define BUTTON_HID_COUNT    5

void hid_mouse_translate(unsigned char* kbinput, short* xaxis, short* yaxis, int endpoint, int length, const unsigned char* urbinput){
    if((endpoint != 2 && endpoint != -2) || length < 10)
        return;
    // EP 2: mouse input
    if(urbinput[0] != 1)
        return;
    // Byte 1 = mouse buttons (bitfield)
    for(int bit = 0; bit < BUTTON_HID_COUNT; bit++){
        if(urbinput[1] & (1 << bit))
            SET_KEYBIT(kbinput, MOUSE_BUTTON_FIRST + bit);
        else
            CLEAR_KEYBIT(kbinput, MOUSE_BUTTON_FIRST + bit);
    }
    // Bytes 5 - 8: movement
    *xaxis += *(short*)(urbinput + 5);
    *yaxis += *(short*)(urbinput + 7);
    // Byte 9: wheel
    char wheel = urbinput[9];
    if(wheel > 0)
        SET_KEYBIT(kbinput, MOUSE_EXTRA_FIRST);         // wheelup
    else
        CLEAR_KEYBIT(kbinput, MOUSE_EXTRA_FIRST);
    if(wheel < 0)
        SET_KEYBIT(kbinput, MOUSE_EXTRA_FIRST + 1);     // wheeldn
    else
        CLEAR_KEYBIT(kbinput, MOUSE_EXTRA_FIRST + 1);
}

void corsair_kbcopy(unsigned char* kbinput, int endpoint, const unsigned char* urbinput){
    if(endpoint == 2 || endpoint == -2){
        if(urbinput[0] != 3)
            return;
        urbinput++;
    }
    memcpy(kbinput, urbinput, N_KEYBYTES_HW);
}

void corsair_mousecopy(unsigned char* kbinput, int endpoint, const unsigned char* urbinput){
    if(endpoint == 2 || endpoint == -2){
        if(urbinput[0] != 3)
            return;
        urbinput++;
    }
    for(int bit = BUTTON_HID_COUNT; bit < N_BUTTONS_HW; bit++){
        int byte = bit / 8;
        uchar test = 1 << (bit % 8);
        if(urbinput[byte] & test)
            SET_KEYBIT(kbinput, MOUSE_BUTTON_FIRST + bit);
        else
            CLEAR_KEYBIT(kbinput, MOUSE_BUTTON_FIRST + bit);
    }
}

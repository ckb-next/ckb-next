#include "device.h"
#include "includes.h"
#include "keymap.h"
#include "usb.h"
#include "input.h"
#include "bragi_proto.h"

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
    { "lshift",     0x04, KEY_LEFTSHIFT }, // 48
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
    { "lctrl",      0x05, KEY_LEFTCTRL }, // 60
    { "lwin",       0x11, KEY_LEFTMETA },
    { "lalt",       0x1d, KEY_LEFTALT }, // 62
    { "hanja",      -1,   KEY_HANJA },
    { "space",      0x35, KEY_SPACE },
    { "hangul",     -1,   KEY_HANGEUL },
    { "katahira",   0x4d, KEY_KATAKANAHIRAGANA },
    { "ralt",       0x59, KEY_RIGHTALT },
    { "rwin",       0x65, KEY_RIGHTMETA },
    { "rmenu",      0x71, KEY_COMPOSE },
    { "profswitch", 0x7d, KEY_CORSAIR },
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
    { "ro",         0x8a, KEY_RO },
    { "equal",      0x07, KEY_EQUAL },
    { "yen",        0x13, KEY_YEN },
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
    { "muhenkan",   0x29, KEY_MUHENKAN },
    { "henkan",     0x41, KEY_HENKAN },
    { "fn",         0x65, KEY_FN },         // STRAFE Fn - rwin on other keyboards
    { 0,            -1,   KEY_NONE },
    { 0,            -1,   KEY_NONE },
    { 0,            -1,   KEY_NONE },
    { 0,            -1,   KEY_NONE },
    { 0,            -1,   KEY_NONE },

    // K95 Platinum
    { "topbar1",      0x90, KEY_NONE },
    { "topbar2",      0x91, KEY_NONE },
    { "topbar3",      0x92, KEY_NONE },
    { "topbar4",      0x9E, KEY_NONE },
    { "topbar5",      0xA0, KEY_NONE },
    { "topbar6",      0x93, KEY_NONE },
    { "topbar7",      0x94, KEY_NONE },
    { "topbar8",      0x95, KEY_NONE },
    { "topbar9",      0x96, KEY_NONE },
    { "topbar10",     0x97, KEY_NONE },
    { "topbar11",     0x98, KEY_NONE },
    { "topbar12",     0x99, KEY_NONE },
    { "topbar13",     0x9A, KEY_NONE },
    { "topbar14",     0x9B, KEY_NONE },
    { "topbar15",     0x9F, KEY_NONE },
    { "topbar16",     0xA2, KEY_NONE },
    { "topbar17",     0xA1, KEY_NONE },
    { "topbar18",     0x9C, KEY_NONE },
    { "topbar19",     0x9D, KEY_NONE },

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
    { "f21",        -1, KEY_F21 },
    { "f22",        -1, KEY_F22 },
    { "f23",        -1, KEY_F23 },
    { "f24",        -1, KEY_F24 },

    // Generic zones (K55, MM800, ST100, etc)
    { "zone1",      0x00, KEY_NONE },
    { "zone2",      0x01, KEY_NONE },
    { "zone3",      0x02, KEY_NONE },
    { "zone4",      0x03, KEY_NONE },
    { "zone5",      0x04, KEY_NONE },
    { "zone6",      0x05, KEY_NONE },
    { "zone7",      0x06, KEY_NONE },
    { "zone8",      0x07, KEY_NONE },
    { "zone9",      0x08, KEY_NONE },
    { "zone10",     0x09, KEY_NONE },
    { "zone11",     0x0A, KEY_NONE },
    { "zone12",     0x0B, KEY_NONE },
    { "zone13",     0x0C, KEY_NONE },
    { "zone14",     0x0D, KEY_NONE },
    { "zone15",     0x0E, KEY_NONE },

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
    { 0,            -1,   KEY_NONE },
    { 0,            -1,   KEY_NONE },
    { 0,            -1,   KEY_NONE },
    { "profdn",     -1, KEY_CORSAIR },
    { "profswitch", -1, KEY_CORSAIR },
    // { "profup",     -1, KEY_CORSAIR },
    // { "profdn",     -1, KEY_CORSAIR },
    // { "optbtn",     -1, KEY_CORSAIR },

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
    { "side",       LED_MOUSE + 5, KEY_NONE },
    { "dpi0",       LED_MOUSE + 6, KEY_NONE },  // HW DPI light levels (0 = sniper, 1..5 = modes)
    { "dpi1",       LED_MOUSE + 7, KEY_NONE },
    { "dpi2",       LED_MOUSE + 8, KEY_NONE },
    { "dpi3",       LED_MOUSE + 9, KEY_NONE },
    { "dpi4",       LED_MOUSE + 10, KEY_NONE },
    { "dpi5",       LED_MOUSE + 11, KEY_NONE },
};

// LUT for HID -> Corsair scancodes (-1 for no scan code, -2 for currently unsupported)
// Modified from Linux drivers/hid/usbhid/usbkbd.c, key codes replaced with keymap array indices and K95 keys added
// Make sure the indices match the keyindex as passed to nprintkey() in notify.c
static const short hid_codes[256] = {
     -1,  -1,  -1,  -1,  37,  54,  52,  39,  27,  40,  41,  42,  32,  43,  44,  45,
     56,  55,  33,  34,  25,  28,  38,  29,  31,  53,  26,  51,  30,  50,  13,  14,
     15,  16,  17,  18,  19,  20,  21,  22,  82,   0,  86,  24,  64,  23,  84,  35,
     79,  80,  81,  46,  47,  12,  57,  58,  59,  36,   1,   2,   3,   4,   5,   6,
      7,   8,   9,  10,  11,  72,  73,  74,  75,  76,  77,  78,  87,  88,  89,  95,
     93,  94,  92, 102, 103, 104, 105, 106, 107, 115, 116, 117, 112, 113, 114, 108,
    109, 110, 118, 119,  49,  69,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
     -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  98,  -2,  -2,  -2,  -2,  -2,  -2,  97,
    130, 131,  -1,  -1,  -1,  -2,  -1,  83,  66,  85, 145, 144,  -2,  -1,  -1,  -1,
     65,  63,  -2,  -2,  -2,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -3,  -1,  -1,  -1,  // <- -3 = non-RGB program key
    120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 136, 137, 138, 139, 140, 141,
     60,  48,  62,  61,  91,  90,  67,  68, 142, 143,  99, 101,  -2, 130, 131,  97,
     -2, 133, 134, 135,  -2,  96,  -2, 132,  -2,  -2,  71,  71,  71,  71,  -1,  -1,
};

static const short bragi_extra_lut[40] = {
     -1, 60,  48,  62,  61,  91,  90,  67,  68, 142, 143,  99, 101,  -2, 130, 131,
     97, -2, 133, 134, 135,  -2,  96,  -2, 132,  -2,  -2, 120, 121, 122, 123, 124,
    125, -2,  -2,  -2,  -2,  -2,  -2,  -2,
};

// There are three types of keyboard input. 6KRO, NKRO and Corsair.
//
// 6KRO is only enabled in BIOS mode, and since we do not touch devices in bios mode,
// we do not need to implement it.
//
// NKRO is enabled during normal operation. Key packets start with 0x01, media key packets with 0x02.
// It does NOT get disabled when the keyboard is in software mode. Handled by hid_kb_translate()
//
// Corsair is enabled only after the keyboard has been set to software mode, all packets start with 0x03.
// Since Corsair input packets are sent along with NKRO ones, in software mode, we need to ignore NKRO.
// Handled by corsair_kbcopy()

/// Process the input depending on type of device. Interpret the actual size of the URB buffer
///
/// device | detect with macro combination | seems to be endpoint # | actual buffer-length | function called
/// ------ | ----------------------------- | ---------------------- | -------------------- | ---------------
/// mouse (RGB and non RGB) | IS_MOUSE | nA | 8, 10 or 11 | hid_mouse_translate()
/// mouse (RGB and non RGB) | IS_MOUSE | nA | MSG_SIZE (64) | corsair_mousecopy()
/// RGB Keyboard | !IS_LEGACY && !IS_MOUSE | 1 | 8 (BIOS Mode) | hid_kb_translate()
/// RGB Keyboard | !IS_LEGACY && !IS_MOUSE | 2 | 5 or 21, KB inactive! | hid_kb_translate()
/// RGB Keyboard | !IS_LEGACY && !IS_MOUSE | 3? | MSG_SIZE | corsair_kbcopy()
/// Legacy Keyboard | IS_LEGACY && !IS_MOUSE | nA | nA | hid_kb_translate()
///

static inline void handle_bragi_key_input(unsigned char* kbinput, const unsigned char* urbinput, int length){
    // Skip the 0x00 0x02 header
    urbinput += 2;

    for(int byte = 0; byte < 13; byte++){
        char input = urbinput[byte];
        for(int bit = 0; bit < 8; bit++){
            int keybit = byte * 8 + bit;
            int scan = hid_codes[keybit];
            if((input >> bit) & 1){
                if(scan >= 0)
                    SET_KEYBIT(kbinput, scan);
                else
                    ckb_warn("Got unknown bragi key press %d", keybit);
            } else if(scan >= 0)
                CLEAR_KEYBIT(kbinput, scan);
        }
    }

    // This is supposed to be the byte at offset 0x01, but because the header has an 0x02 at that position,
    // it's moved to offset 0x15, right after the above loop finishes.
    // Since there's more after this, we just read from a different LUT
    for(int byte = 0; byte < 5; byte++){
        char input = urbinput[13 + byte];
        for(int bit = 0; bit < 8; bit++){
            int keybit = byte * 8 + bit;
            int scan = bragi_extra_lut[keybit];
            if((input >> bit) & 1){
                if(scan >= 0)
                    SET_KEYBIT(kbinput, scan);
                else
                    ckb_warn("Got unknown extended bragi key press %d", keybit);
            } else if(scan >= 0)
                CLEAR_KEYBIT(kbinput, scan);
        }
    }
    /*
    for(int bit = 0; bit < 8; bit++){
        if((urbinput[13] >> bit) & 1)
            SET_KEYBIT(kbinput, hid_codes[bit + 223]);
        else
            CLEAR_KEYBIT(kbinput, hid_codes[bit + 223]);
    }
    
    // Followed by rwin at urbinput[14], bit 0
    if((urbinput[13] >> bit) & 1)
        SET_KEYBIT(kbinput, hid_codes[bit + 223]);
    else
        CLEAR_KEYBIT(kbinput, hid_codes[ + 223]);
    223+8
    */
}

void process_input_urb(void* context, unsigned char* buffer, int urblen, ushort ep){
    if(!urblen)
        return;

    usbdevice* kb = context;

#ifdef DEBUG_USB_INPUT
    print_urb_buffer("Input Recv:", buffer, urblen, NULL, 0, NULL, INDEX_OF(kb, keyboard));
#endif

    // Get first byte of the response
    uchar firstbyte = buffer[0];
    // If the response starts with CMD_GET (0x0e), or it came from ep4 with bragi, that means it needs to go to os_usbrecv()
    if(urblen == MSG_SIZE && (firstbyte == CMD_GET || (kb->protocol == PROTO_BRAGI && ep == 0x84))){
        int retval = pthread_mutex_lock(intmutex(kb));
        if(retval)
            ckb_fatal("Error locking interrupt mutex %i", retval);
        memcpy(kb->interruptbuf, buffer, MSG_SIZE);
        // signal os_usbrecv() that the data is ready.
        retval = pthread_cond_broadcast(intcond(kb));
        if(retval)
            ckb_fatal("Error broadcasting pthread cond %i", retval);
        retval = pthread_mutex_unlock(intmutex(kb));
        if(retval)
            ckb_fatal("Error unlocking interrupt mutex %i", retval);
    } else {
        queued_mutex_lock(imutex(kb));
        if(IS_LEGACY_DEV(kb)) {
            if(IS_MOUSE_DEV(kb))
                m95_mouse_translate(kb->input.keys, &kb->input.rel_x, &kb->input.rel_y, urblen, buffer);
            else
                hid_kb_translate(kb->input.keys, urblen, buffer, 1);
        } else {
            if(IS_MOUSE_DEV(kb)) {
                // HID Mouse Input
                // In HW mode, Bragi mouse is the same as NXP, but without a header
                if(kb->protocol == PROTO_BRAGI) {
                    // When active, we need the movement from the standard hid packet, but the buttons from the extra packet
                    if(kb->active) {
                        if(firstbyte == BRAGI_INPUT_0 && buffer[1] == BRAGI_INPUT_1) {
                            corsair_bragi_mousecopy(kb->input.keys, buffer);
                        } else {
                            kb->input.rel_x += (buffer[5] << 8) | buffer[4];
                            kb->input.rel_y += (buffer[7] << 8) | buffer[6];
                        }
                    } else {
                        hid_mouse_translate(kb->input.keys, &kb->input.rel_x, &kb->input.rel_y, urblen, buffer);
                    }
                } else if(firstbyte == MOUSE_IN) {
                    hid_mouse_translate(kb->input.keys, &kb->input.rel_x, &kb->input.rel_y, urblen, buffer + 1);
                } else if(firstbyte == CORSAIR_IN) { // Corsair Mouse Input
                    corsair_mousecopy(kb->input.keys, buffer);
                } else if(firstbyte == 0x05 && urblen == 21) { // Seems to be on the Ironclaw RGB only
                    corsair_extended_mousecopy(kb->input.keys, buffer);
                } else {
                    ckb_err("Unknown mouse data received in input thread %02x from endpoint %02x", firstbyte, ep);
                }
            } else {
                // Assume Keyboard for everything else for now
                // Accept NKRO only if device is not active
                if(firstbyte == NKRO_KEY_IN || firstbyte == NKRO_MEDIA_IN) {
                    if(!kb->active)
                        hid_kb_translate(kb->input.keys, urblen, buffer, 0);
                } else if(urblen == MSG_SIZE) {
                    if(kb->protocol == PROTO_BRAGI) {
                        handle_bragi_key_input(kb->input.keys, buffer, urblen);
                    } else {
                        if((kb->fwversion >= 0x130 || IS_V2_OVERRIDE(kb)) && firstbyte == CORSAIR_IN) // Ugly hack due to old FW 1.15 packets having no header
                            buffer++;
                        corsair_kbcopy(kb->input.keys, buffer);
                    }
                } else
                    ckb_err("Unknown data received in input thread %02x from endpoint %02x", firstbyte, ep);
            }
        }
        ///
        /// The input data is transformed and copied to the kb structure. Now give it to the OS and unlock the imutex afterwards.
        inputupdate(kb);
        queued_mutex_unlock(imutex(kb));
    }
}

void handle_nkro_key_input(unsigned char* kbinput, const unsigned char* urbinput, int length, int legacy){
    int start = !legacy; // Legacy packets start from 0x00, other ones start from 0x01
    for(int bit = 0; bit < 8; bit++){
        if((urbinput[start] >> bit) & 1)
            SET_KEYBIT(kbinput, hid_codes[bit + 224]);
        else
            CLEAR_KEYBIT(kbinput, hid_codes[bit + 224]);
    }

    int bytelen = (legacy ? 14 : 19);
    for(int byte = 0; byte < bytelen; byte++){
        char input = urbinput[start + byte + 1];
        for(int bit = 0; bit < 8; bit++){
            int keybit = byte * 8 + bit;
            int scan = hid_codes[keybit];
            if((input >> bit) & 1){
                if(scan >= 0)
                    SET_KEYBIT(kbinput, hid_codes[keybit]);
                else
                    ckb_warn("Got unknown NKRO key press %d", keybit);
            } else if(scan >= 0)
                CLEAR_KEYBIT(kbinput, hid_codes[keybit]);
        }
    }
}

void handle_nkro_media_keys(unsigned char* kbinput, const unsigned char* urbinput, int length){
    // Media keys
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

void handle_legacy_6kro_input(unsigned char* kbinput, const unsigned char* urbinput, int length){
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
                ckb_warn("Got unknown legacy 6kro key press %d", urbinput[i]);
        }
    }
}

void hid_kb_translate(unsigned char* kbinput, int length, const unsigned char* urbinput, int legacy){
    if(legacy) {
        switch(length) {
            case 2: // K65 Media keys
                length = 1;
                // fall through
            case 4: // Media Keys
                handle_nkro_media_keys(kbinput, urbinput, length);
                break;
            case 8: // G/MR Keys
                handle_legacy_6kro_input(kbinput, urbinput, length);
                break;
            case 15: // NKRO Input
                handle_nkro_key_input(kbinput, urbinput, length, legacy);
                break;
            default:
                ckb_warn("Got unknown legacy data");
        }
    } else {
        switch(urbinput[0]) {
            case 0x01:
                handle_nkro_key_input(kbinput, urbinput, length, legacy);
                break;
            case 0x02:
                handle_nkro_media_keys(kbinput, urbinput, length);
                break;
            default:
                ckb_warn("Got unknown data");
        }
    }
}

#define BUTTON_HID_COUNT    5

void hid_mouse_translate(unsigned char* kbinput, short* xaxis, short* yaxis, int length, const unsigned char* urbinput){
    // Byte 1 = mouse buttons (bitfield)
    for(int bit = 0; bit < BUTTON_HID_COUNT; bit++){
        if(urbinput[0] & (1 << bit))
            SET_KEYBIT(kbinput, MOUSE_BUTTON_FIRST + bit);
        else
            CLEAR_KEYBIT(kbinput, MOUSE_BUTTON_FIRST + bit);
    }
    // Bytes 5 - 8: movement
    *xaxis += (urbinput[5] << 8) | urbinput[4];
    *yaxis += (urbinput[7] << 8) | urbinput[6];
    // Byte 9: wheel
    char wheel = urbinput[8];
    if(wheel > 0)
        SET_KEYBIT(kbinput, MOUSE_EXTRA_FIRST);         // wheelup
    else
        CLEAR_KEYBIT(kbinput, MOUSE_EXTRA_FIRST);
    if(wheel < 0)
        SET_KEYBIT(kbinput, MOUSE_EXTRA_FIRST + 1);     // wheeldn
    else
        CLEAR_KEYBIT(kbinput, MOUSE_EXTRA_FIRST + 1);
}

void corsair_kbcopy(unsigned char* kbinput, const unsigned char* urbinput){
    memcpy(kbinput, urbinput, N_KEYBYTES_HW);
}

void corsair_mousecopy(unsigned char* kbinput, const unsigned char* urbinput){
    urbinput++;
    for(int bit = BUTTON_HID_COUNT; bit < N_BUTTONS_HW; bit++){
        int byte = bit / 8;
        uchar test = 1 << (bit % 8);
        if(urbinput[byte] & test)
            SET_KEYBIT(kbinput, MOUSE_BUTTON_FIRST + bit);
        else
            CLEAR_KEYBIT(kbinput, MOUSE_BUTTON_FIRST + bit);
    }
}

void corsair_extended_mousecopy(unsigned char* kbinput, const unsigned char* urbinput){
    // This handles the ironclaw 0x05 packets.
    // So far only two possible values exist. In the future this may need to be rewritten
    // in a similar fashion as corsair_mousecopy but we currently do not have enough data to do so.

    if(urbinput[6] & 0b01)
        SET_KEYBIT(kbinput, MOUSE_BUTTON_FIRST + 4);
    else
        CLEAR_KEYBIT(kbinput, MOUSE_BUTTON_FIRST + 4);

    if(urbinput[6] & 0b10)
        SET_KEYBIT(kbinput, MOUSE_BUTTON_FIRST + 3);
    else
        CLEAR_KEYBIT(kbinput, MOUSE_BUTTON_FIRST + 3);
}

void m95_mouse_translate(unsigned char* kbinput, short* xaxis, short* yaxis, int length, const unsigned char* urbinput){
    if(length != 7)
        return;
    unsigned short input = (((unsigned short)urbinput[1]) << 8) | urbinput[0];
    for(int bit = 0; bit < 16; bit++){
        unsigned char current_bit = (input >> bit) & 1;
        if(current_bit)
            SET_KEYBIT(kbinput, MOUSE_BUTTON_FIRST + bit);
        else
            CLEAR_KEYBIT(kbinput, MOUSE_BUTTON_FIRST + bit);
    }

    *xaxis += (urbinput[3] << 8) | urbinput[2];
    *yaxis += (urbinput[5] << 8) | urbinput[4];

    char wheel = urbinput[6];
    if(wheel > 0)
        SET_KEYBIT(kbinput, MOUSE_EXTRA_FIRST);
    else
        CLEAR_KEYBIT(kbinput, MOUSE_EXTRA_FIRST);
    if(wheel < 0)
        SET_KEYBIT(kbinput, MOUSE_EXTRA_FIRST + 1);
    else
        CLEAR_KEYBIT(kbinput, MOUSE_EXTRA_FIRST + 1);
}

#define BRAGI_MOUSE_BUTTONS 16
/*
01 00 == Left
02 00 == Right
04 00 == middle
10 00 == back thumb
08 00 == front thumb
80 00 == DPI Up
00 01 == Dpi Dn
20 00 == Left Front
40 00 == Left Back
00 02 == sniper
*/

// We have to do it this way, because if we patch the keymap, then we'll break standard input
const unsigned char corsair_bragi_lut[BRAGI_MOUSE_BUTTONS] = {
        0x00,
        0x01,
        0x02,
        0x04,
        0x03,
        0x05,
        0x06,
        0x08,
        0x09,
        0x07, // Anything past this is untested
        0x0A,
        0x0B,
        0x0C,
        0x0D,
        0x0E,
        0x0F,
    };

void corsair_bragi_mousecopy(unsigned char* kbinput, const unsigned char* urbinput){
    urbinput += 2;
    for(int bit = 0; bit < BRAGI_MOUSE_BUTTONS; bit++){
        int byte = bit / 8;
        uchar test = 1 << (bit % 8);
        if(urbinput[byte] & test)
            SET_KEYBIT(kbinput, MOUSE_BUTTON_FIRST + corsair_bragi_lut[bit]);
        else
            CLEAR_KEYBIT(kbinput, MOUSE_BUTTON_FIRST + corsair_bragi_lut[bit]);
    }
    
    char wheel = urbinput[2];
    if(wheel > 0)
        SET_KEYBIT(kbinput, MOUSE_EXTRA_FIRST);
    else
        CLEAR_KEYBIT(kbinput, MOUSE_EXTRA_FIRST);
    if(wheel < 0)
        SET_KEYBIT(kbinput, MOUSE_EXTRA_FIRST + 1);
    else
        CLEAR_KEYBIT(kbinput, MOUSE_EXTRA_FIRST + 1);
}

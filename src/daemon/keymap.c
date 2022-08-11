#include "device.h"
#include "includes.h"
#include "keymap.h"
#include "usb.h"
#include "input.h"
#include "bragi_proto.h"
#include "nxp_proto.h"
#include "bragi_notification.h"

// Translates input from HID to a ckb input bitfield.
static void hid_kb_translate(unsigned char* kbinput, int length, const unsigned char* urbinput, int legacy);
static void hid_mouse_translate(usbinput* input, int length, const unsigned char* urbinput);
static void m95_mouse_translate(usbinput* kbinput, int length, const unsigned char* urbinput);

// Copies input from Corsair reports
static inline void corsair_kbcopy(unsigned char* kbinput, const unsigned char* urbinput){
    memcpy(kbinput, urbinput, N_KEYBYTES_HW);
}
static void corsair_mousecopy(unsigned char* kbinput, const unsigned char* urbinput);
static void corsair_extended_mousecopy(unsigned char* kbinput, const unsigned char* urbinput);
static void corsair_bragi_mousecopy(usbdevice* kb, usbinput* input, const unsigned char* urbinput);

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

    // 20 empty entries for the K100 (patched by the bragi keymap)
    { 0,            -1,   KEY_NONE },
    { 0,            -1,   KEY_NONE },
    { 0,            -1,   KEY_NONE },
    { 0,            -1,   KEY_NONE },
    { 0,            -1,   KEY_NONE },
    { 0,            -1,   KEY_NONE },
    { 0,            -1,   KEY_NONE },
    { 0,            -1,   KEY_NONE },
    { 0,            -1,   KEY_NONE },
    { 0,            -1,   KEY_NONE },
    { 0,            -1,   KEY_NONE },
    { 0,            -1,   KEY_NONE },
    { 0,            -1,   KEY_NONE },
    { 0,            -1,   KEY_NONE },
    { 0,            -1,   KEY_NONE },
    { 0,            -1,   KEY_NONE },
    { 0,            -1,   KEY_NONE },
    { 0,            -1,   KEY_NONE },
    { 0,            -1,   KEY_NONE },
    { 0,            -1,   KEY_NONE },

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
    { 0,            -1,   KEY_NONE }, // "profswitch" lives here for mice, however it's patched in manually
                                      // as the keyboard profswitch interferes and needs to be disabled first

    // Extended mouse buttons (wheel is an axis in HW, 6-8 are recognized by the OS but not present in HW)
    { "wheelup",    -1, SCAN_MOUSE | BTN_WHEELUP },
    { "wheeldn",    -1, SCAN_MOUSE | BTN_WHEELDOWN },
    { "mouse6",     -1, SCAN_MOUSE | BTN_FORWARD },
    { "mouse7",     -1, SCAN_MOUSE | BTN_BACK },
    { "mouse8",     -1, SCAN_MOUSE | BTN_TASK },
    { "wheellf",    -1, SCAN_MOUSE | BTN_WHEELLEFT },
    { "wheelrg",    -1, SCAN_MOUSE | BTN_WHEELRIGHT },

    // RGB mouse zones
    { "front",      LED_MOUSE, KEY_NONE },
    { "back",       LED_MOUSE + 1, KEY_NONE },
    { "dpi",        LED_MOUSE + 2, KEY_NONE },  // SW DPI light
    { "wheel",      LED_MOUSE + 3, KEY_NONE },
    { "thumb",      LED_MOUSE + 4, KEY_NONE },
    { "side",       LED_MOUSE + 5, KEY_NONE },
    { "dpi0",       LED_MOUSE + 6, KEY_NONE },  // HW/SW (non realtime) DPI light levels (0 = sniper, 1..5 = modes)
    { "dpi1",       LED_MOUSE + 7, KEY_NONE },
    { "dpi2",       LED_MOUSE + 8, KEY_NONE },
    { "dpi3",       LED_MOUSE + 9, KEY_NONE },
    { "dpi4",       LED_MOUSE + 10, KEY_NONE },
    { "dpi5",       LED_MOUSE + 11, KEY_NONE },
};

// This gets patched on top of the above in keymap_patch.c in the same order
// The following keys are unknown and need to be added
/*
{ "ro",             -1, KEY_RO },
{ "hanja",          -1, KEY_HANJA },
{ "katahira",       -1, KEY_KATAKANAHIRAGANA },
{ "hangul",         -1, KEY_HANGEUL },
the G keys after G6, if they ever make another keyboard with them
{ "yen",            -1, KEY_YEN }, // UNKNOWN
{ "muhenkan",       -1, KEY_MUHENKAN },
{ "henkan",         -1, KEY_HENKAN },
*/
const key keymap_bragi[N_KEYS_BRAGI_PATCH] = {
    { 0,                -1, KEY_NONE },
    { 0,                -1, KEY_NONE },
    { 0,                -1, KEY_NONE },
    { 0,                -1, KEY_NONE },
    { "a",               4, KEY_A },
    { "b",               5, KEY_B },
    { "c",               6, KEY_C },
    { "d",               7, KEY_D },
    { "e",               8, KEY_E },
    { "f",               9, KEY_F },
    { "g",              10, KEY_G },
    { "h",              11, KEY_H },
    { "i",              12, KEY_I },
    { "j",              13, KEY_J },
    { "k",              14, KEY_K },
    { "l",              15, KEY_L },
    { "m",              16, KEY_M },
    { "n",              17, KEY_N },
    { "o",              18, KEY_O },
    { "p",              19, KEY_P },
    { "q",              20, KEY_Q },
    { "r",              21, KEY_R },
    { "s",              22, KEY_S },
    { "t",              23, KEY_T },
    { "u",              24, KEY_U },
    { "v",              25, KEY_V },
    { "w",              26, KEY_W },
    { "x",              27, KEY_X },
    { "y",              28, KEY_Y },
    { "z",              29, KEY_Z },
    { "1",              30, KEY_1 },
    { "2",              31, KEY_2 },
    { "3",              32, KEY_3 },
    { "4",              33, KEY_4 },
    { "5",              34, KEY_5 },
    { "6",              35, KEY_6 },
    { "7",              36, KEY_7 },
    { "8",              37, KEY_8 },
    { "9",              38, KEY_9 },
    { "0",              39, KEY_0 },
    { "enter",          40, KEY_ENTER },
    { "esc",            41, KEY_ESC },
    { "bspace",         42, KEY_BACKSPACE },
    { "tab",            43, KEY_TAB },
    { "space",          44, KEY_SPACE },
    { "minus",          45, KEY_MINUS },
    { "equal",          46, KEY_EQUAL },
    { "lbrace",         47, KEY_LEFTBRACE },
    { "rbrace",         48, KEY_RIGHTBRACE },
    { "bslash",         49, KEY_BACKSLASH },
    { "hash",           50,  KEY_BACKSLASH_ISO },
    { "colon",          51, KEY_SEMICOLON },
    { "quote",          52, KEY_APOSTROPHE },
    { "grave",          53, KEY_GRAVE },
    { "comma",          54, KEY_COMMA },
    { "dot",            55, KEY_DOT },
    { "slash",          56, KEY_SLASH },
    { "caps",           57, KEY_CAPSLOCK },
    { "f1",             58, KEY_F1 },
    { "f2",             59, KEY_F2 },
    { "f3",             60, KEY_F3 },
    { "f4",             61, KEY_F4 },
    { "f5",             62, KEY_F5 },
    { "f6",             63, KEY_F6 },
    { "f7",             64, KEY_F7 },
    { "f8",             65, KEY_F8 },
    { "f9",             66, KEY_F9 },
    { "f10",            67, KEY_F10 },
    { "f11",            68, KEY_F11 },
    { "f12",            69, KEY_F12 },
    { "prtscn",         70, KEY_SYSRQ },
    { "scroll",         71, KEY_SCROLLLOCK },
    { "pause",          72, KEY_PAUSE },
    { "ins",            73, KEY_INSERT },
    { "home",           74, KEY_HOME },
    { "pgup",           75, KEY_PAGEUP },
    { "del",            76, KEY_DELETE },
    { "end",            77, KEY_END },
    { "pgdn",           78, KEY_PAGEDOWN },
    { "right",          79, KEY_RIGHT },
    { "left",           80, KEY_LEFT },
    { "down",           81, KEY_DOWN },
    { "up",             82, KEY_UP },
    { "numlock",        83, KEY_NUMLOCK },
    { "numslash",       84, KEY_KPSLASH },
    { "numstar",        85, KEY_KPASTERISK },
    { "numminus",       86, KEY_KPMINUS },
    { "numplus",        87, KEY_KPPLUS },
    { "numenter",       88, KEY_KPENTER },
    { "num1",           89, KEY_KP1 },
    { "num2",           90, KEY_KP2 },
    { "num3",           91, KEY_KP3 },
    { "num4",           92, KEY_KP4 },
    { "num5",           93, KEY_KP5 },
    { "num6",           94, KEY_KP6 },
    { "num7",           95, KEY_KP7 },
    { "num8",           96, KEY_KP8 },
    { "num9",           97, KEY_KP9 },
    { "num0",           98, KEY_KP0 },
    { "numdot",         99, KEY_KPDOT },
    { "bslash_iso",    100, KEY_102ND },
    { "rmenu",         101, KEY_COMPOSE },
    { "mute",          102, KEY_MUTE },
    { "volup",         103, KEY_VOLUMEUP },
    { "voldn",         104, KEY_VOLUMEDOWN },
    { "lctrl",         105, KEY_LEFTCTRL },
    { "lshift",        106, KEY_LEFTSHIFT },
    { "lalt",          107, KEY_LEFTALT },
    { "lwin",          108, KEY_LEFTMETA },
    { "rctrl",         109, KEY_RIGHTCTRL },
    { "rshift",        110, KEY_RIGHTSHIFT },
    { "ralt",          111, KEY_RIGHTALT },
    { "rwin",          112, KEY_RIGHTMETA },
    { "light",         113, KEY_CORSAIR },
    { "lock",            1, KEY_CORSAIR },
    { 0,                -1, KEY_NONE },
    { 0,                -1, KEY_NONE },
    { 0,                -1, KEY_NONE },
    { 0,                -1, KEY_NONE },
    { 0,                -1, KEY_NONE },
    { 0,                -1, KEY_NONE },
    { 0,                -1, KEY_NONE },
    { "fn",            122, KEY_FN },
    { "stop",          123, KEY_STOPCD },
    { "play",          124, KEY_PLAYPAUSE },
    { "next",          125, KEY_NEXTSONG },
    { "prev",          126, KEY_PREVIOUSSONG },
    { "mr",             -1, KEY_CORSAIR },
    { "profswitch",    128, KEY_CORSAIR },
    { 0,                -1, KEY_NONE },
    { 0,                -1, KEY_NONE },
    { "g1",            131, KEY_CORSAIR },
    { "g2",            132, KEY_CORSAIR },
    { "g3",            133, KEY_CORSAIR },
    { "g4",            134, KEY_CORSAIR },
    { "g5",            135, KEY_CORSAIR },
    { "g6",            136, KEY_CORSAIR },
    { "ctrlwheelb",    137, KEY_NONE },
    { "topbar1",       138, KEY_NONE },
    { "topbar2",       139, KEY_NONE },
    { "topbar3",       140, KEY_NONE },
    { "topbar4",       141, KEY_NONE },
    { "topbar5",       142, KEY_NONE },
    { "topbar6",       143, KEY_NONE },
    { "topbar7",       144, KEY_NONE },
    { "topbar8",       145, KEY_NONE },
    { "topbar9",       146, KEY_NONE },
    { "topbar10",      147, KEY_NONE },
    { "topbar11",      148, KEY_NONE },
    { "topbar12",      149, KEY_NONE },
    { "topbar13",      150, KEY_NONE },
    { "topbar14",      151, KEY_NONE },
    { "topbar15",      152, KEY_NONE },
    { "topbar16",      153, KEY_NONE },
    { "topbar17",      154, KEY_NONE },
    { "topbar18",      155, KEY_NONE },
    { "topbar19",      156, KEY_NONE },
    { "topbar20",      157, KEY_NONE },
    { "topbar21",      158, KEY_NONE },
    { "topbar22",      159, KEY_NONE },
    { "leftbar1",      160, KEY_NONE },
    { "leftbar2",      161, KEY_NONE },
    { "leftbar3",      162, KEY_NONE },
    { "leftbar4",      163, KEY_NONE },
    { "leftbar5",      164, KEY_NONE },
    { "leftbar6",      165, KEY_NONE },
    { "leftbar7",      166, KEY_NONE },
    { "leftbar8",      167, KEY_NONE },
    { "leftbar9",      168, KEY_NONE },
    { "leftbar10",     169, KEY_NONE },
    { "leftbar11",     170, KEY_NONE },
    { "rightbar1",     171, KEY_NONE },
    { "rightbar2",     172, KEY_NONE },
    { "rightbar3",     173, KEY_NONE },
    { "rightbar4",     174, KEY_NONE },
    { "rightbar5",     175, KEY_NONE },
    { "rightbar6",     176, KEY_NONE },
    { "rightbar7",     177, KEY_NONE },
    { "rightbar8",     178, KEY_NONE },
    { "rightbar9",     179, KEY_NONE },
    { "rightbar10",    180, KEY_NONE },
    { "rightbar11",    181, KEY_NONE },
    { "ctrlwheel2",    182, KEY_NONE },
    { "ctrlwheel3",    183, KEY_NONE },
    { "ctrlwheel4",    184, KEY_NONE },
    { "ctrlwheel5",    185, KEY_NONE },
    { "ctrlwheel6",    186, KEY_NONE },
    { "ctrlwheel7",    187, KEY_NONE },
    { "ctrlwheel8",    188, KEY_NONE },
    { "ctrlwheel1",    189, KEY_NONE }, // We want top right to be 1 and top left to be 8
    { "logoleft",      190, KEY_NONE },
    { "logo",          191, KEY_NONE },
    { "logoright",     192, KEY_NONE },
    { "status",          0, KEY_NONE }, // This might need to be moved elsewhere
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
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 111,  -1,  -1,  -1,
    120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 136, 137, 138, 139, 140, 141,
     60,  48,  62,  61,  91,  90,  67,  68, 142, 143,  99, 101,  -2, 130, 131,  97,
     -2, 133, 134, 135,  -2,  96,  -2, 132,  -2,  -2,  71,  71,  71,  71,  -1,  -1,
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

static inline void handle_bragi_key_input(unsigned char* kbinput, const unsigned char* urbinput, int length){
    // Handle the 01 input and 02 media keys
    // On the K57 WL length is 16, but on the K95P XT it is 21
    if(urbinput[0] == NKRO_KEY_IN && length >= 16){
        // Skipping the first two bytes, the following 13 bytes can be copied as-is, with an offset
        // So let's copy them first before we start bodging
        memcpy(kbinput, urbinput + 2, 13);

        // The 2nd URB byte goes after the 13th in the keymap, shifted by 1 (otherwise it'll apply to voldn)
        // This has the modifiers, starting with lcrtl
        kbinput[13] = (kbinput[13] & 1) | (urbinput[1] << 1);

        // Finally, copy the left over rwin that we lost due to the left shift above
        // We can ovewrite the other bits with 0 as they aren't really used in NKRO mode
        kbinput[14] = urbinput[1] >> 7;
    } else if(urbinput[0] == NKRO_MEDIA_IN && length == 3) {
        // This section is similar to handle_nkro_media_keys(), but with different indices due to the different keymap
        // This works because these keys can not be pressed at the same time
        CLEAR_KEYBIT(kbinput, 125);         // next
        CLEAR_KEYBIT(kbinput, 126);         // prev
        CLEAR_KEYBIT(kbinput, 123);         // stop
        CLEAR_KEYBIT(kbinput, 124);         // play

        CLEAR_KEYBIT(kbinput, 102);         // mute
        CLEAR_KEYBIT(kbinput, 103);         // volup
        CLEAR_KEYBIT(kbinput, 104);         // voldn

        // We only care about the first byte
        switch(urbinput[1]){
        case 181:
            SET_KEYBIT(kbinput, 125);   // next
            break;
        case 182:
            SET_KEYBIT(kbinput, 126);   // prev
            break;
        case 183:
            SET_KEYBIT(kbinput, 123);   // stop
            break;
        case 205:
            SET_KEYBIT(kbinput, 124);   // play
            break;
        case 226:
            SET_KEYBIT(kbinput, 102);   // mute
            break;
        case 233:
            SET_KEYBIT(kbinput, 103);   // volup
            break;
        case 234:
            SET_KEYBIT(kbinput, 104);   // voldn
            break;
        }
    } else {
        ckb_err("Invalid length %d and header 0x%hhx combination in handle_bragi_key_input()", length, urbinput[0]);
    }
}

// We just need the first few buttons only, so realistically it doesn't matter
#define BUTTON_BLD_COUNT 5

static inline void hid_mouse_bld_translate(usbinput* input, int length, const unsigned char* urbinput){
    if(length < 4){
        ckb_err("Invalid length %d", length);
        return;
    }
    // Byte 0 = mouse buttons (bitfield)
    for(int bit = 0; bit < BUTTON_BLD_COUNT; bit++){
        if(urbinput[0] & (1 << bit))
            SET_KEYBIT(input->keys, MOUSE_BUTTON_FIRST + bit);
        else
            CLEAR_KEYBIT(input->keys, MOUSE_BUTTON_FIRST + bit);
    }
    // Bytes 1 - 2: movement
    input->rel_x += (signed char)urbinput[1];
    input->rel_y += (signed char)urbinput[2];
    // Byte 3: wheel
    input->whl_rel_y = (signed char)urbinput[3];
}


void process_input_urb(void* context, unsigned char* buffer, int urblen, ushort ep){
    if(!urblen)
        return;

    usbdevice* kb = context;

#ifdef DEBUG_USB_INPUT
    print_urb_buffer("Input:", buffer, urblen, NULL, 0, NULL, INDEX_OF(kb, keyboard), (uchar)ep);
#endif

    // Get first byte of the response
    uchar firstbyte = buffer[0];

    // Legacy and NXP devices don't support children
    // so we don't use targetkb in code specific to those.
    // For when we explicitly need to address the dongle/root device, we also use plain `kb`

    // In HW mode, all wireless events come from the dongle device (0).
    // In SW mode, each (remappable) event comes from the appropriate child device.
    // Unremappable events such as mouse movement continue to come from the dongle device even in SW mode.
    //
    // We need to use heuristics to detect mouse and keyboard packets in both modes
    // to forward them to the appropriate child.
    //
    // All device specific events have the max length of bytes
    usbdevice* targetkb = kb;
    if(kb->protocol == PROTO_BRAGI && IS_DONGLE(kb)){
        pthread_mutex_lock(cmutex(kb));
        if(urblen == kb->out_ep_packet_size){
            // Extract the device id
            int devid = firstbyte & 0x7;
            if(devid && kb->children[devid - 1])
                targetkb = kb->children[devid - 1];
        } else {
            // Guess what device the event needs to go to

            // FIXME: We are only picking the first device for now.
            // We need to keep track of the mouse/keyboard children and use those pointers directly
            // to avoid a for loop with IS_MOUSE and IS_KEYBOARD for every single input packet.
            if(kb->children[0])
                targetkb = kb->children[0];
#ifdef DEBUG_USB_INPUT
            else
                ckb_err("kb->children[0] is NULL. This is expected if a bragi WL device was just turned off.");
#endif
        }
        pthread_mutex_unlock(cmutex(kb));
    }

    // If the response starts with CMD_GET (0x0e) for NXP, or it came from a bragi command EP, that means it needs to go to os_usbrecv()
    if(urblen == kb->out_ep_packet_size && (firstbyte == CMD_GET || (kb->protocol == PROTO_BRAGI && ep == kb->bragi_in_ep))){
#ifdef DEBUG_USB_RECV
    print_urb_buffer("Recv:", buffer, urblen, NULL, 0, NULL, INDEX_OF(targetkb, keyboard), (uchar)ep);
#endif
        int retval = pthread_mutex_lock(intmutex(targetkb));
        if(retval)
            ckb_fatal("Error locking interrupt mutex %i", retval);
        memcpy(targetkb->interruptbuf, buffer, kb->out_ep_packet_size);

        // signal os_usbrecv() that the data is ready.
        retval = pthread_cond_broadcast(intcond(targetkb));
        if(retval)
            ckb_fatal("Error broadcasting pthread cond %i", retval);
        retval = pthread_mutex_unlock(intmutex(targetkb));
        if(retval)
            ckb_fatal("Error unlocking interrupt mutex %i", retval);
    } else if (kb->protocol == PROTO_BRAGI && urblen == kb->out_ep_packet_size && buffer[1] == BRAGI_INPUT_NOTIFY){
        // Process bragi notifications
        bragi_process_notification(kb, targetkb, buffer);
    } else {
        queued_mutex_lock(imutex(targetkb));

        if(IS_LEGACY_DEV(kb)) {
            if(IS_MOUSE_DEV(kb))
                m95_mouse_translate(&kb->input, urblen, buffer);
            else
                hid_kb_translate(kb->input.keys, urblen, buffer, 1);
        } else {
            if(IS_MOUSE_DEV(targetkb)) {
                // HID Mouse Input
                // In HW mode, Bragi mouse is the same as NXP, but without a header
                if(kb->protocol == PROTO_BRAGI) {
                    if(targetkb->active) {
                        // When active, we need the movement from the standard hid packet, but the buttons from the extra packet
                        if(buffer[1] == BRAGI_INPUT_HID) {
                            if(urblen == 64)
                                corsair_bragi_mousecopy(targetkb, &targetkb->input, buffer);
                            else
                                ckb_err("Invalid length in corsair_bragi_mousecopy(). Expected 64, got %d", urblen);
                        } else {
                            targetkb->input.rel_x += (buffer[5] << 8) | buffer[4];
                            targetkb->input.rel_y += (buffer[7] << 8) | buffer[6];
                            // Some bragi devices do not report scrolling in the SW packet
                            // These type of hacks most likely apply to the dongle, and not the device itself in WL mode
                            // Despite that, use targetkb for now, just because corsair_bragi_mousecopy has subkb passed to it, performing the same check.
                            // If the above is in fact true, then we'll need a workaround.
                            if(SW_PKT_HAS_NO_WHEEL(targetkb))
                                targetkb->input.whl_rel_y = (signed char)buffer[8];
                        }
                    } else {
                        hid_mouse_translate(&targetkb->input, urblen, buffer);
                    }
                } else if(firstbyte == MOUSE_IN) {
                    // If we're in bootloader mode, parse the simplified reports
                    // This is only tested against an M65 with bld version 3
                    if(kb->needs_fw_update)
                        hid_mouse_bld_translate(&kb->input, urblen - 1, buffer + 1);
                    else
                        hid_mouse_translate(&kb->input, urblen - 1, buffer + 1);
                } else if(firstbyte == CORSAIR_IN) { // Corsair Mouse Input
                    corsair_mousecopy(kb->input.keys, buffer);
                } else if(firstbyte == 0x05 && urblen == 21) { // Seems to be on the Ironclaw RGB only
                    corsair_extended_mousecopy(kb->input.keys, buffer);
                } else {
                    ckb_err("Unknown mouse data received in input thread %02x from endpoint %02x", firstbyte, ep);
                }
            } else {
                // Assume Keyboard for everything else for now

                // We need to split the bragi code because the size check needs to go first
                if(kb->protocol == PROTO_BRAGI) {
                    if(urblen == 64) {
                        corsair_kbcopy(targetkb->input.keys, buffer + 2);
                        // Check if we need to apply an awful hack to get media keys working
                        if(BRAGI_HAS_MEDIA_MACRO(targetkb)){
                            // if Fn is pressed
                            if(ISSET_KEYBIT(targetkb->input.keys, 122)){
                                // As awful as this hack
                                bool matched = true;
                                if(ISSET_KEYBIT(targetkb->input.keys, 62)) { // F5 -> mute
                                    CLEAR_KEYBIT(targetkb->input.keys, 62);
                                    SET_KEYBIT(targetkb->input.keys, 102);
                                } else if (ISSET_KEYBIT(targetkb->input.keys, 64)) { // F7 -> voldn
                                    CLEAR_KEYBIT(targetkb->input.keys, 64);
                                    SET_KEYBIT(targetkb->input.keys, 104);
                                } else if (ISSET_KEYBIT(targetkb->input.keys, 65)) { // F8 -> volup
                                    CLEAR_KEYBIT(targetkb->input.keys, 65);
                                    SET_KEYBIT(targetkb->input.keys, 103);
                                } else if (ISSET_KEYBIT(targetkb->input.keys, 66)) { // F9 -> stop
                                    CLEAR_KEYBIT(targetkb->input.keys, 66);
                                    SET_KEYBIT(targetkb->input.keys, 123);
                                } else if (ISSET_KEYBIT(targetkb->input.keys, 67)) { // F10 -> prev
                                    CLEAR_KEYBIT(targetkb->input.keys, 67);
                                    SET_KEYBIT(targetkb->input.keys, 126);
                                } else if (ISSET_KEYBIT(targetkb->input.keys, 68)) { // F11 -> playpause
                                    CLEAR_KEYBIT(targetkb->input.keys, 68);
                                    SET_KEYBIT(targetkb->input.keys, 124);
                                } else if (ISSET_KEYBIT(targetkb->input.keys, 69)) { // F12 -> next
                                    CLEAR_KEYBIT(targetkb->input.keys, 69);
                                    SET_KEYBIT(targetkb->input.keys, 125);
                                } else if (ISSET_KEYBIT(targetkb->input.keys, 58)) { // F1 -> winlock
                                    CLEAR_KEYBIT(targetkb->input.keys, 58);
                                    SET_KEYBIT(targetkb->input.keys, 114);
                                } else if (ISSET_KEYBIT(targetkb->input.keys, 60)) { // F3 -> brightnessdn (not supported, map to generic "light")
                                    CLEAR_KEYBIT(targetkb->input.keys, 60);
                                    SET_KEYBIT(targetkb->input.keys, 113);
                                } else if (ISSET_KEYBIT(targetkb->input.keys, 61)) { // F4 -> brightnessup (not supported, map to generic "light")
                                    CLEAR_KEYBIT(targetkb->input.keys, 61);
                                    SET_KEYBIT(targetkb->input.keys, 113);
                                } else {
                                    matched = false;
                                }
                                if(matched)
                                    CLEAR_KEYBIT(targetkb->input.keys, 122);
                            }
                        }
                    } else if(firstbyte == NKRO_KEY_IN || firstbyte == NKRO_MEDIA_IN) {
                        if(!targetkb->active)
                            handle_bragi_key_input(targetkb->input.keys, buffer, urblen);
                    } else {
                        ckb_err("Unknown bragi data received in input thread %02x from endpoint %02x", firstbyte, ep);
                    }
                } else {
#define NXP_STRAFE_MEDIA_WORKAROUND(kb) (kb->vendor == V_CORSAIR && kb->product == P_STRAFE)
#define NXP_STRAFE_MEDIA_MASK 0x3e
//#define NXP_STRAFE_VOL_MASK 0xc
                    // Accept NKRO only if device is not active, unless it's media keys and it's an original strafe with the media key bug
                    if(firstbyte == NKRO_KEY_IN || firstbyte == NKRO_MEDIA_IN) {
                        if(!targetkb->active || (firstbyte == NKRO_MEDIA_IN && NXP_STRAFE_MEDIA_WORKAROUND(targetkb)))
                            hid_kb_translate(targetkb->input.keys, urblen, buffer, 0);
                    } else if(urblen == MSG_SIZE) {
                        if((kb->fwversion >= 0x130 || IS_V2_OVERRIDE(kb)) && firstbyte == CORSAIR_IN) // Ugly hack due to old FW 1.15 packets having no report id
                            buffer++;
                        // If it's an original strafe RGB, to work around the media keys getting stuck down on the 03 packet,
                        // we save the bits before the kbcopy and then restore them afterwards for the 02 packet to take effect.
                        if(NXP_STRAFE_MEDIA_WORKAROUND(targetkb)) {
                            // Store
                            uchar media_key_state_1 = targetkb->input.keys[12];
                            uchar media_key_state_2 = targetkb->input.keys[16];
                            // Apply fresh key data
                            corsair_kbcopy(targetkb->input.keys, buffer);
                            // Restore
                            targetkb->input.keys[12] = (targetkb->input.keys[12] & ~NXP_STRAFE_MEDIA_MASK) | (media_key_state_1 & NXP_STRAFE_MEDIA_MASK);
                            //targetkb->input.keys[16] = (targetkb->input.keys[16] & ~NXP_STRAFE_VOL_MASK) | (media_key_state_2 & NXP_STRAFE_VOL_MASK);
                            // We can just copy this byte as-is, as there are no M or G keys in the strafe.
                            targetkb->input.keys[16] = media_key_state_2;
                        } else {
                            corsair_kbcopy(targetkb->input.keys, buffer);
                        }
                    } else {
                        ckb_err("Unknown data received in input thread %02x from endpoint %02x", firstbyte, ep);
                    }
                }
            }
        }
        ///
        /// The input data is transformed and copied to the kb structure. Now give it to the OS and unlock the imutex afterwards.
        inputupdate(targetkb);
        queued_mutex_unlock(imutex(targetkb));
    }
}

void handle_nkro_key_input(unsigned char* kbinput, const unsigned char* urbinput, int length, int legacy){
    int bytelen = (legacy ? 14 : 19);
    int start = !legacy; // Legacy packets start from 0x00, other ones start from 0x01
    if(length < start + bytelen + 1){
        ckb_err("Invalid length %d legacy %d", length, legacy);
        return;
    }

    for(int bit = 0; bit < 8; bit++){
        if((urbinput[start] >> bit) & 1)
            SET_KEYBIT(kbinput, hid_codes[bit + 224]);
        else
            CLEAR_KEYBIT(kbinput, hid_codes[bit + 224]);
    }

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

void hid_mouse_translate(usbinput* input, int length, const unsigned char* urbinput){
    if(length < 9){
        ckb_err("Invalid length %d", length);
        return;
    }
    // Byte 0 = mouse buttons (bitfield)
    for(int bit = 0; bit < BUTTON_HID_COUNT; bit++){
        if(urbinput[0] & (1 << bit))
            SET_KEYBIT(input->keys, MOUSE_BUTTON_FIRST + bit);
        else
            CLEAR_KEYBIT(input->keys, MOUSE_BUTTON_FIRST + bit);
    }
    // Bytes 5 - 8: movement
    input->rel_x += (urbinput[5] << 8) | urbinput[4];
    input->rel_y += (urbinput[7] << 8) | urbinput[6];
    // Byte 9: wheel
    input->whl_rel_y = (signed char)urbinput[8];
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

void m95_mouse_translate(usbinput* kbinput, int length, const unsigned char* urbinput){
    if(length != 7)
        return;
    unsigned short input = (((unsigned short)urbinput[1]) << 8) | urbinput[0];
    for(int bit = 0; bit < 16; bit++){
        unsigned char current_bit = (input >> bit) & 1;
        if(current_bit)
            SET_KEYBIT(kbinput->keys, MOUSE_BUTTON_FIRST + bit);
        else
            CLEAR_KEYBIT(kbinput->keys, MOUSE_BUTTON_FIRST + bit);
    }

    kbinput->rel_x += (urbinput[3] << 8) | urbinput[2];
    kbinput->rel_y += (urbinput[5] << 8) | urbinput[4];
    kbinput->whl_rel_y = (signed char)urbinput[6];
}

#define BRAGI_MOUSE_BUTTONS 16
#define BRAGI_ONE_BYTE_MOUSE_BUTTONS 8
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

// DPI cycle/up and forwards are just swapped compared to corsair_bragi_lut
const unsigned char harpoon_wl_lut[BRAGI_ONE_BYTE_MOUSE_BUTTONS] = {
    0x00,
    0x01,
    0x02,
    0x05,
    0x03,
    0x04,
    0x06, // This and the one below are unused
    0x08, // but we need them to be non zero so that they don't undo left click
};

// DPI cycle/up and forwards are just swapped compared to corsair_bragi_lut
const unsigned char m55_wl_lut[BRAGI_ONE_BYTE_MOUSE_BUTTONS] = {
    0x00,
    0x01,
    0x02,
    0x04,
    0x03,
    0x06, //forwards
    0x05, //backwards
    0x08, //dpi up?
};

void corsair_bragi_mousecopy(usbdevice* kb, usbinput* input, const unsigned char* urbinput){
    // Increment this only once, as the loop below will increment it the first time as well
    // to skip the 00 02 header.
    urbinput++;

    int buttons = BRAGI_MOUSE_BUTTONS;

    // Some devices only have one byte, so set those to 8 buttons
    // We need a better way to identify this
    if(kb->vendor == V_CORSAIR && (kb->product == P_M55_RGB_PRO || kb->product == P_DARK_CORE_RGB_PRO_SE || kb->product == P_DARK_CORE_RGB_PRO_SE_WL || kb->product == P_HARPOON_WL_U || kb->product == P_DARK_CORE_RGB_PRO || kb->product == P_DARK_CORE_RGB_PRO_WL))
        buttons = BRAGI_ONE_BYTE_MOUSE_BUTTONS;

    // Pick the appropriate LUT. We can't patch the keymap as that will break standard HID input.
    const unsigned char* lut = corsair_bragi_lut;
    if(kb->vendor == V_CORSAIR && kb->product == P_HARPOON_WL_U)
        lut = harpoon_wl_lut;
    else if(kb->vendor == V_CORSAIR && kb->product == P_M55_RGB_PRO)
        lut = m55_wl_lut;

    for(int bit = 0; bit < buttons; bit++){
        int bitinbyte = bit % 8;
        // On every byte change, increment the input packet pointer
        if(bitinbyte == 0)
            urbinput++;

        uchar test = 1 << bitinbyte;
        if(*urbinput & test)
            SET_KEYBIT(input->keys, MOUSE_BUTTON_FIRST + lut[bit]);
        else
            CLEAR_KEYBIT(input->keys, MOUSE_BUTTON_FIRST + lut[bit]);
    }

    // Do not try to read the wheel from the SW packet if there's no data for it
    if(SW_PKT_HAS_NO_WHEEL(kb))
        return;

    // Read the wheel data that's located right after the button data.
    urbinput++;
    input->whl_rel_y = (signed char)*urbinput;
}


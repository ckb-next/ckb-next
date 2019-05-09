#include "usb.h"
#include "keymap_patch.h"
#include "keymap.h"

// The Alt code is used for Fn in the K65
keypatch k65patch[] = {
    { 146, "fn", 0x59, KEY_FN },
};
#define K65PATCH_LEN sizeof(k65patch)/sizeof(*k65patch)

// Vol Up/Dn LED addresses for K63 and K68.
// Possibly for all devices that have LEDs on vol up/dn
keypatch k63patch[] = {
    { 130, "volup", 0x82, KEY_VOLUMEUP },
    { 131, "voldn", 0x8e, KEY_VOLUMEDOWN },
};
#define K63PATCH_LEN sizeof(k63patch)/sizeof(*k63patch)

keypatch m95patch[] = {
    { 208, "mouse7", -1, SCAN_MOUSE | BTN_BACK },
    { 209, "mouse6", -1, SCAN_MOUSE | BTN_FORWARD },
    { 210, "thumb6", -1, KEY_CORSAIR },
    { 211, "dpiup",  -1, KEY_CORSAIR },
    { 212, "dpidn",  -1, KEY_CORSAIR },
    { 218, "sniper", -1, KEY_CORSAIR },

};
#define M95PATCH_LEN sizeof(m95patch)/sizeof(*m95patch)

keypatch legionpatch[] = {
    {  0,    "esc", 0x6e, KEY_ESC },
    {  1,     "f1", 0x70, KEY_F1 },
    {  2,     "f2", 0x71, KEY_F2 },
    {  3,     "f3", 0x72, KEY_F3 },
    {  4,     "f4", 0x73, KEY_F4 },
    {  5,     "f5", 0x74, KEY_F5 },
    {  6,     "f6", 0x75, KEY_F6 },
    {  7,     "f7", 0x76, KEY_F7 },
    {  8,     "f8", 0x77, KEY_F8 },
    {  9,     "f9", 0x78, KEY_F9 },
    { 10,    "f10", 0x79, KEY_F10 },
    { 11,    "f11", 0x7a, KEY_F11 },
    { 12,  "grave", 0x01, KEY_GRAVE },
    { 13,      "1", 0x02, KEY_1 },
    { 14,      "2", 0x03, KEY_2 },
    { 15,      "3", 0x04, KEY_3 },
    { 16,      "4", 0x05, KEY_4 },
    { 17,      "5", 0x06, KEY_5 },
    { 18,      "6", 0x07, KEY_6 },
    { 19,      "7", 0x08, KEY_7 },
    { 20,      "8", 0x09, KEY_8 },
    { 21,      "9", 0x0a, KEY_9 },
    { 22,      "0", 0x0b, KEY_0 },
    { 23,  "minus", 0x0c, KEY_MINUS },
    { 24,    "tab", 0x10, KEY_TAB },
    { 25,      "q", 0x11, KEY_Q },
    { 26,      "w", 0x12, KEY_W },
    { 27,      "e", 0x13, KEY_E },
    { 28,      "r", 0x14, KEY_R },
    { 29,      "t", 0x15, KEY_T },
    { 30,      "y", 0x16, KEY_Y },
    { 31,      "u", 0x17, KEY_U },
    { 32,      "i", 0x18, KEY_I },
    { 33,      "o", 0x19, KEY_O },
    { 34,      "p", 0x1a, KEY_P },
    { 35, "lbrace", 0x1b, KEY_LEFTBRACE },
    { 36,   "caps", 0x8d, KEY_CAPSLOCK },
    { 37,      "a", 0x1f, KEY_A },
    { 38,      "s", 0x20, KEY_S },
    { 39,      "d", 0x21, KEY_D },
    { 40,      "f", 0x22, KEY_F },
    { 41,      "g", 0x23, KEY_F },
    { 42,      "h", 0x24, KEY_H },
    { 43,      "j", 0x25, KEY_J },
    { 44,      "k", 0x26, KEY_K },
    { 45,      "l", 0x27, KEY_L },
    { 46,  "colon", 0x28, KEY_SEMICOLON },
    { 47,  "quote", 0x29, KEY_APOSTROPHE },
    { 48, "lshift", 0x2c, KEY_LEFTSHIFT },
    { 29, "bslash_iso", 0x8e, KEY_102ND },
    { 50,      "z", 0x2e, KEY_Z },
    { 51,      "x", 0x2f, KEY_X },
    { 52,      "c", 0x30, KEY_X },
    { 53,      "v", 0x31, KEY_V },
    { 54,      "b", 0x32, KEY_B },
    { 55,      "n", 0x33, KEY_N },
    { 56,      "m", 0x34, KEY_M },
    { 57,  "comma", 0x35, KEY_COMMA },
    { 58,    "dot", 0x36, KEY_DOT },
    { 59,  "slash", 0x37, KEY_SLASH },
    { 60,  "lctrl", 0x3a, KEY_LEFTCTRL },
    { 61,   "lwin", 0x7f, KEY_LEFTMETA },
    { 62,   "lalt", 0x3c, KEY_LEFTALT },
    // 63 KEY_NONE
    { 63, "rshift2", 0x90, KEY_NONE },
    { 64,  "space", 0x3d, KEY_SPACE },
    // 65 KEY_NONE
    { 65, "space2", 0x91, KEY_NONE },
    // 66 KEY_KATAKANAHIRAGANA
    { 67,   "ralt", 0x3e, KEY_RIGHTALT },
    // 68 KEY_RIGHTMETA
    // 69 KEY_COMPOSE
    // [...]
    { 72,    "f12", 0x7b, KEY_F12 },
    // [...]
    { 79, "rbrace", 0x1c, KEY_RIGHTBRACE },
    { 80, "bslash", 0x1d, KEY_BACKSLASH },
    { 81,   "hash", 0x8f, KEY_BACKSLASH_ISO },
    { 82,  "enter", 0x2b, KEY_ENTER },
    // RO
    { 84,  "equal", 0x0d, KEY_EQUAL },
    // YEN
    { 86,  "bspace", 0x0f, KEY_BACKSPACE },
    { 87,     "del", 0x4c, KEY_DELETE },
    // [..]
    { 90,  "rshift", 0x39, KEY_RIGHTSHIFT },
    { 91,   "rctrl", 0x40, KEY_RIGHTCTRL },
    { 92,      "up", 0x53, KEY_UP },
    { 93,    "left", 0x4f, KEY_LEFT },
    { 94,    "down", 0x54, KEY_DOWN },
    { 95,   "right", 0x59, KEY_RIGHT },
    // [..]
    { 120,     "g1", 0xc1, KEY_NONE },
    { 121,     "g2", 0xc2, KEY_NONE },
    { 122,     "g3", 0xc3, KEY_NONE },
    { 123,     "g4", 0xc4, KEY_NONE },
    { 124,     "g5", 0xc5, KEY_NONE },
    { 125,     "g6", 0xc6, KEY_NONE },
    { 126,    "fan", 0x99, KEY_NONE }, // g7
    { 127,  "ports", 0x98, KEY_NONE }, // g8
    // [..]
    { 146,     "fn", 0x3b, KEY_FN },
    // [..]
    { 173,   "logo", 0x96, KEY_NONE },

// enter (0x8f) == hash
// lshift (0x8e) == bslash_iso
// space (0x91) == space2
// rhsift2 == rshift (0x90)
};

#define LEGIONPATCH_LEN  sizeof(legionpatch)/sizeof(*legionpatch)

keypatches mappatches[] = {
    { P_K68,           k63patch, K63PATCH_LEN },
    { P_K68_NRGB,      k63patch, K63PATCH_LEN },
    { P_K65,           k65patch, K65PATCH_LEN },
    { P_K65_LEGACY,    k65patch, K65PATCH_LEN },
    { P_K63_NRGB,      k63patch, K63PATCH_LEN },
    { P_M95,           m95patch, M95PATCH_LEN },
    { P_Y730,       legionpatch, LEGIONPATCH_LEN },
};
#define KEYPATCHES_LEN sizeof(mappatches)/sizeof(*mappatches)

/// \brief
///
/// Copy the keymap to the usbdevice struct, and iterate through the keypatches array.
/// If a match is found, patch the device-specific keymap in usbdevice.
void patchkeys(usbdevice* kb){
    // Copy the default keymap over
    kb->keymap = malloc(sizeof(keymap));
    memcpy(kb->keymap, keymap, sizeof(keymap));
    // Iterate through the patches for all devices
    for(size_t pos = 0; pos < KEYPATCHES_LEN; pos++){
        if(mappatches[pos].product == kb->product){
            // Iterate through the keys in the selected patch
            for(size_t i = 0; i < mappatches[pos].patchlen; i++){
                keypatch* curpatch = mappatches[pos].patch;
                int idx = curpatch[i].idx;
                kb->keymap[idx].name = curpatch[i].name;
                kb->keymap[idx].led = curpatch[i].led;
                kb->keymap[idx].scan = curpatch[i].scan;
            }
            return;
        }
    }
}

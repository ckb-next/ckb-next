#include "usb.h"
#include "keymap_patch.h"
#include "keymap.h"

// Vol Up/Dn LED addresses for K63 and K68.
// Possibly for all devices that have LEDs on vol up/dn
static const keypatch k63patch[] = {
    { 130, "volup", 0x82, KEY_VOLUMEUP },
    { 131, "voldn", 0x8e, KEY_VOLUMEDOWN },
};

static const keypatch m55patch[] = {
    { 210, "mouse6", -1,        KEY_CORSAIR },
    { 211, "mouse7", -1,        KEY_CORSAIR },
    { 213, "dpiup",  -1,        KEY_CORSAIR },
    { 237, "dpi",    LED_MOUSE, KEY_NONE },
};

static const keypatch m95patch[] = {
    { 208, "mouse7", -1, SCAN_MOUSE | BTN_BACK },
    { 209, "mouse6", -1, SCAN_MOUSE | BTN_FORWARD },
    { 210, "thumb6", -1, KEY_CORSAIR },
    { 211, "dpiup",  -1, KEY_CORSAIR },
    { 212, "dpidn",  -1, KEY_CORSAIR },
    { 218, "sniper", -1, KEY_CORSAIR },

};

static const keypatch icwpatch[] = {
    { 237, "back",  LED_MOUSE,     KEY_NONE },
    { 238, "wheel", LED_MOUSE + 1, KEY_NONE },
    { 239, "front", LED_MOUSE + 2, KEY_NONE },
    { 240, "dpi0",  LED_MOUSE + 3, KEY_NONE },
    { 241, "dpi1",  LED_MOUSE + 4, KEY_NONE },
    { 242, "dpi2",  LED_MOUSE + 5, KEY_NONE },
};

static const keypatch harpoonwlpatch[] = {
    { 237, "dpi",  LED_MOUSE,     KEY_NONE },
    { 238, "back", LED_MOUSE + 1, KEY_NONE },
};

static const keypatch katarproxtpatch[] = {
    { 237, "dpi",  LED_MOUSE,     KEY_NONE },
};

static const keypatch k95legacypatch[] = {
    { 111, "lghtpgm",  0,     KEY_CORSAIR }, // Lighting program key for legacy devices
};

// Shift all generic zones by one and move winlock to 0
static const keypatch k55propatch[] = {
    { 114,  "lock", 0, KEY_CORSAIR },
    { 190, "zone1", 1, KEY_NONE },
    { 191, "zone2", 2, KEY_NONE },
    { 192, "zone3", 3, KEY_NONE },
    { 193, "zone4", 4, KEY_NONE },
    { 194, "zone5", 5, KEY_NONE },
};

// Just winlock
static const keypatch k55proxtpatch[] = {
    { 114,  "lock", 0, KEY_CORSAIR },
};

static const keypatch DCRGBPpatch[] = {
    { 228, "mouse4",  LED_MOUSE + 2, SCAN_MOUSE | BTN_SIDE  }, // "bar1"
    { 229, "mouse5",  LED_MOUSE + 1, SCAN_MOUSE | BTN_EXTRA }, // "bar0"
    { 233, "profdn",             -1, KEY_CORSAIR },
    { 258, "back",    LED_MOUSE + 6, KEY_NONE }, // "logo"
    { 260, "wheel",   LED_MOUSE + 0, KEY_NONE },
    { 261, "thumb",   LED_MOUSE + 5, KEY_NONE },
    { 262, "side",    LED_MOUSE + 7, KEY_NONE },
    { 263, "dpiw0",   LED_MOUSE + 11, KEY_NONE }, // sniper led
    { 264, "dpiw1",   LED_MOUSE + 8, KEY_NONE },
    { 265, "dpiw2",   LED_MOUSE + 9, KEY_NONE },
    { 266, "dpiw3",   LED_MOUSE +10, KEY_NONE },
    { 267, "bar3",    LED_MOUSE + 3, KEY_NONE }, // "bar2"
    { 268, "bar4",    LED_MOUSE + 4, KEY_NONE }, // "bar3"
};

keypatch k100patch[] = {
    {114, "lock", 114, KEY_CORSAIR },
};

keypatch k70tklpatch[] = {
    { 114, "lock",    114, KEY_CORSAIR },
    {   1, "logo",      1, KEY_NONE },
};

keypatch k70propatch[] = {
    { 114, "lock",    114, KEY_CORSAIR },
};


#define ADD_PATCH(vendor, product, patch) \
    { (vendor), (product), (patch), sizeof(patch)/sizeof(*patch) }

static const keypatches mappatches[] = {
    ADD_PATCH(V_CORSAIR, P_K68,          k63patch),
    ADD_PATCH(V_CORSAIR, P_K68_NRGB,     k63patch),
    ADD_PATCH(V_CORSAIR, P_K63_NRGB,     k63patch),
    ADD_PATCH(V_CORSAIR, P_M55_RGB_PRO,  m55patch),
    ADD_PATCH(V_CORSAIR, P_M95,          m95patch),
    ADD_PATCH(V_CORSAIR, P_IRONCLAW_W_U, icwpatch),
    ADD_PATCH(V_CORSAIR, P_HARPOON_WL_U, harpoonwlpatch),
    ADD_PATCH(V_CORSAIR, P_KATAR_PRO_XT, katarproxtpatch),
    ADD_PATCH(V_CORSAIR, P_KATAR_PRO,    katarproxtpatch),
    ADD_PATCH(V_CORSAIR, P_K95_LEGACY,   k95legacypatch),
    ADD_PATCH(V_CORSAIR, P_K55_PRO,      k55propatch),
    ADD_PATCH(V_CORSAIR, P_K55_PRO_XT,   k55proxtpatch),
    ADD_PATCH(V_CORSAIR, P_DARK_CORE_RGB_PRO,   DCRGBPpatch),
    ADD_PATCH(V_CORSAIR, P_DARK_CORE_RGB_PRO_SE,   DCRGBPpatch),
    ADD_PATCH(V_CORSAIR, P_K100_OPTICAL,         k100patch),
    ADD_PATCH(V_CORSAIR, P_K100_MECHANICAL,         k100patch),
    ADD_PATCH(V_CORSAIR, P_K100_OPTICAL_VARIANT,         k100patch),
    ADD_PATCH(V_CORSAIR, P_K70_TKL,      k70tklpatch),
    ADD_PATCH(V_CORSAIR, P_K70_PRO, k70propatch)
};

#define KEYPATCHES_LEN sizeof(mappatches)/sizeof(*mappatches)

#define CLEAR_KEYMAP_ENTRY(x)   { \
    (x).name = NULL; \
    (x).led = -1; \
    (x).scan = KEY_NONE; \
}

/// \brief
///
/// Copy the keymap to the usbdevice struct, and iterate through the keypatches array.
/// If a match is found, patch the device-specific keymap in usbdevice.
void patchkeys(usbdevice* kb){
    // Copy the default keymap over
    memcpy(kb->keymap, keymap, sizeof(keymap));

    // Check if we need to patch the bragi base map
    if(kb->protocol == PROTO_BRAGI)
        memcpy(kb->keymap, keymap_bragi, sizeof(keymap_bragi));

    // If it's a mouse, patch in profswitch
    // This needs to be done after the bragi keymap patch
    // as the position of profswitch differs
    if(IS_MOUSE_DEV(kb)){
#define PROFSWITCH_M_IDX (MOUSE_BUTTON_FIRST + 24)
        int profswitch_kb_idx = -1;
        for(int i = 0; i < LED_GENERIC_FIRST; i++) {
            if(!kb->keymap[i].name)
                continue;
            if(!strcmp(kb->keymap[i].name, "profswitch")) {
                profswitch_kb_idx = i;
                break;
            }
        }

        if(profswitch_kb_idx < 0) {
            ckb_err("Could not find profswitch in base keymap. Not patching profswitch for mouse.");
        } else {
            kb->keymap[PROFSWITCH_M_IDX].name = kb->keymap[profswitch_kb_idx].name;
            kb->keymap[PROFSWITCH_M_IDX].led = -1;
            kb->keymap[PROFSWITCH_M_IDX].scan = KEY_CORSAIR;
            // Clear the keyboard one
            CLEAR_KEYMAP_ENTRY(kb->keymap[profswitch_kb_idx]);
        }
    }

    // Iterate through the patches for all devices
    for(size_t pos = 0; pos < KEYPATCHES_LEN; pos++){
        if(mappatches[pos].vendor == kb->vendor && mappatches[pos].product == kb->product){
            // Iterate through the keys in the selected patch
            for(size_t i = 0; i < mappatches[pos].patchlen; i++){
                const keypatch* curpatch = mappatches[pos].patch;
                size_t idx = curpatch[i].idx;
                kb->keymap[idx].name = curpatch[i].name;
                kb->keymap[idx].led = curpatch[i].led;
                kb->keymap[idx].scan = curpatch[i].scan;
                // Now go through the full keymap so far and remove any entries that
                // either the have same LED as what was patched in, or same name
                for(size_t j = 0; j < sizeof(keymap)/sizeof(*keymap); j++){
                    // Don't delete the freshly patched entry
                    if(j == idx)
                        continue;
                    // Name first
                    if(kb->keymap[j].name && curpatch[i].name && !strcmp(kb->keymap[j].name, curpatch[i].name))
                        CLEAR_KEYMAP_ENTRY(kb->keymap[j]);
                    // LED index
                    // In this case, only reset the LED, as there's potential to break keybinds
                    if(kb->keymap[j].led >= 0 && curpatch[i].led >= 0 && kb->keymap[j].led == curpatch[i].led)
                        kb->keymap[j].led = -1;
                }
            }
            return;
        }
    }
}

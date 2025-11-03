#include "usb.h"
#include "keymap_patch.h"
#include "keymap.h"

// Vol Up/Dn LED addresses for K63 and K68.
// Possibly for all devices that have LEDs on vol up/dn
static const keypatch k63patch[] = {
    { 130, "volup", 0x82, KEY_VOLUMEUP },
    { 131, "voldn", 0x8e, KEY_VOLUMEDOWN },
};

// +20 because of ff88b9bd7ebbe ("Add K100 to GUI and daemon")
// We need to fix this keymap mess ASAP
static const keypatch m55patch[] = {
    { 210+20, "mouse6", -1,        KEY_CORSAIR },
    { 211+20, "mouse7", -1,        KEY_CORSAIR },
    { 213+20, "dpiup",  -1,        KEY_CORSAIR },
    { 237+20, "dpi",    LED_MOUSE, KEY_NONE },
};

static const keypatch m95patch[] = {
    { 208+20, "mouse7", -1, SCAN_MOUSE | BTN_BACK },
    { 209+20, "mouse6", -1, SCAN_MOUSE | BTN_FORWARD },
    { 210+20, "thumb6", -1, KEY_CORSAIR },
    { 211+20, "dpiup",  -1, KEY_CORSAIR },
    { 212+20, "dpidn",  -1, KEY_CORSAIR },
    { 218+20, "sniper", -1, KEY_CORSAIR },

};

static const keypatch icwpatch[] = {
    { 237+20, "back",  LED_MOUSE,     KEY_NONE },
    { 238+20, "wheel", LED_MOUSE + 1, KEY_NONE },
    { 239+20, "front", LED_MOUSE + 2, KEY_NONE },
    { 240+20, "dpi0",  LED_MOUSE + 3, KEY_NONE },
    { 241+20, "dpi1",  LED_MOUSE + 4, KEY_NONE },
    { 242+20, "dpi2",  LED_MOUSE + 5, KEY_NONE },
};

static const keypatch harpoonwlpatch[] = {
    { 237+20, "dpi",  LED_MOUSE,     KEY_NONE },
    { 238+20, "back", LED_MOUSE + 1, KEY_NONE },
};

static const keypatch katarproxtpatch[] = {
    { 237+20, "dpi",  LED_MOUSE,     KEY_NONE },
};

static const keypatch k95legacypatch[] = {
    { 111, "lghtpgm",  0,     KEY_CORSAIR }, // Lighting program key for legacy devices
};

// Shift all generic zones by one and move winlock to 0
static const keypatch k55propatch[] = {
    { 114,  "lock", 0, KEY_CORSAIR },
    { 190+20, "zone1", 1, KEY_NONE },
    { 191+20, "zone2", 2, KEY_NONE },
    { 192+20, "zone3", 3, KEY_NONE },
    { 193+20, "zone4", 4, KEY_NONE },
    { 194+20, "zone5", 5, KEY_NONE },
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

static const keypatch k100patch[] = {
    {114, "lock", 114, KEY_CORSAIR },
};

static const keypatch k70tklpatch[] = {
    { 114, "lock",    114, KEY_CORSAIR },
    {   1, "logo",      1, KEY_NONE },
};

static const keypatch k70propatch[] = {
    { 114, "lock",    114, KEY_CORSAIR },
    { 138, "logo",    138, KEY_NONE },
};

static const keypatch k70corergbpatch[] = {
    { 124, "mr",    -1, KEY_CORSAIR },
    { 127, "play",  -1, KEY_PLAY },
    { 129, "mute",  -1, KEY_MUTE },
};

static const keypatch scimitarbragipatch[] = {
    { 237+20, "front",  LED_MOUSE + 2,     KEY_NONE },
    { 238+20, "back",   LED_MOUSE,         KEY_NONE },
    { 239+20, "dpi",    LED_MOUSE + 4,     KEY_NONE },
    { 240+20, "wheel",  LED_MOUSE + 1,     KEY_NONE },
    { 241+20, "thumb",  LED_MOUSE + 3,     KEY_NONE },
};

static const keypatch k95platinumxtpatch[] = {
    // Shift the topbar LEDs by one compared to the K100 on the keymap
    { 137, NULL,             -1, KEY_NONE },
    { 138, "topbar1",       137, KEY_NONE },
    { 139, "topbar2",       138, KEY_NONE },
    { 140, "topbar3",       139, KEY_NONE },
    { 141, "topbar4",       140, KEY_NONE },
    { 142, "topbar5",       141, KEY_NONE },
    { 143, "topbar6",       142, KEY_NONE },
    { 144, "topbar7",       143, KEY_NONE },
    { 145, "topbar8",       144, KEY_NONE },
    { 146, "topbar9",       145, KEY_NONE },
    { 147, "topbar10",      146, KEY_NONE },
    { 148, "topbar11",      147, KEY_NONE },
    { 149, "topbar12",      148, KEY_NONE },
    { 150, "topbar13",      149, KEY_NONE },
    { 151, "topbar14",      150, KEY_NONE },
    { 152, "topbar15",      151, KEY_NONE },
    { 153, "topbar16",      152, KEY_NONE },
    { 154, "topbar17",      153, KEY_NONE },
    { 155, "topbar18",      154, KEY_NONE },
    { 156, "topbar19",      155, KEY_NONE },
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
    ADD_PATCH(V_CORSAIR, P_K70_TKL,       k70tklpatch),
    ADD_PATCH(V_CORSAIR, P_K70_TKL_CHAMP_OPTIC, k70tklpatch),
    ADD_PATCH(V_CORSAIR, P_K70_PRO,       k70propatch),
    ADD_PATCH(V_CORSAIR, P_K70_PRO_OPTIC, k70propatch),
    ADD_PATCH(V_CORSAIR, P_K70_CORE_RGB, k70corergbpatch),
    ADD_PATCH(V_CORSAIR, P_K70_CORE_RGB_2, k70corergbpatch),
    ADD_PATCH(V_CORSAIR, P_K70_CORE_RGB_3, k70corergbpatch),
    ADD_PATCH(V_CORSAIR, P_SCIMITAR_ELITE_BRAGI, scimitarbragipatch),
    ADD_PATCH(V_CORSAIR, P_K95_PLATINUM_XT, k95platinumxtpatch),
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

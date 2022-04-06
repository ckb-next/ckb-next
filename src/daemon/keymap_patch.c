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

keypatch m55patch[] = {
    { 210, "mouse6", -1,        KEY_CORSAIR },
    { 211, "mouse7", -1,        KEY_CORSAIR },
    { 213, "dpiup",  -1,        KEY_CORSAIR },
    { 237, "dpi",    LED_MOUSE, KEY_NONE },
    { 239, NULL,     0,         KEY_NONE },
};
#define M55PATCH_LEN sizeof(m55patch)/sizeof(*m55patch)

keypatch m95patch[] = {
    { 208, "mouse7", -1, SCAN_MOUSE | BTN_BACK },
    { 209, "mouse6", -1, SCAN_MOUSE | BTN_FORWARD },
    { 210, "thumb6", -1, KEY_CORSAIR },
    { 211, "dpiup",  -1, KEY_CORSAIR },
    { 212, "dpidn",  -1, KEY_CORSAIR },
    { 218, "sniper", -1, KEY_CORSAIR },

};
#define M95PATCH_LEN sizeof(m95patch)/sizeof(*m95patch)

keypatch icwpatch[] = {
    { 237, "back",  LED_MOUSE,     KEY_NONE },
    { 238, "wheel", LED_MOUSE + 1, KEY_NONE },
    { 239, "front", LED_MOUSE + 2, KEY_NONE },
    { 240, "dpi0",  LED_MOUSE + 3, KEY_NONE },
    { 241, "dpi1",  LED_MOUSE + 4, KEY_NONE },
    { 242, "dpi2",  LED_MOUSE + 5, KEY_NONE },
    { 243, NULL,    0,             KEY_NONE },
    { 244, NULL,    0,             KEY_NONE },
    { 245, NULL,    0,             KEY_NONE },
    { 246, NULL,    0,             KEY_NONE },
};
#define ICWPATCH_LEN sizeof(icwpatch)/sizeof(*icwpatch)

keypatch harpoonwlpatch[] = {
    { 237, "dpi",  LED_MOUSE,     KEY_NONE },
    { 238, "back", LED_MOUSE + 1, KEY_NONE },
    { 239, NULL,   0,             KEY_NONE },
    { 240, NULL,   0,             KEY_NONE },
    { 241, NULL,   0,             KEY_NONE },
    { 242, NULL,   0,             KEY_NONE },
    { 243, NULL,   0,             KEY_NONE },
    { 244, NULL,   0,             KEY_NONE },
    { 245, NULL,   0,             KEY_NONE },
    { 246, NULL,   0,             KEY_NONE },
};
#define HARPOONWLPATCH_LEN sizeof(harpoonwlpatch)/sizeof(*harpoonwlpatch)

keypatch katarproxtpatch[] = {
    { 237, "dpi",  LED_MOUSE,     KEY_NONE },
    { 238, NULL,   0,             KEY_NONE },
    { 239, NULL,   0,             KEY_NONE },
    { 240, NULL,   0,             KEY_NONE },
    { 241, NULL,   0,             KEY_NONE },
    { 242, NULL,   0,             KEY_NONE },
    { 243, NULL,   0,             KEY_NONE },
    { 244, NULL,   0,             KEY_NONE },
    { 245, NULL,   0,             KEY_NONE },
    { 246, NULL,   0,             KEY_NONE },
};
#define KATARPROXTPATCH_LEN sizeof(katarproxtpatch)/sizeof(*katarproxtpatch)

keypatch k95legacypatch[] = {
    { 111, "lghtpgm",  0,     KEY_CORSAIR }, // Lighting program key for legacy devices
};
#define K95LEGACYPATCH_LEN sizeof(k95legacypatch)/sizeof(*k95legacypatch)

keypatches mappatches[] = {
    { V_CORSAIR, P_K68,          k63patch,       K63PATCH_LEN },
    { V_CORSAIR, P_K68_NRGB,     k63patch,       K63PATCH_LEN },
    { V_CORSAIR, P_K65,          k65patch,       K65PATCH_LEN },
    { V_CORSAIR, P_K65_LEGACY,   k65patch,       K65PATCH_LEN },
    { V_CORSAIR, P_K63_NRGB,     k63patch,       K63PATCH_LEN },
    { V_CORSAIR, P_M55_RGB_PRO,  m55patch,       M55PATCH_LEN },
    { V_CORSAIR, P_M95,          m95patch,       M95PATCH_LEN },
    { V_CORSAIR, P_IRONCLAW_W_U, icwpatch,       ICWPATCH_LEN },
    { V_CORSAIR, P_HARPOON_WL_U, harpoonwlpatch, HARPOONWLPATCH_LEN },
    { V_CORSAIR, P_KATAR_PRO_XT, katarproxtpatch,KATARPROXTPATCH_LEN },
    { V_CORSAIR, P_KATAR_PRO,    katarproxtpatch,KATARPROXTPATCH_LEN },
    { V_CORSAIR, P_K95_LEGACY,   k95legacypatch, K95LEGACYPATCH_LEN },
};
#define KEYPATCHES_LEN sizeof(mappatches)/sizeof(*mappatches)

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
            kb->keymap[profswitch_kb_idx].name = NULL;
            kb->keymap[profswitch_kb_idx].led = 0;
            kb->keymap[profswitch_kb_idx].scan = KEY_NONE;
        }
    }

    // Iterate through the patches for all devices
    for(size_t pos = 0; pos < KEYPATCHES_LEN; pos++){
        if(mappatches[pos].vendor == kb->vendor && mappatches[pos].product == kb->product){
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

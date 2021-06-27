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

keypatch icwpatch[] = {
    /* idx, name, led, scan */
    { 211, "dpiup",  -1, KEY_CORSAIR },
    { 212, "dpidn",  -1, KEY_CORSAIR },

};
#define ICWPATCH_LEN sizeof(icwpatch)/sizeof(*icwpatch)

keypatches mappatches[] = {
    { V_CORSAIR, P_K68,          k63patch, K63PATCH_LEN },
    { V_CORSAIR, P_K68_NRGB,     k63patch, K63PATCH_LEN },
    { V_CORSAIR, P_K65,          k65patch, K65PATCH_LEN },
    { V_CORSAIR, P_K65_LEGACY,   k65patch, K65PATCH_LEN },
    { V_CORSAIR, P_K63_NRGB,     k63patch, K63PATCH_LEN },
    { V_CORSAIR, P_M95,          m95patch, M95PATCH_LEN },
    // { V_CORSAIR, P_IRONCLAW_W_U, icwpatch, ICWPATCH_LEN },
    // { V_CORSAIR, P_IRONCLAW_W_D, icwpatch, ICWPATCH_LEN },
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
    // Check if we need to patch the bragi base map
    if(kb->protocol == PROTO_BRAGI)
        memcpy(kb->keymap, keymap_bragi, sizeof(keymap_bragi));
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

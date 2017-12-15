#include "usb.h"
#include "keymap_patch.h"
#include "keymap.h"

/*#define K65PATCH_LEN 1
#define K63PATCH_LEN 2*/

// The Alt code is used for Fn in the K65
keypatch k65patch[] = {
    { 146, "fn", 0x59, KEY_FN },
};
#define K65PATCH_LEN sizeof(k65patch)/sizeof(*k65patch)

// Vol Up/Dn LED addresses for K63
keypatch k63patch[] = {
    { 130, "volup", 0x82, KEY_VOLUMEUP },
    { 131, "voldn", 0x8e, KEY_VOLUMEDOWN },
};
#define K63PATCH_LEN sizeof(k63patch)/sizeof(*k63patch)

keypatches mappatches[] = {
    { P_K65_NRGB, k65patch, K65PATCH_LEN },
    { P_K63_NRGB, k63patch, K63PATCH_LEN },
};
#define KEYPATCHES_LEN sizeof(mappatches)/sizeof(*mappatches)

/// \brief
///
/// Iterate through the keypatches array and check for a match against the current device.
/// If one is found, then copy the keymap, patch it, and add it back to the usbdevice struct.
void patchkeys(usbdevice* kb){
    for(unsigned pos = 0; pos < KEYPATCHES_LEN; pos++){
        if(mappatches[pos].product == kb->product){
            key* newmap = malloc(sizeof(keymap));
            memcpy(newmap, keymap, sizeof(keymap));
            // Patch the copied keymap
            for(unsigned i = 0; i < mappatches[pos].patchlen; i++){
                keypatch* curpatch = mappatches[pos].patch;
                int idx = curpatch->idx;
                newmap[idx].name = curpatch->name;
                newmap[idx].led = curpatch->led;
                newmap[idx].scan = curpatch->scan;
            }
            kb->keymap = newmap;
            return;
        }
    }
    // If we got here, the device should use the standard map
    kb->keymap = keymap;
}

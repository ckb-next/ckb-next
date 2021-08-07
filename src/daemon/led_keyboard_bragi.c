#include "led.h"
#include "notify.h"
#include "profile.h"
#include "usb.h"
#include "bragi_proto.h"
#include "bragi_common.h"

#define N_BRAGI_KEYBOARD_KEYS 156
// Compare two light structures, ignore keys
static inline int rgbcmp(const lighting* lhs, const lighting* rhs){
    return memcmp(lhs->r, rhs->r, N_BRAGI_KEYBOARD_KEYS) || memcmp(lhs->g, rhs->g, N_BRAGI_KEYBOARD_KEYS) || memcmp(lhs->b, rhs->b, N_BRAGI_KEYBOARD_KEYS);
}

int updatergb_keyboard_bragi(usbdevice* kb, int force){
    if(!kb->active)
        return 0;
    lighting* lastlight = &kb->profile->lastlight;
    lighting* newlight = &kb->profile->currentmode->light;
    // Don't do anything if the lighting hasn't changed
    if(!force && !lastlight->forceupdate && !newlight->forceupdate
            && !rgbcmp(lastlight, newlight))
        return 0;

    const uchar* r = newlight->r, *g = newlight->g, *b = newlight->b;
    // Copy the RGB data to a big enough packet while skipping the header.
    uchar rgb_pkt[BRAGI_JUMBO_SIZE] = {0};
    uchar* pkt = rgb_pkt + 7 + 4;
    // Red
    memcpy(pkt, r, N_BRAGI_KEYBOARD_KEYS);
    pkt += N_BRAGI_KEYBOARD_KEYS;
    // Green
    memcpy(pkt, g, N_BRAGI_KEYBOARD_KEYS);
    pkt += N_BRAGI_KEYBOARD_KEYS;
    // Blue
    memcpy(pkt, b, N_BRAGI_KEYBOARD_KEYS);

    // Write to device
    if(bragi_write_to_handle_offset(kb, rgb_pkt, BRAGI_LIGHTING_HANDLE, sizeof(rgb_pkt), N_BRAGI_KEYBOARD_KEYS*3))
        return 1;

    newlight->forceupdate = 0;
    memcpy(lastlight, newlight, sizeof(lighting));
    return 0;
}

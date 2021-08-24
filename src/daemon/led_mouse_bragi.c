#include "led.h"
#include "notify.h"
#include "profile.h"
#include "usb.h"
#include "bragi_common.h"
#include "bragi_proto.h"
#include <assert.h>

// Compare two light structures, ignore keys
static int rgbcmp(const lighting* lhs, const lighting* rhs, const size_t zones){
    return memcmp(lhs->r + LED_MOUSE, rhs->r + LED_MOUSE, zones) || memcmp(lhs->g + LED_MOUSE, rhs->g + LED_MOUSE, zones) || memcmp(lhs->b + LED_MOUSE, rhs->b + LED_MOUSE, zones);
}

#define LED_CASE(product, count) case product: ; \
                                    static_assert(count <= N_MOUSE_ZONES_EXTENDED, "count must be equal or less than N_MOUSE_ZONES_EXTENDED"); \
                                    return count

#define CPY_SZ(colour) (sizeof(*(newlight->colour)) * zones)
static inline size_t bragi_led_count(usbdevice* kb){
    if(kb->vendor != V_CORSAIR){
        ckb_err("Vendor is not V_CORSAIR");
        return 0;
    }
    switch(kb->product){
    LED_CASE(P_IRONCLAW_W_U, 6);
    LED_CASE(P_HARPOON_WL_U, 2);
    default:
        ckb_err("Unknown product 0x%hhx", kb->product);
        return 0;
    }
}

int updatergb_mouse_bragi(usbdevice* kb, int force){
    if(!kb->active)
        return 0;
    lighting* lastlight = &kb->profile->lastlight;
    lighting* newlight = &kb->profile->currentmode->light;

    // Ideally this will be moved to the usbdevice struct at some point
    const size_t zones = bragi_led_count(kb);

    // Don't do anything if the lighting hasn't changed
    if(!force && !lastlight->forceupdate && !newlight->forceupdate
            && !rgbcmp(lastlight, newlight, zones))
        return 0;

    uchar pkt[BRAGI_JUMBO_SIZE] = {0};

    static_assert(sizeof(pkt) >= 7 + N_MOUSE_ZONES_EXTENDED * 3, "Bragi RGB packet must be large enough to fit all possible zones");

    memcpy(pkt + 7, newlight->r + LED_MOUSE, CPY_SZ(r));
    memcpy(pkt + 7 + CPY_SZ(r), newlight->g + LED_MOUSE, CPY_SZ(g));
    memcpy(pkt + 7 + CPY_SZ(r) + CPY_SZ(g), newlight->b + LED_MOUSE, CPY_SZ(b));

    if(bragi_write_to_handle(kb, pkt, BRAGI_LIGHTING_HANDLE, sizeof(pkt), 3 * zones))
        return 1;

    lastlight->forceupdate = newlight->forceupdate = 0;

    memcpy(lastlight, newlight, sizeof(lighting));
    return 0;
}

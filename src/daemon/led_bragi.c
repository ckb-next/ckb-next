#include "led.h"
#include "notify.h"
#include "profile.h"
#include "usb.h"
#include "bragi_common.h"
#include "bragi_proto.h"
#include <assert.h>

// Compare two light structures, ignore keys
static inline int rgbcmp(const lighting* lhs, const lighting* rhs, const size_t zones, const size_t led_offset){
    return memcmp(lhs->r + led_offset, rhs->r + led_offset, zones) || memcmp(lhs->g + led_offset, rhs->g + led_offset, zones) || memcmp(lhs->b + led_offset, rhs->b + led_offset, zones);
}

#define LED_CASE_M(product, count) __LED_CASE(product, count, N_MOUSE_ZONES_EXTENDED)
#define LED_CASE_K(product, count) __LED_CASE(product, count, (LED_MOUSE - 1))

#define __LED_CASE(product, count, limit) case product: ; \
                                        static_assert(count <= limit, "count must be equal or less than " #limit " for dev " #product); \
                                        return count

#define CPY_SZ(colour) (sizeof(*(newlight->colour)) * zones)
static inline size_t bragi_led_count(usbdevice* kb){
    if(kb->vendor != V_CORSAIR){
        ckb_err("Vendor is not V_CORSAIR");
        return 0;
    }
    switch(kb->product){
    LED_CASE_M(P_IRONCLAW_W_U, 6);
    LED_CASE_M(P_HARPOON_WL_U, 2);
    LED_CASE_K(P_K95_PLATINUM_XT, 156);
    LED_CASE_K(P_K57_U, 137);
    LED_CASE_K(P_K60_PRO_RGB, 123);
    LED_CASE_M(P_KATAR_PRO_XT, 1);
    LED_CASE_M(P_KATAR_PRO, 1);
    LED_CASE_M(P_M55_RGB_PRO, 2);
    default:
        ckb_err("Unknown product 0x%hx", kb->product);
        return 0;
    }
}

static int updatergb_bragi(usbdevice* kb, int force, const size_t led_offset){
    if(!kb->active)
        return 0;
    lighting* lastlight = &kb->profile->lastlight;
    lighting* newlight = &kb->profile->currentmode->light;

    // Ideally this will be moved to the usbdevice struct at some point
    const size_t zones = bragi_led_count(kb);

    // Don't do anything if the lighting hasn't changed
    if(!force && !lastlight->forceupdate && !newlight->forceupdate
            && !rgbcmp(lastlight, newlight, zones, led_offset))
        return 0;

    uchar pkt[BRAGI_JUMBO_SIZE] = {0};

    // Since the blank pkt is used to check if the lights are off, we need to make sure it's sufficiently large
    static_assert(LED_MOUSE <= sizeof(pkt), "pkt is not large enough to check if all zones are off");
    // Switch LEDs off if its all black, because being able to just switch them off even in hw mode is really nice
    int newon  = memcmp( newlight->r + led_offset, pkt, CPY_SZ(r)) ||
                 memcmp( newlight->g + led_offset, pkt, CPY_SZ(g)) ||
                 memcmp( newlight->b + led_offset, pkt, CPY_SZ(b));
    int laston = memcmp(lastlight->r + led_offset, pkt, CPY_SZ(r)) ||
                 memcmp(lastlight->g + led_offset, pkt, CPY_SZ(g)) ||
                 memcmp(lastlight->b + led_offset, pkt, CPY_SZ(b));
    if (newon != laston || force)
        bragi_set_property(kb, BRAGI_BRIGHTNESS, newon ? 1000 : 0);

    static_assert(sizeof(pkt) >= 7 + N_KEYS_EXTENDED * 3, "Bragi RGB packet must be large enough to fit all possible zones in the keymap");

    memcpy(pkt + 7, newlight->r + led_offset, CPY_SZ(r));
    memcpy(pkt + 7 + CPY_SZ(r), newlight->g + led_offset, CPY_SZ(g));
    memcpy(pkt + 7 + CPY_SZ(r) + CPY_SZ(g), newlight->b + led_offset, CPY_SZ(b));

    if(bragi_write_to_handle(kb, pkt, BRAGI_LIGHTING_HANDLE, sizeof(pkt), 3 * zones))
        return 1;

    lastlight->forceupdate = newlight->forceupdate = 0;

    memcpy(lastlight, newlight, sizeof(lighting));
    return 0;
}

int updatergb_mouse_bragi(usbdevice* kb, int force){
    return updatergb_bragi(kb, force, LED_MOUSE);
}

int updatergb_keyboard_bragi(usbdevice* kb, int force){
    return updatergb_bragi(kb, force, 0);
}

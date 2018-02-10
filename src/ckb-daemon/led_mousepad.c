#include "led.h"
#include "notify.h"
#include "profile.h"
#include "usb.h"

// Compare two light structures, ignore keys
static int rgbcmp(const lighting* lhs, const lighting* rhs){
    return memcmp(lhs->r + LED_MOUSE, rhs->r + LED_MOUSE, N_MOUSE_ZONES) || memcmp(lhs->g + LED_MOUSE, rhs->g + LED_MOUSE, N_MOUSE_ZONES) || memcmp(lhs->b + LED_MOUSE, rhs->b + LED_MOUSE, N_MOUSE_ZONES);
}

int updatergb_mousepad(usbdevice* kb, int force){
    if(!kb->active)
        return 0;
    lighting* lastlight = &kb->profile->lastlight;
    lighting* newlight = &kb->profile->currentmode->light;
    // Don't do anything if the lighting hasn't changed
    if(!force && !lastlight->forceupdate && !newlight->forceupdate
            && !rgbcmp(lastlight, newlight))
        return 0;
    lastlight->forceupdate = newlight->forceupdate = 0;

    // We pretend the Polaris is a mouse with 15 LED zones.
    int num_zones = 15;
    // Send the RGB values for each zone to the mouse
    uchar data_pkt[MSG_SIZE] = {
        0x07, 0x22, num_zones, 0x00, 0, // RGB colors
    };
    uchar* rgb_data = &data_pkt[4];
    for(int i = 0; i < num_zones; i++){
        *rgb_data++ = newlight->r[i];
        *rgb_data++ = newlight->g[i];
        *rgb_data++ = newlight->b[i];
    }
    // Send RGB data
    if(!usbsend(kb, data_pkt, 1))
        return -1;
    memcpy(lastlight, newlight, sizeof(lighting));
    return 0;
}

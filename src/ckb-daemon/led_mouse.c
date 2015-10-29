#include "led.h"
#include "notify.h"
#include "profile.h"
#include "usb.h"

// Compare two light structures, ignore keys
static int rgbcmp(const lighting* lhs, const lighting* rhs){
    return memcmp(lhs->r + LED_MOUSE, rhs->r + LED_MOUSE, N_MOUSE_ZONES) || memcmp(lhs->g + LED_MOUSE, rhs->g + LED_MOUSE, N_MOUSE_ZONES) || memcmp(lhs->b + LED_MOUSE, rhs->b + LED_MOUSE, N_MOUSE_ZONES);
}

// Return true if all mouse zones are black
// Automatically returns false for all devices except M65, because they don't like this packet
static int isblack(const usbdevice* kb, const lighting* light){
    if(!IS_M65(kb))
        return 0;
    uchar black[N_MOUSE_ZONES] = { 0 };
    return !memcmp(light->r + LED_MOUSE, black, sizeof(black)) && !memcmp(light->g + LED_MOUSE, black, sizeof(black)) && !memcmp(light->b + LED_MOUSE, black, sizeof(black));
}

int updatergb_mouse(usbdevice* kb, int force){
    if(!kb->active)
        return 0;
    lighting* lastlight = &kb->profile->lastlight;
    lighting* newlight = &kb->profile->currentmode->light;
    // Don't do anything if the lighting hasn't changed
    if(!force && !lastlight->forceupdate && !newlight->forceupdate
            && !rgbcmp(lastlight, newlight))
        return 0;
    lastlight->forceupdate = newlight->forceupdate = 0;

    // Send the RGB values for each zone to the mouse
    uchar data_pkt[2][MSG_SIZE] = {
        { 0x07, 0x22, N_MOUSE_ZONES, 0x01, 0 }, // RGB colors
        { 0x07, 0x05, 0x02, 0 }                 // Lighting on/off
    };
    uchar* rgb_data = &data_pkt[0][4];
    for(int i = 0; i < N_MOUSE_ZONES; i++){
        *rgb_data++ = i + 1;
        *rgb_data++ = newlight->r[LED_MOUSE + i];
        *rgb_data++ = newlight->g[LED_MOUSE + i];
        *rgb_data++ = newlight->b[LED_MOUSE + i];
    }
    // Send RGB data
    if(!usbsend(kb, data_pkt[0], 1))
        return -1;
    int was_black = isblack(kb, lastlight), is_black = isblack(kb, newlight);
    if(is_black){
        // If the lighting is black, send the deactivation packet (M65 only)
        if(!usbsend(kb, data_pkt[1], 1))
            return -1;
    } else if(was_black || force){
        // If the lighting WAS black, or if we're on forced update, send the activation packet
        data_pkt[1][4] = 1;
        if(!usbsend(kb, data_pkt[1], 1))
            return -1;
    }

    memcpy(lastlight, newlight, sizeof(lighting));
    return 0;
}

int savergb_mouse(usbdevice* kb, lighting* light, int mode){
    uchar data_pkt[MSG_SIZE] = { 0x07, 0x13, 0x10, 1, 0 };
    // Save each RGB zone, minus the DPI light which is sent in the DPI packets
    int zonecount = IS_SCIMITAR(kb) ? 4 : IS_SABRE(kb) ? 3 : 2;
    for(int i = 0; i < zonecount; i++){
        int led = LED_MOUSE + i;
        if(led >= LED_DPI)
            led++;          // Skip DPI light
        data_pkt[4] = light->r[led];
        data_pkt[5] = light->g[led];
        data_pkt[6] = light->b[led];
        if(!usbsend(kb, data_pkt, 1))
            return -1;
        // Set packet for next zone
        data_pkt[2]++;
    }
    return 0;
}

int loadrgb_mouse(usbdevice* kb, lighting* light, int mode){
    uchar data_pkt[MSG_SIZE] = { 0x0e, 0x13, 0x10, 1, 0 };
    uchar in_pkt[MSG_SIZE] = { 0 };
    // Load each RGB zone
    int zonecount = IS_SCIMITAR(kb) ? 4 : IS_SABRE(kb) ? 3 : 2;
    for(int i = 0; i < zonecount; i++){
        if(!usbrecv(kb, data_pkt, in_pkt))
            return -1;
        if(memcmp(in_pkt, data_pkt, 4)){
            ckb_err("Bad input header\n");
            return -2;
        }
        // Copy data
        int led = LED_MOUSE + i;
        if(led >= LED_DPI)
            led++;          // Skip DPI light
        light->r[led] = in_pkt[4];
        light->g[led] = in_pkt[5];
        light->b[led] = in_pkt[6];
        // Set packet for next zone
        data_pkt[2]++;
    }
    return 0;
}

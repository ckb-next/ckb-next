#include "led.h"
#include "notify.h"
#include "profile.h"
#include "usb.h"
#include "bragi_proto.h"

#define N_BRAGI_KEYBOARD_KEYS 156
#define BRAGI_RGB_PKT_SIZE 12+4+468+7
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
    lastlight->forceupdate = newlight->forceupdate = 0;

    const uchar* r = newlight->r, *g = newlight->g, *b = newlight->b;
    // Copy the RGB data to a big packet.
    // Payload length is 468 bytes, but the 4 bytes at the start don't count for some reason.
    // Buffer needs (4 + 156*3 + 12 padding) + 7 bytes for the first header.
    // 0xd4010000 == 468
    uchar rgb_pkt[BRAGI_RGB_PKT_SIZE] = {BRAGI_MAGIC, BRAGI_WRITE_DATA, BRAGI_LIGHTING_HANDLE, 0xd4, 0x01, 0x00, 0x00};
    // Red
    memcpy(rgb_pkt + 7 + 4, r, N_BRAGI_KEYBOARD_KEYS);
    // Green
    memcpy(rgb_pkt, g, N_BRAGI_KEYBOARD_KEYS);
    // Blue
    memcpy(rgb_pkt, b, N_BRAGI_KEYBOARD_KEYS);

    uchar* pkt = rgb_pkt;
    // Now start sending 64 byte chunks.
    // First packet is ready as-is
#warning "Check if the device responded with success"
    uchar response[64] = {0};
    if(!usbrecv(kb, pkt, response))
        return 1;

    // Second to eigth packet
    // Get to the end of the last packet, go back 3 bytes and insert the header for the continue write
    while((pkt += 64 - 3) < rgb_pkt + BRAGI_RGB_PKT_SIZE) {
        pkt[0] = BRAGI_MAGIC;
        pkt[1] = BRAGI_CONTINUE_WRITE;
        pkt[2] = BRAGI_LIGHTING_HANDLE;
        // Send the new packet
        if(!usbrecv(kb, pkt, response))
            return 1;
    }

    memcpy(lastlight, newlight, sizeof(lighting));
    return 0;
}

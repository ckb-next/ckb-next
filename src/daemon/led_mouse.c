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

static int updatergb_darkcore(usbdevice* kb, lighting* lastlight, lighting* newlight){
    // Dark Core zone bits:
    // 1: Front
    // 2: Thumb
    // 4: Rear

    // Dark Core commands:
    // 00: Colour Shift
    // 01: Colour Pulse
    // 03: Rainbow
    // 07: Static Colour
    // FF: No animation (black)
    uchar opacity_pkt[MSG_SIZE] = {
        CMD_SET, 0xad, 0x00, 0x00, 100, 0
    };
    if(!usbsend(kb, opacity_pkt, 1))
        return -1;

    // Sending each zone individually doesn't work, so we have to check and merge colours.
    // TODO: clean this mess up.
    int colour_count = 0;
    int colours[3][4] = { { 0 }, { 0 }, { 0 } }; // { Red, Green, Blue, Bitmask }
    for (int colour = 0; colour < 3; colour++) {
        int should_add = 1;
        for (int cmp_colour = 0; cmp_colour <= colour_count - 1; cmp_colour++) {
            ckb_info("colours[colour]: %2hhx%2hhx%2hhx colours[cmp_colour]: %2hhx%2hhx%2hhx\n", 
                    colours[colour][0], colours[colour][1], colours[colour][2],
                    colours[cmp_colour][0], colours[cmp_colour][1], colours[cmp_colour][2]);
            if (newlight->r[colour] == colours[cmp_colour][0] && // Red
                newlight->g[colour] == colours[cmp_colour][1] && // Green
                newlight->b[colour] == colours[cmp_colour][2]) { // Blue

                should_add = 0;
                colours[cmp_colour][3] |= 1 << colour; // Add bit to bitmask.
                break;
            }
        }
        if (should_add) {
            colours[colour][0] = newlight->r[colour];
            colours[colour][1] = newlight->g[colour];
            colours[colour][2] = newlight->b[colour];
            colours[colour][3] = 1 << colour;
            colour_count++;
        }
    }

    for (int zone = 0; zone < colour_count; zone++) {
        uchar data_pkt[MSG_SIZE] = {
            CMD_SET, 0xaa, 0
        };
        data_pkt[4]  = colours[zone][3]; // Bitmask.
        data_pkt[5]  = 7;                // Command (static colour).
        data_pkt[8]  = 100;              // Opacity (100%).
        data_pkt[9]  = colours[zone][0];
        data_pkt[10] = colours[zone][1];
        data_pkt[11] = colours[zone][2];
        // Colour gets sent twice for some reason.
        data_pkt[12] = colours[zone][0];
        data_pkt[13] = colours[zone][1];
        data_pkt[14] = colours[zone][2];
        data_pkt[15] = 5; // Terminator byte?
        if(!usbsend(kb, data_pkt, 1))
            return -1;
    }

    memcpy(lastlight, newlight, sizeof(lighting));
    return 0;
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

    // The Dark Core has its own lighting protocol.
    if(IS_DARK_CORE(kb))
        return updatergb_darkcore(kb, lastlight, newlight);

    // Prevent writing to DPI LEDs or non-existent LED zones for the Glaive.
    int num_zones = IS_GLAIVE(kb) ? 3 : N_MOUSE_ZONES;
    // Send the RGB values for each zone to the mouse
    uchar data_pkt[2][MSG_SIZE] = {
        { CMD_SET, FIELD_M_COLOR, num_zones, 0x01, 0 }, // RGB colors
        { CMD_SET, FIELD_LIGHTING, MODE_SOFTWARE, 0 }   // Lighting on/off
    };
    uchar* rgb_data = &data_pkt[0][4];
    for(int i = 0; i < N_MOUSE_ZONES; i++){
        if (IS_GLAIVE(kb) && i != 0 && i != 1 && i != 5)
	    continue;
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
        data_pkt[1][4] = MODE_HARDWARE;
        if(!usbsend(kb, data_pkt[1], 1))
            return -1;
    }

    memcpy(lastlight, newlight, sizeof(lighting));
    return 0;
}

int savergb_mouse(usbdevice* kb, lighting* light, int mode){
    (void)mode;

    uchar data_pkt[MSG_SIZE] = { CMD_SET, FIELD_MOUSE, MOUSE_HWCOLOR, 1, 0 };
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
    (void)mode;

    uchar data_pkt[MSG_SIZE] = { CMD_GET, FIELD_MOUSE, MOUSE_HWCOLOR, 1, 0 };
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

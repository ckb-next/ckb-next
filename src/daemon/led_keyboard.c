#include <stdint.h>

#include "led.h"
#include "notify.h"
#include "profile.h"
#include "usb.h"
#include "nxp_proto.h"

// Define an ordered dithering table by using bit reversion.
#define BR1(x) ((((x) & 0xaa) >> 1) | (((x) & 0x55) << 1))
#define BR2(x) (((BR1(x) & 0xcc) >> 2) | ((BR1(x) & 0x33) << 2))
#define BR4(x) (((BR2(x) & 0xf0) >> 4) | ((BR2(x) & 0x0f) << 4))
#define O0(i) BR4(i),
#define O1(i) O0(i) O0((i) + 1)
#define O2(i) O1(i) O1((i) + 2)
#define O3(i) O2(i) O2((i) + 4)
#define O4(i) O3(i) O3((i) + 8)
#define O5(i) O4(i) O4((i) + 16)
#define O6(i) O5(i) O5((i) + 32)
#define O7(i) O6(i) O6((i) + 64)
#define O8(i) O7(i) O7((i) + 127)

static uchar bit_reverse_table[256] = { O8(0) };

static uchar ordered8to3(int index, uchar value){
    int m = value * 7;
    int b = m / 255;
    if((m % 255) > bit_reverse_table[index & 0xff])
        b++;
    return b;
}

static uchar quantize8to3(int index, uchar value){
    (void)index;

    return value >> 5;
}

static int update_sidelights(usbdevice* kb) {
    lighting* newlight = &kb->profile->currentmode->light;
    uchar data_pkt[MSG_SIZE] = {
        CMD_SET, FIELD_LIGHTING, MODE_SIDELIGHT, 0, !!newlight->sidelight, 0x00
    };
    if(!usbsend(kb, data_pkt, sizeof(data_pkt), 1))
        return -1;
    return 0;
}

static void makergb_512(const lighting* light, uchar data_pkt[5][MSG_SIZE],
                        uchar (*ditherfn)(int, uchar)){
    uchar r[N_KEYS_HW / 2], g[N_KEYS_HW / 2], b[N_KEYS_HW / 2];
    // Compress RGB values to a 512-color palette
    for(int i = 0; i < N_KEYS_HW; i += 2){
        char r1 = ditherfn(i, light->r[i]), r2 = ditherfn(i + 1, light->r[i + 1]);
        char g1 = ditherfn(i, light->g[i]), g2 = ditherfn(i + 1, light->g[i + 1]);
        char b1 = ditherfn(i, light->b[i]), b2 = ditherfn(i + 1, light->b[i + 1]);
        r[i / 2] = (7 - r2) << 4 | (7 - r1);
        g[i / 2] = (7 - g2) << 4 | (7 - g1);
        b[i / 2] = (7 - b2) << 4 | (7 - b1);
    }
    memcpy(data_pkt[0] + 4, r, 60);
    memcpy(data_pkt[1] + 4, r + 60, 12);
    memcpy(data_pkt[1] + 16, g, 48);
    memcpy(data_pkt[2] + 4, g + 48, 24);
    memcpy(data_pkt[2] + 28, b, 36);
    memcpy(data_pkt[3] + 4, b + 36, 36);
}

static void makergb_full(const lighting* light, uchar data_pkt[12][MSG_SIZE]){
    const uchar* r = light->r, *g = light->g, *b = light->b;
    // Red
    memcpy(data_pkt[0] + 4, r, 60);
    memcpy(data_pkt[1] + 4, r + 60, 60);
    memcpy(data_pkt[2] + 4, r + 120, 60);
    // Green (final R packet is blank)
    memcpy(data_pkt[4] + 4, g, 60);
    memcpy(data_pkt[5] + 4, g + 60, 60);
    memcpy(data_pkt[6] + 4, g + 120, 60);
    // Blue (final G packet is blank)
    memcpy(data_pkt[8] + 4, b, 60);
    memcpy(data_pkt[9] + 4, b + 60, 60);
    memcpy(data_pkt[10] + 4, b + 120, 60);
}

static void makergb_k55(const lighting* light, uchar data_pkt[MSG_SIZE]){
    const uchar* r = light->r, *g = light->g, *b = light->b;
    // The K55 uses RGBRGBRGB colouring.
    for (int i = 0; i < 3; i++) {
        int index = (i * 3) + 4;
        data_pkt[index + 0] = r[i];
        data_pkt[index + 1] = g[i];
        data_pkt[index + 2] = b[i];
    }
}

static int rgbcmp(const lighting* lhs, const lighting* rhs){
    // Compare two light structures, up until the first three generic zones for the K55.
    // We compare led scancodes, not keymap array indices
    // 0xA2 is the highest scancode (topbar16) + 1 because that's an index and we need length
    return memcmp(lhs->r, rhs->r, 0xA2 + 1) || memcmp(lhs->g, rhs->g, 0xA2 + 1) || memcmp(lhs->b, rhs->b, 0xA2 + 1);
}

int updatergb_kb(usbdevice* kb, int force){
    if(!kb->active)
        return 0;
    lighting* lastlight = &kb->profile->lastlight;
    lighting* newlight = &kb->profile->currentmode->light;
    // Don't do anything if the lighting hasn't changed
    if(!force && !lastlight->forceupdate && !newlight->forceupdate
            && !rgbcmp(lastlight, newlight) && lastlight->sidelight == newlight->sidelight)   // strafe sidelights
        return 0;
    lastlight->forceupdate = newlight->forceupdate = 0;

    if(IS_K63_WL(kb))
        return updatergb_wireless(kb, lastlight, newlight);

    if (kb->product == P_K55 || kb->product == P_K66 || kb->product == P_K68_NRGB) {
        // The K55 and K68 NRGB don't support winlock setting through the
        // normal packets, so we have to use a different packet to set it.
        // 8 is the winlock ("lock") led position in keymap.c
        uchar winlock_status = (newlight->r[8] || newlight->g[8] || newlight->b[8]);
        uchar winlock_pkt[MSG_SIZE] = {
            CMD_SET, FIELD_LIGHTING, MODE_WINLOCK, 0, winlock_status, 0
        };
        if (!usbsend(kb, winlock_pkt, sizeof(winlock_pkt), 1))
            return -1;
    }

    // Don't send data if the keyboard has no lighting.
    if (!HAS_FEATURES(kb, FEAT_RGB))
        return 0;

    if (IS_K55(kb)) {
        // The K55 has its own packet type, because it only has three lighting zones.
        uchar data_pkt[MSG_SIZE] = { 0x07, 0x25, 0x00 };
        makergb_k55(newlight, data_pkt);
        if (!usbsend(kb, data_pkt, sizeof(data_pkt), 1))
            return -1;
    } else if(IS_FULLRANGE(kb)) {
        // Update strafe sidelights if necessary
        if (IS_STRAFE(kb) && update_sidelights(kb))
            return -1;

        // 16.8M color lighting works fine on strafe and is the only way it actually works
        uchar data_pkt[12][MSG_SIZE] = {
            // Red
            { CMD_WRITE_BULK, 0x01, 0x3c, 0 },
            { CMD_WRITE_BULK, 0x02, 0x3c, 0 },
            { CMD_WRITE_BULK, 0x03, 0x30, 0 },
            { CMD_SET, FIELD_KB_COLOR, COLOR_RED, 0x03, 0x01, 0},
            // Green
            { CMD_WRITE_BULK, 0x01, 0x3c, 0 },
            { CMD_WRITE_BULK, 0x02, 0x3c, 0 },
            { CMD_WRITE_BULK, 0x03, 0x30, 0 },
            { CMD_SET, FIELD_KB_COLOR, COLOR_GREEN, 0x03, 0x01, 0},
            // Blue
            { CMD_WRITE_BULK, 0x01, 0x3c, 0 },
            { CMD_WRITE_BULK, 0x02, 0x3c, 0 },
            { CMD_WRITE_BULK, 0x03, 0x30, 0 },
            { CMD_SET, FIELD_KB_COLOR, COLOR_BLUE, 0x03, 0x02, 0}
        };
        makergb_full(newlight, data_pkt);

        // Monochrome devices only use the red channel, so only send the first four packets
        int data_pkt_count = 12;
        if(IS_MONOCHROME_DEV(kb))
            data_pkt_count = 4;

        if(!usbsend(kb, data_pkt[0], MSG_SIZE, data_pkt_count))
            return -1;
    } else {
        // Update strafe sidelights if necessary
        if (IS_STRAFE(kb) && update_sidelights(kb))
            return -1;
        // On older keyboards it looks flickery and causes lighting glitches, so we don't use it.
        uchar data_pkt[5][MSG_SIZE] = {
            { CMD_WRITE_BULK, 0x01, 60, 0 },
            { CMD_WRITE_BULK, 0x02, 60, 0 },
            { CMD_WRITE_BULK, 0x03, 60, 0 },
            { CMD_WRITE_BULK, 0x04, 36, 0 },
            { CMD_SET, FIELD_KB_9BCLR, 0x00, 0x00, 0xD8 }
        };
        makergb_512(newlight, data_pkt, kb->dither ? ordered8to3 : quantize8to3);
        if(!usbsend(kb, data_pkt[0], MSG_SIZE, 5))
            return -1;
    }

    memcpy(lastlight, newlight, sizeof(lighting));
    return 0;
}

int savergb_kb(usbdevice* kb, lighting* light, int mode){
    if(kb->fwversion >= 0x0120 || IS_V2_OVERRIDE(kb)){
        uchar data_pkt[12][MSG_SIZE] = {
            // Red
            { CMD_WRITE_BULK, 0x01, 60, 0 },
            { CMD_WRITE_BULK, 0x02, 60, 0 },
            { CMD_WRITE_BULK, 0x03, 24, 0 },
            { CMD_SET, FIELD_KB_HWCLR, 0x03, 0x01, 0x01, mode + 1, COLOR_RED},
            // Green
            { CMD_WRITE_BULK, 0x01, 60, 0 },
            { CMD_WRITE_BULK, 0x02, 60, 0 },
            { CMD_WRITE_BULK, 0x03, 24, 0 },
            { CMD_SET, FIELD_KB_HWCLR, 0x03, 0x01, 0x01, mode + 1, COLOR_GREEN },
            // Blue
            { CMD_WRITE_BULK, 0x01, 60, 0 },
            { CMD_WRITE_BULK, 0x02, 60, 0 },
            { CMD_WRITE_BULK, 0x03, 24, 0 },
            { CMD_SET, FIELD_KB_HWCLR, 0x03, 0x01, 0x01, mode + 1, COLOR_BLUE }
        };
        makergb_full(light, data_pkt);
        if(!usbsend(kb, data_pkt[0], MSG_SIZE, 12))
            return -1;
        if (IS_STRAFE(kb)){ // end save
            uchar save_end_pkt[MSG_SIZE] = { CMD_SET, FIELD_KB_HWCLR, 0x04, 0x01, 0x01 };
            if(!usbsend(kb, save_end_pkt, sizeof(save_end_pkt), 1))
                return -1;
        }
    } else {
        uchar data_pkt[5][MSG_SIZE] = {
            { CMD_WRITE_BULK, 0x01, 60, 0 },
            { CMD_WRITE_BULK, 0x02, 60, 0 },
            { CMD_WRITE_BULK, 0x03, 60, 0 },
            { CMD_WRITE_BULK, 0x04, 36, 0 },
            { CMD_SET, FIELD_KB_HWCLR, 0x02, 0x00, 0x01, mode + 1 }
        };
        makergb_512(light, data_pkt, kb->dither ? ordered8to3 : quantize8to3);
        if(!usbsend(kb, data_pkt[0], MSG_SIZE, 5))
            return -1;
    }
    return 0;
}

int loadrgb_kb(usbdevice* kb, lighting* light, int mode){
    if(kb->fwversion >= 0x0120 || IS_V2_OVERRIDE(kb)){
        uchar data_pkt[12][MSG_SIZE] = {
            { CMD_GET, FIELD_KB_HWCLR, 0x03, 0x01, 0x01, mode + 1, COLOR_RED },
            { CMD_READ_BULK, 0x01, 60, 0 },
            { CMD_READ_BULK, 0x02, 60, 0 },
            { CMD_READ_BULK, 0x03, 24, 0 },
            { CMD_GET, FIELD_KB_HWCLR, 0x03, 0x01, 0x01, mode + 1, COLOR_GREEN },
            { CMD_READ_BULK, 0x01, 60, 0 },
            { CMD_READ_BULK, 0x02, 60, 0 },
            { CMD_READ_BULK, 0x03, 24, 0 },
            { CMD_GET, FIELD_KB_HWCLR, 0x03, 0x01, 0x01, mode + 1, COLOR_BLUE },
            { CMD_READ_BULK, 0x01, 60, 0 },
            { CMD_READ_BULK, 0x02, 60, 0 },
            { CMD_READ_BULK, 0x03, 24, 0 },
        };
        uchar in_pkt[4][MSG_SIZE] = {
            { CMD_GET, FIELD_KB_HWCLR, 0x03, 0x01 },
            { CMD_READ_BULK, 0x01, 60, 0 },
            { CMD_READ_BULK, 0x02, 60, 0 },
            { CMD_READ_BULK, 0x03, 24, 0 },
        };

        /// Since Firmware Version 2.05 for K95RGB the answers for getting the stored color-maps from the hardware
        /// has changed a bit. So comparing for the correct answer cannot validate against the cmd,
        /// and has to be done against a third map.
        /// Up to now we know, that K70RGB Pro and K70 Lux RGB have firmware version 2.04 and having the problem also.
        /// So we have to determine in the most inner loop the firmware version and type of KB to select the correct compare-table.

        uchar cmp_pkt[4][4] = {
            { CMD_GET, FIELD_KB_HWCLR, 0x03, 0x01 },
            { CMD_GET, CMD_READ_BULK, 0x01, 60 },
            { CMD_GET, CMD_READ_BULK, 0x02, 60 },
            { CMD_GET, CMD_READ_BULK, 0x03, 24 },
        };
        /// Read colors
        uchar* colors[3] = { light->r, light->g, light->b };
        for(int clr = 0; clr < 3; clr++){
            for(int i = 0; i < 4; i++){
                if(!usbrecv(kb, data_pkt[i + clr * 4], MSG_SIZE, in_pkt[i]))
                    return -1;

                uchar* comparePacket = data_pkt[i + clr * 4];   ///< That is the old comparison method: you get back what you sent.
                /// Normally a firmware version >= 2.05 runs with the new compare array.
                /// Up to now there is a 2.04 running in K70 RGB Lux with the same behavior.
                /// It seems that K70RGB has the same problem
                if ((kb->fwversion >= 0x205)
                        || ((kb->fwversion >= 0x204)
                            && ((kb->product == P_K70_LUX_NRGB) || (kb->product == P_K70_LUX)))) {
                    comparePacket = cmp_pkt[i];
                }

                if (memcmp(in_pkt[i], comparePacket, 4)) {
                    ckb_err("Bad input header");
                    ckb_err("color = %d, i = %d, mode = %d\nOutput (Request): %2.2x %2.2x %2.2x %2.2x\nInput(Reply): %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x", clr, i, mode,
                        comparePacket[0], comparePacket[1], comparePacket[2], comparePacket[3],
                        in_pkt[i][0], in_pkt[i][1], in_pkt[i][2], in_pkt[i][3], in_pkt[i][4], in_pkt[i][5], in_pkt[i][6], in_pkt[i][7]);
                    in_pkt[2][0] = 0x99;
                    in_pkt[2][1] = 0x99;
                    in_pkt[2][2] = 0x99;
                    in_pkt[2][3] = 0x99;
                    usbrecv(kb, in_pkt[2], MSG_SIZE, in_pkt[2]); // just to find it in the wireshark log
                    return -1;
                }
            }
            // Copy colors to lighting. in_pkt[0] is irrelevant.
            memcpy(colors[clr], in_pkt[1] + 4, 60);
            memcpy(colors[clr] + 60, in_pkt[2] + 4, 60);
            memcpy(colors[clr] + 120, in_pkt[3] + 4, 24);
        }
    } else {
        uchar data_pkt[5][MSG_SIZE] = {
            { CMD_GET, FIELD_KB_HWCLR, 0x02, 0x01, 0x01, mode + 1, 0 },
            { CMD_READ_BULK, 0x01, 60, 0 },
            { CMD_READ_BULK, 0x02, 60, 0 },
            { CMD_READ_BULK, 0x03, 60, 0 },
            { CMD_READ_BULK, 0x04, 36, 0 },
        };
        uchar in_pkt[4][MSG_SIZE] = {
            { CMD_READ_BULK, 0x01, 60, 0 },
            { CMD_READ_BULK, 0x02, 60, 0 },
            { CMD_READ_BULK, 0x03, 60, 0 },
            { CMD_READ_BULK, 0x04, 36, 0 },
        };
        // Write initial packet
        if(!usbsend(kb, data_pkt[0], MSG_SIZE, 1))
            return -1;
        // Read colors
        for(int i = 1; i < 5; i++){
            if(!usbrecv(kb, data_pkt[i], MSG_SIZE, in_pkt[i - 1]))
                return -1;
            if(memcmp(in_pkt[i - 1], data_pkt[i], 4)){
                ckb_err("Bad input header");
                return -1;
            }
        }
        // Copy the data back to the mode
        uint8_t mr[N_KEYS_HW / 2], mg[N_KEYS_HW / 2], mb[N_KEYS_HW / 2];
        memcpy(mr,      in_pkt[0] +  4, 60);
        memcpy(mr + 60, in_pkt[1] +  4, 12);
        memcpy(mg,      in_pkt[1] + 16, 48);
        memcpy(mg + 48, in_pkt[2] +  4, 24);
        memcpy(mb,      in_pkt[2] + 28, 36);
        memcpy(mb + 36, in_pkt[3] +  4, 36);
        // Unpack LED data to 8bpc format
        for(int i = 0; i < N_KEYS_HW; i++){
            int     i_2 = i / 2;
            uint8_t r, g, b;

            // 3-bit intensities stored in alternate nybbles.
            if (i & 1) {
                r = 7 - (mr[i_2] >> 4);
                g = 7 - (mg[i_2] >> 4);
                b = 7 - (mb[i_2] >> 4);
            } else {
                r = 7 - (mr[i_2] & 0x0F);
                g = 7 - (mg[i_2] & 0x0F);
                b = 7 - (mb[i_2] & 0x0F);
            }
            // Scale 3-bit values up to 8 bits.
            light->r[i] = r << 5 | r << 2 | r >> 1;
            light->g[i] = g << 5 | g << 2 | g >> 1;
            light->b[i] = b << 5 | b << 2 | b >> 1;
        }
    }
    return 0;
}

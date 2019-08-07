#include "led.h"
#include "notify.h"
#include "profile.h"
#include "usb.h"

// Compare two light structures, ignore keys
static int rgbcmp(const lighting* lhs, const lighting* rhs, uchar fan_number){
    return memcmp(lhs->r, rhs->r, N_LL120_ZONES_PER_FAN * fan_number) || memcmp(lhs->g, rhs->g, N_LL120_ZONES_PER_FAN * fan_number) || memcmp(lhs->b, rhs->b, N_LL120_ZONES_PER_FAN * fan_number);
}

int updatergb_lightning_node(usbdevice* kb, int force){
    if(!kb->active)
        return 0;
    if(kb->number_of_fans == 0)
        return 0;
    lighting* lastlight = &kb->profile->lastlight;
    lighting* newlight = &kb->profile->currentmode->light;
    // Don't do anything if the lighting hasn't changed
    if(!force && !lastlight->forceupdate && !newlight->forceupdate
            && !rgbcmp(lastlight, newlight, kb->number_of_fans))
        return 0;
    lastlight->forceupdate = newlight->forceupdate = 0;

    // INDEX 1 = r, 2 = g, 3 = b, 4 = s, 5 = t, 6 = u
    // s, t, u are R, G, B extended for fans 3-6
    for (uchar i = 1; i <= 6; i++) {
        uchar pktcolor[MSG_SIZE] = {
            0x32, 
            0x00,
            // For Fan [1-3], packet must beginning by => 0x32, 0x00, 0x00, 0x32 
            // End for Fan [4-6], packet must beginning by => 0x32, 0x00, 0x32, 0x2e 
            (i > 3) ? 0x32 : 0x00,
            (i > 3) ? 0x2e : 0x32,
        };

        uchar* color_array_ptr;
        // For red color the fifth Bytes must be equals to 0x00
        // For green color the fifth Bytes must be equals to 0x01
        // For blue color the fifth Bytes must be equals to 0x02
        if (i == 1 || i == 4) {
            pktcolor[4] = 0x00; // red color
            color_array_ptr = &newlight->r[0];
        }
        if (i == 2 || i == 5) {
            pktcolor[4] = 0x01; // green color
            color_array_ptr = &newlight->g[0];
        }
        if (i == 3 || i == 6) {
            pktcolor[4] = 0x02; // blue color
            color_array_ptr = &newlight->b[0];
        }

        uchar* rgb_data = &pktcolor[5];
        const int PTR_FAN_4 = N_LL120_ZONES_PER_FAN * 4;
        if (i < 4) { // For Fans [1-3]
            for (uint j = 0; j < N_LL120_ZONES_PER_FAN * (kb->number_of_fans > 3 ? 3 : kb->number_of_fans); j++) {
                *rgb_data++ = *(color_array_ptr + j);
            }

            // If we have 4 or more fans, we need to take 3 bytes from the 4th fan and put
            // it on this message.
            if (kb->number_of_fans > 3) {
                for (int j = 0; j < 3; j++) {
                    *rgb_data++ = *(color_array_ptr + (PTR_FAN_4 + j));
                }
            }
        } else { // For Fans [4-6]
            if (kb->number_of_fans > 3) {
                // Continuation of 4th fan from r/g/b runs
                for (int j = 3; j < N_LL120_ZONES_PER_FAN; j++) {
                    *rgb_data++ = *(color_array_ptr + (PTR_FAN_4 + j));
                }

                for (uint j = 3; j < N_LL120_ZONES_PER_FAN * kb->number_of_fans; j++) {
                    *rgb_data++ = *(color_array_ptr + j);
                }
            }
        }

        if(!usbsend(kb, pktcolor, 1)) return -1;
    }

    uchar pktend[MSG_SIZE] = {0x33, 0xff};
    if(!usbsend(kb, pktend, 1)) return -1;
    memcpy(lastlight, newlight, sizeof(lighting));
    return 0;
}

int preambule_fan_lightning_node(usbdevice* kb,uchar numberOfFans) {
    ckb_info("PReambule begin send\n");
    uchar pkt1[MSG_SIZE] = {0x37};

    // 0x35 - Init
    uchar pkt2[MSG_SIZE] = {0x35, 0x00, 0x00, numberOfFans << 4, 0x00, 0x01, 0x01};
    uchar pkt3[MSG_SIZE] = {0x3b, 0x00, 0x01};
    uchar pkt4[MSG_SIZE] = {0x38, 0x00, 0x02};
    uchar pkt5[MSG_SIZE] = {0x34};
    uchar pkt6[MSG_SIZE] = {0x37, 0x01};
    uchar pkt7[MSG_SIZE] = {0x34, 0x01};
    uchar pkt8[MSG_SIZE] = {0x38, 0x01, 0x01};
    uchar pkt9[MSG_SIZE] = {0x33, 0xff};
    if(!usbsend(kb, pkt1, 1)) return -1;
    if(!usbsend(kb, pkt2, 1)) return -1;
    if(!usbsend(kb, pkt3, 1)) return -1;
    if(!usbsend(kb, pkt4, 1)) return -1;
    if(!usbsend(kb, pkt5, 1)) return -1;
    if(!usbsend(kb, pkt6, 1)) return -1;
    if(!usbsend(kb, pkt7, 1)) return -1;
    if(!usbsend(kb, pkt8, 1)) return -1;
    if(!usbsend(kb, pkt9, 1)) return -1;

    kb->number_of_fans = numberOfFans;

    ckb_info("PReambule sended\n");
    return 0;
}
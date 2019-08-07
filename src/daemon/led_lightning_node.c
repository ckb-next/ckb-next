#include "led.h"
#include "notify.h"
#include "profile.h"
#include "usb.h"

int updatergb_lightning_node(usbdevice* kb, int force){
    // TODO Implement it
    return 0;
}

int init_lightning_node(usbdevice* kb,uchar numberOfFans) {
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
    return 0;
}
#include "bragi_common.h"
#include "bragi_proto.h"

// Gets a property using the bragi protocol
// Error when return value < 0
int bragi_get_property(usbdevice* kb, const uchar prop) {
    uchar pkt[64] = {BRAGI_MAGIC, BRAGI_GET, prop, 0};
    uchar response[64] = {0};
    if(!usbrecv(kb, pkt, response))
        return -1;
    if(response[2]) {
        ckb_err("Failed to get property 0x%hhx. Error was 0x%hhx", prop, response[2]);
        return -2;
    }
    return response[3];
}

// Sets a property using the bragi protocol
// Error when return value < 0. Success == 0
int bragi_set_property(usbdevice* kb, const uchar prop, const uchar val) {
    uchar pkt[64] = {BRAGI_MAGIC, BRAGI_SET, prop, 0, val, 0};
    uchar response[64] = {0};
    if(!usbrecv(kb, pkt, response))
        return -1;
    if(response[2]) {
        ckb_err("Failed to set property 0x%hhx. Error was 0x%hhx", prop, response[2]);
        return -2;
    }
    return 0;
}

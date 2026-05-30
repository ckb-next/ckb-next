#include "structures.h"
#include "usb.h"
#include "device.h"
#include "bragi_common.h"
#include "command.h"

static const report_descriptor_t bragi_descriptors[] = {
    // 1b62 dongle fwver whatever
    {
        { 0x2b, 0x5a, 0x06, 0xac, 0xbf, 0x47, 0x77, 0x78, 0xfc, 0x31, 0x7d, 0x77, 0x0f, 0x02, 0x73, 0x6c, 0x14, 0x08, 0x37, 0xed, 0xa1, 0x3d, 0xe1, 0xb2, 0xf4, 0x7e, 0x1c, 0x1f, 0xb5, 0x72, 0xbe, 0x23 },
        { .hid_in = true },
        process_input_urb
    },
    //
    {
        { 0x08, 0xaa, 0xe1, 0xb8, 0x1c, 0x11, 0x10, 0xfe, 0xe2, 0x1a, 0x68, 0x8f, 0xa9, 0xa7, 0xfd, 0x9f, 0x10, 0x1f, 0x6e, 0x85, 0xc0, 0xe0, 0xe0, 0x41, 0x01, 0xdd, 0xd6, 0x4f, 0x76, 0xd4, 0x57, 0xeb },
        { .cmd_out = true, .cmd_in = true },
        process_input_urb
    },
    {
        { 0xee, 0x82, 0x85, 0x13, 0x00, 0xf3, 0x1d, 0x70, 0x33, 0xf9, 0x78, 0x1c, 0x12, 0x79, 0x08, 0xbe, 0xe8, 0x7a, 0x7a, 0xce, 0xba, 0xbf, 0x94, 0x8a, 0xd8, 0x0e, 0x99, 0xc5, 0x9f, 0x7a, 0x5b, 0xa6 },
        { .vendor_in = true },
        process_input_urb
    },
    {
        { 0x95, 0x00, 0xab, 0xc9, 0x06, 0xec, 0xd3, 0xac, 0x84, 0x9f, 0xa4, 0xf8, 0xa6, 0x5f, 0xcb, 0xc7, 0x73, 0xbd, 0x96, 0x5d, 0x54, 0xee, 0xbe, 0x81, 0x76, 0x03, 0x25, 0x3c, 0xb5, 0xdd, 0xfc, 0x08 },
        { .hid_extra_in = true },
        process_input_urb
    },
    // same device wired mode
    {
        { 0x29, 0xbd, 0xd4, 0x99, 0xe0, 0x39, 0x31, 0xcc, 0xcc, 0x41, 0xf6, 0xbd, 0xda, 0xe4, 0x46, 0x95, 0x28, 0x94, 0x1c, 0x87, 0x78, 0xe4, 0x35, 0x72, 0x26, 0xdf, 0x89, 0x1c, 0x50, 0x1b, 0xdc, 0xeb },
        { .hid_in = true, .hid_extra_in = true },
        process_input_urb
    },
    // ironclaw wired
    {
        { 0x23, 0xb9, 0xc4, 0xba, 0xe6, 0x11, 0xa9, 0xe4, 0x2f, 0xc3, 0xbb, 0xb7, 0xb6, 0xaf, 0x6e, 0x6a, 0x16, 0x84, 0xb5, 0x0c, 0x2a, 0x6c, 0xe2, 0xca, 0x4a, 0xda, 0x94, 0xb6, 0xdb, 0xcc, 0xa9, 0xdc },
        { .hid_in = true, },
        process_input_urb
    },
    // also ironclaw wired but no idea what this is
    {
        { 0x9d, 0x54, 0x8a, 0xfa, 0x16, 0x76, 0x70, 0xe1, 0x62, 0xe8, 0x97, 0x5e, 0x17, 0x2d, 0x40, 0x74, 0x7d, 0x05, 0x7f, 0xce, 0x30, 0x98, 0xd5, 0x2b, 0x4c, 0x09, 0xc2, 0x4e, 0x88, 0x2c, 0xd0, 0xbb },
        { .hid_extra_in = true, },
        NULL // FIXME
    },
    // Ironclaw wireless same as above
    {
        { 0xb7, 0x14, 0x63, 0x12, 0xcc, 0x97, 0x69, 0xdd, 0x09, 0xd2, 0x79, 0x55, 0x1d, 0x05, 0x84, 0xbd, 0x33, 0x44, 0xd9, 0xf3, 0x4e, 0x4f, 0xdb, 0x99, 0x13, 0xc9, 0x1c, 0x68, 0x46, 0xe5, 0xc7, 0x28 },
        { .hid_extra_in = true, },
        NULL // FIXME
    },
};

void bragi_fill_input_eps(usbdevice* kb)
{
    // Match descriptors to interfaces and install input handlers
    for(uchar j = 0; j < kb->bNumInterfaces; j++) {
        for(size_t i = 0; i < sizeof(bragi_descriptors)/sizeof(*bragi_descriptors); i++) {
            if(!memcmp(bragi_descriptors[i].report_descriptor_hash, kb->hid_interfaces[j].report_descriptor_hash, crypto_hash_sha256_BYTES)) {
                ckb_info("MATCHED DESCR");
                kb->hid_interfaces[j].type = bragi_descriptors[i].type;
                // Input
                if(kb->hid_interfaces[j].endpoints[EP_LOOKUP_IN].bEndpointAddress && (bragi_descriptors[i].type.cmd_in || bragi_descriptors[i].type.hid_in || bragi_descriptors[i].type.vendor_in || bragi_descriptors[i].type.hid_extra_in)) {
                    if(bragi_descriptors[i].type.cmd_in)
                        kb->bragi_in_ep = kb->hid_interfaces[j].endpoints[EP_LOOKUP_IN].bEndpointAddress; // FIXME: Remove
                    kb->hid_interfaces[j].endpoints[EP_LOOKUP_IN].handler = bragi_descriptors[i].handler;
                }

                // Output
                if(kb->hid_interfaces[j].endpoints[EP_LOOKUP_OUT].bEndpointAddress && bragi_descriptors[i].type.cmd_out) {
                    kb->bragi_out_ep = kb->hid_interfaces[j].endpoints[EP_LOOKUP_OUT].bEndpointAddress;
                    kb->out_ep_packet_size = kb->hid_interfaces[j].endpoints[EP_LOOKUP_OUT].wMaxPacketSize;
                }
                ckb_info("AAAA %d %d", kb->hid_interfaces[j].endpoints[EP_LOOKUP_OUT].bEndpointAddress, bragi_descriptors[i].type.cmd_out);

                break;
            }
        }
    }
}
/*

void bragi_fill_input_eps(usbdevice* kb)
{
    // FIXME: Use descriptors
    // Pick the correct input and output EPs.
    // Most use 0x84 and 4, but there are a few exceptions...
    kb->bragi_out_ep = 0x4;
    kb->bragi_in_ep = 0x84;

    // Exceptions should set the EPs here
    if(kb->vendor == V_CORSAIR){
        switch(kb->product){
            case P_K57_D:
                kb->bragi_out_ep = 0x2;
                kb->bragi_in_ep = 0x82;
                break;
            case P_K57_U:
            case P_K55_PRO:
            case P_K55_PRO_XT:
            case P_K100_OPTICAL:
            case P_K100_MECHANICAL:
            case P_K65_MINI:
            case P_K70_TKL:
            case P_K70_TKL_CHAMP_OPTIC:
            case P_K70_PRO:
            case P_K70_PRO_OPTIC:
                kb->bragi_out_ep = 0x1;
                kb->bragi_in_ep = 0x82;
                break;

        }
    }
}
*/
int bragi_usb_write(usbdevice* kb, void* out, int len, int is_recv, const char* file, int line)
{
    BRAGI_PKT_SIZE_CHECK(len, file, line);

    // If we need to read a response, lock the interrupt mutex
    if(is_recv)
        if(pthread_mutex_lock(intmutex(kb)))
            ckb_fatal("Error locking interrupt mutex in os_usbsend()");

    int res;

    // If this device has a parent, route all IO through it instead, setting our id
    if(kb->parent){
        unsigned char* pkt = out;
        pkt[0] |= kb->bragi_child_id;
        res = os_usb_interrupt_out(kb->parent, kb->parent->bragi_out_ep, kb->parent->out_ep_packet_size, out, file, line);
    } else {
        res = os_usb_interrupt_out(kb, kb->bragi_out_ep, kb->out_ep_packet_size, out, file, line);
    }
    // Unlock on failure
    if(is_recv && res < 1)
        pthread_mutex_unlock(intmutex(kb));

    return res;
}

int bragi_usb_read(usbdevice* kb, void* in, int len, int dummy, const char* file, int line)
{
    BRAGI_PKT_SIZE_CHECK(len, file, line);

    // Wait for max 2s
    int condret = cond_nanosleep(intcond(kb), intmutex(kb), 2000000000);
    if(condret != 0){
        if(pthread_mutex_unlock(intmutex(kb)))
            ckb_fatal("Error unlocking interrupt mutex in os_usbrecv()");
        if(condret == ETIMEDOUT)
            ckb_warn_fn("ckb%d: Timeout while waiting for response", file, line, INDEX_OF(kb, keyboard));
        else
            ckb_warn_fn("Interrupt cond error %i", file, line, condret);
        return -1;
    }
    memcpy(in, kb->interruptbuf, kb->out_ep_packet_size);
    if(pthread_mutex_unlock(intmutex(kb)))
        ckb_fatal("Error unlocking interrupt mutex in os_usbrecv()");

    return len;
}

#include "structures.h"
#include "usb.h"
#include "device.h"
#include "bragi_common.h"
#include "command.h"
#include "json.h"

// FIXME can't be void
void bragi_fill_input_eps(usbdevice* kb)
{
    json_t* descriptors = json_get_descriptors_for_protocol(kb);
    // Match descriptors to interfaces and install input handlers
    for(uchar j = 0; j < kb->bNumInterfaces; j++) {
        for(size_t i = 0; i < json_array_size(descriptors); i++) {
            json_t* descriptor = json_array_get(descriptors, i);
            uchar hash_bytes[crypto_hash_sha256_BYTES];
            if(!json_parse_descriptor_hash(descriptor, hash_bytes)) {
                // FIXME
                return;
            }
            if(!memcmp(hash_bytes, kb->hid_interfaces[j].report_descriptor_hash, crypto_hash_sha256_BYTES)) {
                json_print_descriptor_comment(descriptor);
                if(!json_parse_descriptor_flags(descriptor, &kb->hid_interfaces[j].type)) {
                    // FIXME
                    return;
                }
                // Input
                if(kb->hid_interfaces[j].endpoints[EP_LOOKUP_IN].bEndpointAddress && (kb->hid_interfaces[j].type.cmd_in || kb->hid_interfaces[j].type.hid_in || kb->hid_interfaces[j].type.vendor_in || kb->hid_interfaces[j].type.hid_extra_in)) {
                    if(kb->hid_interfaces[j].type.cmd_in)
                        kb->bragi_in_ep = kb->hid_interfaces[j].endpoints[EP_LOOKUP_IN].bEndpointAddress; // FIXME: Remove

                    if(!json_parse_descriptor_handler(descriptor, kb->hid_interfaces[j].endpoints + EP_LOOKUP_IN)) {
                        return;
                        // FIXME
                    }
                }

                // Output
                if(kb->hid_interfaces[j].endpoints[EP_LOOKUP_OUT].bEndpointAddress && kb->hid_interfaces[j].type.cmd_out) {
                    kb->bragi_out_ep = kb->hid_interfaces[j].endpoints[EP_LOOKUP_OUT].bEndpointAddress;
                    kb->out_ep_packet_size = kb->hid_interfaces[j].endpoints[EP_LOOKUP_OUT].wMaxPacketSize;
                }

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

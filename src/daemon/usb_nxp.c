#include "structures.h"
#include "usb.h"
#include "device.h"
#include "json.h"

void nxp_fill_input_eps(usbdevice* kb)
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
                if(kb->hid_interfaces[j].endpoints[EP_LOOKUP_IN].bEndpointAddress && (kb->hid_interfaces[j].type.cmd_in || kb->hid_interfaces[j].type.hid_in || kb->hid_interfaces[j].type.vendor_in)) {
                    if(!json_parse_descriptor_handler(descriptor, kb->hid_interfaces[j].endpoints + EP_LOOKUP_IN)) {
                        return;
                        // FIXME
                    }
                }

                // Output
                if(kb->hid_interfaces[j].endpoints[EP_LOOKUP_OUT].bEndpointAddress && kb->hid_interfaces[j].type.cmd_out) {
                    kb->bragi_out_ep = kb->hid_interfaces[j].endpoints[EP_LOOKUP_OUT].bEndpointAddress;
                }

                break;
            }
        }
    }

    kb->out_ep_packet_size = MSG_SIZE;

    // FIXME: Proper condition
    // Note, Ctrl Transfers require an index, not an endpoint, which is why kb->bNumInterfaces - 1 works
    // FIXME: figure out the correct index, whether it's hardcoded or the number of the output interface
    if (!(kb->fwversion >= 0x120 || IS_V2_OVERRIDE(kb))) {
        kb->bragi_out_ep = kb->bNumInterfaces - 1;
    }
}

int nxp_usb_write(usbdevice* kb, void* out, int len, int is_recv, const char* file, int line)
{
    if(len != 64)
    {
        ckb_fatal_fn("len != 64 not supported in NXP backend", file, line);
        return -1;
    }

    if (kb->fwversion >= 0x120 || IS_V2_OVERRIDE(kb)){
        // If we need to read a response, lock the interrupt mutex
        if(is_recv)
            if(pthread_mutex_lock(intmutex(kb)))
                ckb_fatal("Error locking interrupt mutex in os_usbsend()");

        int res = os_usb_interrupt_out(kb, kb->bragi_out_ep, len, out, file, line);
        // Unlock on failure
        if(is_recv && res < 1)
            pthread_mutex_unlock(intmutex(kb));
        return res;
    } else {
        ctrltransfer transfer = { 0x21, 0x09, 0x0200, kb->bragi_out_ep, len, 5000, out };
        return os_usb_control(kb, &transfer, file, line);
    }
}

int nxp_usb_read(usbdevice* kb, void* in, int len, int dummy, const char* file, int line)
{
    if(len != 64)
    {
        ckb_fatal_fn("len != 64 not supported in NXP backend", file, line);
        return -1;
    }

    if(kb->fwversion >= 0x120 || IS_V2_OVERRIDE(kb)){
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
        memcpy(in, kb->interruptbuf, len);
        memset(kb->interruptbuf, 0, len);
        if(pthread_mutex_unlock(intmutex(kb)))
            ckb_fatal("Error unlocking interrupt mutex in os_usbrecv()");
        return len;
    } else {
        ctrltransfer transfer = { 0xa1, 0x01, 0x0300, kb->bNumInterfaces - 1, len, 5000, in };
        return os_usb_control(kb, &transfer, file, line);
    }
}

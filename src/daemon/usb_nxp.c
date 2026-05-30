#include "structures.h"
#include "usb.h"
#include "device.h"

static const report_descriptor_t nxp_descriptors[] = {
    // 1b1c K70 fwver 3 HID
    {
        { 0x5e, 0x2b, 0x93, 0x56, 0xbe, 0x7d, 0x1a, 0x61, 0x22, 0x4d, 0x94, 0xaa, 0x84 ,0xf7, 0x02, 0x26, 0xe2, 0xfc, 0x26, 0x42, 0x9a, 0xe8, 0x4b, 0xdd, 0x34, 0x45, 0x9b, 0xb2, 0xf9, 0xa7, 0x5a, 0xc2 },
        { .cmd_in = true, .hid_in = true, .vendor_in = true, .hid_extra_in = true },
        process_input_urb
    },
    // 1b2e M65 fwver 3 HID
    {
        { 0xe7, 0xee, 0x44, 0xc0, 0x33, 0xdd, 0x4e, 0x7d, 0x4f, 0xd4, 0xac, 0x81, 0xee, 0x76, 0xf7, 0xbd, 0x8e, 0xf8, 0x61, 0x53, 0x5c, 0x5f, 0x45, 0xdb, 0xb9, 0x1e, 0xdf, 0xbd, 0xc5, 0x24, 0xef, 0x64 },
        { .cmd_in = true, .hid_in = true, .vendor_in = true, .hid_extra_in = true },
        process_input_urb
    },
    // fwver3 common vendor
    {
        { 0xf4, 0xce, 0x17, 0xcd, 0x77, 0xa9, 0x5f, 0x82, 0x2e, 0xd0, 0xaf, 0x59, 0xd9, 0x65, 0x02, 0x1a, 0x4b, 0xbc, 0xde, 0xc3, 0xb7, 0xe1, 0xf3, 0xd8, 0x0b, 0x55, 0x29, 0xe1, 0xde, 0xb3, 0x0e, 0xee },
        { .cmd_out = true },
        NULL
    },
};

void nxp_fill_input_eps(usbdevice* kb)
{
    // Match descriptors to interfaces and install input handlers
    for(uchar j = 0; j < kb->bNumInterfaces; j++) {
        for(size_t i = 0; i < sizeof(nxp_descriptors)/sizeof(*nxp_descriptors); i++) {
            if(!memcmp(nxp_descriptors[i].report_descriptor_hash, kb->hid_interfaces[j].report_descriptor_hash, crypto_hash_sha256_BYTES)) {
                ckb_info("MATCHED DESCR");
                kb->hid_interfaces[j].type = nxp_descriptors[i].type;
                // Input
                if(kb->hid_interfaces[j].endpoints[EP_LOOKUP_IN].bEndpointAddress && (nxp_descriptors[i].type.cmd_in || nxp_descriptors[i].type.hid_in || nxp_descriptors[i].type.vendor_in)) {
                    kb->hid_interfaces[j].endpoints[EP_LOOKUP_IN].handler = nxp_descriptors[i].handler;
                }

                // Output
                if(kb->hid_interfaces[j].endpoints[EP_LOOKUP_OUT].bEndpointAddress && nxp_descriptors[i].type.cmd_out) {
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

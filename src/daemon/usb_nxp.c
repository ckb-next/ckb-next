#include "structures.h"
#include "usb.h"
#include "device.h"

void nxp_fill_input_eps(usbdevice* kb)
{
    // M65 RGB Ultra has non-standard endpoint layout:
    // Interface 0: 0x82 (boot mouse), Interface 1: 0x84 (control IN), Interface 2: 0x83, Interface 3: 0x81
    if (kb->product == P_M65_RGB_ULTRA) {
        kb->input_endpoints[0] = 0x82;  // Interface 0: boot mouse
        kb->input_endpoints[1] = 0x84;  // Interface 1: control IN
        kb->input_endpoints[2] = 0x83;  // Interface 2
        return;
    }

    int epcount = (kb->product == P_ST100 ? kb->epcount : (kb->epcount - 1));
    for(int i = 0; i < epcount; i++)
        kb->input_endpoints[i] = (i + 1) | 0x80;
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

        // All firmware versions for normal HID devices have the OUT endpoint at the end
        // Devices with no input, such as the Polaris, have it at the start.
        // M65 RGB Ultra has OUT endpoint 0x04 at interface 1
        unsigned int ep;
        if (kb->product == P_M65_RGB_ULTRA)
            ep = 4;  // M65 Ultra uses endpoint 4 OUT (0x04)
        else
            ep = (IS_SINGLE_EP(kb) ? 1 : kb->epcount);
        int res = os_usb_interrupt_out(kb, ep, len, out, file, line);
        // Unlock on failure
        if(is_recv && res < 1)
            pthread_mutex_unlock(intmutex(kb));
        return res;
    } else {
        // Note, Ctrl Transfers require an index, not an endpoint, which is why kb->epcount - 1 works
        ctrltransfer transfer = { 0x21, 0x09, 0x0200, kb->epcount - 1, len, 5000, out };
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
        ctrltransfer transfer = { 0xa1, 0x01, 0x0300, kb->epcount - 1, len, 5000, in };
        return os_usb_control(kb, &transfer, file, line);
    }
}

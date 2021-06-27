#include "structures.h"
#include "usb.h"
#include "device.h"

void bragi_fill_input_eps(usbdevice* kb)
{
    for(int i = 0; i < kb->epcount; i++)
        kb->input_endpoints[i] = (i + 1) | 0x80;
}

int bragi_usb_write(usbdevice* kb, void* out, int len, int is_recv, const char* file, int line)
{
    if(len != 64)
    {
        ckb_fatal_fn("len != 64 not yet supported in Bragi backend", file, line);
        return -1;
    }

    // If we need to read a response, lock the interrupt mutex
    if(is_recv)
        if(pthread_mutex_lock(intmutex(kb)))
            ckb_fatal("Error locking interrupt mutex in os_usbsend()");

    // All firmware versions for normal HID devices have the OUT endpoint at the end
    // Devices with no input, such as the Polaris, have it at the start.
    unsigned int ep = (IS_SINGLE_EP(kb) ? 1 : kb->epcount);
    int res = os_usb_interrupt_out(kb, ep, len, out, file, line);
    // Unlock on failure
    if(is_recv && res < 1)
        pthread_mutex_unlock(intmutex(kb));
    return res;
}

int bragi_usb_read(usbdevice* kb, void* in, int len, int dummy, const char* file, int line)
{
    if(len != 64)
    {
        ckb_fatal_fn("len != 64 not yet supported in Bragi backend", file, line);
        return -1;
    }

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
}

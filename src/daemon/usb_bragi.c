#include "structures.h"
#include "usb.h"
#include "device.h"
#include "bragi_common.h"

void bragi_fill_input_eps(usbdevice* kb)
{
    for(int i = 0; i < kb->epcount; i++)
        kb->input_endpoints[i] = (i + 1) | 0x80;
}

int bragi_usb_write(usbdevice* kb, void* out, int len, int is_recv, const char* file, int line)
{
    BRAGI_PKT_SIZE_CHECK(len, file, line);

    // If we need to read a response, lock the interrupt mutex
    if(is_recv)
        if(pthread_mutex_lock(intmutex(kb)))
            ckb_fatal("Error locking interrupt mutex in os_usbsend()");

    int res = os_usb_interrupt_out(kb, kb->epcount, kb->out_ep_packet_size, out, file, line);
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

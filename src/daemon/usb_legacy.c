#include "structures.h"
#include "usb.h"

void legacy_fill_input_eps(usbdevice* kb)
{
    // FIXME: What do we do with this?
}

int legacy_dev_io(usbdevice* kb, void* out, int len, int is_recv, const char* file, int line)
{
    ctrltransfer* transfer = out;
    int res = os_usb_control(kb, transfer, file, line);

    // This is needed because we send controls without any data, and usbsend/usbrecv assume 0 is failure
    if(res == 0 && transfer->wLength == 0)
        return 1;

    return res;
}

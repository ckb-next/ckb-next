#include "structures.h"
#include "usb.h"

void nxp_fill_input_eps(usbdevice* kb)
{
    int epcount = (kb->product == P_ST100 ? kb->epcount : (kb->epcount - 1));
    for(int i = 0; i < epcount; i++)
        kb->input_endpoints[i] = (i + 1) | 0x80;
}

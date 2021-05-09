#include "structures.h"

void legacy_fill_input_eps(usbdevice* kb)
{
    for(int i = 0; i < kb->epcount; i++)
        kb->input_endpoints[i] = (i + 1) | 0x80;
}

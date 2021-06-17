#include "structures.h"
void nxp_fill_input_eps(usbdevice* kb);
int nxp_usb_write(usbdevice* kb, void* out, int len, int is_recv, const char* file, int line);
int nxp_usb_read(usbdevice* kb, void* in, int len, int dummy, const char* file, int line);

#include "structures.h"
void bragi_fill_input_eps(usbdevice* kb);
int bragi_usb_write(usbdevice* kb, void* out, int len, int is_recv, const char* file, int line);
int bragi_usb_read(usbdevice* kb, void* in, int len, int dummy, const char* file, int line);

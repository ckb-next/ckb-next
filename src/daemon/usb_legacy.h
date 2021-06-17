#include "structures.h"
void legacy_fill_input_eps(usbdevice* kb);
int legacy_dev_io(usbdevice* kb, void* out, int len, int is_recv, const char* file, int line);

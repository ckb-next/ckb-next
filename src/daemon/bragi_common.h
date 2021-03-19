#include "structures.h"
#include "usb.h"
int bragi_get_property(usbdevice* kb, const uchar prop);
int bragi_set_property(usbdevice* kb, const uchar prop, const uchar val);

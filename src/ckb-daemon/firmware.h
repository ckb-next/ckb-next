#ifndef FIRMWARE_H
#define FIRMWARE_H

#include "includes.h"

// Gets firmware version and poll rate from device. Returns 0 on success.
int getfwversion(usbdevice* kb);

// Updates firmware with data at the specified path. Prints notifications on success/failure.
// Returns 0 if the device is ok or -1 if it needs to be removed.
int cmd_fwupdate(usbdevice* kb, usbmode* dummy1, int nnumber, int dummy2, const char* path);

#endif  // FIRMWARE_H

#ifndef FIRMWARE_H
#define FIRMWARE_H

#include "includes.h"

// Gets firmware version from device. Returns 0 on success.
int getfwversion(usbdevice* kb);

// Updates firmware with data at the specified path. Prints notifications on success/failure.
void cmd_fwupdate(usbdevice* kb, const char* path);

#endif

#ifndef DPI_BRAGI_H
#define DPI_BRAGI_H

#include "includes.h"
#include "device.h"

// Command: Set lift height
void cmd_lift_bragi(usbdevice* kb, usbmode* mode, int dummy1, int dummy2, const char* height);
// Command: Enable/disable angle snap
void cmd_snap_bragi(usbdevice* kb, usbmode* mode, int dummy1, int dummy2, const char* enable);
int updatedpi_bragi(usbdevice* kb, int force);

#endif // DPI_BRAGI_H


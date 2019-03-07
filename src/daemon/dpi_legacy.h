#ifndef DPI_LEGACY_H
#define DPI_LEGACY_H

#include "includes.h"
#include "device.h"

// Command: Set lift height
void cmd_lift_legacy(usbdevice* kb, usbmode* mode, int dummy1, int dummy2, const char* height);
// Command: Enable/disable angle snap
void cmd_snap_legacy(usbdevice* kb, usbmode* mode, int dummy1, int dummy2, const char* enable);
int updatedpi_legacy(usbdevice* kb, int force);

#endif // DPI_LEGACY_H


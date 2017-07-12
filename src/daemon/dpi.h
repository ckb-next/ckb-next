#ifndef DPI_H
#define DPI_H

#include "includes.h"
#include "device.h"

// DPI functions are mouse-only; do not use them with keyboards.
// Lock dmutex before using commands (see device.h)

// Sends the current DPI values to a device. force = 0 to update only if changed, force = 1 to update no matter what. Returns 0 on success.
int updatedpi(usbdevice* kb, int force);
// Saves DPI states to device memory. Return 0 on success.
int savedpi(usbdevice* kb, dpiset* dpi, lighting* light);
// Loads DPI states from device memory. Returns 0 on success.
int loaddpi(usbdevice* kb, dpiset* dpi, lighting* light);

// Generates data for a DPI command to match the current state (analagous to printrgb).
// The result must be freed later.
char* printdpi(const dpiset* dpi, const usbdevice* kb);

// Command: Set DPI levels
void cmd_dpi(usbdevice* kb, usbmode* mode, int dummy, const char* stages, const char* values);
// Command: Select current DPI
void cmd_dpisel(usbdevice* kb, usbmode* mode, int dummy1, int dummy2, const char* stage);
// Command: Set lift height
void cmd_lift(usbdevice* kb, usbmode* mode, int dummy1, int dummy2, const char* height);
// Command: Enable/disable angle snap
void cmd_snap(usbdevice* kb, usbmode* mode, int dummy1, int dummy2, const char* enable);

#endif // DPI_H


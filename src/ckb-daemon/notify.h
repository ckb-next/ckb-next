#ifndef NOTIFY_H
#define NOTIFY_H

#include "includes.h"

// Prints output to a keyboard's notification node as well as the root notification node.
// Specify a null keyboard to print info for an unconnected device. Specify a null mode to omit the mode number.
// Setting may be null if kb is not null, in which case kb->setting will be used. They must not both be null.
void nprintf(usbdevice* kb, usbsetting* setting, usbmode* mode, const char* format, ...);

// Notifies of a device connection or disconnection.
void notifyconnect(int index, int connecting);

// Enables or disables notification for a key
void cmd_notify(usbmode* mode, int keycode, const char* toggle);

#endif

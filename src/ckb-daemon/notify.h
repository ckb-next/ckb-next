#ifndef NOTIFY_H
#define NOTIFY_H

#include "includes.h"

// Prints output to a keyboard's notification nodes as well as the root notification nodes.
// Specify a null keyboard to print info for an unconnected device. Specify a null mode to omit the mode number.
// Profile may be null if kb is not null, in which case kb->profile will be used. They must not both be null.
void nprintf(usbdevice* kb, usbprofile* profile, usbmode* mode, const char* format, ...);

// Prints output to the root notification nodes.
void nrprintf(const char* format, ...);

// Notifies of a device connection or disconnection.
void notifyconnect(usbdevice* kb, int connecting);

// Enables or disables notification for a key
void cmd_notify(usbmode* mode, const key* keymap, int keyindex, const char* toggle);

// Gets info about a particular setting.
void getinfo(usbdevice* kb, usbmode* mode, const char* setting);

#endif

#ifndef LED_H
#define LED_H

#include "includes.h"
#include "device.h"

// Note: Lock dmutex before using LED commands (see device.h)

// Send current RGB values to a device. force = 0 to update only if changed, force = 1 to update no matter what. Returns 0 on success.
int updatergb_kb(usbdevice* kb, int force);
int updatergb_mouse(usbdevice* kb, int force);
// Saves RGB data to device memory. Returns 0 on success.
int savergb_kb(usbdevice* kb, lighting* light, int mode);
int savergb_mouse(usbdevice* kb, lighting* light, int mode);
// Loads RGB data from device memory. Returns 0 on success.
int loadrgb_kb(usbdevice* kb, lighting* light, int mode);
int loadrgb_mouse(usbdevice* kb, lighting* light, int mode);

// Generates data for an RGB command to match the given RGB data. Returns a string like "ff0000" or "w:ff0000 a:00ff00 ..."
// The result must be freed later.
char* printrgb(const lighting* light, const usbdevice* kb);

// Command: Update an LED color
void cmd_rgb(usbdevice* kb, usbmode* mode, int dummy, int keyindex, const char* code);

// Command: Turn an indicator off permanently
void cmd_ioff(usbdevice* kb, usbmode* mode, int dummy1, int dummy2, const char* led);
// Command: Turn an indicator on permanently
void cmd_ion(usbdevice* kb, usbmode* mode, int dummy1, int dummy2, const char* led);
// Command: Set an indicator to automatic mode
void cmd_iauto(usbdevice* kb, usbmode* mode, int dummy1, int dummy2, const char* led);
// Command: Set indicator notifications
void cmd_inotify(usbdevice* kb, usbmode* mode, int nnumber, int dummy, const char* led);

#endif  // LED_H

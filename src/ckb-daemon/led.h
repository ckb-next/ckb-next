#ifndef LED_H
#define LED_H

#include "includes.h"
#include "usb.h"

// Initialize RGB data.
void initrgb(keylight* light);
// Update a device's LEDs with RGB data.
void updatergb(usbdevice* kb, int force);
// Saves RGB data for a device profile.
void savergb(usbdevice* kb, int mode);
// Loads RGB data for a device profile. Returns 0 on success.
int loadrgb(usbdevice* kb, keylight* light, int mode);

// Generates data for an RGB command to match the given RGB data. Returns a string like "ff0000" or "w:ff0000 a:00ff00 ..."
// The result must be freed later.
char* printrgb(const keylight* light, const usbdevice* kb);

// Turns LEDs off
void cmd_rgboff(usbdevice* kb, usbmode* mode);
// Turns LEDs on
void cmd_rgbon(usbdevice* kb, usbmode* mode);
// Updates an LED color
void cmd_rgb(usbdevice* kb, usbmode* mode, const key* keymap, int dummy, int keyindex, const char* code);

// Turns an indicator off permanently
void cmd_ioff(usbdevice* kb, usbmode* mode, const key* keymap, int dummy1, int dummy2, const char* led);
// Turns an indicator on permanently
void cmd_ion(usbdevice* kb, usbmode* mode, const key* keymap, int dummy1, int dummy2, const char* led);
// Sets an indicator to automatic mode
void cmd_iauto(usbdevice* kb, usbmode* mode, const key* keymap, int dummy1, int dummy2, const char* led);
// Sets indicator notifications
void cmd_inotify(usbdevice* kb, usbmode* mode, const key* keymap, int nnumber, int dummy, const char* led);

// Daemon frame rate
extern volatile unsigned fps;
// Set frame rate
void setfps(unsigned newfps);

#endif

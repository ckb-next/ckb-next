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
char* printrgb(keylight* light, const key* keymap);

// Turns LEDs off
void cmd_rgboff(usbmode* mode);
// Turns LEDs on
void cmd_rgbon(usbmode* mode);
// Updates an LED color
void cmd_rgb(usbmode* mode, const key* keymap, int keyindex, const char* code);

// Turns an indicator off permanently
void cmd_ioff(usbmode* mode, const key* keymap, int zero, const char* led);
// Turns an indicator on permanently
void cmd_ion(usbmode* mode, const key* keymap, int zero, const char* led);
// Sets an indicator to automatic mode
void cmd_iauto(usbmode* mode, const key* keymap, int zero, const char* led);

// Daemon frame rate
extern volatile unsigned fps;
// Set frame rate
void setfps(unsigned newfps);

#endif

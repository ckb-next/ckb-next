#ifndef LED_H
#define LED_H

#include "includes.h"
#include "usb.h"

// Initialize RGB data.
void initrgb(keylight* light);
// Update a device's LEDs with RGB data.
void updatergb(usbdevice* kb);
// Saves RGB data for a device profile.
void savergb(usbdevice* kb, int mode);
// Loads RGB data for a device profile.
void loadrgb(usbdevice* kb, int mode);

// Turns LEDs off
void cmd_rgboff(usbmode* mode);
// Turns LEDs on
void cmd_rgbon(usbmode* mode);
// Updates an LED color
void cmd_rgb(usbmode* mode, int keyindex, const char* code);

// Turns an indicator off permanently
void cmd_ioff(usbmode* mode, int zero, const char* led);
// Turns an indicator on permanently
void cmd_ion(usbmode* mode, int zero, const char* led);
// Sets an indicator to automatic mode
void cmd_iauto(usbmode* mode, int zero, const char* led);

#endif

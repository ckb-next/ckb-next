#ifndef LED_H
#define LED_H

#include "includes.h"
#include "usb.h"

// Initialize RGB data.
void initrgb(keylight* light);
// Update a device's LEDs with RGB data.
void updateleds(usbdevice* kb);

// Turns LEDs off
void cmd_ledoff(usbmode* mode);
// Turns LEDs on
void cmd_ledon(usbmode* mode);
// Updates an LED color
void cmd_ledrgb(usbmode* mode, int keyindex, const char* code);

#endif

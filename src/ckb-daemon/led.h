#ifndef LED_H
#define LED_H

#include "includes.h"
#include "usb.h"

// Initialize RGB data.
void initrgb(keylight* light);
// Update a device's LEDs with RGB data.
void updateleds(usbdevice* kb);

// Turns LEDs off
void cmd_ledoff(usbprofile* profile);
// Turns LEDs on
void cmd_ledon(usbprofile* profile);
// Updates an LED color
void cmd_ledrgb(usbprofile* profile, int keyindex, const char* code);

#endif

#ifndef LED_H
#define LED_H

#include "includes.h"
#include "usb.h"

// Initialize RGB data.
void initrgb(short** rgb, int* rgbon);
// Update a device's LEDs with RGB data.
void updateleds(usbdevice* kb, short* rgb);

// Turns LEDs off
void cmd_ledoff(usbdevice* kb);
// Turns LEDs on
void cmd_ledon(usbdevice* kb);
// Updates an LED color
void cmd_ledrgb(usbdevice* kb, int keyindex, const char* code);

#endif

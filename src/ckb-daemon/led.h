#ifndef LED_H
#define LED_H

#include "usb.h"

// Initialize RGB data.
void initrgb(short** rgb, int* rgbon);
// Update a device's LEDs with RGB data.
void updateleds(usbdevice* kb, short* rgb);
// Read and process input from an LED FIFO.
void readled(usbdevice* kb, char* line);

#endif

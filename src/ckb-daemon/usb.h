#ifndef USB_H
#define USB_H

#include "includes.h"
#include "keyboard.h"

// Vendor/product codes
#define V_CORSAIR   0x1b1c
#define P_K70       0x1b13
#define P_K95       0x1b11

// Start the USB system. Returns 0 on success
int usbinit();
// USB main loop callback
void usbmainloop();
// Stop the USB system.
void usbdeinit();

// Set up a USB device after all its handles are open. Returns 0 on success
int setupusb(int index);
// Close a USB device and remove device entry. Returns 0 on success
int closeusb(int index);
// Close handles on a USB device
void closehandle(usbdevice* kb);

// Add a message to a USB device to be sent to the device. Returns 0 on success.
int usbqueue(usbdevice* kb, unsigned char* messages, int count);
// Output a message from the USB queue to the device, if any. Returns number of bytes written.
int usbdequeue(usbdevice* kb);
// Gets input from a USB device.
int usbinput(usbdevice* kb, unsigned char* message);

#endif

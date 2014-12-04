#ifndef USB_H
#define USB_H

#include "includes.h"
#include "keyboard.h"

// Vendor/product codes
#define V_CORSAIR       0x1b1c
#define P_K70           0x1b13
#define P_K95           0x1b11
#define V_CORSAIR_STR   "1b1c"
#define P_K70_STR       "1b13"
#define P_K95_STR       "1b11"

// USB delays for when the keyboards get picky about timing
#define DELAY_SHORT     usleep(3000)
#define DELAY_MEDIUM    usleep(10000)
#define DELAY_LONG      usleep(100000)

// Start the USB system. Returns 0 on success
int usbinit();
// Stop the USB system.
void usbdeinit();

// Set up a USB device after all its handles are open. Returns 0 on success
// Threading: Creates device mutex and locks it. Unlocks mutex ONLY if return is -1 (software error). Unlock manually otherwise.
int setupusb(usbdevice* kb);
// Close a USB device and remove device entry. Returns 0 on success
// Threading: Lock the device mutex BEFORE calling this. It will be released.
int closeusb(usbdevice* kb);
// Reset a USB device. Returns 0 on success, -1 if device should be removed
// Threading: Lock the device mutex before calling
int _resetusb(usbdevice* kb, const char* file, int line);
#define resetusb(kb) _resetusb(kb, __FILE_NOPATH__, __LINE__)
// Close handles on a USB device
// Threading: Lock device before use, unlock after finish
void closehandle(usbdevice* kb);

// Add a message to a USB device to be sent to the device. Returns 0 on success.
// Threading: Lock device before use, unlock after finish
int usbqueue(usbdevice* kb, uchar* messages, int count);
// Output a message from the USB queue to the device, if any. Returns number of bytes written, zero on failure, or -1 if the queue was empty.
// If the message was not sent successfully it will not be removed from the queue.
// Threading: Lock device before use, unlock after finish
int _usbdequeue(usbdevice* kb, const char* file, int line);
#define usbdequeue(kb) _usbdequeue(kb, __FILE_NOPATH__, __LINE__)
// Gets input from a USB device.
// Threading: Lock device before use, unlock after finish
int _usbinput(usbdevice* kb, uchar* message, const char* file, int line);
#define usbinput(kb, message) _usbinput(kb, message, __FILE_NOPATH__, __LINE__)

// Try to perform a task, reset device if needed. Returns zero on success. The action will NOT be re-attempted if the reset is successful.
int usb_tryreset(usbdevice* kb);

#endif

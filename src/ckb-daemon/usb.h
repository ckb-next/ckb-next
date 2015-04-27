#ifndef USB_H
#define USB_H

#include "includes.h"
#include "keyboard.h"

// Vendor/product codes
#define V_CORSAIR       0x1b1c
#define V_CORSAIR_STR   "1b1c"

#define P_K65           0x1b17
#define P_K65_STR       "1b17"
#define IS_K65(kb) ((kb)->vendor == V_CORSAIR && (kb)->product == P_K65)

#define P_K70           0x1b13
#define P_K70_STR       "1b13"
#define P_K70_NRGB      0x1b09
#define P_K70_NRGB_STR  "1b09"
#define IS_K70(kb) ((kb)->vendor == V_CORSAIR && ((kb)->product == P_K70 || (kb)->product == P_K70_NRGB))

#define P_K95           0x1b11
#define P_K95_STR       "1b11"
#define P_K95_NRGB      0x1b08
#define P_K95_NRGB_STR  "1b08"
#define IS_K95(kb) ((kb)->vendor == V_CORSAIR && ((kb)->product == P_K95 || (kb)->product == P_K95_NRGB))

const char* vendor_str(short vendor);
const char* product_str(short product);

#define IS_RGB(vendor, product) ((product) != (P_K70_NRGB) && (product) != (P_K95_NRGB))

// USB delays for when the keyboards get picky about timing
#define DELAY_SHORT     usleep(2000)
#define DELAY_MEDIUM    usleep(20000)
#define DELAY_LONG      usleep(200000)

// Start the USB system. Returns 0 on success
int usbinit();
// Stop the USB system.
void usbdeinit();

// Set up a USB device after all its handles are open. Returns 0 on success
// Threading: Creates device mutex and locks it. Unlocks mutex ONLY if return is -1 (software error). Unlock manually otherwise.
int setupusb(usbdevice* kb, short vendor, short product);
// Puts a USB device back into hardware mode. Returns 0 on success.
// Threading: Lock device mutex before calling.
int revertusb(usbdevice* kb);
// Close a USB device and remove device entry. Returns 0 on success
// Threading: Lock the device mutex before calling. It will be released.
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

// Non-RGB K95 command. Returns 0 on success.
int _nk95cmd(usbdevice* kb, uchar bRequest, ushort wValue, const char* file, int line);
#define nk95cmd(kb, command) _nk95cmd(kb, (command) >> 16 & 0xFF, (command) & 0xFFFF, __FILE_NOPATH__, __LINE__)

#define NK95_HWOFF  0x020030    // Hardware playback off
#define NK95_HWON   0x020001    // Hardware playback on
#define NK95_M1     0x140001    // Mode switches
#define NK95_M2     0x140002
#define NK95_M3     0x140003

// Tries to reset a USB device after a failed action. Returns 0 on success.
// The previous action will NOT be re-attempted and the keyboard's USB queue will be cleared.
int usb_tryreset(usbdevice* kb);

#endif

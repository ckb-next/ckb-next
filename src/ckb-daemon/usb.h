#ifndef USB_H
#define USB_H

#include "includes.h"
#include "keymap.h"

// Vendor/product codes
#define V_CORSAIR       0x1b1c
#define V_CORSAIR_STR   "1b1c"

#define P_K65           0x1b17
#define P_K65_STR       "1b17"
#define IS_K65(kb)      ((kb)->vendor == V_CORSAIR && (kb)->product == P_K65)

#define P_K70           0x1b13
#define P_K70_STR       "1b13"
#define P_K70_NRGB      0x1b09
#define P_K70_NRGB_STR  "1b09"
#define IS_K70(kb)      ((kb)->vendor == V_CORSAIR && ((kb)->product == P_K70 || (kb)->product == P_K70_NRGB))

#define P_K95           0x1b11
#define P_K95_STR       "1b11"
#define P_K95_NRGB      0x1b08
#define P_K95_NRGB_STR  "1b08"
#define IS_K95(kb)      ((kb)->vendor == V_CORSAIR && ((kb)->product == P_K95 || (kb)->product == P_K95_NRGB))

#define P_STRAFE            0x1b20
#define P_STRAFE_STR        "1b20"
#define P_STRAFE_NRGB       0x1b15
#define P_STRAFE_NRGB_STR   "1b15"
#define IS_STRAFE(kb)       ((kb)->vendor == V_CORSAIR && ((kb)->product == P_STRAFE || (kb)->product == P_STRAFE_NRGB))

#define P_M65           0x1b12
#define P_M65_STR       "1b12"
#define IS_M65(kb)      ((kb)->vendor == V_CORSAIR && ((kb)->product == P_M65))

#define P_SABRE_O       0x1b14
#define P_SABRE_O_STR   "1b14"
#define P_SABRE_L       0x1b19
#define P_SABRE_L_STR   "1b19"
#define IS_SABRE(kb)    ((kb)->vendor == V_CORSAIR && ((kb)->product == P_SABRE_O || (kb)->product == P_SABRE_L))

#define P_SCIMITAR      0x1b1e
#define P_SCIMITAR_STR  "1b1e"
#define IS_SCIMITAR(kb) ((kb)->vendor == V_CORSAIR && ((kb)->product == P_SCIMITAR))

// NOTE: when adding new devices please update src/ckb/fwupgradedialog.cpp as well.
// It should contain the same vendor/product IDs for any devices supporting firmware updates.

//uncomment to see USB packets sent to the device
//#define DEBUG_USB

const char* vendor_str(short vendor);
const char* product_str(short product);

// RGB vs non-RGB test
// (note: non-RGB Strafe is still considered "RGB" in that it shares the same protocol. The difference is denoted with the "monochrome" feature)
#define IS_RGB(vendor, product)         ((vendor) == (V_CORSAIR) && (product) != (P_K70_NRGB) && (product) != (P_K95_NRGB))
#define IS_MONOCHROME(vendor, product)  ((vendor) == (V_CORSAIR) && (product) == (P_STRAFE_NRGB))
#define IS_RGB_DEV(kb)                  IS_RGB((kb)->vendor, (kb)->product)
#define IS_MONOCHROME_DEV(kb)           IS_MONOCHROME((kb)->vendor, (kb)->product)

// Mouse vs keyboard test
#define IS_MOUSE(vendor, product)       ((vendor) == (V_CORSAIR) && ((product) == (P_M65) || (product) == (P_SABRE_O) || (product) == (P_SABRE_L) || (product) == (P_SCIMITAR)))
#define IS_MOUSE_DEV(kb)                IS_MOUSE((kb)->vendor, (kb)->product)

// USB delays for when the keyboards get picky about timing
#define DELAY_SHORT(kb)     usleep((int)(kb)->usbdelay * 1000)  // base (default: 5ms)
#define DELAY_MEDIUM(kb)    usleep((int)(kb)->usbdelay * 10000) // x10 (default: 50ms)
#define DELAY_LONG(kb)      usleep(100000)                      // long, fixed 100ms
#define USB_DELAY_DEFAULT   5

// Start the USB main loop. Returns program exit code when finished
int usbmain();
// Stop the USB system.
void usbkill();

// Note: Lock a device's dmutex (see device.h) before accessing the USB interface.

// Set up a USB device after its handle is open. Spawns a new thread.
// dmutex must be locked prior to calling this function. The function will unlock it when finished.
void setupusb(usbdevice* kb);
// OS-specific setup. Return 0 on success.
int os_setupusb(usbdevice* kb);
// Per keyboard input thread (OS specific). Will be detached from the main thread, so it needs to clean up its own resources.
void* os_inputmain(void* kb);

// Puts a USB device back into hardware mode. Returns 0 on success.
int revertusb(usbdevice* kb);
// Close a USB device and remove device entry. Returns 0 on success
int closeusb(usbdevice* kb);
void os_closeusb(usbdevice* kb);
// Reset a USB device. Returns 0 on success, -1 if device should be removed
int _resetusb(usbdevice* kb, const char* file, int line);
#define resetusb(kb) _resetusb(kb, __FILE_NOPATH__, __LINE__)
int os_resetusb(usbdevice* kb, const char* file, int line);

// Write data to a USB device. Returns number of bytes written or zero on failure.
int _usbsend(usbdevice* kb, const uchar* messages, int count, const char* file, int line);
#define usbsend(kb, messages, count) _usbsend(kb, messages, count, __FILE_NOPATH__, __LINE__)
// Requests data from a USB device by first sending an output packet and the reading the response. Returns number of bytes read or zero on failure.
int _usbrecv(usbdevice* kb, const uchar* out_msg, uchar* in_msg, const char* file, int line);
#define usbrecv(kb, out_msg, in_msg) _usbrecv(kb, out_msg, in_msg, __FILE_NOPATH__, __LINE__)

// OS: Send a USB message to the device. Return number of bytes written, zero for permanent failure, -1 for try again
int os_usbsend(usbdevice* kb, const uchar* out_msg, int is_recv, const char* file, int line);
// OS: Gets input from a USB device. Return same as above.
int os_usbrecv(usbdevice* kb, uchar* in_msg, const char* file, int line);
// OS: Update HID indicator LEDs (Num Lock, Caps, etc). Read from kb->ileds.
void os_sendindicators(usbdevice* kb);

// Non-RGB K95 command. Returns 0 on success.
int _nk95cmd(usbdevice* kb, uchar bRequest, ushort wValue, const char* file, int line);
#define nk95cmd(kb, command) _nk95cmd(kb, (command) >> 16 & 0xFF, (command) & 0xFFFF, __FILE_NOPATH__, __LINE__)

#define NK95_HWOFF  0x020030    // Hardware playback off
#define NK95_HWON   0x020001    // Hardware playback on
#define NK95_M1     0x140001    // Mode switches
#define NK95_M2     0x140002
#define NK95_M3     0x140003

// Tries to reset a USB device after a failed action. Returns 0 on success.
// The previous action will NOT be re-attempted.
int usb_tryreset(usbdevice* kb);

#endif  // USB_H

#ifndef INPUT_H
#define INPUT_H

#include "includes.h"
#include "usb.h"

// Opens uinput device
int uinputopen(int index, const struct libusb_device_descriptor* descriptor);
// Closes uinput device
void uinputclose(int index);

// Updates keypresses on uinput device
void uinputupdate(usbdevice* kb);
// Read LEDs from the event device and update them (if needed).
void updateindicators(usbdevice* kb, int force);

// Initializes key bindings for a device
void initbind(keybind* bind);
// Frees key binding data for a device
void closebind(keybind* bind);

// Binds a key
void cmd_bind(usbdevice* kb, int keyindex, const char* to);
// Unbinds a key
void cmd_unbind(usbdevice* kb, int keyindex, const char* ignored);
// Resets a key binding
void cmd_rebind(usbdevice* kb, int keyindex, const char* ignored);
// Creates or updates a macro
void cmd_macro(usbdevice* kb, const char* keys, const char* assignment);
// Clears all macros
void cmd_macroclear(usbdevice* kb);

#endif

#ifndef INPUT_H
#define INPUT_H

#include "includes.h"
#include "usb.h"

// Opens uinput device
int inputopen(int index, const struct libusb_device_descriptor* descriptor);
// Closes uinput device
void inputclose(int index);

// Updates keypresses on uinput device
void inputupdate(usbdevice* kb);
// Read LEDs from the event device and update them (if needed).
void updateindicators(usbdevice* kb, int force);

// OS-specific event handlers
// Generate a keypress event
void os_keypress(usbdevice* kb, int scancode, int down);
// Generate a SYN event
void os_kpsync(usbdevice* kb);
// Read the indicator LED state. Returns 1 if it changed
int os_readind(usbdevice* kb);

// Initializes key bindings for a device
void initbind(keybind* bind);
// Frees key binding data for a device
void closebind(keybind* bind);

// Binds a key
void cmd_bind(usbmode* mode, int keyindex, const char* to);
// Unbinds a key
void cmd_unbind(usbmode* mode, int keyindex, const char* ignored);
// Resets a key binding
void cmd_rebind(usbmode* mode, int keyindex, const char* ignored);
// Creates or updates a macro
void cmd_macro(usbmode* mode, const char* keys, const char* assignment);
// Clears all macros
void cmd_macroclear(usbmode* mode);

#endif

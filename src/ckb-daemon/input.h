#ifndef INPUT_H
#define INPUT_H

#include "includes.h"
#include "usb.h"

// Opens uinput device
int inputopen(usbdevice* kb);
// Closes uinput device
void inputclose(usbdevice* kb);

// Updates keypresses on uinput device
void inputupdate(usbdevice* kb);
// Read LEDs from the event device and update them (if needed).
void updateindicators(usbdevice* kb, int force);

// OS-specific event handlers
// Generate a keypress event
void os_keypress(usbdevice* kb, int scancode, int down);
// Generate a SYN event
void os_kpsync(usbdevice* kb);

// Initializes key bindings for a device
void initbind(keybind* bind, const key* keymap);
// Frees key binding data for a device
void closebind(keybind* bind);

// Binds a key
void cmd_bind(usbmode* mode, const key* keymap, int keyindex, const char* to);
// Unbinds a key
void cmd_unbind(usbmode* mode, const key* keymap, int keyindex, const char* ignored);
// Resets a key binding
void cmd_rebind(usbmode* mode, const key* keymap, int keyindex, const char* ignored);
// Creates or updates a macro
void cmd_macro(usbmode* mode, const key* keymap, const char* keys, const char* assignment);
// Clears all macros
void cmd_macroclear(usbmode* mode);

#endif

#ifndef INPUT_H
#define INPUT_H

#include "includes.h"
#include "usb.h"

// Opens input device
int inputopen(usbdevice* kb);
// Closes input device
void inputclose(usbdevice* kb);

// Updates keypresses on input device
void inputupdate(usbdevice* kb);
// Read LEDs from the event device and update them (if needed).
void updateindicators(usbdevice* kb, int force);

// Initializes key bindings for a device
void initbind(keybind* bind, const key* keymap);
// Frees key binding data for a device
void closebind(keybind* bind);

// Binds a key
void cmd_bind(usbdevice* kb, usbmode* mode, int dummy, int keyindex, const char* to);
// Unbinds a key
void cmd_unbind(usbdevice* kb, usbmode* mode, int dummy, int keyindex, const char* ignored);
// Resets a key binding
void cmd_rebind(usbdevice* kb, usbmode* mode, int dummy, int keyindex, const char* ignored);
// Creates or updates a macro
void cmd_macro(usbdevice* kb, usbmode* mode, const char* keys, const char* assignment);
// Clears all macros
void cmd_macroclear(usbdevice* kb, usbmode* mode);

#ifdef OS_LINUX
// Is a key a modifier?
#define IS_MOD(s) ((s) == KEY_CAPSLOCK || (s) == KEY_NUMLOCK || (s) == KEY_SCROLLLOCK || (s) == KEY_LEFTSHIFT || (s) == KEY_RIGHTSHIFT || (s) == KEY_LEFTCTRL || (s) == KEY_RIGHTCTRL || (s) == KEY_LEFTMETA || (s) == KEY_RIGHTMETA || (s) == KEY_LEFTALT || (s) == KEY_RIGHTALT)
#else
// Scroll Lock and Num Lock aren't modifiers on OSX
#define IS_MOD(s) ((s) == KEY_CAPSLOCK || (s) == KEY_LEFTSHIFT || (s) == KEY_RIGHTSHIFT || (s) == KEY_LEFTCTRL || (s) == KEY_RIGHTCTRL || (s) == KEY_LEFTMETA || (s) == KEY_RIGHTMETA || (s) == KEY_LEFTALT || (s) == KEY_RIGHTALT)
#endif

// OS-specific event handlers

// Generate a keypress event
void os_keypress(usbdevice* kb, int scancode, int down);
// Synchronize key input (called after sending key presses)
void os_kpsync(usbdevice* kb);
// Updates indicator state. Should read state, update ileds (applying mask for current mode as appropriate) and send control message to keyboard
void os_updateindicators(usbdevice* kb, int force);

#endif

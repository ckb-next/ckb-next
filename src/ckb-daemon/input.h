#ifndef INPUT_H
#define INPUT_H

#include "includes.h"
#include "usb.h"

// Opens input device
int inputopen(usbdevice* kb);
// Closes input device
void inputclose(usbdevice* kb);

// Updates keypresses on input device. Lock imutex first (see device.h).
void inputupdate(usbdevice* kb);
// Read indicator LED state and send it back to the keyboard if needed. Lock dmutex first.
void updateindicators_kb(usbdevice* kb, int force);

// Initializes key bindings for a device. Lock imutex first.
void initbind(keybind* bind);
// Frees key binding data for a device. Lock imutex first.
void freebind(keybind* bind);

// Note: unlike most commands, bind commands will lock/unlock the device's imutex
// Binds a key
void cmd_bind(usbdevice* kb, usbmode* mode, int dummy, int keyindex, const char* to);
// Unbinds a key
void cmd_unbind(usbdevice* kb, usbmode* mode, int dummy, int keyindex, const char* ignored);
// Resets a key binding
void cmd_rebind(usbdevice* kb, usbmode* mode, int dummy, int keyindex, const char* ignored);
// Creates or updates a macro. Pass null strings to clear all macros
void cmd_macro(usbdevice* kb, usbmode* mode, const char* keys, const char* assignment);

#ifdef OS_LINUX
// Is a key a modifier?
#define IS_MOD(s) ((s) == KEY_CAPSLOCK || (s) == KEY_NUMLOCK || (s) == KEY_SCROLLLOCK || (s) == KEY_LEFTSHIFT || (s) == KEY_RIGHTSHIFT || (s) == KEY_LEFTCTRL || (s) == KEY_RIGHTCTRL || (s) == KEY_LEFTMETA || (s) == KEY_RIGHTMETA || (s) == KEY_LEFTALT || (s) == KEY_RIGHTALT)
#else
// Scroll Lock and Num Lock aren't modifiers on OSX
#define IS_MOD(s) ((s) == KEY_CAPSLOCK || (s) == KEY_LEFTSHIFT || (s) == KEY_RIGHTSHIFT || (s) == KEY_LEFTCTRL || (s) == KEY_RIGHTCTRL || (s) == KEY_LEFTMETA || (s) == KEY_RIGHTMETA || (s) == KEY_LEFTALT || (s) == KEY_RIGHTALT)
#endif

// OS-specific event handlers. Should only be called within the above functions.

// Generate a keypress or mouse button event
void os_keypress(usbdevice* kb, int scancode, int down);
// Generate mouse movement
void os_mousemove(usbdevice* kb, int x, int y);
// Synchronize input (called after sending key presses)
void os_isync(usbdevice* kb);
// Updates indicator state. Should read state, update ileds (applying mask for current mode as appropriate) and send control message to keyboard
void os_updateindicators(usbdevice* kb, int force);

#endif  // INPUT_H

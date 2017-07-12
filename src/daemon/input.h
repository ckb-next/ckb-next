#ifndef INPUT_H
#define INPUT_H

#include "includes.h"
#include "usb.h"

// Open input device. Return 0 on success
int os_inputopen(usbdevice* kb);
// Close input device
void os_inputclose(usbdevice* kb);

// Updates keypresses on input device. Lock imutex first (see device.h).
void inputupdate(usbdevice* kb);
// Read indicator LED state and send it back to the keyboard if needed. Lock dmutex first.
void updateindicators_kb(usbdevice* kb, int force);

// Initializes key bindings for a device. Lock imutex first.
void initbind(binding* bind);
// Frees key binding data for a device. Lock imutex first.
void freebind(binding* bind);

// Note: unlike most commands, bind commands will lock/unlock the device's imutex
// Binds a key
void cmd_bind(usbdevice* kb, usbmode* mode, int dummy, int keyindex, const char* to);
// Unbinds a key
void cmd_unbind(usbdevice* kb, usbmode* mode, int dummy, int keyindex, const char* ignored);
// Resets a key binding
void cmd_rebind(usbdevice* kb, usbmode* mode, int dummy, int keyindex, const char* ignored);
// Creates or updates a macro. Pass null strings to clear all macros
void cmd_macro(usbdevice* kb, usbmode* mode, const int notifynumber, const char* keys, const char* assignment);

#ifdef OS_LINUX
// Is a key a modifier?
#define IS_MOD(s) ((s) == KEY_CAPSLOCK || (s) == KEY_NUMLOCK || (s) == KEY_SCROLLLOCK || (s) == KEY_LEFTSHIFT || (s) == KEY_RIGHTSHIFT || (s) == KEY_LEFTCTRL || (s) == KEY_RIGHTCTRL || (s) == KEY_LEFTMETA || (s) == KEY_RIGHTMETA || (s) == KEY_LEFTALT || (s) == KEY_RIGHTALT || (s) == KEY_FN)
#else
// Scroll Lock and Num Lock aren't modifiers on OSX
#define IS_MOD(s) ((s) == KEY_CAPSLOCK || (s) == KEY_LEFTSHIFT || (s) == KEY_RIGHTSHIFT || (s) == KEY_LEFTCTRL || (s) == KEY_RIGHTCTRL || (s) == KEY_LEFTMETA || (s) == KEY_RIGHTMETA || (s) == KEY_LEFTALT || (s) == KEY_RIGHTALT || (s) == KEY_FN)
#endif

// OS-specific event handlers. Should only be called within the above functions.

// Generate a keypress or mouse button event
void os_keypress(usbdevice* kb, int scancode, int down);
// Generate mouse movement
void os_mousemove(usbdevice* kb, int x, int y);

// Perform OS-specific setup for indicator lights. Called when the device is created. Return 0 on success.
int os_setupindicators(usbdevice* kb);

///
/// \brief struct parameter contains the values for a fresh started macro_play thread.
/// \a parameter_t is the typedef for it.
///
typedef struct parameter {
    usbdevice* kb;
    keymacro* macro;
} parameter_t;

/// \brief struct ptlist is one element in the single linked list to store  macro_play threads waiting for their execution
/// \a ptlist_t is the typedef for it.
///
typedef struct ptlist {
    struct ptlist* next;
    pthread_t thread_id;
} ptlist_t;

#endif  // INPUT_H

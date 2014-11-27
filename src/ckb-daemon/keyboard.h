#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "keyboard_mac.h"

#ifndef KEY_GRAVE_UK
// On OSX the grave key has a different code depending on the layout
// On Linux it does not
#define KEY_GRAVE_UK KEY_GRAVE
#endif

#define KEY_NONE    -1
#define KEY_CORSAIR -2
#define KEY_UNBOUND -3

// Number of keys
#define N_KEYS 144

// Map from key name to LED code and USB scan code
typedef struct {
    const char* name;
    short led;
    short scan;
} key;

// List of keys, ordered according to where they appear in the keyboard input
extern const key keymap_us[N_KEYS];
extern const key keymap_uk[N_KEYS];

// System-selected key map
extern const key* keymap_system;

// Get a keyboard layout from a string name. Returns null if the name is not supported.
// Supported:
//  "us" => US layout
//  "uk" => UK layout
const key* getkeymap(const char* name);

#endif

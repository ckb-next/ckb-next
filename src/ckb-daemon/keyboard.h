#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "keyboard_mac.h"

#ifndef KEY_GRAVE_UK
// On OSX the grave key has a different code depending on the layout
// On Linux it does not
#define KEY_GRAVE_UK KEY_GRAVE
#endif

// Number of keys
#define N_KEYS 144

// Map from key name to LED code and USB scan code
typedef struct {
    const char* name;
    short led;
    short scan;
} key;

// List of keys, ordered according to where they appear in the keyboard input
extern key keymap[N_KEYS];

#endif

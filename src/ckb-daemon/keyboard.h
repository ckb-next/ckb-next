#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "includes.h"
#include "keyboard_mac.h"

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

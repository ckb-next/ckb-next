#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "keyboard_mac.h"

#define KEY_NONE    -1
#define KEY_CORSAIR -2
#define KEY_UNBOUND -3

#ifdef OS_MAC
// On Linux there's no meaningful distinction between these keys
// On OSX this is used for layout auto-detection (see input_mac.c)
#define KEY_BACKSLASH_ISO -99
#else
#define KEY_BACKSLASH_ISO KEY_BACKSLASH
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
extern const key keymap[N_KEYS];

// Translates input from HID to a Corsair RGB input bitfield.
// Use positive endpoint for non-RGB keyboards, negative endpoint for RGB
void hid_translate(unsigned char* kbinput, int endpoint, int length, const unsigned char* urbinput);

#endif

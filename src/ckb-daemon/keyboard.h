#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "keyboard_mac.h"

#ifndef KEY_GRAVE_ISO
// On OSX the grave key has a different code depending on the layout
// On Linux it does not
#define KEY_GRAVE_ISO KEY_GRAVE
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
extern const key keymap_de[N_KEYS];
extern const key keymap_es[N_KEYS];
extern const key keymap_fr[N_KEYS];
extern const key keymap_gb[N_KEYS];
extern const key keymap_se[N_KEYS];
extern const key keymap_us[N_KEYS];

// System-selected key map
extern const key* keymap_system;

// Get a keyboard layout from a string name. Returns null if the name is not supported.
// Supported:
//  "de" => German layout
//  "es" => Spanish layout
//  "fr" => French layout
//  "gb" => UK layout
//  "se" => Swedish layout
//  "us" => US layout
const key* getkeymap(const char* name);
const char* getmapname(const key* layout);

// Translates input from HID to a Corsair RGB input bitfield.
// Use positive endpoint for non-RGB keyboards, negative endpoint for RGB
void hid_translate(unsigned char* kbinput, int endpoint, int length, const unsigned char* urbinput);

#endif

#ifndef KEYMAP_H
#define KEYMAP_H

#include "keymap_mac.h"

// Reserved keys (no input)
#define KEY_NONE    -1
#define KEY_CORSAIR -2
#define KEY_UNBOUND -3

// The mouse wheel is actually a relative axis, but we treat it like a pair of buttons
#define BTN_WHEELUP     0x1f01
#define BTN_WHEELDOWN   0x1f02

#ifdef OS_MAC
// On Linux there's no meaningful distinction between these keys
// On OSX this is used for layout auto-detection (see input_mac.c)
#define KEY_BACKSLASH_ISO 0xfff
#else
#define KEY_BACKSLASH_ISO KEY_BACKSLASH
#endif

// Number of keys (according to keyboard hardware)
#define N_KEYS_KB               144
#define N_KEYBYTES_KB           ((N_KEYS_KB + 7) / 8)
// Keys recognized by the driver but not present on keyboard
#define N_KEYS_EXTRA            12
// Mouse buttons
#define N_MOUSE_BUTTONS         13
#define MOUSE_BUTTON_FIRST      (N_KEYS_KB + N_KEYS_EXTRA)
// Number of keys that generate input
#define N_KEYS_INPUT            (MOUSE_BUTTON_FIRST + N_MOUSE_BUTTONS)
#define N_KEYBYTES_INPUT        ((N_KEYS_INPUT + 7) / 8)
// Mouse zones (have LED codes but don't generate input)
#define LED_MOUSE               N_KEYS_KB
#define N_MOUSE_ZONES           4
#define N_MOUSE_ZONES_EXTENDED  10
// Total number of keys recognized by software
#define N_KEYS_EXTENDED         (N_KEYS_INPUT + N_MOUSE_ZONES_EXTENDED + 2) // 2 sidezones for Strafe
#define N_KEYBYTES_EXTENDED     ((N_KEYS_EXTENDED + 7) / 8)

// Map from key name to LED code and USB scan code
typedef struct {
    const char* name;
    short led;
    short scan;
} key;

// Scancode divisions
#define SCAN_SILENT 0x8000
#define SCAN_KBD    0
#define SCAN_MOUSE  0x1000

// List of keys, ordered according to where they appear in the keyboard input.
// Begins with keyboard keys, followed by extra keys, then mouse buttons, and finally LED zones
extern const key keymap[N_KEYS_EXTENDED];

// Translates input from HID to a ckb input bitfield.
// Use positive endpoint for non-RGB keyboards, negative endpoint for RGB
void hid_kb_translate(unsigned char* kbinput, int endpoint, int length, const unsigned char* urbinput);
void hid_mouse_translate(unsigned char* kbinput, short* xaxis, short* yaxis, int endpoint, int length, const unsigned char* urbinput);

#endif // KEYMAP_H

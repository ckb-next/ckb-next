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

#ifdef OS_MAC_LEGACY
// On Linux there's no meaningful distinction between these keys
// On the OSX legacy build this is used for layout auto-detection (see input_mac_legacy.c)
#define KEY_BACKSLASH_ISO 0xfff
#else
#define KEY_BACKSLASH_ISO KEY_BACKSLASH
#endif

// Number of keys, according to keyboard hardware (NOTE: should be divisible by 8)
#define N_KEYS_HW               152
#define N_KEYBYTES_HW           ((N_KEYS_HW + 7) / 8)
// Light zones (have LED codes but don't generate input)
// Two strafe side lights (although really they are tied into one control) + logo backlight + Platinum top bar
#define N_KEY_ZONES             22
// Additional keys recognized by the driver but may not be present on keyboard
#define N_KEYS_EXTRA            16
// Generic RGB Zones (Polaris/K55/...)
#define N_GENERIC_ZONES         15
#define LED_GENERIC_FIRST       (N_KEYS_HW + N_KEY_ZONES + N_KEYS_EXTRA)
// Mousepad zone count
#define N_MOUSEPAD_ZONES        15
// Mouse buttons
#define N_BUTTONS_HW            20
#define N_BUTTONS_EXTENDED      25
#define MOUSE_BUTTON_FIRST      (N_KEYS_HW + N_KEY_ZONES + N_KEYS_EXTRA + N_GENERIC_ZONES)
#define MOUSE_EXTRA_FIRST       (MOUSE_BUTTON_FIRST + N_BUTTONS_HW)
// Number of keys that generate input
#define N_KEYS_INPUT            (MOUSE_BUTTON_FIRST + N_BUTTONS_EXTENDED)
#define N_KEYBYTES_INPUT        ((N_KEYS_INPUT + 7) / 8)
// Mouse zones
#define LED_MOUSE               N_KEYS_HW
#define N_MOUSE_ZONES           6
#define N_MOUSE_ZONES_EXTENDED  12
// Index of DPI light
#define LED_DPI                 (LED_MOUSE + 2)
// Total number of keys recognized by software
#define N_KEYS_EXTENDED         (N_KEYS_INPUT + N_MOUSE_ZONES_EXTENDED)
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
// Decides which of the following functions it needs to call
void process_input_urb(void* context, unsigned char* buffer, int urblen, ushort ep);

// Translates input from HID to a ckb input bitfield.
// Use positive endpoint for non-RGB keyboards, negative endpoint for RGB
void hid_kb_translate(unsigned char* kbinput, int length, const unsigned char* urbinput, int legacy);
void hid_mouse_translate(unsigned char* kbinput, short* xaxis, short* yaxis, int length, const unsigned char* urbinput);

// Copies input from Corsair reports
void corsair_kbcopy(unsigned char* kbinput, const unsigned char* urbinput);
void corsair_mousecopy(unsigned char* kbinput, const unsigned char* urbinput);

void m95_mouse_translate(unsigned char* kbinput, short* xaxis, short* yaxis, int length, const unsigned char* urbinput);
#endif // KEYMAP_H

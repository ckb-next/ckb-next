#ifndef KEYBOARD_H
#define KEYBOARD_H

// Number of keys
#define N_KEYS 144

// Map from key name to LED code
typedef struct {
    const char* name;
    unsigned char code;
} key;
extern key keymap[];
extern const int N_KEYMAP;

#endif

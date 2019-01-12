#ifndef KEYMAP_WINDOWS_H
#define KEYMAP_WINDOWS_H

#include "os.h"
#ifdef OS_WINDOWS

// Replace Linux key constants with HID scancodes for Windows
//#warning "KEYMAP STUB"

#define KEY_ESC             0x0000
#define KEY_F1              0x0000
#define KEY_F2              0x0000
#define KEY_F3              0x0000
#define KEY_F4              0x0000
#define KEY_F5              0x0000
#define KEY_F6              0x0000
#define KEY_F7              0x0000
#define KEY_F8              0x0000
#define KEY_F9              0x0000
#define KEY_F10             0x0000
#define KEY_F11             0x0000
#define KEY_F12             0x0000
#define KEY_F13             0x0000
#define KEY_F14             0x0000
#define KEY_F15             0x0000
#define KEY_F16             0x0000
#define KEY_F17             0x0000
#define KEY_F18             0x0000
#define KEY_F19             0x0000
#define KEY_F20             0x0000
#define KEY_F21             0x0000
#define KEY_F22             0x0000
#define KEY_F23             0x0000
#define KEY_F24             0x0000

#define KEY_1               0x0000
#define KEY_2               0x0000
#define KEY_3               0x0000
#define KEY_4               0x0000
#define KEY_5               0x0000
#define KEY_6               0x0000
#define KEY_7               0x0000
#define KEY_8               0x0000
#define KEY_9               0x0000
#define KEY_0               0x0000

#define KEY_A               0x0000
#define KEY_B               0x0000
#define KEY_C               0x0000
#define KEY_D               0x0000
#define KEY_E               0x0000
#define KEY_F               0x0000
#define KEY_G               0x0000
#define KEY_H               0x0000
#define KEY_I               0x0000
#define KEY_J               0x0000
#define KEY_K               0x0000
#define KEY_L               0x0000
#define KEY_M               0x0000
#define KEY_N               0x0000
#define KEY_O               0x0000
#define KEY_P               0x0000
#define KEY_Q               0x0000
#define KEY_R               0x0000
#define KEY_S               0x0000
#define KEY_T               0x0000
#define KEY_U               0x0000
#define KEY_V               0x0000
#define KEY_W               0x0000
#define KEY_X               0x0000
#define KEY_Y               0x0000
#define KEY_Z               0x0000

#define KEY_GRAVE           0x0000
#define KEY_102ND           0x0000
#define KEY_MINUS           0x0000
#define KEY_EQUAL           0x0000
#define KEY_BACKSPACE       0x0000
#define KEY_LEFTBRACE       0x0000
#define KEY_RIGHTBRACE      0x0000
#define KEY_BACKSLASH       0x0000
#define KEY_SEMICOLON       0x0000
#define KEY_APOSTROPHE      0x0000
#define KEY_ENTER           0x0000
#define KEY_COMMA           0x0000
#define KEY_DOT             0x0000
#define KEY_SLASH           0x0000
#define KEY_SPACE           0x0000

#define KEY_TAB             0x0000
#define KEY_CAPSLOCK        0x0000
#define KEY_LEFTSHIFT       0x0000
#define KEY_RIGHTSHIFT      0x0000
#define KEY_LEFTCTRL        0x0000
#define KEY_RIGHTCTRL       0x0000
#define KEY_LEFTMETA        0x0000
#define KEY_RIGHTMETA       0x0000
#define KEY_LEFTALT         0x0000
#define KEY_RIGHTALT        0x0000
#define KEY_COMPOSE         0x0000
#define KEY_FN              0x03 //0x0000

#define KEY_SYSRQ           0x0000
#define KEY_SCROLLLOCK      0x0000
#define KEY_PAUSE           0x0000
#define KEY_INSERT          0x0000
#define KEY_HOME            0x0000
#define KEY_PAGEUP          0x0000
#define KEY_DELETE          0x0000
#define KEY_END             0x0000
#define KEY_PAGEDOWN        0x0000

#define KEY_UP              0x0000
#define KEY_LEFT            0x0000
#define KEY_DOWN            0x0000
#define KEY_RIGHT           0x0000

#define KEY_NUMLOCK         0x0000
#define KEY_KPSLASH         0x0000
#define KEY_KPASTERISK      0x0000
#define KEY_KPMINUS         0x0000
#define KEY_KPPLUS          0x0000
#define KEY_KPENTER         0x0000
#define KEY_KPDOT           0x0000
#define KEY_KP1             0x0000
#define KEY_KP2             0x0000
#define KEY_KP3             0x0000
#define KEY_KP4             0x0000
#define KEY_KP5             0x0000
#define KEY_KP6             0x0000
#define KEY_KP7             0x0000
#define KEY_KP8             0x0000
#define KEY_KP9             0x0000
#define KEY_KP0             0x0000

#define KEY_YEN             0x0000
#define KEY_RO              0x0000
#define KEY_KATAKANAHIRAGANA 0x0000
#define KEY_HENKAN          0x0000
#define KEY_MUHENKAN        0x0000

#define KEY_MUTE            0x0000
#define KEY_VOLUMEUP        0x0000
#define KEY_VOLUMEDOWN      0x0000
#define KEY_STOPCD          0x0000
#define KEY_PREVIOUSSONG    0x0000
#define KEY_PLAYPAUSE       0x0000
#define KEY_NEXTSONG        0x0000
#define KEY_BRIGHTNESSUP    0x006f
#define KEY_BRIGHTNESSDOWN  0x0070
#define KEY_EJECTCD         0x0000
#define KEY_POWER           0x0030


#define BTN_LEFT            1
#define BTN_RIGHT           2
#define BTN_MIDDLE          3
#define BTN_SIDE            4
#define BTN_EXTRA           5
#define BTN_FORWARD         6
#define BTN_BACK            7
#define BTN_TASK            8

#endif // OS_WINDOWS

#endif // KEYMAP_WINDOWS_H

#ifndef KEYMAP_WINDOWS_H
#define KEYMAP_WINDOWS_H

#include "os.h"
#ifdef OS_WINDOWS

// Replace Linux key constants with HID scancodes for Windows
//#warning "KEYMAP STUB"
#define IS_MODIFIER(c)       (c > 0x100 && c < 0x200)
#define KEY_LEFTSHIFT       0x0102
#define KEY_RIGHTSHIFT      0x0120
#define KEY_LEFTCTRL        0x0101
#define KEY_RIGHTCTRL       0x0110
#define KEY_LEFTMETA        0x0108
#define KEY_RIGHTMETA       0x0180
#define KEY_LEFTALT         0x0104
#define KEY_RIGHTALT        0x0140

#define KEY_ESC             0x0029
#define KEY_F1              0x003A
#define KEY_F2              0x003B
#define KEY_F3              0x003C
#define KEY_F4              0x003D
#define KEY_F5              0x003E
#define KEY_F6              0x003F
#define KEY_F7              0x0040
#define KEY_F8              0x0041
#define KEY_F9              0x0042 
#define KEY_F10             0x0043
#define KEY_F11             0x0044
#define KEY_F12             0x0045
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

#define KEY_1               0x001E
#define KEY_2               0x001F
#define KEY_3               0x0020
#define KEY_4               0x0021
#define KEY_5               0x0022
#define KEY_6               0x0023
#define KEY_7               0x0024
#define KEY_8               0x0025
#define KEY_9               0x0026
#define KEY_0               0x0027

#define KEY_A               0x0004
#define KEY_B               0x0005
#define KEY_C               0x0006
#define KEY_D               0x0007
#define KEY_E               0x0008
#define KEY_F               0x0009
#define KEY_G               0x000A
#define KEY_H               0x000B
#define KEY_I               0x000C
#define KEY_J               0x000D
#define KEY_K               0x000E
#define KEY_L               0x000F
#define KEY_M               0x0010
#define KEY_N               0x0011
#define KEY_O               0x0012
#define KEY_P               0x0013
#define KEY_Q               0x0014
#define KEY_R               0x0015
#define KEY_S               0x0016
#define KEY_T               0x0017
#define KEY_U               0x0018
#define KEY_V               0x0019
#define KEY_W               0x001A
#define KEY_X               0x001B
#define KEY_Y               0x001C
#define KEY_Z               0x001D

#define KEY_GRAVE           0x0000
#define KEY_102ND           0x0000
#define KEY_MINUS           0x002D
#define KEY_EQUAL           0x002E
#define KEY_BACKSPACE       0x002A
#define KEY_LEFTBRACE       0x002F
#define KEY_RIGHTBRACE      0x0030
#define KEY_BACKSLASH       0x0031
#define KEY_SEMICOLON       0x0033
#define KEY_APOSTROPHE      0x0034
#define KEY_ENTER           0x0028
#define KEY_COMMA           0x0036
#define KEY_DOT             0x0037
#define KEY_SLASH           0x0038
#define KEY_SPACE           0x002C

#define KEY_TAB             0x002B
#define KEY_CAPSLOCK        0x0039
#define KEY_COMPOSE         0x0076
#define KEY_FN              0x0000

#define KEY_SYSRQ           0x0046
#define KEY_SCROLLLOCK      0x0047
#define KEY_PAUSE           0x0048
#define KEY_INSERT          0x0049
#define KEY_HOME            0x004A
#define KEY_PAGEUP          0x004B
#define KEY_DELETE          0x004C
#define KEY_END             0x004D
#define KEY_PAGEDOWN        0x004E

#define KEY_UP              0x0052
#define KEY_LEFT            0x0050
#define KEY_DOWN            0x0051
#define KEY_RIGHT           0x004F

#define KEY_NUMLOCK         0x0053
#define KEY_KPSLASH         0x0054
#define KEY_KPASTERISK      0x0055
#define KEY_KPMINUS         0x0056
#define KEY_KPPLUS          0x0057
#define KEY_KPENTER         0x0058
#define KEY_KPDOT           0x0063
#define KEY_KP1             0x0059
#define KEY_KP2             0x005A
#define KEY_KP3             0x005B
#define KEY_KP4             0x005C
#define KEY_KP5             0x005D
#define KEY_KP6             0x005E
#define KEY_KP7             0x005F
#define KEY_KP8             0x0060
#define KEY_KP9             0x0061
#define KEY_KP0             0x0062

#define KEY_YEN             0x0089
#define KEY_RO              0x0087
#define KEY_KATAKANAHIRAGANA 0x0000
#define KEY_HENKAN          0x0000
#define KEY_MUHENKAN        0x0000

//#define IS_
#define KEY_MUTE            0x007F
#define KEY_VOLUMEUP        0x0080
#define KEY_VOLUMEDOWN      0x0081
#define KEY_STOPCD          0x0078
#define KEY_PREVIOUSSONG    0x0000
#define KEY_PLAYPAUSE       0x0000
#define KEY_NEXTSONG        0x0000
#define KEY_BRIGHTNESSUP    0x0000
#define KEY_BRIGHTNESSDOWN  0x0000
#define KEY_EJECTCD         0x0000
#define KEY_POWER           0x0000


#define BTN_LEFT            1
#define BTN_RIGHT           2
#define BTN_MIDDLE          4
#define BTN_SIDE            0
#define BTN_EXTRA           0
#define BTN_FORWARD         0
#define BTN_BACK            0
#define BTN_TASK            0

// FIXME: stubs below
#define KEY_HANJA           0x0000
#define KEY_HANGEUL         0x0000

#endif // OS_WINDOWS

#endif // KEYMAP_WINDOWS_H

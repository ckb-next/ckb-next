#ifndef KEYMAP_MAC_H
#define KEYMAP_MAC_H

#include "os.h"
#ifdef OS_MAC

#ifdef OS_MAC_LEGACY

#include "keymap_mac_legacy.h"

#else

// Replace Linux key constants with HID scancodes for macOS w/VirtualHIDDevice

#define KEY_ESC             kHIDUsage_KeyboardEscape
#define KEY_F1              kHIDUsage_KeyboardF1
#define KEY_F2              kHIDUsage_KeyboardF2
#define KEY_F3              kHIDUsage_KeyboardF3
#define KEY_F4              kHIDUsage_KeyboardF4
#define KEY_F5              kHIDUsage_KeyboardF5
#define KEY_F6              kHIDUsage_KeyboardF6
#define KEY_F7              kHIDUsage_KeyboardF7
#define KEY_F8              kHIDUsage_KeyboardF8
#define KEY_F9              kHIDUsage_KeyboardF9
#define KEY_F10             kHIDUsage_KeyboardF10
#define KEY_F11             kHIDUsage_KeyboardF11
#define KEY_F12             kHIDUsage_KeyboardF12
#define KEY_F13             kHIDUsage_KeyboardF13
#define KEY_F14             kHIDUsage_KeyboardF14
#define KEY_F15             kHIDUsage_KeyboardF15
#define KEY_F16             kHIDUsage_KeyboardF16
#define KEY_F17             kHIDUsage_KeyboardF17
#define KEY_F18             kHIDUsage_KeyboardF18
#define KEY_F19             kHIDUsage_KeyboardF19
#define KEY_F20             kHIDUsage_KeyboardF20
#define KEY_F21             kHIDUsage_KeyboardF21
#define KEY_F22             kHIDUsage_KeyboardF22
#define KEY_F23             kHIDUsage_KeyboardF23
#define KEY_F24             kHIDUsage_KeyboardF24

#define KEY_1               kHIDUsage_Keyboard1
#define KEY_2               kHIDUsage_Keyboard2
#define KEY_3               kHIDUsage_Keyboard3
#define KEY_4               kHIDUsage_Keyboard4
#define KEY_5               kHIDUsage_Keyboard5
#define KEY_6               kHIDUsage_Keyboard6
#define KEY_7               kHIDUsage_Keyboard7
#define KEY_8               kHIDUsage_Keyboard8
#define KEY_9               kHIDUsage_Keyboard9
#define KEY_0               kHIDUsage_Keyboard0

#define KEY_A               kHIDUsage_KeyboardA
#define KEY_B               kHIDUsage_KeyboardB
#define KEY_C               kHIDUsage_KeyboardC
#define KEY_D               kHIDUsage_KeyboardD
#define KEY_E               kHIDUsage_KeyboardE
#define KEY_F               kHIDUsage_KeyboardF
#define KEY_G               kHIDUsage_KeyboardG
#define KEY_H               kHIDUsage_KeyboardH
#define KEY_I               kHIDUsage_KeyboardI
#define KEY_J               kHIDUsage_KeyboardJ
#define KEY_K               kHIDUsage_KeyboardK
#define KEY_L               kHIDUsage_KeyboardL
#define KEY_M               kHIDUsage_KeyboardM
#define KEY_N               kHIDUsage_KeyboardN
#define KEY_O               kHIDUsage_KeyboardO
#define KEY_P               kHIDUsage_KeyboardP
#define KEY_Q               kHIDUsage_KeyboardQ
#define KEY_R               kHIDUsage_KeyboardR
#define KEY_S               kHIDUsage_KeyboardS
#define KEY_T               kHIDUsage_KeyboardT
#define KEY_U               kHIDUsage_KeyboardU
#define KEY_V               kHIDUsage_KeyboardV
#define KEY_W               kHIDUsage_KeyboardW
#define KEY_X               kHIDUsage_KeyboardX
#define KEY_Y               kHIDUsage_KeyboardY
#define KEY_Z               kHIDUsage_KeyboardZ

#define KEY_GRAVE           kHIDUsage_KeyboardGraveAccentAndTilde
#define KEY_102ND           kHIDUsage_KeyboardNonUSBackslash
#define KEY_MINUS           kHIDUsage_KeyboardHyphen
#define KEY_EQUAL           kHIDUsage_KeyboardEqualSign
#define KEY_BACKSPACE       kHIDUsage_KeyboardDeleteOrBackspace
#define KEY_LEFTBRACE       kHIDUsage_KeyboardOpenBracket
#define KEY_RIGHTBRACE      kHIDUsage_KeyboardCloseBracket
#define KEY_BACKSLASH       kHIDUsage_KeyboardBackslash
#define KEY_SEMICOLON       kHIDUsage_KeyboardSemicolon
#define KEY_APOSTROPHE      kHIDUsage_KeyboardQuote
#define KEY_ENTER           kHIDUsage_KeyboardReturnOrEnter
#define KEY_COMMA           kHIDUsage_KeyboardComma
#define KEY_DOT             kHIDUsage_KeyboardPeriod
#define KEY_SLASH           kHIDUsage_KeyboardSlash
#define KEY_SPACE           kHIDUsage_KeyboardSpacebar

#define KEY_TAB             kHIDUsage_KeyboardTab
#define KEY_CAPSLOCK        kHIDUsage_KeyboardCapsLock
#define KEY_LEFTSHIFT       kHIDUsage_KeyboardLeftShift
#define KEY_RIGHTSHIFT      kHIDUsage_KeyboardRightShift
#define KEY_LEFTCTRL        kHIDUsage_KeyboardLeftControl
#define KEY_RIGHTCTRL       kHIDUsage_KeyboardRightControl
#define KEY_LEFTMETA        kHIDUsage_KeyboardLeftGUI
#define KEY_RIGHTMETA       kHIDUsage_KeyboardRightGUI
#define KEY_LEFTALT         kHIDUsage_KeyboardLeftAlt
#define KEY_RIGHTALT        kHIDUsage_KeyboardRightAlt
#define KEY_COMPOSE         kHIDUsage_KeyboardMenu
#define KEY_FN              0x03 //kHIDUsage_AV_TopCase_KeyboardFn

#define KEY_SYSRQ           kHIDUsage_KeyboardF13
#define KEY_SCROLLLOCK      kHIDUsage_KeyboardF14
#define KEY_PAUSE           kHIDUsage_KeyboardF15
#define KEY_INSERT          kHIDUsage_KeyboardInsert
#define KEY_HOME            kHIDUsage_KeyboardHome
#define KEY_PAGEUP          kHIDUsage_KeyboardPageUp
#define KEY_DELETE          kHIDUsage_KeyboardDeleteForward
#define KEY_END             kHIDUsage_KeyboardEnd
#define KEY_PAGEDOWN        kHIDUsage_KeyboardPageDown

#define KEY_UP              kHIDUsage_KeyboardUpArrow
#define KEY_LEFT            kHIDUsage_KeyboardLeftArrow
#define KEY_DOWN            kHIDUsage_KeyboardDownArrow
#define KEY_RIGHT           kHIDUsage_KeyboardRightArrow

#define KEY_NUMLOCK         kHIDUsage_KeypadNumLock
#define KEY_KPSLASH         kHIDUsage_KeypadSlash
#define KEY_KPASTERISK      kHIDUsage_KeypadAsterisk
#define KEY_KPMINUS         kHIDUsage_KeypadHyphen
#define KEY_KPPLUS          kHIDUsage_KeypadPlus
#define KEY_KPENTER         kHIDUsage_KeypadEnter
#define KEY_KPDOT           kHIDUsage_KeypadPeriod
#define KEY_KP1             kHIDUsage_Keypad1
#define KEY_KP2             kHIDUsage_Keypad2
#define KEY_KP3             kHIDUsage_Keypad3
#define KEY_KP4             kHIDUsage_Keypad4
#define KEY_KP5             kHIDUsage_Keypad5
#define KEY_KP6             kHIDUsage_Keypad6
#define KEY_KP7             kHIDUsage_Keypad7
#define KEY_KP8             kHIDUsage_Keypad8
#define KEY_KP9             kHIDUsage_Keypad9
#define KEY_KP0             kHIDUsage_Keypad0

#define KEY_YEN             kHIDUsage_KeyboardInternational3
#define KEY_RO              kHIDUsage_KeyboardInternational1
#define KEY_KATAKANAHIRAGANA kHIDUsage_KeyboardInternational2
#define KEY_HENKAN          kHIDUsage_KeyboardInternational4
#define KEY_MUHENKAN        kHIDUsage_KeyboardInternational5

#define KEY_CONSUMER        0x800
#define IS_CONSUMER(scan)   ((scan) >= KEY_CONSUMER)
#define KEY_MUTE            (KEY_CONSUMER + kHIDUsage_Csmr_Mute)
#define KEY_VOLUMEUP        (KEY_CONSUMER + kHIDUsage_Csmr_VolumeIncrement)
#define KEY_VOLUMEDOWN      (KEY_CONSUMER + kHIDUsage_Csmr_VolumeDecrement)
#define KEY_STOPCD          (KEY_CONSUMER + kHIDUsage_Csmr_Stop)
#define KEY_PREVIOUSSONG    (KEY_CONSUMER + kHIDUsage_Csmr_ScanPreviousTrack)
#define KEY_PLAYPAUSE       (KEY_CONSUMER + kHIDUsage_Csmr_PlayOrPause)
#define KEY_NEXTSONG        (KEY_CONSUMER + kHIDUsage_Csmr_ScanNextTrack)
#define KEY_BRIGHTNESSUP    (KEY_CONSUMER + 0x006f)
#define KEY_BRIGHTNESSDOWN  (KEY_CONSUMER + 0x0070)
#define KEY_EJECTCD         (KEY_CONSUMER + kHIDUsage_Csmr_Eject)
#define KEY_POWER           (KEY_CONSUMER + 0x0030)
#define IS_VENDOR(scan)     ((scan) >= KEY_CONSUMER && (scan) < KEY_CONSUMER + kHIDUsage_Csmr_Play)


#define BTN_LEFT            1
#define BTN_RIGHT           2
#define BTN_MIDDLE          3
#define BTN_SIDE            4
#define BTN_EXTRA           5
#define BTN_FORWARD         6
#define BTN_BACK            7
#define BTN_TASK            8

#endif //OS_MAC_LEGACY

#endif // OS_MAC

#endif // KEYMAP_MAC_H

#ifndef LEGACY_PROTO_H
#define LEGACY_PROTO_H

// The legacy devices are mostly hardware controlled.
// On the Vengeance K95 (legacy) each mode has its own lighting settings that are set by the user
// by holding down the lighting programming button until the ring around starts flashing.
//
// Keys with hardcoded functionality (such as brightness and mode buttons) still send scancodes
// so that software can keep track of what is going on in hardware.
// It is impossible to unbind the hardware functionality.


/// \brief nk95cmd() macro is used to send legacy keyboard commands
/// the command structure is different:
/// \n Just the bits 23..16 are used as bits 7..0 for bRequest
/// \n Bits 15..0 are used as wValue
#define nk95cmd(kb, command) kb->vtable.write(kb, &(ctrltransfer) { .bRequestType = 0x40, .bRequest = (command) >> 16 & 0xFF, .wValue = (command) & 0xFFFF, .wIndex = 0, .wLength = 0, .timeout = 5000, .data = NULL}, 0, 0, __FILE_NOPATH__, __LINE__)

// Set Hardware playback off
#define NK95_HWOFF  0x020030

// Hardware playback on
#define NK95_HWON   0x020001

// Switch to mode 1
#define NK95_M1     0x140001

// Switch to mode 2
#define NK95_M2     0x140002

// Switch to mode 3
#define NK95_M3     0x140003

// These could be implemented in the future

// To set the brightness in software
#define NK95_BRIGHTNESS_0       0x310000

#define NK95_BRIGHTNESS_33      0x310001

#define NK95_BRIGHTNESS_66      0x310002

#define NK95_BRIGHTNESS_100     0x310003

#endif

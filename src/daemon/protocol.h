// This file is purely macros, so it can be included multiple times without harm.
// For more detail, please see https://github.com/ckb-next/corsair-protocol.

// Corsair protocol commands.
#define CMD_SET        0x07 // Write a single field.
#define CMD_GET        0x0e // Read a single field.

#define CMD_WRITE_BULK 0x7f // Write a stream of up to 5 packets at a time.
#define CMD_READ_BULK  0xff // Read a stream of up to 5 packets at a time.

#define FIELD_IDENT    0x01 // Firmware identification. Read only.
#define FIELD_RESET    0x02 // Reset the device. Write only.
#define FIELD_SPECIAL  0x04 // Special function mode, like G-keys and sniper buttons. Write only.
#define FIELD_LIGHTING 0x05 // Lighting mode hardware/software. Write only.
#define FIELD_POLLRATE 0x0a // Change device poll rate. Write only.
#define FIELD_FW_START 0x0c // Begin firmware update. Write only.
#define FIELD_FW_DATA  0x0d // Firmware update data. Write only.
#define FIELD_MOUSE    0x13 // Mouse subcommands. Read and write, subcommand-depending.
#define FIELD_KB_HWCLR 0x14 // Keyboard hardware colour. Read and write.
#define FIELD_M_PROFID 0x15 // Mouse profile ID. Read and write.
#define FIELD_M_PROFNM 0x16 // Mouse profile name. Read and write, UTF-16LE.
#define FIELD_KB_HWANM 0x17 // Hardware animation commands used by the K95 Platinum. Read and write, subcommand-depending.
#define FIELD_M_COLOR  0x22 // Mouse software colour change. Write only.
#define FIELD_MP_COLOR 0x22 // Mousepad software colour change. The command is the same as for mice, but the format is different. Write only.
#define FIELD_KB_ZNCLR 0x25 // Zoned colour change used by the K55. Write only.
#define FIELD_KB_9BCLR 0x27 // 9-bit colour change used by !IS_FULLRANGE keyboards. Write only.
#define FIELD_KB_COLOR 0x28 // 24-bit colour change used by IS_FULLRANGE keyboards. Write only.
#define FIELD_KEYINPUT 0x40 // Key input mode; whether an event should trigger a HID or Corsair event (or both). Write only.
#define FIELD_BATTERY  0x50 // Battery status. Read only.

// Used for FIELD_RESET.
#define RESET_NORMAL   0x00 // A medium-speed reset.
#define RESET_FAST     0x01 // A faster reset.
#define RESET_BLD      0xaa // Reboot to bootloader; mounting as a virtual FAT12 device.
#define RESET_SLOW     0xf0 // A slow reset sent after firmware update.

// Used for FIELD_SPECIAL and FIELD_LIGHTING.
#define MODE_HARDWARE  0x01 // This should be hardware controlled and not generate an event.
#define MODE_SOFTWARE  0x02 // This should be software controlled and generate an event.
#define MODE_SIDELIGHT 0x08 // Strafe sidelighting control.
#define MODE_WINLOCK   0x09 // K68 winlock control.

// Used for FIELD_MOUSE. All subcommands are read/write.
#define MOUSE_DPI      0x02 // Mouse DPI mode.
#define MOUSE_LIFT     0x03 // Mouse lift height.
#define MOUSE_SNAP     0x04 // Mouse angle snap.
#define MOUSE_DPIMASK  0x05 // Mouse DPI enabled mask.
#define MOUSE_HWCOLOR  0x10 // Mouse hardware colour base (0x10-0x15).
#define MOUSE_DPIPROF  0xd0 // Mouse DPI profile info.

// Used for FIELD_KB_COLOR.
#define COLOR_RED      0x01 // The red channel of RGB.
#define COLOR_GREEN    0x02 // The green channel of RGB.
#define COLOR_BLUE     0x03 // The blue channel of RGB.

// HID input commands
// Keyboards
#define NKRO_KEY_IN    0x01 // NKRO key input
#define NKRO_MEDIA_IN  0x02 // NKRO media key input
// Mice
#define MOUSE_IN       0x01 // Standard HID mouse input (Position/Buttons)
// Common
#define CORSAIR_IN     0x03 // Corsair input. Doesn't apply to FW v1.15 on first gen devices.

#ifndef BRAGI_PROTO_H
#define BRAGI_PROTO_H

#define BRAGI_MAGIC         0x08

// Commands
#define BRAGI_SET           0x01
#define BRAGI_GET           0x02
#define BRAGI_CLOSE_HANDLE  0x05
#define BRAGI_WRITE_DATA    0x06
#define BRAGI_CONTINUE_WRITE 0x07
#define BRAGI_READ_DATA     0x08
#define BRAGI_PROBE_HANDLE  0x09
#define BRAGI_OPEN_HANDLE   0x0d

// What to get/set
#define BRAGI_POLLRATE      0x01
#define BRAGI_BRIGHTNESS    0x02
#define BRAGI_MODE          0x03
#define BRAGI_ANGLE_SNAP    0x07
#define BRAGI_APP_VER       0x13
#define BRAGI_BLD_VER       0x14
#define BRAGI_RADIO_APP_VER 0x15
#define BRAGI_RADIO_BLD_VER 0x16
#define BRAGI_DPI_INDEX     0x1E
#define BRAGI_DPI_MASK      0x1F
#define BRAGI_DPI_X         0x21
#define BRAGI_DPI_Y         0x22
#define BRAGI_DPI0_COLOR    0x2F
#define BRAGI_DPI1_COLOR    0x30
#define BRAGI_DPI2_COLOR    0x31
#define BRAGI_HWLAYOUT      0x41
#define BRAGI_BRIGHTNESS_COARSE 0x44

// Untested. Only known because CUE complained about it in its logs
#define BRAGI_MAX_POLLRATE  0x96

// 0x18-0x1A contain the DPI values for DPI0,1,2 0x23-0x28 contain the seperated DPIX and Y values for DPI0,1,2

// Read only dev info
#define BRAGI_VID           0x11
#define BRAGI_PID           0x12
#define BRAGI_BATTERY_LEVEL 0x0f
#define BRAGI_BATTERY_STATUS 0x10

// Bragi mode
#define BRAGI_MODE_HARDWARE 0x01
#define BRAGI_MODE_SOFTWARE 0x02

// Pollrate
#define BRAGI_POLLRATE_1MS  0x04
#define BRAGI_POLLRATE_2MS  0x03
#define BRAGI_POLLRATE_4MS  0x02
#define BRAGI_POLLRATE_8MS  0x01

// Dongle subdevice-related information
#define BRAGI_SUBDEVICE_BITFIELD 0x36

// Resources (used to get handles)
#define BRAGI_RES_LIGHTING  0x01
#define BRAGI_RES_LIGHTING_MONOCHROME 0x10
#define BRAGI_RES_PAIRINGID 0x05

// ckb-specific macros
#define BRAGI_LIGHTING_HANDLE 0x00
#define BRAGI_GENERIC_HANDLE  0x01 // This is used for quick read/writes. Do NOT leave it open.

// HID input (2 bytes)
#define BRAGI_INPUT_0       0x00
#define BRAGI_INPUT_NOTIFY  0x01
#define BRAGI_INPUT_HID     0x02

// Notification IDs
#define BRAGI_NOTIFICATION_CONNECTIONSTATUS 0x36

// Errors
#define BRAGI_ERROR_NOTSUPPORTED 0x05

#endif // BRAGI_PROTO_H

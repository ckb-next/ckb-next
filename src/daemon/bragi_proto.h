#ifndef BRAGI_PROTO_H
#define BRAGI_PROTO_H

#define BRAGI_MAGIC         0x08

// Commands
#define BRAGI_SET           0x01
#define BRAGI_GET           0x02
#define BRAGI_WRITE_DATA    0x06
#define BRAGI_OPEN_HANDLE   0x0d

// What to get/set
#define BRAGI_POLLRATE      0x01
#define BRAGI_MODE          0x03

// Bragi mode
#define BRAGI_MODE_HARDWARE 0x01
#define BRAGI_MODE_SOFTWARE 0x02

// Pollrate
#define BRAGI_POLLRATE_1MS  0x04
#define BRAGI_POLLRATE_2MS  0x03
#define BRAGI_POLLRATE_4MS  0x02
#define BRAGI_POLLRATE_8MS  0x01

// Resources (used to get handles)
#define BRAGI_RES_LIGHTING  0x01
#define BRAGI_RES_PAIRINGID 0x05

// ckb-specific macros
#define BRAGI_LIGHTING_HANDLE 0x00

// HID input (2 bytes)
#define BRAGI_INPUT_0 0x00
#define BRAGI_INPUT_1 0x02

#endif // BRAGI_PROTO_H

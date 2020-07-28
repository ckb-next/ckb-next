#ifndef BRAGI_PROTO_H
#define BRAGI_PROTO_H

#define BRAGI_MAGIC         0x08

// Commands
#define BRAGI_SET           0x01
#define BRAGI_GET           0x02
#define BRAGI_LIGHT_DATA    0x06

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

// Unknown
#define BRAGI_LIGHTING      0x12

#endif // BRAGI_PROTO_H

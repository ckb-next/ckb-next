#ifndef M65_ULTRA_PROTO_H
#define M65_ULTRA_PROTO_H

/*
 * M65 RGB Ultra Protocol
 * ======================
 *
 * The M65 RGB Ultra uses a custom HID protocol distinct from both NXP and Bragi.
 * All command packets are 64 bytes with a 0x08 magic prefix byte.
 *
 * Packet Format:
 *   Byte 0: Magic (0x08)
 *   Byte 1: Command type
 *   Byte 2: Subcommand / Parameter
 *   Byte 3: Reserved (usually 0x00)
 *   Bytes 4+: Command-specific data
 *
 * Endpoints:
 *   EP 0x04 (OUT): Commands to device
 *   EP 0x84 (IN):  Responses from device
 *   EP 0x82 (IN):  HID mouse input (boot protocol)
 */

/* Protocol magic byte - identifies M65 Ultra protocol packets */
#define M65U_MAGIC              0x08

/* Command types (byte 1) */
#define M65U_CMD_SET            0x01    /* Write a property */
#define M65U_CMD_GET            0x02    /* Read a property */
#define M65U_CMD_COMMIT         0x05    /* Commit pending changes */
#define M65U_CMD_RGB            0x06    /* Send RGB color data */
#define M65U_CMD_RGB_MODE       0x0D    /* Enable/disable software RGB control */

/* Subcommands for SET/GET (byte 2) */
#define M65U_SUB_BRIGHTNESS     0x02    /* Brightness: 0-1000, 16-bit LE */
#define M65U_SUB_SW_MODE        0x03    /* Software mode: 0x02=SW, 0x00=HW */
#define M65U_SUB_FIRMWARE       0x13    /* Firmware version query */
#define M65U_SUB_DPI_X          0x21    /* DPI X axis: 16-bit LE */
#define M65U_SUB_DPI_Y          0x22    /* DPI Y axis: 16-bit LE */

/* Software/Hardware mode values for M65U_SUB_SW_MODE */
#define M65U_MODE_HARDWARE      0x00
#define M65U_MODE_SOFTWARE      0x02

/*
 * RGB Packet Format
 * =================
 * Header: 08 06 00 09 00 00 00
 * Data (bytes 7-15): Planar RGB format
 *   Bytes 7-9:   Red values   [Logo, Wheel, DPI]
 *   Bytes 10-12: Green values [Logo, Wheel, DPI]
 *   Bytes 13-15: Blue values  [Logo, Wheel, DPI]
 */
#define M65U_RGB_ZONE_ID        0x09    /* Zone identifier for RGB data packet */
#define M65U_RGB_INIT_ZONE_ID   0x08    /* Zone identifier for RGB init packet */
#define M65U_RGB_HEADER_LEN     7       /* Bytes before RGB data */
#define M65U_RGB_ZONE_COUNT     3       /* Logo, Wheel, DPI */
#define M65U_RGB_DATA_LEN       9       /* 3 zones x 3 colors (planar format) */

/* RGB mode enable/disable values for M65U_CMD_RGB_MODE */
#define M65U_RGB_MODE_OFF       0x00
#define M65U_RGB_MODE_ON        0x01
#define M65U_RGB_MODE_INIT      0x02    /* Used during initialization */

/* RGB Zone indices (offset within each color plane) */
#define M65U_ZONE_LOGO          0
#define M65U_ZONE_WHEEL         1
#define M65U_ZONE_DPI           2

/* Brightness range (0-1000 maps to 0-100%) */
#define M65U_BRIGHTNESS_MIN     0
#define M65U_BRIGHTNESS_MAX     1000
#define M65U_BRIGHTNESS_DEFAULT 1000    /* 100% */

/* DPI range and stages */
#define M65U_DPI_MIN            100
#define M65U_DPI_MAX            26000
#define M65U_DPI_STAGE_COUNT    5

/* Commit packet parameter (byte 2) */
#define M65U_COMMIT_PARAM       0x01

/*
 * Input Packet Format (64-byte packets from EP 0x84)
 * ==================================================
 * When byte[1] == 0x02: Button state report
 *   Byte 2: Button bits
 *
 * When byte[0] == 0x00 && byte[1] == 0x02: Firmware response
 *   Bytes 3-5: Firmware version (major, minor, patch)
 */
#define M65U_INPUT_REPORT_ID    0x02
#define M65U_BTN_BYTE           2       /* Button state byte index */

/* Button bit masks (in byte 2 of input report) */
#define M65U_BTN_LEFT           (1 << 0)
#define M65U_BTN_RIGHT          (1 << 1)
#define M65U_BTN_MIDDLE         (1 << 2)
#define M65U_BTN_BACK           (1 << 3)    /* Mouse4/Back */
#define M65U_BTN_FORWARD        (1 << 4)    /* Mouse5/Forward */
#define M65U_BTN_DPI_UP         (1 << 5)
#define M65U_BTN_DPI_DOWN       (1 << 6)
#define M65U_BTN_SNIPER         (1 << 7)

/* Firmware response packet identification */
#define M65U_FW_RESP_BYTE0      0x00
#define M65U_FW_RESP_BYTE1      0x02

/* Helper macro to check if a packet is a firmware response */
#define M65U_IS_FW_RESPONSE(buf) ((buf)[0] == M65U_FW_RESP_BYTE0 && (buf)[1] == M65U_FW_RESP_BYTE1)

#endif /* M65_ULTRA_PROTO_H */

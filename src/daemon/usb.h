#ifndef USB_H
#define USB_H

#include <ckbnextconfig.h>

#include "includes.h"
#include "keymap.h"

/// \file usb.h
/// Definitions for using USB interface
///
/// \details Vendor/product codes
///
/// The list of defines in the first part of the file describes the various types of equipment from Corsair
/// and summarizes them according to specific characteristics.
/// \n Each device type is described with a define:
///  - The device ID with which the device can be recognized on the USB as a ushort
///
/// First entry-pair is the Provider ID (vendorID) from Corsair.
///
/// Block No. | contains | Devices are bundled via
/// --------- | -------- | -----------------------
/// 1 | The first block contains the K63 Non RGB Keyboard.
/// 2 | the K65-like keyboards, regardless of their properties (RGB, ...). | In summary, they can be queried using the macro IS_K65().
/// 3 | K68 keyboards | IS_K68().
/// 4 | the K70-like Keyboards with all their configuration types | summarized by IS_K70().
/// 5 | the K95 series keyboards | collected with the macro IS_K95().
/// 6 | strafe keyboards | IS_STRAFE()
/// 7 | M65 mice with and without RGB | IS_M65()
/// 8 | Sabre mice | IS_SABRE()
/// 9 | Scimitar mice | IS_SCIMITAR()
/// 10| Harpoon mice | IS_HARPOON()
/// 11| Glaive mice | IS_GLAIVE()
///

/// \brief For the following Defines please see "Detailed Description"
///
/// \warning When adding new devices please update src/gui/fwupgradedialog.cpp as well.
/// \n It should contain the same vendor/product IDs for any devices supporting firmware updates.
/// \n In the same way, all other corresponding files have to be supplemented or modified:
/// Currently known for this are \b usb_linux.c and \b usb_mac.c
///
#define V_CORSAIR            0x1b1c

#define P_K55                0x1b3d
#define P_K55_PRO            0x1ba4
#define P_K55_PRO_XT         0x1ba1
#define IS_K55(kb)           ((kb)->vendor == V_CORSAIR && ((kb)->product == P_K55 || (kb)->product == P_K55_PRO || (kb)->product == P_K55_PRO_XT))


#define P_K57_D              0x1b62
#define P_K57_U              0x1b6e

#define P_K60_PRO_RGB        0x1ba0
#define P_K60_PRO_RGB_LP     0x1bad
#define P_K60_PRO_RGB_SE     0x1b8d
#define P_K60_PRO_MONO       0x1b83
#define P_K60_PRO_TKL        0x1bc7
#define IS_K60PRORGB(kb)     ((kb)->vendor == V_CORSAIR && ((kb)->product == P_K60_PRO_RGB || (kb)->product == P_K60_PRO_RGB_LP || (kb)->product == P_K60_PRO_RGB_SE || (kb)->product == P_K60_PRO_MONO || (kb)->product == P_K60_PRO_TKL))

#define P_K63_NRGB           0x1b40
#define P_K63_NRGB_WL        0x1b45 /* wireless */
#define P_K63_NRGB_WL2       0x1b50 /* wireless */
#define P_K63_NRGB_WL3       0x1b8c /* wireless */
#define P_K63_NRGB_WL4       0x1b8f /* wireless */
#define IS_K63_WL(kb)        ((kb)->vendor == V_CORSAIR && ((kb)->product == P_K63_NRGB_WL || (kb)->product == P_K63_NRGB_WL2 || (kb)->product == P_K63_NRGB_WL3 || (kb)->product == P_K63_NRGB_WL4))
#define IS_K63(kb)           (IS_K63_WL(kb) || ((kb)->vendor == V_CORSAIR && ((kb)->product == P_K63_NRGB)))

#define P_K65                0x1b17
#define P_K65_LEGACY         0x1b07
#define P_K65_LUX            0x1b37
#define P_K65_RFIRE          0x1b39
#define P_K65_MINI           0x1baf
#define IS_K65(kb)           ((kb)->vendor == V_CORSAIR && ((kb)->product == P_K65 || (kb)->product == P_K65_LEGACY || (kb)->product == P_K65_LUX || (kb)->product == P_K65_RFIRE || (kb)->product == P_K65_MINI))

#define P_K66                0x1b41
#define IS_K66(kb)           ((kb)->vendor == V_CORSAIR && ((kb)->product == P_K66))

#define P_K68                0x1b4f
#define P_K68_NRGB           0x1b3f
#define IS_K68(kb)           ((kb)->vendor == V_CORSAIR && ((kb)->product == P_K68 || (kb)->product == P_K68_NRGB))

#define P_K70                0x1b13
#define P_K70_LEGACY         0x1b09
#define P_K70_LUX            0x1b33
#define P_K70_LUX_NRGB       0x1b36
#define P_K70_RFIRE          0x1b38
#define P_K70_RFIRE_NRGB     0x1b3a
#define P_K70_MK2            0x1b49
#define P_K70_MK2SE          0x1b6b
#define P_K70_MK2LP          0x1b55
#define P_K70_TKL            0x1b73
#define P_K70_PRO            0x1bb3
#define IS_K70(kb) ((kb)->vendor == V_CORSAIR && ((kb)->product == P_K70 || (kb)->product == P_K70_LEGACY || (kb)->product == P_K70_RFIRE || (kb)->product == P_K70_RFIRE_NRGB || (kb)->product == P_K70_LUX || (kb)->product == P_K70_LUX_NRGB || (kb)->product == P_K70_MK2 || (kb)->product == P_K70_MK2SE || (kb)->product == P_K70_MK2LP || (kb)->product == P_K70_TKL || (kb)->product == P_K70_PRO))

// The Legacy K90 behaves like a Legacy K95.
#define P_K90_LEGACY         0x1b02
#define P_K95                0x1b11
#define P_K95_LEGACY         0x1b08
#define P_K95_PLATINUM       0x1b2d
#define P_K95_PLATINUM_XT    0x1b89 // Don't think this needs to be in the macro
#define IS_K95(kb)           ((kb)->vendor == V_CORSAIR && ((kb)->product == P_K90_LEGACY || (kb)->product == P_K95 || (kb)->product == P_K95_LEGACY || (kb)->product == P_K95_PLATINUM))

#define P_STRAFE             0x1b20
#define P_STRAFE_NRGB        0x1b15 /* 3-bit lighting, 9-bit protocol */
#define P_STRAFE_NRGB_2      0x1b44 /* 8-bit lighting, 24-bit protocol */
#define P_STRAFE_MK2         0x1b48
#define IS_STRAFE(kb)        ((kb)->vendor == V_CORSAIR && ((kb)->product == P_STRAFE || (kb)->product == P_STRAFE_NRGB || (kb)->product == P_STRAFE_NRGB_2 || (kb)->product == P_STRAFE_MK2))

#define P_K100_OPTICAL       0x1b7c
#define P_K100_MECHANICAL    0x1b7d
#define P_K100_OPTICAL_VARIANT        0x1bc5

#define P_M55_RGB_PRO        0x1b70

#define P_M65                0x1b12
#define P_M65_PRO            0x1b2e
#define P_M65_RGB_ELITE      0x1b5a
#define IS_M65(kb)           ((kb)->vendor == V_CORSAIR && ((kb)->product == P_M65 || (kb)->product == P_M65_PRO || (kb)->product == P_M65_RGB_ELITE))

#define P_M95                0x1b06

#define P_SABRE_O            0x1b14 /* optical */
#define P_SABRE_L            0x1b19 /* laser */
#define P_SABRE_N            0x1b2f /* new? */
#define P_SABRE_O2           0x1b32 /* Observed on a CH-9000111-EU model SABRE */
#define IS_SABRE(kb)         ((kb)->vendor == V_CORSAIR && ((kb)->product == P_SABRE_O || (kb)->product == P_SABRE_L || (kb)->product == P_SABRE_N || (kb)->product == P_SABRE_O2))

#define P_SCIMITAR           0x1b1e
#define P_SCIMITAR_PRO       0x1b3e
#define P_SCIMITAR_ELITE     0x1b8b
#define IS_SCIMITAR(kb)      ((kb)->vendor == V_CORSAIR && ((kb)->product == P_SCIMITAR || (kb)->product == P_SCIMITAR_PRO || (kb)->product == P_SCIMITAR_ELITE))

#define P_HARPOON            0x1b3c
#define P_HARPOON_PRO        0x1b75
#define IS_HARPOON(kb)       ((kb)->vendor == V_CORSAIR && ((kb)->product == P_HARPOON || (kb)->product == P_HARPOON_PRO))

#define P_GLAIVE             0x1b34
#define P_GLAIVE_PRO         0x1b74
#define IS_GLAIVE(kb)        ((kb)->vendor == V_CORSAIR && ((kb)->product == P_GLAIVE || (kb)->product == P_GLAIVE_PRO))

#define P_KATAR              0x1b22
#define P_KATAR_PRO          0x1b93
#define P_KATAR_PRO_XT       0x1bac
#define IS_KATAR(kb)         ((kb)->vendor == V_CORSAIR && (kb)->product == P_KATAR)

#define P_IRONCLAW           0x1b5d
#define IS_IRONCLAW(kb)      ((kb)->vendor == V_CORSAIR && (kb)->product == P_IRONCLAW)

#define P_NIGHTSWORD         0x1b5c
#define IS_NIGHTSWORD(kb)    ((kb)->vendor == V_CORSAIR && (kb)->product == P_NIGHTSWORD)

#define P_DARK_CORE          0x1b35 /* wired */
#define P_DARK_CORE_WL       0x1b64 /* wireless */
#define P_DARK_CORE_SE       0x1b4b /* wired */
#define P_DARK_CORE_SE_WL    0x1b51 /* wireless */
// This is _ONLY_ for the NXP dark core
#define IS_DARK_CORE_NXP(kb) ((kb)->vendor == V_CORSAIR && ((kb)->product == P_DARK_CORE || (kb)->product == P_DARK_CORE_WL || (kb)->product == P_DARK_CORE_SE || (kb)->product == P_DARK_CORE_SE_WL))

#define P_IRONCLAW_W_U       0x1b4c /* Ironclaw RGB Wireless USB */
#define P_IRONCLAW_W_D       0x1b66 /* Ironclaw RGB Wireless Dongle */
#define P_DARK_CORE_RGB_PRO          0x1b80 /* wired */
#define P_DARK_CORE_RGB_PRO_WL       0x1b81/* wireless - dongle  Defining Dark Core RGB Pro SE here as it uses the same protocol as Ironclaw */
#define P_DARK_CORE_RGB_PRO_SE       0x1b7e /* wired */
#define P_DARK_CORE_RGB_PRO_SE_WL    0x1b7f/* wireless - dongle  Defining Dark Core RGB Pro SE here as it uses the same protocol as Ironclaw */
#define IS_IRONCLAW_W(kb)    ((kb)->vendor == V_CORSAIR && ((kb)->product == P_IRONCLAW_W_U || (kb)->product == P_IRONCLAW_W_D || (kb)->product == P_DARK_CORE_RGB_PRO || (kb)->product == P_DARK_CORE_RGB_PRO_WL || (kb)->product == P_DARK_CORE_RGB_PRO_SE || (kb)->product == P_DARK_CORE_RGB_PRO_SE_WL))

#define P_HARPOON_WL_U       0x1b5e
#define P_HARPOON_WL_D       0x1b65
#define IS_HARPOON_WL(kb)    ((kb)->vendor == V_CORSAIR && ((kb)->product == P_HARPOON_WL_U || (kb)->product == P_HARPOON_WL_D))

#define P_POLARIS            0x1b3b
#define IS_POLARIS(kb)       ((kb)->vendor == V_CORSAIR && ((kb)->product == P_POLARIS))

#define P_ST100              0x0a34
#define IS_ST100(kb)         ((kb)->vendor == V_CORSAIR && ((kb)->product == P_ST100))

#define P_GENERIC_BRAGI_DONGLE 0x1ba6

extern const size_t N_MODELS;

typedef struct _device_desc {
    ushort idVendor;
    ushort idProduct;
} device_desc;

extern const device_desc models[];

extern device_desc ignored_devices[];

///
/// Uncomment to see USB packets sent to the device
// #define DEBUG_USB_SEND

///
/// Uncomment to see USB packets received from the device through os_usbrecv()
// #define DEBUG_USB_RECV

///
/// Uncomment to see USB packets received from the device through the input thread
// #define DEBUG_USB_INPUT

///
/// \brief vendor_str Vendor/product string representations
/// \param vendor \a ushort vendor ID
/// \return a string: either "" or "corsair"
const char* vendor_str(ushort vendor);

///
/// \brief product_str returns a condensed view on what type of device we have.
/// \param product is the \a ushort USB device product ID
/// \return string to identify a type of device (see below)
const char* product_str(ushort product);

// Used for devices that use the CUE protocol but have no backlight
#define HAS_NO_LIGHTS(kb)               (IS_K66(kb))

/// RGB vs non-RGB test
/// (note: non-RGB Strafe is still considered "RGB" in that it shares the same protocol.
/// The difference is denoted with the "monochrome" feature).
#define IS_LEGACY(vendor, product)      ((vendor) == (V_CORSAIR) && ((product) == (P_K65_LEGACY) || (product) == (P_K70_LEGACY) || (product) == P_K90_LEGACY || (product) == (P_K95_LEGACY) || (product) == (P_M95)))

/// The difference between non RGB and monochrome is, that monochrome has lights, but just in one color.
/// nonRGB has no lights.
/// Change this if new \b monochrome devices are added
#define IS_MONOCHROME(vendor, product)  ((vendor) == (V_CORSAIR) && ((product) == (P_K63_NRGB) || (product) == (P_K63_NRGB_WL) || (product) == (P_K63_NRGB_WL2) || (product) == (P_K63_NRGB_WL3) || (product) == (P_K63_NRGB_WL4) || (product) == (P_K68_NRGB) || (product) == (P_STRAFE_NRGB) || (product) == (P_STRAFE_NRGB_2) || (product) == (P_K60_PRO_MONO)))

/// For calling with a usbdevice*, vendor and product are extracted and IS_LEGACY() is returned.
#define IS_LEGACY_DEV(kb)               IS_LEGACY((kb)->vendor, (kb)->product)

/// For calling with a usbdevice*, vendor and product are extracted and IS_MONOCHROME() is returned.
#define IS_MONOCHROME_DEV(kb)           IS_MONOCHROME((kb)->vendor, (kb)->product)

/// Full color range (16.8M) vs partial color range (512)
#define IS_FULLRANGE(kb)                (!IS_LEGACY((kb)->vendor, (kb)->product) && (kb)->product != P_K65 && (kb)->product != P_K70 && (kb)->product != P_K95 && (kb)->product != P_STRAFE_NRGB)

/// Mouse vs keyboard test
#define IS_MOUSE(vendor, product)       ((vendor) == (V_CORSAIR) && ((product) == (P_M55_RGB_PRO) || (product) == (P_M65) || (product) == (P_M65_PRO) || (product) == (P_M65_RGB_ELITE) || (product) == (P_M95) || (product) == (P_SABRE_O) || (product) == (P_SABRE_L) || (product) == (P_SABRE_N) || (product) == (P_SCIMITAR) || (product) == (P_SCIMITAR_PRO) || (product) == (P_SCIMITAR_ELITE) || (product) == (P_SABRE_O2) || (product) == (P_GLAIVE) || (product) == (P_HARPOON) || (product) == (P_HARPOON_PRO) || (product) == (P_KATAR) || (product) == (P_KATAR_PRO) || (product) == (P_KATAR_PRO_XT) || (product) == (P_IRONCLAW) || (product) == (P_NIGHTSWORD) || (product) == (P_DARK_CORE) || (product) == (P_DARK_CORE_WL) || (product) == (P_DARK_CORE_SE) || (product) == (P_DARK_CORE_SE_WL) || (product) == (P_IRONCLAW_W_U) || (product) == (P_IRONCLAW_W_D) || (product) == (P_DARK_CORE_RGB_PRO) || (product) == (P_DARK_CORE_RGB_PRO_WL) || (product) == (P_DARK_CORE_RGB_PRO_SE) || (product) == (P_DARK_CORE_RGB_PRO_SE_WL) || (product) == P_HARPOON_WL_U || (product) == P_HARPOON_WL_D || (product) == P_GLAIVE_PRO))

/// For calling with a usbdevice*, vendor and product are extracted and IS_MOUSE() is returned.
#define IS_MOUSE_DEV(kb)                IS_MOUSE((kb)->vendor, (kb)->product)

/// Used to apply quirks and features to the PLATINUM devices.
#define IS_PLATINUM(kb)                 ((kb)->vendor == V_CORSAIR && ((kb)->product == P_K95_PLATINUM))

// Mousepad test
#define IS_MOUSEPAD(vendor, product)    ((vendor) == (V_CORSAIR) && (product) == (P_POLARIS))
#define IS_MOUSEPAD_DEV(kb)             IS_MOUSEPAD((kb)->vendor, (kb)->product)

// Devices that are considered experimental and are either not fully tested, or aren't fully implemented
#define IS_EXPERIMENTAL(vendor, product) ((vendor) == V_CORSAIR && ((product) == P_K63_NRGB_WL || (product) == P_K63_NRGB_WL2 || (product) == P_K63_NRGB_WL3 || (product) == P_K63_NRGB_WL4 || (product) == P_DARK_CORE || (product) == P_DARK_CORE_WL || (product) == P_DARK_CORE_SE || (product) == P_DARK_CORE_SE_WL || (product) == P_IRONCLAW_W_U || (product) == P_IRONCLAW_W_D || (product) == P_DARK_CORE_RGB_PRO_SE || (product) == P_DARK_CORE_RGB_PRO_SE_WL || (product) == P_HARPOON_WL_U || (product) == P_HARPOON_WL_D || (product) == P_K57_U || (product) == P_K57_D || (product) == P_DARK_CORE_RGB_PRO || (product) == P_DARK_CORE_RGB_PRO_WL || (product) == P_GENERIC_BRAGI_DONGLE || (product) == P_K65_MINI))

/// Some devices cause usbhid to spend a long time initialising it. To work around this, we intentionally uncleanly
/// deinitialise the device, skipping the usbhid handover.
#define NEEDS_UNCLEAN_EXIT(kb)          ((kb)->product == P_K65_RFIRE || (kb)->product == P_K70_RFIRE || (kb)->product == P_K70_RFIRE_NRGB || (kb)->product == P_K95)

/// Used for new devices that come with V3 firmware endpoint configuration out of the factory, but have fwversion < 0x300.
/// Note: only the RGB variant of the K68 needs a v3 override.
/// Note: P_K70_MK2 doesn't seem to require this, but it was added as a precaution
#define IS_V3_OVERRIDE(kb)              ((kb)->product == P_K68 || (kb)->product == P_K70_MK2 || (kb)->product == P_K70_MK2SE || (kb)->product == P_STRAFE_MK2 || (kb)->product == P_IRONCLAW || (kb)->product == P_NIGHTSWORD || (kb)->product == P_SCIMITAR_ELITE || IS_DARK_CORE_NXP(kb) || (kb)->product == P_IRONCLAW_W_U || (kb)->product == P_IRONCLAW_W_D || (kb)->product == P_GLAIVE_PRO)

/// Used when a device has a firmware with a low version number that uses the new endpoint configuration.
#define IS_V2_OVERRIDE(kb)              (IS_V3_OVERRIDE(kb) || IS_PLATINUM(kb) || IS_K63(kb) || IS_K68(kb) || IS_HARPOON(kb) || IS_GLAIVE(kb) || IS_KATAR(kb) || (kb)->product == P_STRAFE_NRGB_2 || IS_POLARIS(kb) || IS_ST100(kb) || (kb)->product == P_SCIMITAR_PRO || (kb)->product == P_K66)

/// Used for devices that have a single IN endpoint, and no HID input
#define IS_SINGLE_EP(kb)                (IS_POLARIS(kb) || IS_ST100(kb))

/// Used for devices which support wireless.
#define IS_WIRELESS_DEV(kb)             (IS_WIRELESS((kb)->vendor, (kb)->product))

/// Used for ID combinations which support wireless.
#define IS_WIRELESS(vendor, product)    ((vendor) == V_CORSAIR && ((product) == P_DARK_CORE || (product) == P_DARK_CORE_WL || (product) == P_DARK_CORE_SE || (product) == P_DARK_CORE_SE_WL || (product) == P_DARK_CORE_RGB_PRO || (product) == P_DARK_CORE_RGB_PRO_WL || (product) == P_DARK_CORE_RGB_PRO_SE || (product) == P_DARK_CORE_RGB_PRO_SE_WL || (product) == P_K63_NRGB_WL || (product) == P_K63_NRGB_WL2 || (product) == P_K63_NRGB_WL3 || (product) == P_K63_NRGB_WL4 || (product) == P_IRONCLAW_W_U || (product) == P_IRONCLAW_W_D || (product) == P_K57_U || (product) == P_K57_D || (product) == P_HARPOON_WL_U || (product) == P_HARPOON_WL_D || (product) == P_GENERIC_BRAGI_DONGLE))
/// Used for devices which are connected via dongle instead of cable.
#define IS_DONGLE(kb)                   ((kb)->product == P_DARK_CORE_WL || (kb)->product == P_DARK_CORE_SE_WL || (kb)->product == P_DARK_CORE_RGB_PRO_WL || (kb)->product == P_DARK_CORE_RGB_PRO_SE_WL || (kb)->product == P_IRONCLAW_W_D || (kb)->product == P_K63_NRGB_WL2 || (kb)->product == P_K63_NRGB_WL4 || (kb)->product == P_K57_D || (kb)->product == P_HARPOON_WL_D || (kb)->product == P_GENERIC_BRAGI_DONGLE)

/// Used for devices that use a file-based hardware animation system.
#define USES_FILE_HWSAVE(kb)            ((kb)->product == P_K95_PLATINUM || (kb)->product == P_K70_MK2 || (kb)->product == P_K70_MK2SE || (kb)->product == P_K70_PRO || (kb)->product == P_STRAFE_MK2 || (kb)->product == P_GLAIVE || (kb)->product == P_SCIMITAR_PRO || (kb)->product == P_SCIMITAR_ELITE || (kb)->product == P_K70_MK2LP || (kb)->product == P_M65_RGB_ELITE || (kb)->product == P_IRONCLAW || IS_K63_WL(kb) || IS_DARK_CORE_NXP(kb) || (kb)->product == P_IRONCLAW_W_U || (kb)->product == P_IRONCLAW_W_D || (kb)->product == P_NIGHTSWORD)

/// Devices here support setting the pollrate through software
#define SUPPORTS_ADJRATE(kb)            ((kb)->product == P_K60_PRO_TKL || (kb)->product == P_K60_PRO_RGB || (kb)->product == P_K60_PRO_MONO || (kb)->product == P_K60_PRO_RGB_LP || (kb)->product == P_K60_PRO_RGB_SE || (kb)->product == P_K63_NRGB || (kb)->product == P_K66 || (kb)->product == P_K68 || (kb)->product == P_K68_NRGB || (kb)->product == P_K70_MK2 || (kb)->product == P_K70_MK2SE || (kb)->product == P_K70_MK2LP || (kb)->product == P_K70_PRO || (kb)->product == P_K95_PLATINUM || (kb)->product == P_STRAFE || (kb)->product == P_STRAFE_NRGB || (kb)->product == P_STRAFE_NRGB_2 || (kb)->product == P_STRAFE_MK2 || (kb)->product == P_M55_RGB_PRO || (kb)->product == P_M65 || (kb)->product == P_M65_PRO || (kb)->product == P_M65_RGB_ELITE || (kb)->product == P_M95 || (kb)->product == P_SABRE_O || (kb)->product == P_SABRE_L || (kb)->product == P_SABRE_N || (kb)->product == P_SABRE_O2 || (kb)->product == P_SCIMITAR || (kb)->product == P_SCIMITAR_ELITE || (kb)->product == P_SCIMITAR_PRO || (kb)->product == P_HARPOON || (kb)->product == P_HARPOON_PRO || (kb)->product == P_GLAIVE || (kb)->product == P_KATAR || (kb)->product == P_KATAR_PRO_XT || (kb)->product == P_KATAR_PRO || (kb)->product == P_NIGHTSWORD || IS_DARK_CORE_NXP(kb) || (kb)->product == P_K95_PLATINUM_XT || (kb)->product == P_GLAIVE_PRO || (kb)->product == P_K55_PRO || (kb)->product == P_K55_PRO_XT || (kb)->product == P_GENERIC_BRAGI_DONGLE)

// Bragi devices that use macros for media keys. Currently used to apply a terrible workaround.
#define BRAGI_HAS_MEDIA_MACRO(kb)       (IS_K60PRORGB(kb))

// Devices that have volume wheels (or don't)
#define DEV_HAS_VOLWHEEL(kb)            (!(IS_K65(kb) || IS_K63(kb) || (kb)->product == P_K57_U || IS_K55(kb) || BRAGI_HAS_MEDIA_MACRO(kb)))

// Devices that use the NXP protocol and have the DPI stage RGB data in the DPI packet
#define NXP_RGB_IN_DPI_PKT(kb)          ((kb)->vendor == V_CORSAIR && ((kb)->product == P_GLAIVE_PRO || IS_DARK_CORE_NXP(kb)))

#define DELAY_100MS()       \
        clock_nanosleep(CLOCK_MONOTONIC, 0, &(struct timespec) {.tv_nsec = 100000000}, NULL)

// This is used in NXP only. Would be nice if it can be removed.
#define DELAY_30MS()        \
        clock_nanosleep(CLOCK_MONOTONIC, 0, &(struct timespec) {.tv_nsec = 30000000}, NULL)

// This should be removed in the future when we implement autodetection
#define USES_BRAGI(vendor, product)                  ((vendor) == (V_CORSAIR) && ((product) == (P_M55_RGB_PRO) || (product) == (P_IRONCLAW_W_U) || (product) == (P_IRONCLAW_W_D) || (product) == (P_K95_PLATINUM_XT) || (product) == (P_DARK_CORE_RGB_PRO_SE) || (product) == (P_DARK_CORE_RGB_PRO_SE_WL) || (product) == P_HARPOON_WL_U || (product) == P_HARPOON_WL_D || (product) == P_K57_U || (product) == P_K57_D || (product) == P_KATAR_PRO_XT || (product) == P_KATAR_PRO || (product) == P_K60_PRO_RGB || (product) == P_K60_PRO_RGB_LP || (product) == P_K60_PRO_RGB_SE || (product) == P_K60_PRO_MONO || (product) == P_K60_PRO_TKL || (product) == P_K55_PRO || (product) == P_K55_PRO_XT || (product) == (P_DARK_CORE_RGB_PRO) || (product) == (P_DARK_CORE_RGB_PRO_WL) || (product) == P_GENERIC_BRAGI_DONGLE || (product) == P_K100_OPTICAL || (product) == P_K100_MECHANICAL || (product) == P_K100_OPTICAL_VARIANT || (product) == P_K65_MINI || (product) == P_K70_TKL || (product) == P_K70_PRO))

// Devices that use bragi jumbo packets (1024 bytes)
#define USES_BRAGI_JUMBO(vendor, product)           ((vendor) == (V_CORSAIR) && ((product) == P_K100_OPTICAL || (product) == P_K100_MECHANICAL || (product) == P_K100_OPTICAL_VARIANT || (product) == P_K65_MINI || (product) == P_K70_TKL || (product) == P_K70_PRO))

// Used for devices that have the scroll wheel packet in the hardware hid packet only
#define SW_PKT_HAS_NO_WHEEL(kb)                     ((kb)->vendor == V_CORSAIR && ((kb)->product == P_M55_RGB_PRO || (kb)->product == P_KATAR_PRO_XT || (kb)->product == P_KATAR_PRO))


/// Start the USB main loop. Returns program exit code when finished
int usbmain();

/// Stop the USB system.
void usbkill();

/// \attention Lock a device's dmutex (see device.h) before accessing the USB interface.
///
/// \brief setupusb starts a thread with kb as parameter and _setupusb() as entrypoint.
/// \param kb THE usbdevice* used everywhere
/// \param[OUT] kb->thread is used to store the thread ID of the fresh created thread.
void setupusb(usbdevice* kb);

///
/// \brief os_setupusb OS-specific setup for a specific usb device.
/// \param kb THE usbdevice*
/// \return 0 on success, -1 otherwise.
int os_setupusb(usbdevice* kb);

///
/// \brief os_inputmain is run in a separate thread and will be detached from the main thread, so it needs to clean up its own resources.
/// \param context THE usbdevice* ; Because os_inputmain() is started as a new thread, its formal parameter is named "context".
/// \return null
///
void* os_inputmain(void* context);

/// \brief revertusb sets a given device to inactive (hardware controlled) mode if not a fw-ugrade is indicated
/// \param kb THE usbdevice*
/// \return 0 on success or if device needs firmware upgrade, -1 otherwise
int revertusb(usbdevice* kb);

///
/// \brief closeusb Close a USB device and remove device entry.
/// \param[IN,OUT] kb
/// \return Returns 0 (everytime. No error handling is done!)
int closeusb(usbdevice* kb);

///
/// \brief os_closeusb unclaim it, destroy the udev device and clear data structures at kb
/// \param[IN,OUT] kb THE usbdevice*
///
void os_closeusb(usbdevice* kb);

///
/// \brief _resetusb Reset a USB device.
/// \param kb THE usbdevice*
/// \param file filename for error messages
/// \param line line where it is called for error messages
/// \return Returns 0 on success, -1 if device should be removed
int _resetusb(usbdevice* kb, const char* file, int line);

/// resetusb() is just a macro to call _resetusb() with debuggin constants (file, lineno)
#define resetusb(kb) _resetusb(kb, __FILE_NOPATH__, __LINE__)

///
/// \brief os_resetusb is the os specific implementation for resetting usb
/// \param kb THE usbdevice*
/// \param file filename for error messages
/// \param line line where it is called for error messages
/// \return Returns 0 on success, -2 if device should be removed and -1 if reset should by tried again
int os_resetusb(usbdevice* kb, const char* file, int line);

///
/// \brief _usbsend send a logical message completely to the given device
/// \param kb THE usbdevice*
/// \param[IN] messages a Pointer to the first byte of the logical message
/// \param[IN] count how many MSG_SIZE buffers is the logical message long?
/// \param[IN] file for debugging
/// \param[IN] line for debugging
/// \param[in] reset_stop global variable is read
/// \return number of Bytes sent (ideal == count * MSG_SIZE);\n 0 if a block could not be sent and it was not a timeout OR \b reset_stop was required
int _usbsend(usbdevice* kb, void* messages, size_t msg_len, int count, const char* file, int line);

/// \brief usbsend macro is used to wrap _usbsend() with debugging information (file and lineno)
/// \param kb THE usbdevice*
/// \param[IN] messages a Pointer to the first byte of the logical message
/// \param[IN] count how many MSG_SIZE buffers is the logical message long?
#define usbsend(kb, messages, msg_len, count) _usbsend(kb, messages, msg_len, count, __FILE_NOPATH__, __LINE__)

///
/// \brief _usbrecv Request data from a USB device by first sending an output packet and then reading the response.
/// \param kb THE usbdevice*
/// \param[IN] out_msg What information does the caller want from the device?
/// \param[OUT] in_msg Here comes the answer; The names represent the usb view, not the view of this function! So INput from usb is OUTput of this function.
/// \param[IN] file for debugging
/// \param[IN] line for debugging
/// \param[IN] reset_stop global variable is read
/// \return number of bytes read or zero on failure.
int _usbrecv(usbdevice* kb, void* out_msg, size_t msg_len, uchar* in_msg, const char* file, int line);

/// \brief usbrecv macro is used to wrap _usbrecv() with debugging information (file and lineno)
/// \param kb THE usbdevice*
/// \param[IN] out_msg What information does the caller want from the device?
/// \param[OUT] in_msg Here comes the answer; The names represent the usb view, not the view of this function! So INput from usb is OUTput of this function.
#define usbrecv(kb, out_msg, msg_len, in_msg) _usbrecv(kb, out_msg, msg_len, in_msg, __FILE_NOPATH__, __LINE__)

///
/// \brief usb_tryreset does what the name means: Try to reset the usb via resetusb()
/// \param[in,out] kb THE usbdevice*
/// \param[in] reset_stop global variable is read
/// \return 0 on success, -1 otherwise
int usb_tryreset(usbdevice* kb);

void print_urb_buffer(const char* prefix, const unsigned char* buffer, int actual_length, const char* file, int line, const char* function, int devnum, const uchar ep);

int os_usb_control(usbdevice* kb, ctrltransfer* transfer, const char* file, int line);

// receive message from initial sighandler socketpair communication
extern int sighandler_pipe[2];
extern void exithandler(int type);

void reactivate_devices();

typedef struct _dpi_list {
    ushort dev;
    ushort dpi; // 0 for device-specific way to query (currently unused)
} dpi_list;

extern const dpi_list mouse_dpi_list[];

int os_usb_interrupt_out(usbdevice* kb, unsigned int ep, unsigned int len, uchar* data, const char* file, int line);

///
/// \brief Wait for devices to be reactivated after suspend.
///
/// Should be called before communicating with any device.
///
/// \warning dmutex may NOT be held by the calling thread.
void wait_until_suspend_processed();

#endif  // USB_H

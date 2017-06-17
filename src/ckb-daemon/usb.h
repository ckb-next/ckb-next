#ifndef USB_H
#define USB_H

#include "includes.h"
#include "keymap.h"

/// \file usb.h
/// Definitions for using USB interface
///
/// \details Vendor/product codes
///
/// The list of defines in the first part of the file describes the various types of equipment from Corsair
/// and summarizes them according to specific characteristics.
/// \n Each device type is described with two defines:
/// - On the one hand the device ID with which the device can be recognized on the USB as a short
/// - and on the other hand the same representation as a string, but without leading "0x".
///
/// First entry-pair is the Provider ID (vendorID) from Corsair.
///
/// Block No. | contains | Devices are bundled via
/// --------- | -------- | -----------------------
/// 1 | The first block contains the K65-like keyboards, regardless of their properties (RGB, ...). | In summary, they can be queried using the macro IS_K65().
/// 2 | the K70-like Keyboards with all their configuration types | summarized by IS_K70().
/// 3 | the K95 series keyboards | collected with the macro IS_K95().
/// 4 | strafe keyboards | IS_STRAFE()
/// 5 | M65 mice with and without RGB | IS_M65()
/// 6 | The SABRE and HARPOON mice.\n Maybe this will be divided int two different blocks later because of different nummber of special keys | IS_SABRE()
/// 7 | The Scimitar mouse devices | IS_SCIMITAR()
///

/// \brief For the following Defines please see "Detailed Description"
///
/// \warning When adding new devices please update src/ckb/fwupgradedialog.cpp as well.
/// \n It should contain the same vendor/product IDs for any devices supporting firmware updates.
/// \n In the same way, all other corresponding files have to be supplemented or modified:
/// Currently known for this are \b usb_linux.c and \b usb_mac.c
///
#define V_CORSAIR       0x1b1c
#define V_CORSAIR_STR   "1b1c"

#define P_K65           0x1b17
#define P_K65_STR       "1b17"
#define P_K65_NRGB      0x1b07
#define P_K65_NRGB_STR  "1b07"
#define P_K65_LUX       0x1b37
#define P_K65_LUX_STR   "1b37"
#define P_K65_RFIRE     0x1b39
#define P_K65_RFIRE_STR "1b39"
#define IS_K65(kb)      ((kb)->vendor == V_CORSAIR && ((kb)->product == P_K65 || (kb)->product == P_K65_NRGB || (kb)->product == P_K65_LUX || (kb)->product == P_K65_RFIRE))

#define P_K70                0x1b13
#define P_K70_STR            "1b13"
#define P_K70_NRGB           0x1b09
#define P_K70_NRGB_STR       "1b09"
#define P_K70_LUX            0x1b33
#define P_K70_LUX_STR        "1b33"
#define P_K70_LUX_NRGB	     0x1b36
#define P_K70_LUX_NRGB_STR   "1b36"
#define P_K70_RFIRE          0x1b38
#define P_K70_RFIRE_STR      "1b38"
#define P_K70_RFIRE_NRGB     0x1b3a
#define P_K70_RFIRE_NRGB_STR "1b3a"
#define IS_K70(kb)           ((kb)->vendor == V_CORSAIR && ((kb)->product == P_K70 || (kb)->product == P_K70_NRGB || (kb)->product == P_K70_RFIRE || (kb)->product == P_K70_RFIRE_NRGB || (kb)->product == P_K70_LUX || (kb)->product == P_K70_LUX_NRGB))

#define P_K95           0x1b11
#define P_K95_STR       "1b11"
#define P_K95_NRGB      0x1b08
#define P_K95_NRGB_STR  "1b08"
#define P_K95_PLATINUM  0x1b2d
#define P_K95_PLATINUM_STR "1b2d"
#define IS_K95(kb)      ((kb)->vendor == V_CORSAIR && ((kb)->product == P_K95 || (kb)->product == P_K95_NRGB || (kb)->product == P_K95_PLATINUM))

#define P_STRAFE            0x1b20
#define P_STRAFE_STR        "1b20"
#define P_STRAFE_NRGB       0x1b15
#define P_STRAFE_NRGB_STR   "1b15"
#define IS_STRAFE(kb)       ((kb)->vendor == V_CORSAIR && ((kb)->product == P_STRAFE || (kb)->product == P_STRAFE_NRGB))

#define P_M65           0x1b12
#define P_M65_STR       "1b12"
#define P_M65_PRO       0x1b2e
#define P_M65_PRO_STR   "1b2e"
#define IS_M65(kb)      ((kb)->vendor == V_CORSAIR && ((kb)->product == P_M65 || (kb)->product == P_M65_PRO))

#define P_SABRE_O       0x1b14  /* optical */
#define P_SABRE_O_STR   "1b14"
#define P_SABRE_L       0x1b19  /* laser */
#define P_SABRE_L_STR   "1b19"
#define P_SABRE_N       0x1b2f  /* new? */
#define P_SABRE_N_STR   "1b2f"
#define P_SABRE_O2      0x1b32 /* Observed on a CH-9000111-EU model SABRE */
#define P_SABRE_O2_STR  "1b32"
#define IS_SABRE(kb)    ((kb)->vendor == V_CORSAIR && ((kb)->product == P_SABRE_O || (kb)->product == P_SABRE_L || (kb)->product == P_SABRE_N || (kb)->product == P_SABRE_O2))

#define P_SCIMITAR          0x1b1e
#define P_SCIMITAR_STR      "1b1e"
#define P_SCIMITAR_PRO      0x1b3e
#define P_SCIMITAR_PRO_STR  "1b3e"
#define IS_SCIMITAR(kb) ((kb)->vendor == V_CORSAIR && ((kb)->product == P_SCIMITAR || (kb)->product == P_SCIMITAR_PRO))

///
/// uncomment the following Define to see USB packets sent to the device
// #define DEBUG_USB

///
///uncomment to see USB packets sent from the device
// #define DEBUG_USB_RECV

///
/// \brief vendor_str Vendor/product string representations
/// \param vendor \a short vendor ID
/// \return a string: either "" or "corsair"
const char* vendor_str(short vendor);

///
/// \brief product_str returns a condensed view on what type of device we have.
/// \param product is the \a short USB device product ID
/// \return string to identify a type of device (see below)
const char* product_str(short product);

/// RGB vs non-RGB test
/// (note: non-RGB Strafe is still considered "RGB" in that it shares the same protocol.
/// The difference is denoted with the "monochrome" feature).
#define IS_RGB(vendor, product)         ((vendor) == (V_CORSAIR) && (product) != (P_K65_NRGB) && (product) != (P_K70_NRGB) && (product) != (P_K95_NRGB))

/// The difference between non RGB and monochrome is, that monochrome has lights, but just in one color.
/// nonRGB has no lights.
/// Change this if new \b monochrome devices are added
#define IS_MONOCHROME(vendor, product)  ((vendor) == (V_CORSAIR) && (product) == (P_STRAFE_NRGB))

/// For calling with a usbdevice*, vendor and product are extracted and IS_RGB() is returned.
#define IS_RGB_DEV(kb)                  IS_RGB((kb)->vendor, (kb)->product)

/// For calling with a usbdevice*, vendor and product are extracted and IS_MONOCHROME() is returned.
#define IS_MONOCHROME_DEV(kb)           IS_MONOCHROME((kb)->vendor, (kb)->product)

/// Full color range (16.8M) vs partial color range (512)
#define IS_FULLRANGE(kb)                (IS_RGB((kb)->vendor, (kb)->product) && (kb)->product != P_K65 && (kb)->product != P_K70 && (kb)->product != P_K95)

/// Mouse vs keyboard test
#define IS_MOUSE(vendor, product)       ((vendor) == (V_CORSAIR) && ((product) == (P_M65) || (product) == (P_M65_PRO) || (product) == (P_SABRE_O) || (product) == (P_SABRE_L) || (product) == (P_SABRE_N) || (product) == (P_SCIMITAR) || (product) == (P_SCIMITAR_PRO) || (product) == (P_SABRE_O2)))

/// For calling with a usbdevice*, vendor and product are extracted and IS_MOUSE() is returned.
#define IS_MOUSE_DEV(kb)                IS_MOUSE((kb)->vendor, (kb)->product)

/// USB delays for when the keyboards get picky about timing
/// That was the original comment, but it is used anytime.
/// The short delay is used before any send or receive
#define DELAY_SHORT(kb)     usleep((int)(kb)->usbdelay * 1000)  // base (default: 5ms)

/// the medium delay is used after sending a command before waiting for the answer.
#define DELAY_MEDIUM(kb)    usleep((int)(kb)->usbdelay * 10000) // x10 (default: 50ms)

/// The longest delay takes place where something went wrong (eg when resetting the device)
#define DELAY_LONG(kb)      usleep(100000)                      // long, fixed 100ms

/// This constant is used to initialize \b kb->usbdelay.
/// It is used in many places (see macros above) but often also overwritten to the fixed value of 10.
/// Pure Hacker code.
#define USB_DELAY_DEFAULT   5

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
/// \return number of Bytes sent (ideal == count * MSG_SIZE);\n 0 if a block could not be sent and it was not a timeout OR \b reset_stop was required or \b hwload_mode is not set to "always"
int _usbsend(usbdevice* kb, const uchar* messages, int count, const char* file, int line);

/// \brief usbsend macro is used to wrap _usbsend() with debugging information (file and lineno)
/// \param kb THE usbdevice*
/// \param[IN] messages a Pointer to the first byte of the logical message
/// \param[IN] count how many MSG_SIZE buffers is the logical message long?
#define usbsend(kb, messages, count) _usbsend(kb, messages, count, __FILE_NOPATH__, __LINE__)

///
/// \brief _usbrecv Request data from a USB device by first sending an output packet and then reading the response.
/// \param kb THE usbdevice*
/// \param[IN] out_msg What information does the caller want from the device?
/// \param[OUT] in_msg Here comes the answer; The names represent the usb view, not the view of this function! So INput from usb is OUTput of this function.
/// \param[IN] file for debugging
/// \param[IN] line for debugging
/// \param[IN] reset_stop global variable is read
/// \return number of bytes read or zero on failure.
int _usbrecv(usbdevice* kb, const uchar* out_msg, uchar* in_msg, const char* file, int line);

/// \brief usbrecv macro is used to wrap _usbrecv() with debugging information (file and lineno)
/// \param kb THE usbdevice*
/// \param[IN] out_msg What information does the caller want from the device?
/// \param[OUT] in_msg Here comes the answer; The names represent the usb view, not the view of this function! So INput from usb is OUTput of this function.
#define usbrecv(kb, out_msg, in_msg) _usbrecv(kb, out_msg, in_msg, __FILE_NOPATH__, __LINE__)

/// \details
/// \brief os_usbsend sends a data packet (MSG_SIZE = 64) Bytes long
/// \param kb THE usbdevice*
/// \param out_msg the MSGSIZE char long buffer to send
/// \param is_recv if true, just send an ioctl for further reading packets. If false, send the data at \b out_msg.
/// \param file for debugging
/// \param line for debugging
/// \return -1 on timeout (try again), 0 on hard error, numer of bytes sent otherwise
int os_usbsend(usbdevice* kb, const uchar* out_msg, int is_recv, const char* file, int line);

///
/// \brief os_usbrecv receives a max MSGSIZE long buffer from usb device
/// \param kb THE usbdevice*
/// \param in_msg the buffer to fill with the message received
/// \param file for debugging
/// \param line for debugging
/// \return -1 on timeout, 0 on hard error, numer of bytes received otherwise
int os_usbrecv(usbdevice* kb, uchar* in_msg, const char* file, int line);

///
/// \brief os_sendindicators update the indicators for the special keys (Numlock, Capslock and what else?)
/// \param kb THE usbdevice*
void os_sendindicators(usbdevice* kb);

///
/// \brief _nk95cmd If we control a non RGB keyboard, set the keyboard via ioctl with usbdevfs_ctrltransfer
/// \param kb THE usbdevice*
/// \param bRequest the byte array with the usb request
/// \param wValue a usb wValue
/// \param file for error message
/// \param line for error message
/// \return 1 (true) on failure, 0 (false) on success.
int _nk95cmd(usbdevice* kb, uchar bRequest, ushort wValue, const char* file, int line);

/// \brief nk95cmd() macro is used to wrap _nk95cmd() with debugging information (file and lineno).
/// the command structure is different:
/// \n Just the bits 23..16 are used as bits 7..0 for bRequest
/// \n Bits 15..0 are used as wValue
#define nk95cmd(kb, command) _nk95cmd(kb, (command) >> 16 & 0xFF, (command) & 0xFFFF, __FILE_NOPATH__, __LINE__)

/// Hardware-specific commands for the K95 nonRGB,
/// \see [usb2.0 documentation for details](http://www.usb.org/developers/docs/usb_20.zip).
/// Set Hardware playback off
#define NK95_HWOFF  0x020030

/// Hardware playback on
#define NK95_HWON   0x020001

/// Switch to mode 1
#define NK95_M1     0x140001

/// Switch to mode 2
#define NK95_M2     0x140002

/// Switch to mode 3
#define NK95_M3     0x140003

///
/// \brief usb_tryreset does what the name means: Try to reset the usb via resetusb()
/// \param[in,out] kb THE usbdevice*
/// \param[in] reset_stop global variable is read
/// \return 0 on success, -1 otherwise
int usb_tryreset(usbdevice* kb);

#endif  // USB_H

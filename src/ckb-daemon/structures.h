#ifndef STRUCTURES_H
#define STRUCTURES_H

#include "includes.h"
#include "keymap.h"

// Profile ID structure
typedef struct {
    char guid[16];
    char modified[4];
} usbid;

// Used to manipulate key bitfields
// The do-while business is a hack to make statements like "if(a) SET_KEYBIT(...); else CLEAR_KEYBIT(...);" work
#define SET_KEYBIT(array, index) do { (array)[(index) / 8] |= 1 << ((index) % 8); } while(0)
#define CLEAR_KEYBIT(array, index) do { (array)[(index) / 8] &= ~(1 << ((index) % 8)); } while(0)

// Indicator LEDs
#define I_NUM       1
#define I_CAPS      2
#define I_SCROLL    4

// Maximum number of notification nodes
#define OUTFIFO_MAX 10

// Action triggered when activating a macro
typedef struct {
    short scan;         // Key scancode, OR
    short rel_x, rel_y; // Mouse movement
    char down;          // 0 for keyup, 1 for keydown (ignored if rel_x != 0 || rel_y != 0)
} macroaction;

// Key macro
typedef struct {
    macroaction* actions;
    int actioncount;
    uchar combo[N_KEYBYTES_INPUT];
    char triggered;
} keymacro;

// Key bindings for a mode (keyboard + mouse)
typedef struct {
    // Base bindings
    int base[N_KEYS_INPUT];
    // Macros
    keymacro* macros;
    int macrocount;
    int macrocap;
} binding;
#define MACRO_MAX   1024

// DPI settings for mice
#define DPI_COUNT   6
#define LIFT_MIN    1
#define LIFT_MAX    5
typedef struct {
    // DPI levels
    ushort x[DPI_COUNT];
    ushort y[DPI_COUNT];
    uchar current;
    // Enabled modes (bitfield)
    uchar enabled;
    // Lift height (lowest to highest)
    uchar lift;
    // Angle snap enabled?
    uchar snap;
    // Send to device even if unchanged? (used when initializing profiles)
    uchar forceupdate;
} dpiset;

// Lighting structure for a mode
typedef struct {
    uchar r[N_KEYS_HW + N_MOUSE_ZONES_EXTENDED];
    uchar g[N_KEYS_HW + N_MOUSE_ZONES_EXTENDED];
    uchar b[N_KEYS_HW + N_MOUSE_ZONES_EXTENDED];
    uchar forceupdate;
    uchar sidelight; // strafe sidelight
} lighting;

// Native mode structure
#define MD_NAME_LEN 16
typedef struct {
    lighting light;
    binding bind;
    dpiset dpi;
    // Name and UUID
    usbid id;
    ushort name[MD_NAME_LEN];
    // Key notification settings (bitfield - 0: off, 1: on)
    uchar notify[OUTFIFO_MAX][N_KEYBYTES_INPUT];
    // Indicators permanently off/on
    uchar ioff, ion;
    // Notify mask for indicator LEDs
    uchar inotify[OUTFIFO_MAX];
} usbmode;

// Native profile structure
#define PR_NAME_LEN 16
#define MODE_COUNT  6
typedef struct {
    // Modes
    usbmode mode[MODE_COUNT];
    // Currently-selected mode
    usbmode* currentmode;
    // Last data sent to the device
    lighting lastlight;
    dpiset lastdpi;
    // Profile name and UUID
    ushort name[PR_NAME_LEN];
    usbid id;
} usbprofile;

// Hardware profile structure
#define HWMODE_K70 1
#define HWMODE_K95 3
#define HWMODE_MAX 3
typedef struct {
    // RGB settings
    lighting light[HWMODE_MAX];
    dpiset dpi[HWMODE_MAX];
    // Profile (0) and mode (1...HWMODE_MAX) IDs
    usbid id[HWMODE_MAX + 1];
    // Profile and mode names
    ushort name[HWMODE_MAX + 1][MD_NAME_LEN];
} hwprofile;

// Keyboard/mouse input tracking
typedef struct {
    uchar keys[N_KEYBYTES_INPUT];
    uchar prevkeys[N_KEYBYTES_INPUT];
    short rel_x, rel_y;
} usbinput;

// Device features
#define FEAT_RGB        0x001   // RGB backlighting?
#define FEAT_MONOCHROME 0x002   // RGB protocol but single-color only?
#define FEAT_POLLRATE   0x004   // Known poll rate?
#define FEAT_ADJRATE    0x008   // Adjustable poll rate?
#define FEAT_BIND       0x010   // Rebindable keys?
#define FEAT_NOTIFY     0x020   // Key notifications?
#define FEAT_FWVERSION  0x040   // Has firmware version?
#define FEAT_FWUPDATE   0x080   // Has firmware update?
#define FEAT_HWLOAD     0x100   // Hardware load enabled?

#define FEAT_ANSI       0x200   // ANSI/ISO layout toggle (Mac only - not needed on Linux)
#define FEAT_ISO        0x400
#define FEAT_MOUSEACCEL 0x800   // Mouse acceleration (also Mac only)

// Standard feature sets
#define FEAT_COMMON     (FEAT_BIND | FEAT_NOTIFY | FEAT_FWVERSION | FEAT_MOUSEACCEL | FEAT_HWLOAD)
#define FEAT_STD_RGB    (FEAT_COMMON | FEAT_RGB | FEAT_POLLRATE | FEAT_FWUPDATE)
#define FEAT_STD_NRGB   (FEAT_COMMON)
#define FEAT_LMASK      (FEAT_ANSI | FEAT_ISO)

// Feature test (usbdevice* kb, int feat)
#define HAS_FEATURES(kb, feat)      (((kb)->features & (feat)) == (feat))
#define HAS_ANY_FEATURE(kb, feat)   (!!((kb)->features & (feat)))

// Bricked firmware?
#define NEEDS_FW_UPDATE(kb) ((kb)->fwversion == 0 && HAS_FEATURES((kb), FEAT_FWUPDATE | FEAT_FWVERSION))

// Lines per scroll (OSX only)
#define SCROLL_ACCELERATED  0
#define SCROLL_MIN          1
#define SCROLL_MAX          10

// vtables for keyboards/mice (see command.h)
extern const union devcmd vtable_keyboard;
extern const union devcmd vtable_keyboard_nonrgb;
extern const union devcmd vtable_mouse;

// Structure for tracking keyboard/mouse devices
#define KB_NAME_LEN 34
#define SERIAL_LEN  34
#define MSG_SIZE    64
#define IFACE_MAX   4
typedef struct {
    // Function table (see command.h)
    const union devcmd* vtable;
    // I/O devices
    // Note that FDs have 1 added to them, because these are initialized to zero when the program starts but zero is technically a valid FD
    // So the actual value is (fd - 1)
#ifdef OS_LINUX
    // USB device
    struct udev_device* udev;
    int handle;
    // uinput handles
    int uinput_kb, uinput_mouse;
#else
    // USB identifier
    uint32_t location_id[IFACE_MAX + 1];
    // Device handles
    usb_dev_t handle;
    usb_iface_t ifusb[IFACE_MAX];
    hid_dev_t ifhid[IFACE_MAX];
    io_object_t rm_notify[IFACE_MAX * 2 + 1];   // one for each: handle, ifusb, ifhid
    int epcount_hid, epcount_usb;
    // Result code from the last USB transfer
    kern_return_t lastresult;
    // Input handle
    io_connect_t event;
    // Key-repeat info
    CFRunLoopRef input_loop;
    CFRunLoopTimerRef krtimer;
    struct timespec keyrepeat;
    short lastkeypress;
    // Modifier/mouse button state
    pthread_t indicthread;
    IOOptionBits modifiers;
    uchar mousestate;
    char scroll_rate;
#endif
    // Number of endpoints on the USB device
    int epcount;
    // Thread used for USB/devnode communication. To close: lock mutexes, set handle to zero, unlock, then wait for thread to stop
    pthread_t thread;
    // Thread for device input. Will close on its own when the device is terminated.
    pthread_t inputthread;
    // Keyboard settings
    usbprofile* profile;
    // Hardware modes. Null if not read yet
    hwprofile* hw;
    // Command FIFO
    int infifo;
    // Notification FIFOs, or zero if a FIFO is closed
    int outfifo[OUTFIFO_MAX];
    // Features (see F_ macros)
    ushort features;
    // Whether the keyboard is being actively controlled by the driver
    char active;
    // Device name
    char name[KB_NAME_LEN+1]; // increase by 1 for the trailing \0 for names that are exactly KB_NAME_LEN, e.g. "Corsair STRAFE RGB Gaming Keyboard"
    // Device serial number
    char serial[SERIAL_LEN];
    // USB vendor and product IDs
    short vendor, product;
    // Firmware version
    ushort fwversion;
    // Poll rate (ms), or -1 if unsupported
    char pollrate;
    // USB protocol delay (ms)
    char usbdelay;
    // Current input state
    usbinput input;
    // Indicator LED state
    uchar hw_ileds, hw_ileds_old, ileds;
    // Color dithering in use
    char dither;
} usbdevice;

#endif  // STRUCTURES_H

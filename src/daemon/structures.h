#ifndef STRUCTURES_H
#define STRUCTURES_H

#include "includes.h"
#include "keymap.h"
#include "command.h"
#ifdef OS_MAC
#include "input_mac_vhid.h" // For the VirtualHIDDevice structs
#endif

// Profile ID structure
typedef struct {
    char guid[16];
    char modified[4];
} usbid;

// Used to manipulate key bitfields
// The do-while business is a hack to make statements like "if(a) SET_KEYBIT(...); else CLEAR_KEYBIT(...);" work
#define SET_KEYBIT(array, index) do { (array)[(index) / 8] |= 1 << ((index) % 8); } while(0)
#define CLEAR_KEYBIT(array, index) do { (array)[(index) / 8] &= ~(1 << ((index) % 8)); } while(0)
#define ISSET_KEYBIT(array, index) ((array)[(index) / 8] & (1 << ((index) % 8)))

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
    uint32_t delay;     // us delay after action; UINT_MAX for use global delay
    uint32_t delay_max; // us delay. If != 0 then a delay is randomly picked from delay to delay_max
} macroaction;

struct _macro_param;

// Key macro
typedef struct {
    macroaction* actions;
    int actioncount;
    uchar combo[N_KEYBYTES_INPUT];

    /*
     * The number of times the key has been pressed and released.  It is therefore
     * even if the key is up, and odd if the key is down.  Whenever the macro execution
     * completes:
     *  - The counter is decremented by two, unless it is at 1.
     *  - If state > 0, the macro is repeated, with a delay if state == 1.
     *
     * If the key is released during the delay, the counter is set to 0, and the running
     * thread shuts itself down immediately.
     */
    char triggered;
    struct _macro_param* param;
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
    uchar r[N_KEYS_EXTENDED];
    uchar g[N_KEYS_EXTENDED];
    uchar b[N_KEYS_EXTENDED];
    uchar forceupdate;
    uchar sidelight; // strafe sidelight
} lighting;

// Native mode structure
#define MD_NAME_LEN 16
typedef struct usbmode_ {
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
    signed char whl_rel_x, whl_rel_y;
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

#define FEAT_DONGLE     0x1000  // Is mouse detachable (supports wireless)?
#define FEAT_WIRELESS   0x2000  // Is wireless?
#define FEAT_BATTERY    0x4000  // Has a battery that can be read?

// Standard feature sets
#define FEAT_COMMON     (FEAT_BIND | FEAT_NOTIFY | FEAT_FWVERSION | FEAT_MOUSEACCEL | FEAT_HWLOAD)
#define FEAT_STD_RGB    (FEAT_COMMON | FEAT_RGB | FEAT_POLLRATE | FEAT_FWUPDATE)
#define FEAT_STD_LEGACY (FEAT_COMMON)
#define FEAT_LMASK      (FEAT_ANSI | FEAT_ISO)

// Feature test (usbdevice* kb, int feat)
#define HAS_FEATURES(kb, feat)      (((kb)->features & (feat)) == (feat))
#define HAS_ANY_FEATURE(kb, feat)   (!!((kb)->features & (feat)))

// Bricked firmware?
#define NEEDS_FW_UPDATE(kb) ((kb)->needs_fw_update && HAS_FEATURES((kb), FEAT_FWUPDATE | FEAT_FWVERSION))

// Lines per scroll (OSX only)
#define SCROLL_ACCELERATED  0
#define SCROLL_MIN          1
#define SCROLL_MAX          10

// Physical layout types; update if more are discovered.
// Numbering is Corsair layout byte plus one.
#define LAYOUT_NONE       0 // Layout is irrelevant - use for mice or mousepads, never keyboards!
#define LAYOUT_ANSI       1 // North American layout with a long enter key.
#define LAYOUT_ISO        2 // European layout with a tall enter key.
#define LAYOUT_ABNT       3 // Brazilian layout with a short right shift.
#define LAYOUT_JIS        4 // Japanese layout with a shorter spacebar, toggle keys, and a tall enter key..
#define LAYOUT_DUBEOLSIK  5 // Korean layout with a shorter spacebar, toggle keys and a long enter key.
#define LAYOUT_UNKNOWN   64 // Keyboard type not yet implemented; file a bug!

// vtables for keyboards/mice (see command.h)
extern const union devcmd vtable_keyboard;
extern const union devcmd vtable_keyboard_wireless;
extern const union devcmd vtable_keyboard_legacy;
extern const union devcmd vtable_mouse;
extern const union devcmd vtable_mouse_wireless;
extern const union devcmd vtable_mousepad;
extern const union devcmd vtable_mouse_legacy;
extern const union devcmd vtable_bragi_mouse;
extern const union devcmd vtable_bragi_keyboard;
extern const union devcmd vtable_bragi_dongle;

typedef enum protocol_
{
    PROTO_NXP,
    PROTO_BRAGI,
} protocol_t;

typedef enum pollrate_ {
    POLLRATE_UNKNOWN = -1,
    POLLRATE_8MS,
    POLLRATE_4MS,
    POLLRATE_2MS,
    POLLRATE_1MS,
    POLLRATE_05MS,
    POLLRATE_025MS,
    POLLRATE_01MS,
    POLLRATE_COUNT,
} pollrate_t;


// Structure for tracking keyboard/mouse devices
#define KB_NAME_LEN         64
#define SERIAL_LEN          35
#define MSG_SIZE            64
#define BRAGI_JUMBO_SIZE    1024
#define MAX_MSG_SIZE        BRAGI_JUMBO_SIZE
#define IFACE_MAX           4
#define USB_EP_MAX          16
#define MAX_CHILDREN        8
#define PAIR_ID_SIZE        8

struct usbdevice_;
typedef struct usbdevice_ {
    // Function table (see command.h)
    union devcmd vtable;
    // I/O devices
    // Note that FDs have 1 added to them, because these are initialized to zero when the program starts but zero is technically a valid FD
    // So the actual value is (fd - 1)
#ifdef OS_LINUX
    // USB device
    struct udev_device* udev;
    int handle;
    // uinput handles
    int uinput_kb, uinput_mouse;
    // keyboard led thread
    pthread_t* ledthread;
    // Hack to get the led thread to shut down if stuck waiting for dmutex
    int shutdown_ledthread;
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
#ifndef OS_MAC_LEGACY
    io_connect_t event_mouse;
    vhid_kbinput kbinput_key;
    vhid_kbconsumerinput kbinput_consumer;
    vhid_kbavtopcaseinput kbinput_avtopcase;
    vhid_kbvendorinput kbinput_vendor;
    vhid_mouseinput mouseinput;
#endif
    // FIXME: Find out if input_loop can be removed from vhid builds
    // Key-repeat info
    CFRunLoopRef input_loop;
#ifdef OS_MAC_LEGACY
    CFRunLoopTimerRef krtimer;
    struct timespec keyrepeat;
    short lastkeypress;
#endif
    // Modifier/mouse button state
    pthread_t indicthread;
    IOOptionBits modifiers;
#ifdef OS_MAC_LEGACY
    uchar mousestate;
    char scroll_rate;
#endif
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
    char name[KB_NAME_LEN];
    // Device serial number
    char serial[SERIAL_LEN];
    // USB vendor and product IDs
    ushort vendor, product;
    // Firmware version
    uint32_t fwversion, bldversion, radioappversion, radiobldversion;
    // Poll rate and max
    pollrate_t pollrate, maxpollrate;
    // Physical device layout; LAYOUT_NONE if irrelevant, LAYOUT_UNKNOWN if unimplemented.
    uchar layout;
    // USB protocol delay (ms)
    char usbdelay;
    // Current input state
    usbinput input;
    // Indicator LED state
    uchar hw_ileds, hw_ileds_old, ileds;
    // Color dithering in use
    char dither;
    // Keymap that should be applied to this device
    key keymap[sizeof(keymap)];
    // Buffer used to store non-HID interrupt reads from the input thread.
    uchar interruptbuf[MAX_MSG_SIZE];
    // Endpoints the main input thread should listen to
    // Must always end with 0, and endpoints should be 0x80 | i
    uchar input_endpoints[USB_EP_MAX+1];
    // Protocol
    protocol_t protocol;
    // Poll thread
    pthread_t* pollthread;
    // Size in bytes of the primary output endpoint
    int out_ep_packet_size;
#ifndef NDEBUG
    // List of led scancodes that have been encountered since the last rgb update
    struct {
        ushort led;
        int index;
    } encounteredleds[N_KEYS_EXTENDED];
#endif
    // Parent device (for wireless dongles supporting multiple subdevices)
    struct usbdevice_* parent;
    // Children (if this is a parent)
    struct usbdevice_* children[MAX_CHILDREN];
    // Bragi child device id
    unsigned char bragi_child_id;
    // Battery information
    enum {
        BATT_STATUS_UNKNOWN,
        BATT_STATUS_CHARGING,
        BATT_STATUS_DISCHARGING,
        BATT_STATUS_CHARGED,
    } battery_status;
    unsigned char battery_level;
    enum {
        DEV_STATUS_DISCONNECTED,
        DEV_STATUS_CONNECTING,
        DEV_STATUS_CONNECTED,
        DEV_STATUS_DISCONNECTING,
    } status;
    uchar bragi_out_ep;
    uchar bragi_in_ep;
    uchar wl_pairing_id[PAIR_ID_SIZE];
    bool needs_fw_update;
} usbdevice;

#endif  // STRUCTURES_H

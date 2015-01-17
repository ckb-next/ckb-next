#ifndef STRUCTURES_H
#define STRUCTURES_H

#include "includes.h"
#include "keyboard.h"

// Key binding structures

// Action triggered when activating a macro
typedef struct {
    short scan;
    // down = 0 for keyup, down = 1 for keydown
    char down;
} macroaction;

// Key macro
typedef struct {
    macroaction* actions;
    int actioncount;
    uchar combo[N_KEYS / 8];
    char triggered;
} keymacro;

// Key bindings for a device/profile
typedef struct {
    // Base bindings
    int base[N_KEYS];
    // Macros
    keymacro* macros;
    int macrocount;
    int macrocap;
} keybind;
#define MACRO_MAX   1024

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

// End key bind structures

// Lighting structure for a mode
typedef struct {
    char r[N_KEYS / 2];
    char g[N_KEYS / 2];
    char b[N_KEYS / 2];
    char enabled;
} keylight;

// ID structure
typedef struct {
    char guid[16];
    char modified[4];
} usbid;

#define PR_NAME_LEN 16
#define MD_NAME_LEN 16

#define HWMODE_K70 1
#define HWMODE_K95 3
#define HWMODE_MAX 3

// Hardware profile structure
typedef struct {
    // RGB settings
    keylight light[HWMODE_MAX];
    // Mode/profile IDs
    usbid id[HWMODE_MAX + 1];
    // Mode/profile names
    ushort name[HWMODE_MAX + 1][MD_NAME_LEN];
} hwprofile;

// Native mode structure
typedef struct {
    keylight light;
    keybind bind;
    // Key notification settings
    uchar notify[OUTFIFO_MAX][N_KEYS / 8];
    // Name and UUID
    ushort name[MD_NAME_LEN];
    usbid id;
    // Indicators permanently off/on
    uchar ioff, ion;
    // Notify mask for indicator LEDs
    uchar inotify[OUTFIFO_MAX];
} usbmode;

// Native profile structure
#define SERIAL_LEN  33
typedef struct {
    // Modes
    usbmode* mode;
    int modecount;
    int modecap;
    // Currently-selected mode
    usbmode* currentmode;
    // Key map (locale)
    const key* keymap;
    // Device serial number
    char serial[SERIAL_LEN];
    // Name and UUID
    ushort name[PR_NAME_LEN];
    usbid id;
} usbprofile;
#define MODE_MAX    100

// Device features
#define FEAT_RGB        1   // RGB backlighting?
#define FEAT_POLLRATE   2   // Known poll rate?
#define FEAT_BIND       4   // Rebindable keys?
#define FEAT_NOTIFY     8   // Key notifications?
#define FEAT_FWVERSION  16  // Has firmware version?
#define FEAT_FWUPDATE   32  // Has firmware update?

// Standard feature sets
#define FEAT_COMMON     (FEAT_BIND | FEAT_NOTIFY | FEAT_FWVERSION)
#define FEAT_STD_RGB    (FEAT_COMMON | FEAT_RGB | FEAT_POLLRATE | FEAT_FWUPDATE)
#define FEAT_STD_NRGB   (FEAT_COMMON)

// Feature test (usbdevice* kb, int feat)
#define HAS_FEATURES(kb, feat)    ((kb)->features & (feat))

// Structure for tracking keyboard devices
#define NAME_LEN    33
#define QUEUE_LEN   40
#define MSG_SIZE    64
typedef struct {
    // I/O devices
#ifdef OS_LINUX
    struct udev_device* udev;
    struct usbdevfs_urb urb[3];
    uchar urbinput[32];
    int handle;
    int uinput;
    int event;
    pthread_t usbthread;
#endif
#ifdef OS_MAC
    IOReturn lastError;
    IOHIDDeviceRef handle;
    IOHIDDeviceRef handles[4];
    CGEventSourceRef event;
    CGEventFlags lflags, rflags, eventflags;
    long keypresstime;
    short lastkeypress;
#endif
    // A mutex used for USB controls. Needs to be locked before reading or writing the handle
    pthread_mutex_t mutex;
    // Similar, but used only for key input.
    pthread_mutex_t keymutex;
    // Keyboard settings
    usbprofile profile;
    // Hardware modes. Null if not read yet
    hwprofile* hw;
    // Last RGB data sent to the device
    keylight lastlight;
    // Command FIFO
    int infifo;
    // Notification FIFO
    int outfifo[OUTFIFO_MAX];
    // Interrupt transfers (keypresses)
    uchar kbinput[MSG_SIZE];
    uchar prevkbinput[N_KEYS / 8];
    // USB output queue
    uchar* queue[QUEUE_LEN];
    int queuecount;
    // Features (see F_ macros)
    int features;
    // Poll rate (ns), or -1 if unsupported
    int pollrate;
    // USB vendor and product IDs
    short vendor, product;
    // Firmware version
    ushort fwversion;
    // Indicator LED state
    uchar ileds;
    // Keyboard type (65, 70, or 95 for keyboards, -1 for root)
    char model;
    // Device name
    char name[NAME_LEN];
} usbdevice;

#endif

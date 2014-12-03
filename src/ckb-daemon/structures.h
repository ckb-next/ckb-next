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
    uchar notify[N_KEYS / 8];
    // Indicators permanently off/on
    uchar ioff, ion;
    // Name and UUID
    ushort name[MD_NAME_LEN];
    usbid id;
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

// Structure for tracking keyboard devices
#define NAME_LEN    33
#define QUEUE_LEN   40
#define MSG_SIZE    64
#define OUTFIFO_MAX 10
typedef struct {
    // I/O devices
#ifdef OS_LINUX
    struct udev_device* udev;
    struct usbdevfs_urb urb[3];
    char unusedinput[36];
    int handle;
    int uinput;
    int event;
    pthread_t usbthread;
#endif
#ifdef OS_MAC
    IOHIDDeviceRef handle;
    IOHIDDeviceRef handles[4];
    CGEventSourceRef event;
    CGEventFlags eflags;
    long keypresstime;
    short lastkeypress;
#endif
    // A mutex used for USB controls. Needs to be locked before reading or writing the handle
    pthread_mutex_t mutex;
    // Similar, but used only for key input.
    pthread_mutex_t keymutex;
    // Command FIFO
    int infifo;
    // Notification FIFO
    int outfifo[OUTFIFO_MAX];
    // Interrupt transfers (keypresses)
    uchar intinput[MSG_SIZE];
    uchar previntinput[N_KEYS / 8];
    // Last RGB data sent to the device
    keylight lastlight;
    // Indicator LED state
    uchar ileds;
    // USB output queue
    uchar* queue[QUEUE_LEN];
    int queuecount;
    // Keyboard settings
    usbprofile profile;
    // Hardware modes. Null if not read yet
    hwprofile* hw;
    // Firmware version
    short fwversion;
    // Keyboard type (70 or 95 for keyboards, -1 for root)
    char model;
    // Device name
    char name[NAME_LEN];
} usbdevice;

#endif

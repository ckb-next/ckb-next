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

// Lighting structure for a device/profile
typedef struct {
    char r[N_KEYS / 2];
    char g[N_KEYS / 2];
    char b[N_KEYS / 2];
    char enabled;
} keylight;

// ID structure
typedef struct {
    char guid[16];
    char modified[2];
} usbid;

// Mode structure
#define MD_NAME_LEN 16
typedef struct {
    keylight light;
    keybind bind;
    // Key notification settings
    uchar notify[N_KEYS / 8];
    // Indicators permanently off/on
    uchar ioff, ion;
    // Name and UUID
    unsigned short name[MD_NAME_LEN];
    usbid id;
} usbmode;

// Profile structure
#define PR_NAME_LEN 16
typedef struct {
    // Modes
    usbmode* mode;
    int modecount;
    int modecap;
    // Currently-selected mode
    usbmode* currentmode;
    // Name and UUID
    unsigned short name[PR_NAME_LEN];
    usbid id;
} usbprofile;
#define MODE_MAX    16

// Structure to store settings for a USB device, whether or not it's plugged in
#define SERIAL_LEN  33
typedef struct {
    usbprofile profile;
    char serial[SERIAL_LEN];
} usbsetting;

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
    char unusedinput[32];
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
    // Command FIFO
    int infifo;
    // Notification FIFO
    int outfifo[OUTFIFO_MAX];
    // Keyboard type (70 or 95 for keyboards, -1 for root)
    int model;
    // Interrupt transfers (keypresses)
    uchar intinput[MSG_SIZE];
    uchar previntinput[N_KEYS / 8];
    // Indicator LED state
    uchar ileds;
    // USB output queue
    uchar* queue[QUEUE_LEN];
    int queuecount;
    // Keyboard settings
    usbsetting setting;
    // Device name
    char name[NAME_LEN];
} usbdevice;

#endif

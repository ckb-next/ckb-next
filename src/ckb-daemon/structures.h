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
    unsigned char combo[N_KEYS / 8];
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
    unsigned short name[MD_NAME_LEN];
    usbid id;
} usbmode;

// Profile structure
#define PR_NAME_LEN 16
typedef struct {
    usbmode* mode;
    int modecount;
    int modecap;
    usbmode* currentmode;
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
typedef struct {
    // I/O devices
#ifdef OS_LINUX
    struct libusb_device_descriptor descriptor;
    libusb_device* dev;
    libusb_device_handle* handle;
    struct libusb_transfer* keyint;
    int uinput;
    int event;
#endif
#ifdef OS_MAC
    IOHIDDeviceRef handle;
    IOHIDDeviceRef handles[4];
    CGEventSourceRef event;
    CGEventFlags eflags;
    long keypresstime;
    short lastkeypress;
#endif
    pthread_mutex_t mutex;
    // Keyboard type (70 or 95 for keyboards, -1 for root)
    int model;
    // Interrupt transfers (keypresses)
    unsigned char intinput[MSG_SIZE];
    unsigned char previntinput[N_KEYS / 8];
    // Indicator LED state
    unsigned char ileds;
    // Command FIFO
    int fifo;
    // USB output queue
    unsigned char* queue[QUEUE_LEN];
    int queuecount;
    // Keyboard settings
    usbsetting setting;
    // Device name
    char name[NAME_LEN];
} usbdevice;

#endif

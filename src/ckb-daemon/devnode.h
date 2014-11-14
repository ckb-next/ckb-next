#ifndef IO_H
#define IO_H

#include "includes.h"
#include "usb.h"

// rm -rf
int rm_recursive(const char* path);

// Device path base ("/dev/input/ckb" or "/tmp/ckb")
const char *const devpath;

// Simple file permissions
#define S_READDIR (S_IRWXU | S_IRGRP | S_IROTH | S_IXGRP | S_IXOTH)
#define S_READ (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR)
#define S_READWRITE (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP | S_IWOTH)

// Update the list of connected devices.
void updateconnected();
// Create a dev path for the keyboard at index. Returns 0 on success.
int makedevpath(usbdevice* kb);
// Remove the dev path for the keyboard at index. Returns 0 on success.
int rmdevpath(usbdevice* kb);

// Creates a notification node for the specified keyboard.
int mknotifynode(usbdevice* kb, int notify);
// Removes a notification node for the specified keyboard.
int rmnotifynode(usbdevice* kb, int notify);

// Custom readline is needed for FIFOs. fopen()/getline() will die if the data is sent in too fast.
unsigned readlines(int fd, const char** input);

// Command operations
typedef enum {
    NONE,
    DEVICE,
    MODE,
    NAME,
    PROFILENAME,

    LAYOUT,
    BIND,
    UNBIND,
    REBIND,
    MACRO,

    FPS,
    RGB,
    IOFF,
    ION,
    IAUTO,

    NOTIFY,
    NOTIFYON,
    NOTIFYOFF,
    GET
} cmd;
typedef void (*cmdhandler)(usbmode*, const key*, int, const char*);

// Reads input from the command FIFO
void readcmd(usbdevice* kb, const char* line);

#endif

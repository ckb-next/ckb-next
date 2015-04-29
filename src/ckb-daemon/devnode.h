#ifndef IO_H
#define IO_H

#include "includes.h"
#include "usb.h"

// Device path base ("/dev/input/ckb" or "/var/run/ckb")
const char *const devpath;

// Group ID for the control nodes. -1 to give read/write access to everybody
extern long gid;

// Simple file permissions
#define S_READDIR (S_IRWXU | S_IRGRP | S_IROTH | S_IXGRP | S_IXOTH)
#define S_READ (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR)
#define S_READWRITE (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP | S_IWOTH)
#define S_CUSTOM (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define S_CUSTOM_R (S_IRUSR | S_IWUSR | S_IRGRP)

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

// Writes a keyboard's firmware version and poll rate to its device node.
void writefwnode(usbdevice* kb);

// Custom readline is needed for FIFOs. fopen()/getline() will die if the data is sent in too fast.
unsigned readlines(int fd, const char** input);

// Command operations
typedef enum {
    NONE,
    MODE,
    SWITCH,
    HWLOAD,
    HWSAVE,
    ERASE,
    ERASEPROFILE,
    NAME,
    PROFILENAME,
    ID,
    PROFILEID,

    ACTIVE,
    IDLE,

    LAYOUT,
    BIND,
    UNBIND,
    REBIND,
    MACRO,

    RGB,
    IOFF,
    ION,
    IAUTO,

    NOTIFY,
    INOTIFY,
    NOTIFYON,
    NOTIFYOFF,
    GET,

    FWUPDATE
} cmd;
typedef void (*cmdhandler)(usbdevice*, usbmode*, int, int, const char*);

// Reads input from the command FIFO
void readcmd(usbdevice* kb, const char* line);

#endif

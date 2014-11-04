#ifndef IO_H
#define IO_H

#include "includes.h"
#include "usb.h"

// rm -rf
int rm_recursive(const char* path);

// Device path base ("/dev/input/ckb")
const char *const devpath;

// Simple file permissions
#define S_READDIR (S_IRWXU | S_IRGRP | S_IROTH | S_IXGRP | S_IXOTH)
#define S_READ (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR)
#define S_READWRITE (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP | S_IWOTH)

// Update the list of connected devices.
void updateconnected();
// Create a dev path for the keyboard at index. Returns 0 on success.
int makedevpath(int index);

// Custom readline is needed for FIFOs. fopen()/getline() will die if the data is sent in too fast.
int readlines(int fd, char*** lines);

// Command operations
typedef enum {
    NONE,
    DEVICE,
    MODE,
    NAME,
    PROFILENAME,

    BIND,
    UNBIND,
    REBIND,
    MACRO,

    RGB,
} cmd;
typedef void (*cmdhandler)(usbmode*, int, const char*);

// Reads input from the command FIFO
void readcmd(usbdevice* kb, const char* line);

#endif

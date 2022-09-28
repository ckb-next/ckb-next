#ifndef DEVNODE_H
#define DEVNODE_H

#include "includes.h"
#include "usb.h"

/// Device path base ("/dev/input/ckb" or "/var/run/ckb")
extern const char devpath[];
pid_t is_pid_running();

/// Group ID for the control nodes. -1 to give read/write access to everybody
extern long gid;

// Simple file permissions
#define S_READDIR (S_IRWXU | S_IRGRP | S_IROTH | S_IXGRP | S_IXOTH)
#define S_READ (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR)
#define S_READWRITE (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP | S_IWOTH)
#define S_CUSTOM (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define S_CUSTOM_R (S_IRUSR | S_IWUSR | S_IRGRP)

/// Update the list of connected devices.
void updateconnected(usbdevice* kb);

/// Create a dev path for the keyboard at index. Returns 0 on success.
int mkdevpath(usbdevice* kb);

/// Remove the dev path for the keyboard at index. Returns 0 on success.
int rmdevpath(usbdevice* kb);

/// Creates a notification node for the specified keyboard.
int mknotifynode(usbdevice* kb, int notify);

/// Removes a notification node for the specified keyboard.
int rmnotifynode(usbdevice* kb, int notify);

/// Writes a keyboard's firmware version and poll rate to its device node.
int mkfwnode(usbdevice* kb);

/// Custom readline is needed for FIFOs. fopen()/getline() will die if the data is sent in too fast.
#define MAX_BUFFER (1024 * 128)
typedef struct {
    char buf[MAX_BUFFER];
    size_t leftover_bytes;
    char* next_start;
} readlines_ctx;
int readline_fifo(int fd, readlines_ctx* ctx);

#endif  // DEVNODE_H

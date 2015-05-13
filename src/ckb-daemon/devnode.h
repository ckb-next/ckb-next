#ifndef DEVNODE_H
#define DEVNODE_H

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
int mkdevpath(usbdevice* kb);
// Remove the dev path for the keyboard at index. Returns 0 on success.
int rmdevpath(usbdevice* kb);

// Creates a notification node for the specified keyboard.
int mknotifynode(usbdevice* kb, int notify);
// Removes a notification node for the specified keyboard.
int rmnotifynode(usbdevice* kb, int notify);

// Writes a keyboard's firmware version and poll rate to its device node.
int mkfwnode(usbdevice* kb);

// Custom readline is needed for FIFOs. fopen()/getline() will die if the data is sent in too fast.
typedef struct _readlines_ctx* readlines_ctx;
void readlines_ctx_init(readlines_ctx* ctx);
void readlines_ctx_free(readlines_ctx ctx);
unsigned readlines(int fd, readlines_ctx ctx, const char** input);

#endif  // DEVNODE_H

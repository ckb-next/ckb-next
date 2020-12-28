#ifndef DEVNODE_H
#define DEVNODE_H

#include "includes.h"
#include "usb.h"

/// Device path base ("/dev/input/ckb" or "/var/run/ckb")
extern const char *const devpath;

/// Group ID for the control nodes. -1 to give read/write access to everybody
extern long gid;

//#ifndef OS_WINDOWS
// Simple file permissions
#define S_READDIR (S_IRWXU | S_IRGRP | S_IROTH | S_IXGRP | S_IXOTH)
#define S_READ (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR)
#define S_READWRITE (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP | S_IWOTH)
#define S_CUSTOM (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define S_CUSTOM_R (S_IRUSR | S_IWUSR | S_IRGRP)
//#endif

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
typedef struct _readlines_ctx* readlines_ctx;
void readlines_ctx_init(readlines_ctx* ctx);
void readlines_ctx_free(readlines_ctx ctx);

/// Platform-specific function implementations found in devnode_*.c
// FIXME: maybe move to a new file
void check_chown(const char* pathname, uid_t owner, long group);
int create_dev_cmd_fifo(const char* const path, usbdevice* kb);
int _mknotifynode(usbdevice* kb, int notify);
int _rmnotifynode(usbdevice* kb, int notify);
int rm_recursive(const char* path);
int mkdir_p(const char* pathname, const mode_t mode);

#ifdef OS_WINDOWS
int do_pipe_read(HANDLE fd, readlines_ctx ctx, char* buffer, const int offset, const int buffersize);
void check_fchown(HANDLE fd, uid_t owner, long group);
#else
int do_pipe_read(const int fd, readlines_ctx ctx, char* buffer, const int offset, const int buffersize);
void check_fchown(int fd, uid_t owner, long group);
#endif

unsigned readlines(usbdevice* kb, readlines_ctx ctx, const char** input);

#define S_GID_READ  (gid >= 0 ? S_CUSTOM_R : S_READ)
#define MAX_BUFFER (1024 * 1024 - 1)
struct _readlines_ctx {
    char* buffer;
    int buffersize;
    int leftover, leftoverlen;
};

#endif  // DEVNODE_H

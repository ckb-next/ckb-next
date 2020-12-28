#include "device.h"
#include "devnode.h"
#include "firmware.h"
#include "input.h"
#include "led.h"
#include "notify.h"
#include "profile.h"
#include "devnode_common.h"
#include <limits.h>
#include <ckbnextconfig.h>

// OSX doesn't like putting FIFOs in /dev for some reason
#ifndef OS_MAC
const char *const devpath = "/dev/input/ckb";
#else
const char *const devpath = "/var/run/ckb";
#endif

long gid = -1;

int do_pipe_read(const int fd, readlines_ctx ctx, char* buffer, const int offset, const int buffersize){
    ssize_t length = read(fd, buffer + offset, buffersize - offset);
    if(length <= 0)
        return 0;
    if(length > INT_MAX)
    {
        ckb_err("Pipe read length overflow %lu", length);
        length = INT_MAX;
    }
//    ckb_info("READ %d %.*s %d %d %ld", fd, (int)length-1, buffer, offset, buffersize, length);
    return (int)length;
}

int create_dev_cmd_fifo(const char* const path, usbdevice* kb){
    char inpath[strlen(path) + 5];
    snprintf(inpath, sizeof(inpath), "%s/cmd", path);
    if(mkfifo(inpath, gid >= 0 ? S_CUSTOM : S_READWRITE) != 0
            // Open the node in RDWR mode because RDONLY will lock the thread
            // Add one to the FD because 0 is a valid descriptor, but ckb uses 0 for uninitialized devices
            || (kb->infifo = open(inpath, O_RDWR) + 1) == 0) {
        ckb_err("Unable to create %s: %s", inpath, strerror(errno));
        return 1;
    }
    return 0;
}

void check_chown(const char *pathname, uid_t owner, long group){
    if (group >= 0) {
        if (chown(pathname, owner, group) < 0) {
            ckb_warn("Chown call failed %s: %s", pathname, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
}

void check_fchown(int fd, uid_t owner, long group){
    fd--;
    if (group >= 0) {
        if (fchown(fd, owner, group) < 0) {
            ckb_warn("FChown call failed: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
}

int _mknotifynode(usbdevice* kb, int notify){
    if(notify < 0 || notify >= OUTFIFO_MAX)
        return -1;
    if(kb->outfifo[notify] != 0)
        return 0;
    // Create the notification node
    char index = (INDEX_OF(kb, keyboard) % 10) + '0';
    char notify_char = (notify % 10) + '0';
    char outpath[strlen(devpath) + 10 * sizeof(char)];
    snprintf(outpath, sizeof(outpath), "%s%c/notify%c", devpath, index, notify_char);
    if(mkfifo(outpath, S_GID_READ) != 0 || (kb->outfifo[notify] = open(outpath, O_RDWR | O_NONBLOCK) + 1) == 0){
        // Add one to the FD because 0 is a valid descriptor, but ckb uses 0 for uninitialized devices
        ckb_warn("Unable to create %s: %s", outpath, strerror(errno));
        kb->outfifo[notify] = 0;
        remove(outpath);
        return -1;
    }
    check_fchown(kb->outfifo[notify], 0, gid);
    return 0;
}

int _rmnotifynode(usbdevice* kb, int notify){
    if(notify < 0 || notify >= OUTFIFO_MAX || !kb->outfifo[notify])
        return -1;
    char index = (INDEX_OF(kb, keyboard) % 10) + '0';
    char notify_char = (notify % 10) + '0';
    char outpath[strlen(devpath) + 10 * sizeof(char)];
    snprintf(outpath, sizeof(outpath), "%s%c/notify%c", devpath, index, notify_char);
    // Close FIFO
    close(kb->outfifo[notify] - 1);
    kb->outfifo[notify] = 0;
    // Delete node
    int res = remove(outpath);
    return res;
}

int rmdevpath(usbdevice* kb){
    euid_guard_start;
    int index = INDEX_OF(kb, keyboard);
    if(kb->infifo != 0){
        int fd = kb->infifo - 1;
#ifdef OS_MAC
        fcntl(fd, F_SETFL, O_RDWR | O_NONBLOCK); // hack to prevent the following hack from blocking if the GUI was running
#endif
        if (write(fd, "\n", 1) < 0)
            ckb_warn("Unable to write to filedescriptor %d: %s", fd, strerror(errno));
        close(fd);
        kb->infifo = 0;
    }
    for(int i = 0; i < OUTFIFO_MAX; i++)
        _rmnotifynode(kb, i);
    char path[strlen(devpath) + 2];
    snprintf(path, sizeof(path), "%s%d", devpath, index);
    if(rm_recursive(path) != 0 && errno != ENOENT){
        ckb_warn("Unable to delete %s: %s", path, strerror(errno));
        euid_guard_stop;
        return -1;
    }
    ckb_info("Removed device path %s", path);
    euid_guard_stop;
    return 0;
}

int mkdir_p(const char* pathname, const mode_t mode){
    return mkdir(pathname, mode);
}

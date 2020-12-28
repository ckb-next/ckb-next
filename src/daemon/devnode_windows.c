#include "device.h"
#include "devnode.h"
#include "firmware.h"
#include "input.h"
#include "led.h"
#include "notify.h"
#include "profile.h"
#include <ckbnextconfig.h>
#include <windows.h>
#include "devnode_common.h"
#include <limits.h>
#define BUFSIZE 512

const char *const devpath = "C:\\ProgramData\\ckb-next\\ckb";
const char *const pipepath = "\\\\.\\pipe\\ckb-next\\ckb";


long gid = -1;

int do_pipe_read(HANDLE fd, readlines_ctx ctx, char* buffer, const int offset, const int buffersize){
    DWORD length = 0;
    BOOL success = ReadFile(fd, buffer + offset, buffersize - offset, &length, NULL);
    if(!success)
        return -1;
    if(!length)
        return 0;
    if(length > INT_MAX)
    {
        ckb_err("Pipe read length overflow %lu", length);
        length = INT_MAX;
    }
    return (int)length;
}

int create_dev_cmd_fifo(const char* const path, usbdevice* kb){
    (void)path;
    char inpath[strlen(pipepath) + 6];
    snprintf(inpath, sizeof(inpath), "%s%d\\cmd", pipepath, INDEX_OF(kb, keyboard));
    kb->infifo = CreateNamedPipe(TEXT(inpath), PIPE_ACCESS_INBOUND, PIPE_WAIT, 1, 1024, 1024, 120 * 1000, NULL);
    if(kb->infifo == INVALID_HANDLE_VALUE){
        ckb_err("Unable to create %s: %ld\n", inpath, GetLastError());
        return 1;
    }
    return 0;
}

void check_chown(const char *pathname, uid_t owner, long group){

}

void check_fchown(HANDLE fd, uid_t owner, long group){

}

int _mknotifynode(usbdevice* kb, int notify){
    if(notify < 0 || notify >= OUTFIFO_MAX)
        return -1;
    if(kb->outfifo[notify] != 0)
        return 0;
    // Create the notification node
    char index = (INDEX_OF(kb, keyboard) % 10) + '0';
    char notify_char = (notify % 10) + '0';
    char outpath[strlen(pipepath) + 10 * sizeof(char)];
    snprintf(outpath, sizeof(outpath), "%s%c\\notify%c", pipepath, index, notify_char);
    kb->outfifo[notify] = CreateNamedPipe(TEXT(outpath), PIPE_ACCESS_OUTBOUND , PIPE_WAIT, 1, 1024, 1024, 120 * 1000, NULL);
    if(kb->outfifo[notify] == INVALID_HANDLE_VALUE){
        ckb_warn("Unable to create %s: %ld\n", outpath, GetLastError());
        kb->outfifo[notify] = 0;
        return -1;
    }
    check_fchown(kb->outfifo[notify], 0, gid);
    return 0;
}

int _rmnotifynode(usbdevice* kb, int notify){
    if(notify < 0 || notify >= OUTFIFO_MAX || !kb->outfifo[notify])
        return -1;
    CloseHandle(kb->outfifo[notify]);
    kb->outfifo[notify] = 0;
    return 0;
}

int rmdevpath(usbdevice* kb){
    euid_guard_start;
    int index = INDEX_OF(kb, keyboard);
    if(kb->infifo != 0){
        CloseHandle(kb->infifo);
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
    return mkdir(pathname);
}

// FIXME: Dedup this. It's in extra_mac.c
void *memrchr(const void *s, int c, size_t n){
    const char* cs = s;
    for(size_t i = n; i > 0; i--){
        if(cs[i - 1] == c)
            return (void*)(cs + i - 1);
    }
    return 0;
}


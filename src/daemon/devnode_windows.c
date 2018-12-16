#include "device.h"
#include "devnode.h"
#include "firmware.h"
#include "input.h"
#include "led.h"
#include "notify.h"
#include "profile.h"
#include <ckbnextconfig.h>
#include <windows.h>
#define BUFSIZE 512

const char *const devpath = "C:\\ProgramData\\ckb-next\\ckb";
const char *const pipepath = "\\\\.\\pipe\\ckb-next\\ckb";


long gid = -1;
#define S_GID_READ  (gid >= 0 ? S_CUSTOM_R : S_READ)

int rm_recursive(const char* path){
    DIR* dir = opendir(path);
    if(!dir)
        return remove(path);
    struct dirent* file;
    while((file = readdir(dir)))
    {
        if(!strcmp(file->d_name, ".") || !strcmp(file->d_name, ".."))
            continue;
        char path2[FILENAME_MAX];
        snprintf(path2, FILENAME_MAX, "%s/%s", path, file->d_name);
        int stat = rm_recursive(path2);
        if(stat != 0)
            return stat;
    }
    closedir(dir);
    return remove("C:\\ProgramData\\ckb-next\\ckb0\\");
}

void check_chown(const char *pathname, uid_t owner, long group){
    if (group >= 0) {
        if (chown(pathname, owner, group) < 0) {
            ckb_warn("Chown call failed %s: %s\n", pathname, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
}

void check_fchown(int fd, uid_t owner, long group){
    if (group >= 0) {
        if (fchown(fd, owner, group) < 0) {
            ckb_warn("FChown call failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
}

void check_chmod(const char *pathname, mode_t mode){
    if(chmod(pathname, mode) < 0) {
        ckb_warn("Chmod call failed %s: %s\n", pathname, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void updateconnected(){
    pthread_mutex_lock(devmutex);
    char cpath[strlen(devpath) + 12];
    snprintf(cpath, sizeof(cpath), "%s0/connected", devpath);
    FILE* cfile = fopen(cpath, "w");
    if(!cfile){
        ckb_warn("Unable to update %s: %s\n", cpath, strerror(errno));
        pthread_mutex_unlock(devmutex);
        return;
    }
    int written = 0;
    for(int i = 1; i < DEV_MAX; i++){
        if(IS_CONNECTED(keyboard + i)){
            written = 1;
            fprintf(cfile, "%s%d %s %s\n", devpath, i, keyboard[i].serial, keyboard[i].name);
        }
    }
    if(!written)
        fputc('\n', cfile);
    fclose(cfile);

    check_chmod(cpath, S_GID_READ);
    check_chown(cpath, 0, gid);

    pthread_mutex_unlock(devmutex);
}

int mknotifynode(usbdevice* kb, int notify){
    if(notify < 0 || notify >= OUTFIFO_MAX)
        return -1;
    if(kb->outfifo[notify] != 0)
        return 0;
    // Create the notification node
    char index = (INDEX_OF(kb, keyboard) % 10) + '0';
    char notify_char = (notify % 10) + '0';
    char outpath[strlen(pipepath) + 10 * sizeof(char)];
    snprintf(outpath, sizeof(outpath), "%s%c\\notify%c", pipepath, index, notify_char);
    kb->outfifo[notify] = CreateNamedPipe(TEXT(outpath), PIPE_ACCESS_INBOUND | PIPE_ACCESS_OUTBOUND , PIPE_WAIT, 1, 1024, 1024, 120 * 1000, NULL);
    if(kb->outfifo[notify] == INVALID_HANDLE_VALUE){
        ckb_warn("Unable to create %s: %d\n", outpath, GetLastError());
        kb->outfifo[notify] = 0;
        return -1;
    }
    return 0;
}

int rmnotifynode(usbdevice* kb, int notify){
    if(notify < 0 || notify >= OUTFIFO_MAX || !kb->outfifo[notify])
        return -1;
    CloseHandle(kb->outfifo[notify]);
    kb->outfifo[notify] = 0;
    return 0;
}

static void printnode(const char* path, const char* str){
    FILE* file = fopen(path, "w");
    if(file){
        fputs(str, file);
        fputc('\n', file);
        fclose(file);
        check_chmod(path, S_GID_READ);
        check_chown(path, 0, gid);
    } else {
        ckb_warn("Unable to create %s: %s\n", path, strerror(errno));
        remove(path);
    }
}

static char* layoutstr(char layout){
    if(layout == LAYOUT_NONE)
        return "";
    if(layout == LAYOUT_ANSI)
        return "ansi";
    if(layout == LAYOUT_ISO)
        return "iso";
    if(layout == LAYOUT_ABNT)
        return "abnt";
    if(layout == LAYOUT_JIS)
        return "jis";
    if(layout == LAYOUT_DUBEOLSIK)
        return "dubeolsik";
    return "unknown";
}

static int _mkdevpath(usbdevice* kb){
    int index = INDEX_OF(kb, keyboard);
    // Create the control path
    char path[strlen(devpath) + 2];
    snprintf(path, sizeof(path), "%s%d", devpath, index);
    /*if(rm_recursive(path) != 0 && errno != ENOENT){
        ckb_err("Unable to delete %s: %s\n", path, strerror(errno));
        return -1;
    }*/

    if(mkdir(path) != 0 && errno != EEXIST){
        ckb_err("Unable to create %s: %s\n", path, strerror(errno));
        rm_recursive(path);
        return -1;
    }

    check_chown(path, 0, gid);

    if(kb == keyboard + 0){
        // Root keyboard: write a list of devices
        updateconnected();
        // Write version number
        char vpath[sizeof(path) + 8];
        snprintf(vpath, sizeof(vpath), "%s/version", path);
        FILE* vfile = fopen(vpath, "w");
        if(vfile){
            fprintf(vfile, "%s\n", CKB_NEXT_VERSION_STR);
            fclose(vfile);
            check_chmod(vpath, S_GID_READ);
            check_chown(vpath, 0, gid);
        } else {
            ckb_warn("Unable to create %s: %s\n", vpath, strerror(errno));
            remove(vpath);
        }
        // Write PID
        char ppath[sizeof(path) + 4];
        snprintf(ppath, sizeof(ppath), "%s/pid", path);
        FILE* pfile = fopen(ppath, "w");
        if(pfile){
            fprintf(pfile, "%u\n", getpid());
            fclose(pfile);
            check_chmod(ppath, S_READ);
            check_chown(vpath, 0, gid);
        } else {
            ckb_warn("Unable to create %s: %s\n", ppath, strerror(errno));
            remove(ppath);
        }
    } else {
        // Create command FIFO
        char inpath[strlen(pipepath) + 6];
        snprintf(inpath, sizeof(inpath), "%s%d\\cmd", pipepath, index);
        kb->infifo = CreateNamedPipe(TEXT(inpath), PIPE_ACCESS_INBOUND | PIPE_ACCESS_OUTBOUND , PIPE_WAIT, 1, 1024, 1024, 120 * 1000, NULL);
        if(kb->infifo == INVALID_HANDLE_VALUE){
            ckb_err("Unable to create %s: %d\n", inpath, GetLastError());
            rm_recursive(path);
            kb->infifo = 0;
            return -1;
        }

        // Create notification FIFO
        mknotifynode(kb, 0);

        // Write the model and serial to files
        char mpath[sizeof(path) + 6], spath[sizeof(path) + 7], ipath[sizeof(path) + 10], lpath[sizeof(path) + 7];
        snprintf(mpath, sizeof(mpath), "%s/model", path);
        snprintf(spath, sizeof(spath), "%s/serial", path);
        snprintf(ipath, sizeof(ipath), "%s/productid", path);
        snprintf(lpath, sizeof(lpath), "%s/layout", path);

        char productid[5];
        snprintf(productid, 5, "%04x", kb->product);

        printnode(mpath, kb->name);
        printnode(spath, kb->serial);
        printnode(ipath, productid);
        printnode(lpath, layoutstr(kb->layout));

        // Write the keyboard's features
        char fpath[sizeof(path) + 9];
        snprintf(fpath, sizeof(fpath), "%s/features", path);
        FILE* ffile = fopen(fpath, "w");
        if(ffile){
            fprintf(ffile, "%s %s", vendor_str(kb->vendor), product_str(kb->product));
            if(HAS_FEATURES(kb, FEAT_MONOCHROME))
                fputs(" monochrome", ffile);
            if(HAS_FEATURES(kb, FEAT_RGB))
                fputs(" rgb", ffile);
            if(HAS_FEATURES(kb, FEAT_POLLRATE))
                fputs(" pollrate", ffile);
            if(HAS_FEATURES(kb, FEAT_ADJRATE))
                fputs(" adjrate", ffile);
            if(HAS_FEATURES(kb, FEAT_BIND))
                fputs(" bind", ffile);
            if(HAS_FEATURES(kb, FEAT_NOTIFY))
                fputs(" notify", ffile);
            if(HAS_FEATURES(kb, FEAT_FWVERSION))
                fputs(" fwversion", ffile);
            if(HAS_FEATURES(kb, FEAT_FWUPDATE))
                fputs(" fwupdate", ffile);
            if(HAS_FEATURES(kb, FEAT_HWLOAD))
                fputs(" hwload", ffile);
            fputc('\n', ffile);
            fclose(ffile);
            check_chmod(fpath, S_GID_READ);
            check_chown(fpath, 0, gid);
        } else {
            ckb_warn("Unable to create %s: %s\n", fpath, strerror(errno));
            remove(fpath);
        }
        // Write firmware version and poll rate
        mkfwnode(kb);
    }
    return 0;
}

int mkdevpath(usbdevice* kb){
    euid_guard_start;
    int res = _mkdevpath(kb);
    euid_guard_stop;
    return res;
}

int rmdevpath(usbdevice* kb){
    int index = INDEX_OF(kb, keyboard);
    if(kb->infifo != 0){
        CloseHandle(kb->infifo);
        kb->infifo = 0;
    }
    for(int i = 0; i < OUTFIFO_MAX; i++)
        rmnotifynode(kb, i);
    char path[strlen(devpath) + 2];
    snprintf(path, sizeof(path), "%s%d", devpath, index);
    if(rm_recursive(path) != 0 && errno != ENOENT){
        ckb_warn("Unable to delete %s: %s\n", path, strerror(errno));
        euid_guard_stop;
        return -1;
    }
    ckb_info("Removed device path %s\n", path);
    return 0;
}

int mkfwnode(usbdevice* kb){
    int index = INDEX_OF(kb, keyboard);
    char fwpath[strlen(devpath) + 12];
    snprintf(fwpath, sizeof(fwpath), "%s%d/fwversion", devpath, index);
    FILE* fwfile = fopen(fwpath, "w");
    if(fwfile){
        fprintf(fwfile, "%04x", kb->fwversion);
        fputc('\n', fwfile);
        fclose(fwfile);
        check_chmod(fwpath, S_GID_READ);
        check_chown(fwpath, 0, gid);
    } else {
        ckb_warn("Unable to create %s: %s\n", fwpath, strerror(errno));
        remove(fwpath);
        return -1;
    }
    char ppath[strlen(devpath) + 11];
    snprintf(ppath, sizeof(ppath), "%s%d/pollrate", devpath, index);
    FILE* pfile = fopen(ppath, "w");
    if(pfile){
        fprintf(pfile, "%d ms", kb->pollrate);
        fputc('\n', pfile);
        fclose(pfile);
        check_chmod(ppath, S_GID_READ);
        check_chown(ppath, 0, gid);
    } else {
        ckb_warn("Unable to create %s: %s\n", fwpath, strerror(errno));
        remove(ppath);
        return -2;
    }
    return 0;
}

#define MAX_BUFFER (1024 * 1024 - 1)
struct _readlines_ctx {
    char* buffer;
    int buffersize;
    int leftover, leftoverlen;
};

void readlines_ctx_init(readlines_ctx* ctx){
    // Allocate buffers to store data
    *ctx = calloc(1, sizeof(struct _readlines_ctx));
    int buffersize = (*ctx)->buffersize = 4095;
    (*ctx)->buffer = malloc(buffersize + 1);
}

void readlines_ctx_free(readlines_ctx ctx){
    free(ctx->buffer);
    free(ctx);
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

unsigned readlines(HANDLE fd, readlines_ctx ctx, const char** input){
    // Move any data left over from a previous read to the start of the buffer
    char* buffer = ctx->buffer;
    int buffersize = ctx->buffersize;
    int leftover = ctx->leftover, leftoverlen = ctx->leftoverlen;
    memcpy(buffer, buffer + leftover, leftoverlen);
    // Read data from the pipe
    DWORD length = 0;
    BOOL success = ReadFile(fd, buffer + leftoverlen, buffersize - leftoverlen, &length, NULL);
    length = (success ? length : 0) + leftoverlen;
    leftover = ctx->leftover = leftoverlen = ctx->leftoverlen = 0;
    if(!success){
        *input = 0;
        return -1;
    }
    if(!length){
        *input = 0;
        return 0;
    }
    // Continue buffering until all available input is read or there's no room left
    while(length == buffersize){
        if(buffersize == MAX_BUFFER)
            break;
        int oldsize = buffersize;
        buffersize += 4096;
        ctx->buffersize = buffersize;
        buffer = ctx->buffer = realloc(buffer, buffersize + 1);
        DWORD length2 = 0;
        if(!ReadFile(fd, buffer + oldsize, buffersize - oldsize, &length2, NULL))
            break;
        length += length2;
    }
    buffer[length] = 0;
    //printf("In: %s\n", buffer);
    // Input should be issued one line at a time and should end with a newline.
    char* lastline = memrchr(buffer, '\n', length);
    if(lastline == buffer + length - 1){
        // If the buffer ends in a newline, process the whole string
        *input = buffer;
        return length;
    } else if(lastline){
        // Otherwise, chop off the last line but process everything else
        *lastline = 0;
        leftover = ctx->leftover = lastline + 1 - buffer;
        leftoverlen = ctx->leftoverlen = length - leftover;
        *input = buffer;
        return leftover - 1;
    } else {
        // If a newline wasn't found at all, process the whole buffer next time
        *input = 0;
        if(length == MAX_BUFFER){
            // Unless the buffer is completely full, in which case discard it
            ckb_warn("Too much input (1MB). Dropping.\n");
            return 0;
        }
        leftoverlen = ctx->leftoverlen = length;
        return 0;
    }
}

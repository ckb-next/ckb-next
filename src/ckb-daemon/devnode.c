#include "device.h"
#include "devnode.h"
#include "firmware.h"
#include "input.h"
#include "led.h"
#include "notify.h"
#include "profile.h"

// OSX doesn't like putting FIFOs in /dev for some reason
#ifndef OS_MAC
const char *const devpath = "/dev/input/ckb";
#else
const char *const devpath = "/var/run/ckb";
#endif

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
    return remove(path);
}

void _updateconnected(){
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
    chmod(cpath, S_GID_READ);
    if(gid >= 0)
        chown(cpath, 0, gid);
    pthread_mutex_unlock(devmutex);
}

void updateconnected(){
    euid_guard_start;
    _updateconnected();
    euid_guard_stop;
}

int _mknotifynode(usbdevice* kb, int notify){
    if(notify < 0 || notify >= OUTFIFO_MAX)
        return -1;
    if(kb->outfifo[notify] != 0)
        return 0;
    // Create the notification node
    int index = INDEX_OF(kb, keyboard);
    char outpath[strlen(devpath) + 10];
    snprintf(outpath, sizeof(outpath), "%s%d/notify%d", devpath, index, notify);
    if(mkfifo(outpath, S_GID_READ) != 0 || (kb->outfifo[notify] = open(outpath, O_RDWR | O_NONBLOCK) + 1) == 0){
        // Add one to the FD because 0 is a valid descriptor, but ckb uses 0 for uninitialized devices
        ckb_warn("Unable to create %s: %s\n", outpath, strerror(errno));
        kb->outfifo[notify] = 0;
        remove(outpath);
        return -1;
    }
    if(gid >= 0)
        fchown(kb->outfifo[notify] - 1, 0, gid);
    return 0;
}

int mknotifynode(usbdevice* kb, int notify){
    euid_guard_start;
    int res = _mknotifynode(kb, notify);
    euid_guard_stop;
    return res;
}

int _rmnotifynode(usbdevice* kb, int notify){
    if(notify < 0 || notify >= OUTFIFO_MAX || !kb->outfifo[notify])
        return -1;
    int index = INDEX_OF(kb, keyboard);
    char outpath[strlen(devpath) + 10];
    snprintf(outpath, sizeof(outpath), "%s%d/notify%d", devpath, index, notify);
    // Close FIFO
    close(kb->outfifo[notify] - 1);
    kb->outfifo[notify] = 0;
    // Delete node
    int res = remove(outpath);
    return res;
}

int rmnotifynode(usbdevice* kb, int notify){
    euid_guard_start;
    int res = _rmnotifynode(kb, notify);
    euid_guard_stop;
    return res;
}

static int _mkdevpath(usbdevice* kb){
    int index = INDEX_OF(kb, keyboard);
    // Create the control path
    char path[strlen(devpath) + 2];
    snprintf(path, sizeof(path), "%s%d", devpath, index);
    if(rm_recursive(path) != 0 && errno != ENOENT){
        ckb_err("Unable to delete %s: %s\n", path, strerror(errno));
        return -1;
    }
    if(mkdir(path, S_READDIR) != 0){
        ckb_err("Unable to create %s: %s\n", path, strerror(errno));
        rm_recursive(path);
        return -1;
    }
    if(gid >= 0)
        chown(path, 0, gid);

    if(kb == keyboard + 0){
        // Root keyboard: write a list of devices
        _updateconnected();
        // Write version number
        char vpath[sizeof(path) + 8];
        snprintf(vpath, sizeof(vpath), "%s/version", path);
        FILE* vfile = fopen(vpath, "w");
        if(vfile){
            fprintf(vfile, "%s\n", CKB_VERSION_STR);
            fclose(vfile);
            chmod(vpath, S_GID_READ);
            if(gid >= 0)
                chown(vpath, 0, gid);
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
            chmod(ppath, S_READ);
            if(gid >= 0)
                chown(vpath, 0, gid);
        } else {
            ckb_warn("Unable to create %s: %s\n", ppath, strerror(errno));
            remove(ppath);
        }
    } else {
        // Create command FIFO
        char inpath[sizeof(path) + 4];
        snprintf(inpath, sizeof(inpath), "%s/cmd", path);
        if(mkfifo(inpath, gid >= 0 ? S_CUSTOM : S_READWRITE) != 0
                // Open the node in RDWR mode because RDONLY will lock the thread
                || (kb->infifo = open(inpath, O_RDWR) + 1) == 0){
            // Add one to the FD because 0 is a valid descriptor, but ckb uses 0 for uninitialized devices
            ckb_err("Unable to create %s: %s\n", inpath, strerror(errno));
            rm_recursive(path);
            kb->infifo = 0;
            return -1;
        }
        if(gid >= 0)
            fchown(kb->infifo - 1, 0, gid);

        // Create notification FIFO
        _mknotifynode(kb, 0);

        // Write the model and serial to files
        char mpath[sizeof(path) + 6], spath[sizeof(path) + 7];
        snprintf(mpath, sizeof(mpath), "%s/model", path);
        snprintf(spath, sizeof(spath), "%s/serial", path);
        FILE* mfile = fopen(mpath, "w");
        if(mfile){
            fputs(kb->name, mfile);
            fputc('\n', mfile);
            fclose(mfile);
            chmod(mpath, S_GID_READ);
            if(gid >= 0)
                chown(mpath, 0, gid);
        } else {
            ckb_warn("Unable to create %s: %s\n", mpath, strerror(errno));
            remove(mpath);
        }
        FILE* sfile = fopen(spath, "w");
        if(sfile){
            fputs(kb->serial, sfile);
            fputc('\n', sfile);
            fclose(sfile);
            chmod(spath, S_GID_READ);
            if(gid >= 0)
                chown(spath, 0, gid);
        } else {
            ckb_warn("Unable to create %s: %s\n", spath, strerror(errno));
            remove(spath);
        }
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
            fputc('\n', ffile);
            fclose(ffile);
            chmod(fpath, S_GID_READ);
            if(gid >= 0)
                chown(fpath, 0, gid);
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
    euid_guard_start;
    int index = INDEX_OF(kb, keyboard);
    if(kb->infifo != 0){
#ifdef OS_LINUX
        write(kb->infifo - 1, "\n", 1); // hack to prevent the FIFO thread from perma-blocking
#endif
        close(kb->infifo - 1);
        kb->infifo = 0;
    }
    for(int i = 0; i < OUTFIFO_MAX; i++)
        _rmnotifynode(kb, i);
    char path[strlen(devpath) + 2];
    snprintf(path, sizeof(path), "%s%d", devpath, index);
    if(rm_recursive(path) != 0 && errno != ENOENT){
        ckb_warn("Unable to delete %s: %s\n", path, strerror(errno));
        euid_guard_stop;
        return -1;
    }
    ckb_info("Removed device path %s\n", path);
    euid_guard_stop;
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
        chmod(fwpath, S_GID_READ);
        if(gid >= 0)
            chown(fwpath, 0, gid);
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
        chmod(ppath, S_GID_READ);
        if(gid >= 0)
            chown(ppath, 0, gid);
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

unsigned readlines(int fd, readlines_ctx ctx, const char** input){
    // Move any data left over from a previous read to the start of the buffer
    char* buffer = ctx->buffer;
    int buffersize = ctx->buffersize;
    int leftover = ctx->leftover, leftoverlen = ctx->leftoverlen;
    memcpy(buffer, buffer + leftover, leftoverlen);
    // Read data from the file
    ssize_t length = read(fd, buffer + leftoverlen, buffersize - leftoverlen);
    length = (length < 0 ? 0 : length) + leftoverlen;
    leftover = ctx->leftover = leftoverlen = ctx->leftoverlen = 0;
    if(length <= 0){
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
        ssize_t length2 = read(fd, buffer + oldsize, buffersize - oldsize);
        if(length2 <= 0)
            break;
        length += length2;
    }
    buffer[length] = 0;
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

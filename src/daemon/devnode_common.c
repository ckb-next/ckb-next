#include "device.h"
#include "devnode.h"
#include "firmware.h"
#include "input.h"
#include "led.h"
#include "notify.h"
#include "profile.h"
#include <ckbnextconfig.h>

void check_chmod(const char *pathname, mode_t mode){
    if(chmod(pathname, mode) < 0) {
        ckb_warn("Chmod call failed %s: %s", pathname, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

///
/// \brief _updateconnected Update the list of connected devices.
///
/// \<devicepath\> normally is /dev/input/ckb or /input/ckb.
/// \n Open the normal file under \<devicepath\>0/connected for writing.
/// For each device connected, print its devicepath+number,
/// the serial number of the usb device and the usb name of the device connected to that usb interface.
/// \n eg:
/// \n /dev/input/ckb1 0F022014ABABABABABABABABABABA999 Corsair K95 RGB Gaming Keyboard
/// \n /dev/input/ckb2 0D02303DBACBACBACBACBACBACBAC998 Corsair M65 RGB Gaming Mouse
///
/// Set the file ownership to root.
/// If the glob var gid is explicitly set to something different from -1 (the initial value), set file permission to 640, else to 644.
/// This is used if you start the daemon with --gid=\<GID\> Parameter.
///
/// Because several independent threads may call updateconnected(), protect that procedure with locking/unlocking of \b devmutex.
///
void _updateconnected(usbdevice* kb){
    queued_mutex_lock(devmutex);
    char cpath[strlen(devpath) + 12];
    snprintf(cpath, sizeof(cpath), "%s0/connected", devpath);
    FILE* cfile = fopen(cpath, "w");
    if(!cfile){
        ckb_warn("Unable to update %s: %s", cpath, strerror(errno));
        queued_mutex_unlock(devmutex);
        return;
    }
    int written = 0;
    if(kb != keyboard){
        for(int i = 1; i < DEV_MAX; i++){
#ifdef DEBUG_MUTEX
            ckb_info("Locking ckb%d in _updateconnected()", i);
#endif
            queued_mutex_lock(devmutex + i);
            if(kb->status == STATUS_CONNECTED){
                written = 1;
                fprintf(cfile, "%s%d %s %s\n", devpath, i, keyboard[i].serial, keyboard[i].name);
            }
#ifdef DEBUG_MUTEX
            ckb_info("Unlocking ckb%d in _updateconnected()", i);
#endif
            queued_mutex_unlock(devmutex + i);
        }
    }
    if(!written)
        fputc('\n', cfile);
    fclose(cfile);

    check_chmod(cpath, S_GID_READ);
    check_chown(cpath, 0, gid);

    queued_mutex_unlock(devmutex);
}

void updateconnected(usbdevice* kb){
    euid_guard_start;
    _updateconnected(kb);
    euid_guard_stop;
}

int mknotifynode(usbdevice* kb, int notify){
    euid_guard_start;
    int res = _mknotifynode(kb, notify);
    euid_guard_stop;
    return res;
}

int rmnotifynode(usbdevice* kb, int notify){
    euid_guard_start;
    int res = _rmnotifynode(kb, notify);
    euid_guard_stop;
    return res;
}

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
#ifdef OS_WINDOWS
    // FIXME: hack because for some reason windows can't delete these directories
    return remove("C:\\ProgramData\\ckb-next\\ckb0\\");
#else
    return remove(path);
#endif
}

void printnode(const char* path, const char* str){
    FILE* file = fopen(path, "w");
    if(file){
        fputs(str, file);
        fputc('\n', file);
        fclose(file);
        check_chmod(path, S_GID_READ);
        check_chown(path, 0, gid);
    } else {
        ckb_warn("Unable to create %s: %s", path, strerror(errno));
        remove(path);
    }
}

char* layoutstr(const char layout){
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
    // This currently does not work on windows
#ifndef OS_WINDOWS
    if(rm_recursive(path) != 0 && errno != ENOENT){
        ckb_err("Unable to delete %s: %s", path, strerror(errno));
        return -1;
    }
#endif
    if(mkdir_p(path, S_READDIR) != 0
#ifdef OS_WINDOWS
    && errno != EEXIST
#endif
        ){
        ckb_err("Unable to create %s: %s", path, strerror(errno));
        rm_recursive(path);
        return -1;
    }

    check_chown(path, 0, gid);

    if(kb == keyboard + 0){
        // Root keyboard: write a list of devices
        _updateconnected(kb);
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
            ckb_warn("Unable to create %s: %s", vpath, strerror(errno));
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
            ckb_warn("Unable to create %s: %s", ppath, strerror(errno));
            remove(ppath);
        }
    } else {
        // Create command FIFO
        if(create_dev_cmd_fifo(path, kb)){
            rm_recursive(path);
            kb->infifo = 0;
            return -1;
        }

        check_fchown(kb->infifo, 0, gid);

        // Create notification FIFO
        _mknotifynode(kb, 0);

        // Write the model and serial to files
        char mpath[sizeof(path) + 6], spath[sizeof(path) + 7], ipath[sizeof(path) + 10], lpath[sizeof(path) + 7], dpath[sizeof(path) + 4];
        snprintf(mpath, sizeof(mpath), "%s/model", path);
        snprintf(spath, sizeof(spath), "%s/serial", path);
        snprintf(ipath, sizeof(ipath), "%s/productid", path);
        snprintf(lpath, sizeof(lpath), "%s/layout", path);
        snprintf(dpath, sizeof(dpath), "%s/dpi", path);

        char productid[5];
        snprintf(productid, 5, "%04x", kb->product);

        char dpistr[6] = ""; // ushort max
        for(int i = 0; mouse_dpi_list[i].dev; i++){
            if(mouse_dpi_list[i].dev == kb->product){
                snprintf(dpistr, 6, "%hu", mouse_dpi_list[i].dpi);
                break;
            }
        }

        printnode(mpath, kb->name);
        printnode(spath, kb->serial);
        printnode(ipath, productid);
        printnode(lpath, layoutstr(kb->layout));
        printnode(dpath, dpistr);

        // Write the device's features
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
            ckb_warn("Unable to create %s: %s", fpath, strerror(errno));
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
        ckb_warn("Unable to create %s: %s", fwpath, strerror(errno));
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
        ckb_warn("Unable to create %s: %s", fwpath, strerror(errno));
        remove(ppath);
        return -2;
    }
    return 0;
}

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

unsigned readlines(usbdevice* kb, readlines_ctx ctx, const char** input){
#ifdef OS_WINDOWS
    HANDLE fd = kb->infifo;
#else
    int fd = kb->infifo - 1;
#endif
    // Move any data left over from a previous read to the start of the buffer
    char* buffer = ctx->buffer;
    int buffersize = ctx->buffersize;
    int leftover = ctx->leftover, leftoverlen = ctx->leftoverlen;
    memcpy(buffer, buffer + leftover, leftoverlen);
    // Read data from the pipe
    int length = do_pipe_read(fd, ctx, buffer, leftoverlen, buffersize);
    leftover = ctx->leftover = leftoverlen = ctx->leftoverlen = 0;
    if(length <= 0){
        length = leftoverlen;
        *input = NULL;
        return length;
    }
    length += leftoverlen;

    // Continue buffering until all available input is read or there's no room left
    while(length == buffersize){
        if(buffersize == MAX_BUFFER)
            break;
        int oldsize = buffersize;
        buffersize += 4096;
        ctx->buffersize = buffersize;
        buffer = ctx->buffer = realloc(buffer, buffersize + 1);
        int length2 = do_pipe_read(fd, ctx, buffer, oldsize, buffersize);
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
            ckb_warn("Too much input (1MB). Dropping.");
            return 0;
        }
        leftoverlen = ctx->leftoverlen = length;
        return 0;
    }
}

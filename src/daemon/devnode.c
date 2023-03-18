#include "device.h"
#include "devnode.h"
#include "firmware.h"
#include "input.h"
#include "led.h"
#include "notify.h"
#include "profile.h"
#include <ckbnextconfig.h>

// OSX doesn't like putting FIFOs in /dev for some reason
// Don't make these pointers, as doing so will result in sizeof() not producing the correct result.
#ifndef OS_MAC
#define DEVPATH "/dev/input/ckb"
#else
#define DEVPATH "/var/run/ckb"
#endif

const char devpath[] = DEVPATH;
#define DEVPATH_LEN (sizeof(devpath) - 1)

const char pidpath[] = DEVPATH "0/pid";

pid_t is_pid_running(void){
    FILE* pidfile = fopen(pidpath, "r");
    if(pidfile){
        pid_t pid;
        if(fscanf(pidfile, "%d", &pid) == EOF)
            ckb_err("PID fscanf returned EOF (%s)", strerror(errno));
        fclose(pidfile);
        if(pid > 0){
            // kill -s 0 checks if the PID is active but doesn't send a signal
            if(!kill(pid, 0))
                return pid;
        }
    }
    return 0;
}

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

void check_chown(const char *pathname, uid_t owner, long group){
    if (group >= 0) {
        if (chown(pathname, owner, group) < 0) {
            ckb_warn("Chown call failed %s: %s", pathname, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
}

void check_fchown(int fd, uid_t owner, long group){
    if (group >= 0) {
        if (fchown(fd, owner, group) < 0) {
            ckb_warn("FChown call failed: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
}

void check_chmod(const char *pathname, mode_t mode){
    if(chmod(pathname, mode) < 0) {
        ckb_warn("Chmod call failed %s: %s", pathname, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

///
/// \brief _updateconnected Update the list of connected devices.
///
/// Keeps track of device state separately (so that there's no need to lock every single dmutex)
/// dmutex for kb should already be locked before calling this
void _updateconnected(usbdevice* kb){
    queued_mutex_lock(devmutex);
    static char names[DEV_MAX][KB_NAME_LEN] = {{0}};
    static char serials[DEV_MAX][SERIAL_LEN] = {{0}};

    char cpath[DEVPATH_LEN + 12];
    snprintf(cpath, sizeof(cpath), "%s0/connected", devpath);
    FILE* cfile = fopen(cpath, "w");
    if(!cfile){
        ckb_err("Unable to open %s: %s", cpath, strerror(errno));
        queued_mutex_unlock(devmutex);
        return;
    }

    // Don't write anything if we just want to create the paths
    if(kb != keyboard){
        const int index = INDEX_OF(kb, keyboard);

        // Update the arrays with the status of the devices
        if(kb->status == DEV_STATUS_CONNECTED){
            strcpy(names[index], kb->name);
            strcpy(serials[index], kb->serial);
        } else {
            names[index][0] = '\0';
            serials[index][0] = '\0';
        }

        for(int i = 1; i < DEV_MAX; i++)
            if(serials[i][0])
                fprintf(cfile, "%s%d %s %s\n", devpath, i, serials[i], names[i]);
        fputc('\n', cfile);
    }

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

int _mknotifynode(usbdevice* kb, int notify){
    if(notify < 0 || notify >= OUTFIFO_MAX)
        return -1;
    if(kb->outfifo[notify] != 0)
        return 0;
    // Create the notification node
    char index = (INDEX_OF(kb, keyboard) % 10) + '0';
    char notify_char = (notify % 10) + '0';
    char outpath[DEVPATH_LEN + 10 * sizeof(char)];
    snprintf(outpath, sizeof(outpath), "%s%c/notify%c", devpath, index, notify_char);
    if(mkfifo(outpath, S_GID_READ) != 0 || (kb->outfifo[notify] = open(outpath, O_RDWR | O_NONBLOCK) + 1) == 0){
        // Add one to the FD because 0 is a valid descriptor, but ckb uses 0 for uninitialized devices
        ckb_warn("Unable to create %s: %s", outpath, strerror(errno));
        kb->outfifo[notify] = 0;
        remove(outpath);
        return -1;
    }
    check_fchown(kb->outfifo[notify] - 1, 0, gid);
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
    char index = (INDEX_OF(kb, keyboard) % 10) + '0';
    char notify_char = (notify % 10) + '0';
    char outpath[DEVPATH_LEN + 10 * sizeof(char)];
    snprintf(outpath, sizeof(outpath), "%s%c/notify%c", devpath, index, notify_char);
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

static void printnode(const char* path, const char* str){
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
    char path[DEVPATH_LEN + 2];
    snprintf(path, sizeof(path), "%s%d", devpath, index);
    if(rm_recursive(path) != 0 && errno != ENOENT){
        ckb_err("Unable to delete %s: %s", path, strerror(errno));
        return -1;
    }
    if(mkdir(path, S_READDIR) != 0){
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
        char inpath[sizeof(path) + 4];
        snprintf(inpath, sizeof(inpath), "%s/cmd", path);
        if(mkfifo(inpath, gid >= 0 ? S_CUSTOM : S_READWRITE) != 0
                // Open the node in RDWR mode because RDONLY will lock the thread
                || (kb->infifo = open(inpath, O_RDWR) + 1) == 0){
            // Add one to the FD because 0 is a valid descriptor, but ckb uses 0 for uninitialized devices
            ckb_err("Unable to create %s: %s", inpath, strerror(errno));
            rm_recursive(path);
            kb->infifo = 0;
            return -1;
        }

        check_fchown(kb->infifo - 1, 0, gid);

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
        // NOTE: Features that have their own files, which can be blank to indicate lack of support should NOT be added here.
        // It is implied that if the file is blank or does not exist, then the feature is not supported.
        // FIXME: Apply this to pollrate after a few versions (the flag is only kept for backwards compat)
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
            if(HAS_FEATURES(kb, FEAT_FWUPDATE))
                fputs(" fwupdate", ffile);
            if(HAS_FEATURES(kb, FEAT_HWLOAD))
                fputs(" hwload", ffile);
            if(HAS_FEATURES(kb, FEAT_DONGLE))
                fputs(" dongle", ffile);
            if(HAS_FEATURES(kb, FEAT_WIRELESS))
                fputs(" wireless", ffile);
            if(HAS_FEATURES(kb, FEAT_BATTERY))
                fputs(" battery", ffile);

            if(kb->brightness_mode == BRIGHTNESS_HARDWARE_COARSE)
                fputs(" hwbright_coarse", ffile);
            else if(kb->brightness_mode == BRIGHTNESS_HARDWARE_FINE)
                fputs(" hwbright_fine", ffile);
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
    char path[DEVPATH_LEN + 2];
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

static inline uchar FWBcdToBin(const uchar v){
    return ((v >> 4) * 10) + (v & 0xF);
}

static inline void FWtoThreeSegments(FILE* fwfile, uint32_t ver, usbdevice* kb){
    // If the device doesn't support reading fw ver, or the version is UINT32_MAX (which means we couldn't read the version), leave blank
    // Latter happens when trying to request a bragi radio version for a device that doesn't have one
    // 0 is a valid fw version (when app is corrupt)
    if(!HAS_FEATURES(kb, FEAT_FWVERSION) || ver == UINT32_MAX)
        return;

    // NXP/Legacy devices use BCD, but also bragi devices if we can't read the fw ver (and only have bcdDevice)
    // At the moment we overwrite the bcdDevice for bragi, so we do not need to handle it
    // If in the future we encounter a bragi device which does not report an APP fw ver, this will need to be changed
    if(kb->protocol == PROTO_BRAGI)
        fprintf(fwfile, "%hhu.%hhu.%hhu", (uchar)(ver >> 16 & 0xFF), (uchar)(ver >> 8 & 0xFF), (uchar)(ver & 0xFF));
    else
        fprintf(fwfile, "%hhu.%hhu", FWBcdToBin(ver >> 8 & 0xFF), FWBcdToBin(ver & 0xFF));
}

static const char* const pollrate_to_str[POLLRATE_COUNT] = {
    [POLLRATE_8MS] = "8 ms",
    [POLLRATE_4MS] = "4 ms",
    [POLLRATE_2MS] = "2 ms",
    [POLLRATE_1MS] = "1 ms",
    [POLLRATE_05MS] = "0.5 ms",
    [POLLRATE_025MS] = "0.25 ms",
    [POLLRATE_01MS] = "0.1 ms",
};

int mkfwnode(usbdevice* kb){
    int index = INDEX_OF(kb, keyboard);
    char fwpath[DEVPATH_LEN + 12];
    snprintf(fwpath, sizeof(fwpath), "%s%d/fwversion", devpath, index);
    FILE* fwfile = fopen(fwpath, "w");
    if(fwfile){
        // Start with APP ver
        // Force 0 if we require a fw update
        if(NEEDS_FW_UPDATE(kb))
            FWtoThreeSegments(fwfile, 0, kb);
        else
            FWtoThreeSegments(fwfile, kb->fwversion, kb);
        fputc('\n', fwfile);

        // Followed by BLD ver
        FWtoThreeSegments(fwfile, kb->bldversion, kb);
        fputc('\n', fwfile);

        // Followed by WL radio APP ver
        FWtoThreeSegments(fwfile, kb->radioappversion, kb);
        fputc('\n', fwfile);

        // Followed by WL radio BLD ver
        FWtoThreeSegments(fwfile, kb->radiobldversion, kb);
        fputc('\n', fwfile);

        // Closing newline
        fputc('\n', fwfile);
        fclose(fwfile);
        check_chmod(fwpath, S_GID_READ);
        check_chown(fwpath, 0, gid);
    } else {
        ckb_warn("Unable to create %s: %s", fwpath, strerror(errno));
        remove(fwpath);
        return -1;
    }
    char ppath[DEVPATH_LEN + 11];
    snprintf(ppath, sizeof(ppath), "%s%d/pollrate", devpath, index);
    FILE* pfile = fopen(ppath, "w");
    if(pfile){
        if(kb->pollrate != POLLRATE_UNKNOWN)
            fputs(pollrate_to_str[kb->pollrate], pfile);
        fputc('\n', pfile);
        if(kb->maxpollrate != POLLRATE_UNKNOWN)
            fputs(pollrate_to_str[kb->maxpollrate], pfile);
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

// 0 means EOF, < 0 means retry.
// This function needs to return all the commands read at once so that the parser can handle duplicate commands
int readline_fifo(int fd, readlines_ctx* ctx){
    if(ctx->next_start) {
        // Move anything left over back to the beginning
        memmove(ctx->buf, ctx->next_start, ctx->leftover_bytes);
    }

    const size_t max_read = MAX_BUFFER - ctx->leftover_bytes;

    // Read fresh data
    ssize_t ret = read(fd, ctx->buf + ctx->leftover_bytes, max_read);
    if(ret <= 0)
        return ret;

    // Find the last newline and terminate the string there
    char* last = memrchr(ctx->buf, '\n', ctx->leftover_bytes + ret);
    ctx->next_start = last;

    // No newline found yet
    if(!last){
        if((size_t)ret == max_read){
            ckb_warn("String too long (%dKB). Dropping...", MAX_BUFFER/1024);
            ctx->leftover_bytes = 0;
        } else {
            ctx->leftover_bytes += ret;
        }
        return -1;
    }

    *last = '\0';

    ctx->leftover_bytes = (ret + ctx->leftover_bytes) - (last - ctx->buf) - 1;

    return 1;
}

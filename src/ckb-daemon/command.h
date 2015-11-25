#ifndef COMMAND_H
#define COMMAND_H

#include "includes.h"

// Command operations
typedef enum {
    // Special - handled by readcmd, no device functions
    NONE        = -10,
    MODE        = -9,   CMD_FIRST = MODE,
    SWITCH      = -8,
    LAYOUT      = -7,
    ACCEL       = -6,
    SCROLLSPEED = -5,
    NOTIFYON    = -4,
    NOTIFYOFF   = -3,
    FPS         = -2,
    DITHER      = -1,

    // Hardware data
    HWLOAD      = 0,    CMD_VT_FIRST = 0,
    HWSAVE,
    FWUPDATE,
    POLLRATE,

    // Software control on/off
    ACTIVE,
    IDLE,

    // Profile/mode metadata
    ERASE,
    ERASEPROFILE,
    NAME,
    PROFILENAME,
    ID,
    PROFILEID,

    // LED control
    RGB,
    IOFF,
    ION,
    IAUTO,

    // Key binding control
    BIND,
    UNBIND,
    REBIND,
    MACRO,

    // DPI control
    DPI,
    DPISEL,
    LIFT,
    SNAP,

    // Notifications and output
    NOTIFY,
    INOTIFY,
    GET,

    CMD_LAST = GET
} cmd;
#define CMD_COUNT       (CMD_LAST - CMD_FIRST + 2)
#define CMD_DEV_COUNT   (CMD_LAST - CMD_VT_FIRST + 1)

// Device command vtable. Most have a standard prototype, but a few are exceptions.
// Not all parameters are used by all commands - they are sometimes repurposed or used as dummy arguemnts. See relevant headers.
typedef void (*cmdhandler)(usbdevice* kb, usbmode* modeidx, int notifyidx, int keyindex, const char* parameter);    // Normal command
typedef int (*cmdhandler_io)(usbdevice* kb, usbmode* modeidx, int notifyidx, int keyindex, const char* parameter);  // Command with hardware I/O - returns zero on success
typedef void (*cmdhandler_mac)(usbdevice* kb, usbmode* modeidx, int notifyidx, const char* keys, const char* assignment); // Macro command has a different left-side handler
typedef union devcmd {
    // Commands can be accessed by name or by position
    cmdhandler      do_cmd[CMD_DEV_COUNT];
    cmdhandler_io   do_io[CMD_DEV_COUNT];
    cmdhandler_mac  do_macro[CMD_DEV_COUNT];
    struct {
        // profile.h
        cmdhandler_io hwload;
        cmdhandler_io hwsave;
        // firmware.h
        cmdhandler_io fwupdate;
        // device.h
        cmdhandler_io pollrate;

        // device.h
        cmdhandler_io active;
        cmdhandler_io idle;

        // profile.h
        cmdhandler erase;
        cmdhandler eraseprofile;
        cmdhandler name;
        cmdhandler profilename;
        cmdhandler id;
        cmdhandler profileid;

        // led.h
        cmdhandler rgb;
        cmdhandler ioff;
        cmdhandler ion;
        cmdhandler iauto;

        // input.h
        cmdhandler bind;
        cmdhandler unbind;
        cmdhandler rebind;
        cmdhandler_mac macro;

        // dpi.h
        cmdhandler_mac dpi;
        cmdhandler dpisel;
        cmdhandler lift;
        cmdhandler snap;

        // notify.h
        cmdhandler notify;
        cmdhandler inotify;
        cmdhandler get;

        // Extra functions not command-related
        // device.h
        int (*start)(usbdevice* kb, int makeactive);
        void (*setmodeindex)(usbdevice* kb, int index);
        // profile.h
        void (*allocprofile)(usbdevice* kb);
        int (*loadprofile)(usbdevice* kb);
        void (*freeprofile)(usbdevice* kb);
        // led.h
        int (*updatergb)(usbdevice* kb, int force);
        // input.h
        void (*updateindicators)(usbdevice* kb, int force);
        // dpi.h
        int (*updatedpi)(usbdevice* kb, int force);
    };
} devcmd;

// Parse input from FIFO. Lock dmutex first (see device.h)
// This function is also responsible for calling all of the cmd_ functions. They should not be invoked elsewhere.
int readcmd(usbdevice* kb, const char* line);

#endif  // COMMAND_H


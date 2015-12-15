#include "command.h"
#include "device.h"
#include "dpi.h"
#include "firmware.h"
#include "input.h"
#include "led.h"
#include "notify.h"
#include "profile.h"

// Do-nothing functions
static void cmd_none(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4){
}
static int cmd_io_none(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4){
    return 0;
}
static void cmd_macro_none(usbdevice* kb, usbmode* dummy1, int dummy2, const char* dummy3, const char* dummy4){
}
static int loadprofile_none(usbdevice* kb){
    return 0;
}
static void int1_void_none(usbdevice* kb, int dummy){
}
static int int1_int_none(usbdevice* kb, int dummy){
    return 0;
}

// RGB keyboard vtable
const devcmd vtable_keyboard = {
    .hwload = cmd_hwload_kb,
    .hwsave = cmd_hwsave_kb,
    .fwupdate = cmd_fwupdate,
    .pollrate = cmd_io_none,

    .active = cmd_active_kb,
    .idle = cmd_idle_kb,

    .erase = cmd_erase,
    .eraseprofile = cmd_eraseprofile,
    .name = cmd_name,
    .profilename = cmd_profilename,
    .id = cmd_id,
    .profileid = cmd_profileid,

    .rgb = cmd_rgb,
    .ioff = cmd_ioff,
    .ion = cmd_ion,
    .iauto = cmd_iauto,

    .bind = cmd_bind,
    .unbind = cmd_unbind,
    .rebind = cmd_rebind,
    .macro = cmd_macro,

    .dpi = cmd_macro_none,
    .dpisel = cmd_none,
    .lift = cmd_none,
    .snap = cmd_none,

    .notify = cmd_notify,
    .inotify = cmd_inotify,
    .get = cmd_get,

    .start = start_dev,
    .setmodeindex = int1_void_none,
    .allocprofile = allocprofile,
    .loadprofile = loadprofile,
    .freeprofile = freeprofile,
    .updatergb = updatergb_kb,
    .updateindicators = updateindicators_kb,
    .updatedpi = int1_int_none
};

// Non-RGB keyboard vtable (K70)
const devcmd vtable_keyboard_nonrgb = {
    .hwload = cmd_io_none,
    .hwsave = cmd_io_none,
    .fwupdate = cmd_io_none,
    .pollrate = cmd_io_none,

    .active = cmd_io_none,
    .idle = cmd_io_none,

    .erase = cmd_erase,
    .eraseprofile = cmd_eraseprofile,
    .name = cmd_name,
    .profilename = cmd_profilename,
    .id = cmd_id,
    .profileid = cmd_profileid,

    .rgb = cmd_none,
    .ioff = cmd_ioff,
    .ion = cmd_ion,
    .iauto = cmd_iauto,

    .bind = cmd_bind,
    .unbind = cmd_unbind,
    .rebind = cmd_rebind,

    .dpi = cmd_macro_none,
    .dpisel = cmd_none,
    .lift = cmd_none,
    .snap = cmd_none,

    .notify = cmd_notify,
    .inotify = cmd_inotify,
    .get = cmd_get,

    .start = start_kb_nrgb,
    .setmodeindex = setmodeindex_nrgb,
    .allocprofile = allocprofile,
    .loadprofile = loadprofile_none,
    .freeprofile = freeprofile,
    .updatergb = int1_int_none,
    .updateindicators = updateindicators_kb,
    .updatedpi = int1_int_none
};

// RGB mouse vtable
const devcmd vtable_mouse = {
    .hwload = cmd_hwload_mouse,
    .hwsave = cmd_hwsave_mouse,
    .fwupdate = cmd_fwupdate,
    .pollrate = cmd_pollrate,

    .active = cmd_active_mouse,
    .idle = cmd_idle_mouse,

    .erase = cmd_erase,
    .eraseprofile = cmd_eraseprofile,
    .name = cmd_name,
    .profilename = cmd_profilename,
    .id = cmd_id,
    .profileid = cmd_profileid,

    .rgb = cmd_rgb,
    .ioff = cmd_none,
    .ion = cmd_none,
    .iauto = cmd_none,

    .bind = cmd_bind,
    .unbind = cmd_unbind,
    .rebind = cmd_rebind,

    .dpi = cmd_dpi,
    .dpisel = cmd_dpisel,
    .lift = cmd_lift,
    .snap = cmd_snap,

    .notify = cmd_notify,
    .inotify = cmd_none,
    .get = cmd_get,

    .start = start_dev,
    .setmodeindex = int1_void_none,
    .allocprofile = allocprofile,
    .loadprofile = loadprofile,
    .freeprofile = freeprofile,
    .updatergb = updatergb_mouse,
    .updateindicators = int1_void_none,
    .updatedpi = updatedpi
};

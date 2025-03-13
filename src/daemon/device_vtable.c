#include "command.h"
#include "device.h"
#include "profile.h"
#include "dpi.h"
#include "firmware.h"
#include "input.h"
#include "led.h"
#include "notify.h"
#include "profile.h"
#include "usb_legacy.h"
#include "usb_nxp.h"
#include "usb_bragi.h"

// Do-nothing functions
static void cmd_none(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4){
    (void)kb;
    (void)dummy1;
    (void)dummy2;
    (void)dummy3;
    (void)dummy4;
}
static int cmd_io_none(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4){
    (void)kb;
    (void)dummy1;
    (void)dummy2;
    (void)dummy3;
    (void)dummy4;

    return 0;
}
static void cmd_macro_none(usbdevice* kb, usbmode* dummy1, int dummy2, const char* dummy3, const char* dummy4){
    (void)kb;
    (void)dummy1;
    (void)dummy2;
    (void)dummy3;
    (void)dummy4;
}
static int loadprofile_none(usbdevice* kb){
    (void)kb;

    return 0;
}
static void int1_void_none(usbdevice* kb, int dummy){
    (void)kb;
    (void)dummy;
}
static int int1_int_none(usbdevice* kb, int dummy){
    (void)kb;
    (void)dummy;

    return 0;
}
static void int0_void_none(usbdevice* kb){
    (void)kb;
}
static int cmd_pollrate_none(usbdevice* kb, pollrate_t poll){
    (void)kb;
    (void)poll;
    return 0;
}
#if 0
static int cmd_io_stub(usbdevice* kb, void* ptr, int len, int is_recv, const char* file, int line){
    ckb_fatal_fn("This should never be called", file, line);
    return -1;
}
#endif

/// \brief RGB keyboard vtable holds functions for each device type.
///
const devcmd vtable_keyboard = {
    .hwload = cmd_hwload_kb,
    .hwsave = cmd_hwsave_kb,
    .fwupdate = cmd_fwupdate,
    .pollrate = cmd_pollrate,

    .active = cmd_active_kb,
    .idle = cmd_idle_kb,

    .erase = cmd_erase,
    .eraseprofile = cmd_eraseprofile,
    .name = cmd_name,
    .profilename = cmd_profilename,
    .id = cmd_id,
    .profileid = cmd_profileid,

    .rgb = cmd_rgb,
    .hwanim = cmd_hwanim,
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
    .setmodeindex = int1_void_none,             ///< is just for non rgb keyboards
    .allocprofile = allocprofile,
    .loadprofile = loadprofile,
    .freeprofile = freeprofile,
    .updatergb = updatergb_kb,
    .updateindicators = updateindicators_kb,
    .updatedpi = int1_int_none,                 ///< This is for mice only
    .reset = nxp_reset,
    .fill_input_eps = nxp_fill_input_eps,
    .write = nxp_usb_write,
    .read = nxp_usb_read,
    .get_battery_info = int0_void_none,
    .delay = nxp_delay,
    .setfps = nxp_kb_setfps,
};

const devcmd vtable_keyboard_wireless = {
    .hwload = cmd_hwload_kb,
    .hwsave = cmd_hwsave_kb,
    .fwupdate = cmd_fwupdate,
    .pollrate = cmd_pollrate,

    .active = cmd_active_kb,
    .idle = cmd_idle_kb,

    .erase = cmd_erase,
    .eraseprofile = cmd_eraseprofile,
    .name = cmd_name,
    .profilename = cmd_profilename,
    .id = cmd_id,
    .profileid = cmd_profileid,

    .rgb = cmd_rgb,
    .hwanim = cmd_hwanim,
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
    .updatedpi = int1_int_none,
    .reset = nxp_reset,
    .fill_input_eps = nxp_fill_input_eps,
    .write = nxp_usb_write,
    .read = nxp_usb_read,
    .get_battery_info = nxp_get_battery_info,
    .delay = nxp_delay,
    .setfps = nxp_kb_setfps,
};

// Legacy keyboard vtable (K70)
const devcmd vtable_keyboard_legacy = {
    .hwload = cmd_io_none,
    .hwsave = cmd_io_none,
    .fwupdate = cmd_io_none,
    .pollrate = cmd_pollrate_none,

    .active = cmd_io_none,
    .idle = cmd_io_none,

    .erase = cmd_erase,
    .eraseprofile = cmd_eraseprofile,
    .name = cmd_name,
    .profilename = cmd_profilename,
    .id = cmd_id,
    .profileid = cmd_profileid,

    .rgb = cmd_none,
    .hwanim = cmd_hwanim,
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

    .start = start_kb_legacy,
    .setmodeindex = setmodeindex_legacy,
    .allocprofile = allocprofile,
    .loadprofile = loadprofile_none,
    .freeprofile = freeprofile,
    .updatergb = int1_int_none,
    .updateindicators = updateindicators_kb,
    .updatedpi = int1_int_none,
    .reset = cmd_none,
    .fill_input_eps = legacy_fill_input_eps,
    .write = legacy_dev_io,
    .read = legacy_dev_io,
    .get_battery_info = int0_void_none,
    .delay = legacy_delay,
    .setfps = int1_void_none, // Legacy devices have a fixed delay
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
    .hwanim = cmd_hwanim,
    .ioff = cmd_none,
    .ion = cmd_none,
    .iauto = cmd_none,

    .bind = cmd_bind,
    .unbind = cmd_unbind,
    .rebind = cmd_rebind,
    .macro = cmd_macro,

    .dpi = cmd_dpi,
    .dpisel = cmd_dpisel,
    .lift = cmd_lift,
    .snap = cmd_snap,

    .notify = cmd_notify,
    .inotify = cmd_none,
    .get = cmd_get,

    .start = start_dev,
    .setmodeindex = int1_void_none,         ///< Mice do not have different modes
    .allocprofile = allocprofile,           ///< same for all keyboards and mice
    .loadprofile = loadprofile,             ///< same for all keyboards and mice
    .freeprofile = freeprofile,             ///< same for all keyboards and mice
    .updatergb = updatergb_mouse,           ///< special for mice
    .updateindicators = int1_void_none,     ///< Mice do not have keyboard indicators like num
    .updatedpi = updatedpi,                 ///< special for mice
    .reset = nxp_reset,
    .fill_input_eps = nxp_fill_input_eps,
    .write = nxp_usb_write,
    .read = nxp_usb_read,
    .get_battery_info = int0_void_none,
    .delay = nxp_delay,
    .setfps = nxp_mouse_setfps,
};

const devcmd vtable_mouse_wireless = {
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
    .hwanim = cmd_hwanim,
    .ioff = cmd_none,
    .ion = cmd_none,
    .iauto = cmd_none,

    .bind = cmd_bind,
    .unbind = cmd_unbind,
    .rebind = cmd_rebind,
    .macro = cmd_macro,

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
    .updatedpi = updatedpi,
    .reset = nxp_reset,
    .fill_input_eps = nxp_fill_input_eps,
    .write = nxp_usb_write,
    .read = nxp_usb_read,
    .get_battery_info = nxp_get_battery_info,
    .delay = nxp_delay,
    .setfps = nxp_mouse_setfps,
};

// RGB Mousepad vtable
const devcmd vtable_mousepad = {
    .hwload = cmd_io_none,
    .hwsave = cmd_io_none,
    .fwupdate = cmd_fwupdate,
    .pollrate = cmd_pollrate_none,

    .active = cmd_active_mouse,
    .idle = cmd_idle_mouse,

    .erase = cmd_erase,
    .eraseprofile = cmd_eraseprofile,
    .name = cmd_name,
    .profilename = cmd_profilename,
    .id = cmd_id,
    .profileid = cmd_profileid,

    .rgb = cmd_rgb,
    .hwanim = cmd_hwanim,
    .ioff = cmd_none,
    .ion = cmd_none,
    .iauto = cmd_none,

    .bind = cmd_none,
    .unbind = cmd_none,
    .rebind = cmd_none,
    .macro = cmd_macro_none,

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
    .updatergb = updatergb_mousepad,
    .updateindicators = int1_void_none,
    .updatedpi = int1_int_none,
    .reset = nxp_reset,
    .fill_input_eps = nxp_fill_input_eps,
    .write = nxp_usb_write,
    .read = nxp_usb_read,
    .get_battery_info = int0_void_none,
    .delay = nxp_delay,
    .setfps = nxp_mouse_setfps,
};

// Legacy mouse vtable
const devcmd vtable_mouse_legacy = {
    .hwload = cmd_io_none,
    .hwsave = cmd_io_none,
    .fwupdate = cmd_io_none,
    .pollrate = cmd_pollrate_legacy,

    .active = cmd_io_none,
    .idle = cmd_io_none,

    .erase = cmd_erase,
    .eraseprofile = cmd_eraseprofile,
    .name = cmd_name,
    .profilename = cmd_profilename,
    .id = cmd_id,
    .profileid = cmd_profileid,

    .rgb = cmd_rgb,
    .hwanim = cmd_hwanim,
    .ioff = cmd_ioff,
    .ion = cmd_ion,
    .iauto = cmd_iauto,

    .bind = cmd_bind,
    .unbind = cmd_unbind,
    .rebind = cmd_rebind,
    .macro = cmd_macro,

    .dpi = cmd_dpi,
    .dpisel = cmd_dpisel,
    .lift = cmd_lift_legacy,
    .snap = cmd_snap_legacy,

    .notify = cmd_notify,
    .inotify = cmd_inotify,
    .get = cmd_get,

    .start = start_mouse_legacy, //
    .setmodeindex = int1_void_none,
    .allocprofile = allocprofile,
    .loadprofile = loadprofile_none,
    .freeprofile = freeprofile,
    .updatergb = updatergb_mouse_legacy,
    .updateindicators = int1_void_none,
    .updatedpi = updatedpi_legacy,
    .reset = cmd_none,
    .fill_input_eps = legacy_fill_input_eps,
    .write = legacy_dev_io,
    .read = legacy_dev_io,
    .get_battery_info = int0_void_none,
    .delay = legacy_delay,
    .setfps = int1_void_none, // Legacy devices have a fixed delay
};

// Bragi vtables
const devcmd vtable_bragi_mouse = {
    .hwload = cmd_io_none,
    .hwsave = cmd_io_none,
    .fwupdate = cmd_io_none,
    .pollrate = cmd_pollrate_bragi,

    .active = cmd_active_bragi,
    .idle = cmd_idle_bragi,
    .pair = cmd_none,

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

    .dpi = cmd_dpi,
    .dpisel = cmd_dpisel,
    .lift = cmd_none,
    .snap = cmd_snap_bragi,

    .notify = cmd_notify,
    .inotify = cmd_inotify,
    .get = cmd_get,

    .start = start_mouse_bragi,
    .setmodeindex = int1_void_none,
    .allocprofile = allocprofile,
    .loadprofile = loadprofile_none,
    .freeprofile = freeprofile,
    .updatergb = updatergb_mouse_bragi,
    .updateindicators = int1_void_none,
    .updatedpi = updatedpi_bragi,
    .reset = cmd_none,
    .fill_input_eps = bragi_fill_input_eps,
    .write = bragi_usb_write,
    .read = bragi_usb_read,
    .get_battery_info = bragi_get_battery_info,
    .delay = bragi_delay,
    .setfps = int1_void_none, // Bragi devices respond to everything, so no need for delays
};

const devcmd vtable_bragi_keyboard = {
    .hwload = cmd_io_none,
    .hwsave = cmd_io_none,
    .fwupdate = cmd_io_none,
    .pollrate = cmd_pollrate_bragi,

    .active = cmd_active_bragi,
    .idle = cmd_idle_bragi,
    .pair = cmd_none,

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

    .start = start_keyboard_bragi,
    .setmodeindex = int1_void_none,
    .allocprofile = allocprofile,
    .loadprofile = loadprofile_none,
    .freeprofile = freeprofile,
    .updatergb = updatergb_keyboard_bragi,
    .updateindicators = updateindicators_kb,
    .updatedpi = int1_int_none,
    .reset = cmd_none,
    .fill_input_eps = bragi_fill_input_eps,
    .write = bragi_usb_write,
    .read = bragi_usb_read,
    .get_battery_info = bragi_get_battery_info,
    .delay = bragi_delay,
    .setfps = int1_void_none, // Bragi devices respond to everything, so no need for delays
};

const devcmd vtable_bragi_dongle = {
    .hwload = cmd_io_none,
    .hwsave = cmd_io_none,
    .fwupdate = cmd_io_none,
    .pollrate = cmd_pollrate_none,

    .active = cmd_io_none,
    .idle = cmd_io_none,
    .pair = cmd_pair_bragi,

    .erase = cmd_none,
    .eraseprofile = cmd_none,
    .name = cmd_name,
    .profilename = cmd_none,
    .id = cmd_none,
    .profileid = cmd_none,

    .rgb = cmd_none,
    .ioff = cmd_none,
    .ion = cmd_none,
    .iauto = cmd_none,

    .bind = cmd_none,
    .unbind = cmd_none,
    .rebind = cmd_none,
    .macro = cmd_macro_none,

    .dpi = cmd_macro_none,
    .dpisel = cmd_none,
    .lift = cmd_none,
    .snap = cmd_none,

    .notify = cmd_none,
    .inotify = cmd_none,
    .get = cmd_none,

    .start = start_dongle_bragi,
    .setmodeindex = int1_void_none,
    .allocprofile = allocprofile,
    .loadprofile = loadprofile_none,
    .freeprofile = freeprofile,
    .updatergb = int1_int_none,
    .updateindicators = int1_void_none,
    .updatedpi = int1_int_none,
    .reset = cmd_none,
    .fill_input_eps = bragi_fill_input_eps,
    .write = bragi_usb_write,
    .read = bragi_usb_read,
    .get_battery_info = int0_void_none,
    .delay = bragi_delay,
    .setfps = int1_void_none, // Bragi devices respond to everything, so no need for delays
};

const devcmd vtable_bragi_mousepad = {
    .hwload = cmd_io_none,
    .hwsave = cmd_io_none,
    .fwupdate = cmd_io_none,
    .pollrate = cmd_pollrate_none,

    .active = cmd_active_bragi,
    .idle = cmd_idle_bragi,
    .pair = cmd_none,

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

    .start = start_mouse_bragi,
    .setmodeindex = int1_void_none,
    .allocprofile = allocprofile,
    .loadprofile = loadprofile_none,
    .freeprofile = freeprofile,
    .updatergb = updatergb_keyboard_bragi,
    .updateindicators = int1_void_none,
    .updatedpi = int1_int_none,
    .reset = cmd_none,
    .fill_input_eps = bragi_fill_input_eps,
    .write = bragi_usb_write,
    .read = bragi_usb_read,
    .get_battery_info = int0_void_none,
    .delay = bragi_delay,
    .setfps = int1_void_none, // Bragi devices respond to everything, so no need for delays
};

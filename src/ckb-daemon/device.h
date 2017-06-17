#ifndef DEVICE_H
#define DEVICE_H

#include "includes.h"
#include "keymap.h"

// Connected devices
#define DEV_MAX     9
extern usbdevice keyboard[DEV_MAX];
// Is a device active?
#ifdef OS_LINUX
#define IS_CONNECTED(kb) ((kb) && (kb)->handle && (kb)->uinput_kb && (kb)->uinput_mouse)
#else
#define IS_CONNECTED(kb) ((kb) && (kb)->handle && (kb)->event)
#endif
// A mutex used for USB controls. Needs to be locked before reading or writing the device handle or accessing its profile
extern pthread_mutex_t devmutex[DEV_MAX];
#define dmutex(kb) (devmutex + INDEX_OF(kb, keyboard))
// Similar, but for key input. Also needs to be locked before accessing output FIFOs.
// When adding or removing a device you must lock BOTH mutexes, dmutex first.
extern pthread_mutex_t inputmutex[DEV_MAX];
#define imutex(kb) (inputmutex + INDEX_OF(kb, keyboard))

// Needed to synchronize sending macro-keys to the os and sending color info to the device
extern pthread_mutex_t macromutex[DEV_MAX];
#define mmutex(kb) (macromutex + INDEX_OF(kb, keyboard))
extern pthread_mutex_t macromutex2[DEV_MAX];
#define mmutex2(kb) (macromutex2 + INDEX_OF(kb, keyboard))
extern pthread_cond_t macrovar[DEV_MAX];
#define mvar(kb) (macrovar + INDEX_OF(kb, keyboard))

// Sets up device hardware, after software initialization is finished. Also used during resets
// Should be called only from setupusb/resetusb
int start_dev(usbdevice* kb, int makeactive);
int start_kb_nrgb(usbdevice* kb, int makeactive);

// Activates/deactives software control on a keyboard. Return 0 on success
int setactive_kb(usbdevice* kb, int active);
int setactive_mouse(usbdevice* kb, int active);
///
/// \brief setactive() calls via the corresponding kb->vtable either the active() or the idle() function.
/// \n active() is called if the parameter makeactive is true, idle if it is false.
/// \n What function is called effectively is device dependent. Have a look at \a device_vtable.c for more information.
#define setactive(kb, makeactive) ((makeactive) ? (kb)->vtable->active((kb), 0, 0, 0, 0) : (kb)->vtable->idle((kb), 0, 0, 0, 0))

// Command: Activate a keyboard
int cmd_active_kb(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4);
int cmd_active_mouse(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4);
// Command: Deactivate a keyboard
int cmd_idle_kb(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4);
int cmd_idle_mouse(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4);

// Command: Set poll rate (mouse only)
int cmd_pollrate(usbdevice* kb, usbmode* dummy1, int dummy2, int rate, const char* dummy3);

// Sets a device's current mode index. This is only used on the non-RGB K95; the RGB keyboards have no gettable HW index.
void setmodeindex_nrgb(usbdevice* kb, int index);


// Per-key input settings for device setup
// The upper nybble controls input mode. 0x80 generates a normal HID interrupt, 0x40 generates a proprietary interrupt. 0xc0 generates both.
// The exceptions are the proprietary Corsair keys, which only report HID input in BIOS mode and only report Corsair input in non-BIOS mode.
// In BIOS mode, the Corsair input is disabled no matter what.
#define IN_HID          0x80
#define IN_CORSAIR      0x40

// The lower nybble controls various hardware actions
#define ACT_LIGHT       1
#define ACT_NEXT        3
#define ACT_NEXT_NOWRAP 5
#define ACT_LOCK        8
#define ACT_MR_RING     9
#define ACT_M1          10
#define ACT_M2          11
#define ACT_M3          12

#endif  // DEVICE_H

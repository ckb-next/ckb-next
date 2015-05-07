#ifndef DEVICE_H
#define DEVICE_H

#include "includes.h"
#include "keymap.h"

// Connected devices
#define DEV_MAX     9
extern usbdevice keyboard[DEV_MAX];
// Is a device active?
#ifdef OS_LINUX
#define IS_CONNECTED(kb) ((kb) && (kb)->handle && (kb)->uinput)
#else
#define IS_CONNECTED(kb) ((kb) && (kb)->handle && (kb)->event)
#endif
// A mutex used when accessing the device table. Lock this before adding/removing/iterating devices, unlock when finished.
// Note: you must lock the list mutex BEFORE any device mutexes, if you intend to lock both
extern pthread_mutex_t devlistmutex;
// A mutex used for USB controls. Needs to be locked before reading or writing the device handle or accessing its profile
extern pthread_mutex_t devmutex[DEV_MAX];
#define dmutex(kb) (devmutex + INDEX_OF(kb, keyboard))
// Similar, but for key input. Also needs to be locked before accessing output FIFOs.
extern pthread_mutex_t inputmutex[DEV_MAX];
#define imutex(kb) (inputmutex + INDEX_OF(kb, keyboard))

// Sets up device hardware, after software initialization is finished. Also used during resets
// Should be called only from setupusb/resetusb
int start_dev(usbdevice* kb, int makeactive);
int start_kb_nrgb(usbdevice* kb, int makeactive);

// Activates/deactives software control on a keyboard. Return 0 on success
int setactive_kb(usbdevice* kb, int active);
int setactive_mouse(usbdevice* kb, int active);
#define setactive(kb, makeactive) ((makeactive) ? (kb)->vtable->active((kb), 0, 0, 0, 0) : (kb)->vtable->idle((kb), 0, 0, 0, 0))

// Command: Activate a keyboard
int cmd_active_kb(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4);
int cmd_active_mouse(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4);
// Command: Deactivate a keyboard
int cmd_idle_kb(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4);
int cmd_idle_mouse(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4);

// Sets a device's current mode index. This is only used on the non-RGB K95; the RGB keyboards have no gettable HW index.
void setmodeindex_nrgb(usbdevice* kb, int index);

#endif  // DEVICE_H

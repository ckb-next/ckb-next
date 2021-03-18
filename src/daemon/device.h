#ifndef DEVICE_H
#define DEVICE_H

#include "includes.h"
#include "keymap.h"
#include <ckbnextconfig.h>

// Connected devices
// If this gets raised to a double digit number, the changes introduced by commit f24988ab68a94 will need to be modified to accommodate for that
#define DEV_MAX     9
extern usbdevice keyboard[DEV_MAX];
// Is a device active?
#ifdef OS_LINUX
#define IS_CONNECTED(kb) ((kb) && (kb)->handle && (((kb)->uinput_kb && (kb)->uinput_mouse) || IS_SINGLE_EP(kb)))
#else
#define IS_CONNECTED(kb) ((kb) && (kb)->handle && (kb)->event)
#endif

#ifdef NO_FAIR_MUTEX_QUEUEING
#define QUEUED_MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER
typedef pthread_mutex_t queued_mutex_t;
#else
// A fair queue mutex construct; contenders get to grab the mutex in the order they attempted to acquire it
typedef struct queued_mutex{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    unsigned long next_in, next_waiting;
} queued_mutex_t;

#define QUEUED_MUTEX_INITIALIZER {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0, 0}
#endif

void queued_mutex_lock(queued_mutex_t* mutex);   // Lock a queued_mutex
int queued_mutex_trylock(queued_mutex_t* mutex); // Try to lock a queued_mutex without blocking; returns 0 on success.
void queued_mutex_unlock(queued_mutex_t* mutex); // Unlock a queued_mutex
void queued_cond_nanosleep(pthread_cond_t *restrict cond, queued_mutex_t *restrict mutex, const uint32_t ns);
int cond_nanosleep(pthread_cond_t *restrict cond, pthread_mutex_t *restrict mutex, uint32_t ns);

#ifdef DEBUG_MUTEX
#define MUTEX_DBG(str, kb, mutexarray) (ckb_info(str " accessed in %s:%d (%s) at ckb%d, TID 0x%lx, MID %p", __FILE__, __LINE__, __func__, INDEX_OF(kb, keyboard), pthread_self(), mutexarray + INDEX_OF(kb, keyboard)) & 0)
#else
#define MUTEX_DBG(a, b, c) 0
#endif
// A mutex used for USB controls. Needs to be locked before reading or writing the device handle or accessing its profile
extern queued_mutex_t devmutex[DEV_MAX];
#define dmutex(kb) (devmutex + INDEX_OF(kb, keyboard) + MUTEX_DBG("DMUTEX", kb, devmutex))
// Similar, but for key input. Also needs to be locked before accessing output FIFOs.
// When adding or removing a device you must lock BOTH mutexes, dmutex first.
extern queued_mutex_t inputmutex[DEV_MAX];
#define imutex(kb) (inputmutex + INDEX_OF(kb, keyboard) + MUTEX_DBG("IMUTEX", kb, inputmutex))

// Needed to synchronize sending macro-keys to the os and sending color info to the device
extern queued_mutex_t macromutex[DEV_MAX];
#define mmutex(kb) (macromutex + INDEX_OF(kb, keyboard))
extern pthread_mutex_t macromutex2[DEV_MAX];
#define mmutex2(kb) (macromutex2 + INDEX_OF(kb, keyboard))
extern pthread_cond_t macrovar[DEV_MAX];
#define mvar(kb) (macrovar + INDEX_OF(kb, keyboard))
extern pthread_cond_t macroint[DEV_MAX];
#define mintvar(kb) (macroint + INDEX_OF(kb, keyboard))

// Mutex used for transfering URB Interrupt data between threads
extern pthread_mutex_t interruptmutex[DEV_MAX];
#define intmutex(kb) (interruptmutex + INDEX_OF(kb, keyboard))
// Pthread cond for the above
extern pthread_cond_t interruptcond[DEV_MAX];
#define intcond(kb) (interruptcond + INDEX_OF(kb, keyboard))

int init_cond_monotonic(void);

// Sets up device hardware, after software initialization is finished. Also used during resets
// Should be called only from setupusb/resetusb
int start_dev(usbdevice* kb, int makeactive);
int start_kb_legacy(usbdevice* kb, int makeactive);
int start_mouse_legacy(usbdevice* kb, int makeactive);
int start_mouse_bragi(usbdevice* kb, int makeactive);
int start_keyboard_bragi(usbdevice* kb, int makeactive);

// Activates/deactives software control on a device. Return 0 on success
int setactive_kb(usbdevice* kb, int active);
int setactive_mouse(usbdevice* kb, int active);
///
/// \brief setactive() calls via the corresponding kb->vtable either the active() or the idle() function.
/// \n active() is called if the parameter makeactive is true, idle if it is false.
/// \n What function is called effectively is device dependent. Have a look at \a device_vtable.c for more information.
#define setactive(kb, makeactive) ((makeactive) ? (kb)->vtable->active((kb), 0, 0, 0, 0) : (kb)->vtable->idle((kb), 0, 0, 0, 0))

// Command: Activate a device
int cmd_active_kb(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4);
int cmd_active_mouse(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4);
int cmd_active_bragi(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4);
// Command: Deactivate a device
int cmd_idle_kb(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4);
int cmd_idle_mouse(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4);
int cmd_idle_bragi(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4);
// Command: Set poll rate
int cmd_pollrate(usbdevice* kb, usbmode* dummy1, int dummy2, int rate, const char* dummy3);
int cmd_pollrate_legacy(usbdevice* kb, usbmode* dummy1, int dummy2, int rate, const char* dummy3);
int cmd_pollrate_bragi(usbdevice* kb, usbmode* dummy1, int dummy2, int rate, const char* dummy3);

// Reads the battery of a wireless device
void* read_battery(void* context);

// Sets a device's current mode index. This is only used on the legacy K95; the RGB keyboards have no gettable HW index.
void setmodeindex_legacy(usbdevice* kb, int index);

// Resets devices using the NXP HID protocol
void nxp_reset(usbdevice* kb, usbmode* mode, int dummy1, int dummy2, const char* type);

// Used to clear the input struct and force an rgb update when switching between hw and sw modes
void clear_input_and_rgb(usbdevice* kb, const int active);

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

#ifndef OS_H
#define OS_H

// OS definitions

#ifdef __linux
#define OS_LINUX
#endif
#ifdef __APPLE__
#define OS_MAC
#endif

#if !defined(OS_LINUX) && !defined(OS_MAC)
#error Your OS is not supported. Edit os.h if you want to compile anyway.
#endif

// OS-specific includes

#ifdef OS_LINUX

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <features.h>
#include <libudev.h>
#include <linux/uinput.h>
#include <linux/usbdevice_fs.h>

#ifndef UINPUT_VERSION
#define UINPUT_VERSION 2
#endif

#endif  // OS_LINUX

#ifdef OS_MAC

#define __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES 1
#include <AssertMacros.h>

#include <Availability.h>
#include <Carbon/Carbon.h>
#include <IOKit/IOMessage.h>
#include <IOKit/hid/IOHIDDevicePlugin.h>
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hidsystem/IOHIDLib.h>
#include <IOKit/hidsystem/ev_keymap.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/usb/USB.h>

typedef IOHIDDeviceDeviceInterface**    hid_dev_t;
typedef IOUSBDeviceInterface182**       usb_dev_t;
typedef IOUSBInterfaceInterface183**    usb_iface_t;

// Various POSIX functions that aren't present on OSX

void *memrchr(const void *s, int c, size_t n);

#if __MAC_OS_X_VERSION_MAX_ALLOWED < 101200
typedef int clockid_t;
#define CLOCK_MONOTONIC 1
#endif

#define TIMER_ABSTIME   1

int clock_gettime(clockid_t clk_id, struct timespec *tp);
int clock_nanosleep(clockid_t clock_id, int flags, const struct timespec *rqtp, struct timespec *rmtp);

#endif  // OS_MAC

// The OSX process on legacy builds needs to change its EUID to post events, so thread safety must be ensured
// Otherwise the EUID is always root
#ifdef OS_MAC_LEGACY

extern pthread_mutex_t _euid_guard;
#define euid_guard_start    pthread_mutex_lock(&_euid_guard)
#define euid_guard_stop     pthread_mutex_unlock(&_euid_guard)

#else

#define euid_guard_start
#define euid_guard_stop

#endif // OS_MAC_LEGACY

#endif  // OS_H

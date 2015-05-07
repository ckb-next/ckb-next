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

#include <Carbon/Carbon.h>
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hidsystem/IOHIDLib.h>
#include <IOKit/hidsystem/ev_keymap.h>

// Various POSIX functions that aren't present on OSX

void *memrchr(const void *s, int c, size_t n);

#define CLOCK_MONOTONIC 1
#define TIMER_ABSTIME   1
typedef int clockid_t;
int clock_gettime(clockid_t clk_id, struct timespec *tp);
int clock_nanosleep(clockid_t clock_id, int flags, const struct timespec *rqtp, struct timespec *rmtp);

#endif  // OS_MAC

#endif  // OS_H

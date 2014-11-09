#ifndef OS_H
#define OS_H

// OS definitions

#ifdef __linux
#define OS_LINUX
#endif
#ifdef __APPLE__
#define OS_MAC
#endif

// OS-specific includes

#ifdef OS_LINUX

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include <features.h>
#include <libusb-1.0/libusb.h>
#include <linux/uinput.h>

#ifndef UINPUT_VERSION
#define UINPUT_VERSION 2
#endif

// Mutexes kill libusb. Disable them for now...
#define pthread_mutex_lock(mutex)
#define pthread_mutex_unlock(mutex)

#endif  // OS_LINUX

#ifdef OS_MAC

#include <Carbon/Carbon.h>
#include <IOKit/hid/IOHIDLib.h>

#endif  // OS_MAC

#endif

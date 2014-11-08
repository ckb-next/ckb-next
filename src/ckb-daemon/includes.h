#ifndef INCLUDES_H
#define INCLUDES_H

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
#include <linux/uinput.h>

#ifndef UINPUT_VERSION
#define UINPUT_VERSION 2
#endif

#endif  // OS_LINUX

#ifdef OS_MAC

#include <Carbon/Carbon.h>

#endif  // OS_MAC

#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <iconv.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/signal.h>
#include <sys/stat.h>

#include <libusb-1.0/libusb.h>

#endif

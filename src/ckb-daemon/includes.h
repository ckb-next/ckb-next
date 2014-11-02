#ifndef INCLUDES_H
#define INCLUDES_H

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include <features.h>

#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/errno.h>
#include <sys/signal.h>
#include <sys/stat.h>

#include <libusb-1.0/libusb.h>

#include <linux/uinput.h>

#ifndef UINPUT_VERSION
#define UINPUT_VERSION 2
#endif

#endif

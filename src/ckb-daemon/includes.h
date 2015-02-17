#ifndef INCLUDES_H
#define INCLUDES_H

#include "os.h"

#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <iconv.h>
#include <locale.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/time.h>

// Unsigned char/short definition
typedef unsigned char uchar;
typedef unsigned short ushort;
// Gets the index of an object within an array
#define INDEX_OF(entry, array) (int)(entry - array)

// Better __FILE__ macro
#define __FILE_NOPATH__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

// Timespec utilities
void timespec_add(struct timespec* timespec, long nanoseconds);
#define timespec_gt(left, right)    ((left).tv_sec > (right).tv_sec || ((left).tv_sec == (right).tv_sec && (left).tv_nsec > (right).tv_nsec))
#define timespec_eq(left, right)    ((left).tv_sec == (right).tv_sec && (left).tv_nsec == (right).tv_nsec)
#define timespec_ge(left, right)    ((left).tv_sec > (right).tv_sec || ((left).tv_sec == (right).tv_sec && (left).tv_nsec >= (right).tv_nsec))
#define timespec_lt(left, right)    (!timespec_ge(left, right))
#define timespec_le(left, right)    (!timespec_gt(left, right))

#include "structures.h"

#endif

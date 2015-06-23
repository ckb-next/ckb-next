#ifndef INCLUDES_H
#define INCLUDES_H

#include "os.h"

#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <iconv.h>
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

// Output helpers
// Use ckb_* to output info or ckb_*_fn to override the file/line numbers (useful when describing where a function was invoked from)
#define ckb_fatal_nofile(fmt, args...)          fprintf(stderr, "[F] " fmt, ## args)
#define ckb_fatal_fn(fmt, file, line, args...)  fprintf(stderr, "[F] %s (via %s:%d): " fmt, __func__, file, line, ## args)
#define ckb_fatal(fmt, args...)                 fprintf(stderr, "[F] %s (%s:%d): " fmt, __func__, __FILE_NOPATH__, __LINE__, ## args)
#define ckb_err_nofile(fmt, args...)            fprintf(stderr, "[E] " fmt, ## args)
#define ckb_err_fn(fmt, file, line, args...)    fprintf(stderr, "[E] %s (via %s:%d): " fmt, __func__, file, line, ## args)
#define ckb_err(fmt, args...)                   fprintf(stderr, "[E] %s (%s:%d): " fmt, __func__, __FILE_NOPATH__, __LINE__, ## args)
#define ckb_warn_nofile(fmt, args...)           printf("[W] " fmt, ## args)
#define ckb_warn_fn(fmt, file, line, args...)   printf("[W] %s (via %s:%d): " fmt, __func__, file, line, ## args)
#define ckb_warn(fmt, args...)                  printf("[W] %s (%s:%d): " fmt, __func__, __FILE_NOPATH__, __LINE__, ## args)
#define ckb_info_nofile(fmt, args...)           printf("[I] " fmt, ## args)
#define ckb_info_fn(fmt, file, line, args...)   printf("[I] " fmt, ## args)
#define ckb_info(fmt, args...)                  printf("[I] " fmt, ## args)

// Timespec utilities
void timespec_add(struct timespec* timespec, long nanoseconds);
#define timespec_gt(left, right)    ((left).tv_sec > (right).tv_sec || ((left).tv_sec == (right).tv_sec && (left).tv_nsec > (right).tv_nsec))
#define timespec_eq(left, right)    ((left).tv_sec == (right).tv_sec && (left).tv_nsec == (right).tv_nsec)
#define timespec_ge(left, right)    ((left).tv_sec > (right).tv_sec || ((left).tv_sec == (right).tv_sec && (left).tv_nsec >= (right).tv_nsec))
#define timespec_lt(left, right)    (!timespec_ge(left, right))
#define timespec_le(left, right)    (!timespec_gt(left, right))

// Common structs
#include "structures.h"

#endif  // INCLUDES_H

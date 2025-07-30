#ifndef INCLUDES_H
#define INCLUDES_H

#include "os.h"

#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <iconv.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <inttypes.h>

#include <errno.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

// Unsigned char/short definition
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
// Gets the index of an object within an array
#define INDEX_OF(entry, array) (int)(entry - array)

// Compile with -DCKB_OUTPUT_TO_STDERR if you want to separate error messages from normal status updates.
// (probably not useful because the errors don't mean much without context)
#ifdef CKB_OUTPUT_TO_STDERR
#define ckb_s_out   stdout
#define ckb_s_err   stderr
#else
#define ckb_s_out   stdout
#define ckb_s_err   stdout
#endif

// Better __FILE__ macro
#define __FILE_NOPATH__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

// Output helpers
// Use ckb_* to output info or ckb_*_fn to override the file/line numbers (useful when describing where a function was invoked from)
#define ckb_fatal_nofile(fmt, args...)          fprintf(ckb_s_err, "[F] " fmt "\n", ## args)
#define ckb_fatal_fn(fmt, file, line, args...)  fprintf(ckb_s_err, "[F] %s (via %s:%d): " fmt "\n", __func__, file, line, ## args)
#define ckb_fatal(fmt, args...)                 fprintf(ckb_s_err, "[F] %s (%s:%d): " fmt "\n", __func__, __FILE_NOPATH__, __LINE__, ## args)
#define ckb_err_nofile(fmt, args...)            fprintf(ckb_s_err, "[E] " fmt "\n", ## args)
#define ckb_err_fn(fmt, file, line, args...)    fprintf(ckb_s_err, "[E] %s (via %s:%d): " fmt "\n", __func__, file, line, ## args)
#define ckb_err(fmt, args...)                   fprintf(ckb_s_err, "[E] %s (%s:%d): " fmt "\n", __func__, __FILE_NOPATH__, __LINE__, ## args)
#define ckb_warn_nofile(fmt, args...)           fprintf(ckb_s_out, "[W] " fmt "\n", ## args)
#define ckb_warn_fn(fmt, file, line, args...)   fprintf(ckb_s_out, "[W] %s (via %s:%d): " fmt "\n", __func__, file, line, ## args)
#define ckb_warn(fmt, args...)                  fprintf(ckb_s_out, "[W] %s (%s:%d): " fmt "\n", __func__, __FILE_NOPATH__, __LINE__, ## args)
#define ckb_info_nofile(fmt, args...)           fprintf(ckb_s_out, "[I] " fmt "\n", ## args)
#define ckb_info_fn(fmt, file, line, args...)   fprintf(ckb_s_out, "[I] " fmt "\n", ## args)
#define ckb_info(fmt, args...)                  fprintf(ckb_s_out, "[I] " fmt "\n", ## args)

// Timespec utilities
void timespec_add(struct timespec* timespec, int64_t nanoseconds);
#define timespec_gt(left, right)    ((left).tv_sec > (right).tv_sec || ((left).tv_sec == (right).tv_sec && (left).tv_nsec > (right).tv_nsec))
#define timespec_eq(left, right)    ((left).tv_sec == (right).tv_sec && (left).tv_nsec == (right).tv_nsec)
#define timespec_ge(left, right)    ((left).tv_sec > (right).tv_sec || ((left).tv_sec == (right).tv_sec && (left).tv_nsec >= (right).tv_nsec))
#define timespec_lt(left, right)    (!timespec_ge(left, right))
#define timespec_le(left, right)    (!timespec_gt(left, right))

// Common structs
#include "structures.h"

#define THREAD_NAME_MAX 16
#define SIGHANDLER_SENDER   0
#define SIGHANDLER_RECEIVER 1

#endif  // INCLUDES_H

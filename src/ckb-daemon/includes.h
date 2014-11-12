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

// Unsigned char definition
typedef unsigned char uchar;
// Gets the index of an object within an array
#define INDEX_OF(entry, array) (int)(((unsigned long)(entry) - (unsigned long)(array)) / sizeof(*entry))

#include "structures.h"

#endif

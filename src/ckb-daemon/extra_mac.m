#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#include <mach/mach_time.h>
#include "includes.h"

// ¯\_(ツ)_/¯

int keyrepeatdelay(){
    return round([NSEvent keyRepeatDelay] * 1000.);
}

int keyrepeatinterval(){
    return round([NSEvent keyRepeatInterval] * 1000.);
}

void *memrchr(const void *s, int c, size_t n){
    const char* cs = s;
    for(size_t i = n; i > 0; i--){
        if(cs[i - 1] == c)
            return (void*)(cs + i - 1);
    }
    return 0;
}

unsigned long getfactor(){
    static unsigned long factor = 0;
    if(factor)
        return factor;
    mach_timebase_info_data_t info;
    mach_timebase_info(&info);
    return factor = info.numer / info.denom;
}

int clock_gettime(clockid_t clk_id, struct timespec *tp){
    if(clk_id != CLOCK_MONOTONIC){
        errno = EINVAL;
        return -1;
    }
    unsigned long factor = getfactor();
    unsigned long nsec = mach_absolute_time() * factor;
    tp->tv_sec = nsec / 1000000000;
    tp->tv_nsec = nsec % 1000000000;
    return 0;
}

int clock_nanosleep(clockid_t clock_id, int flags, const struct timespec *rqtp, struct timespec *rmtp){
    if(clock_id != CLOCK_MONOTONIC)
        return EINVAL;
    if(flags == TIMER_ABSTIME){
        // Absolute time
        // Determine the amount of time left to wait
        struct timespec curtime, realtime;
        clock_gettime(clock_id, &curtime);
        if(curtime.tv_sec > rqtp->tv_sec || (curtime.tv_sec == rqtp->tv_sec && curtime.tv_nsec >= rqtp->tv_nsec))
            // Time already passed
            return 0;
        realtime.tv_sec = rqtp->tv_sec - curtime.tv_sec;
        realtime.tv_nsec = rqtp->tv_nsec - curtime.tv_nsec;
        if(realtime.tv_nsec < 0){
            // Borrow on underflow
            realtime.tv_sec--;
            realtime.tv_nsec += 1000000000;
        }
        int res = nanosleep(&realtime, 0);
        if(!res)
            return 0;
        return errno;
    }
    // Relative time
    int res = nanosleep(rqtp, rmtp);
    if(!res)
        return 0;
    return errno;
}

// Creates a media key event. This isn't possible using a standard CGEventCreate.
CGEventRef media_event(uint data1, uint modifiers){
    NSEvent* event = [NSEvent otherEventWithType:NSSystemDefined location:CGPointMake(0, 0) modifierFlags:modifiers timestamp:[[NSProcessInfo processInfo] systemUptime] windowNumber:0 context:0 subtype:NX_SUBTYPE_AUX_CONTROL_BUTTONS data1:data1 data2:0];
    return [event CGEvent];
}

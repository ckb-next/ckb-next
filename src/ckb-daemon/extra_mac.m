#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#include <string.h>

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
        if(cs[i] == c)
            return (void*)(cs + i);
    }
    return 0;
}

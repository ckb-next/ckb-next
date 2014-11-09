#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

// ¯\_(ツ)_/¯

int keyrepeatdelay(){
    return round([NSEvent keyRepeatDelay] * 1000.);
}

int keyrepeatinterval(){
    return round([NSEvent keyRepeatInterval] * 1000.);
}

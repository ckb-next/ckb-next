#import <AudioToolbox/AudioServices.h>
#import <Foundation/Foundation.h>
#include "media.h"

muteState getMuteState(){
    // Get the system's default audio device
    AudioObjectPropertyAddress propertyAddress = {
        kAudioHardwarePropertyDefaultOutputDevice,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster
    };
    AudioDeviceID deviceID = 0;
    UInt32 dataSize = sizeof(deviceID);
    if(AudioHardwareServiceGetPropertyData(kAudioObjectSystemObject, &propertyAddress, 0, NULL, &dataSize, &deviceID)
            != kAudioHardwareNoError){
        return UNKNOWN;
    }
    // Grab the mute property
    AudioObjectPropertyAddress propertyAddress2 = {
        kAudioDevicePropertyMute,
        kAudioDevicePropertyScopeOutput,
        kAudioObjectPropertyElementMaster
    };
    if(!AudioHardwareServiceHasProperty(deviceID, &propertyAddress2))
        return UNKNOWN;
    bool state = 0;
    dataSize = sizeof(state);
    if(AudioHardwareServiceGetPropertyData(deviceID, &propertyAddress2, 0, NULL, &dataSize, &state)
            != kAudioHardwareNoError)
        return UNKNOWN;
    return state ? MUTED : UNMUTED;
}

void disableAppNap(){
    [[NSProcessInfo processInfo] beginActivityWithOptions:NSActivityUserInitiatedAllowingIdleSystemSleep reason:@"Keyboard animation"];
}

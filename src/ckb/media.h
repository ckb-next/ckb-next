#ifndef MEDIA_H
#define MEDIA_H

#ifdef __cplusplus

#define EXTERN_C            extern "C"
#define ENUM_C(name)        enum name
#define ENUM_END_C(name)

#else

#define EXTERN_C            extern
#define ENUM_C(name)        typedef enum
#define ENUM_END_C(name)    name

#endif

// Gets the default audio device's mute state
ENUM_C(muteState) {
    UNKNOWN = -1,
    UNMUTED,
    MUTED
} ENUM_END_C(muteState);
EXTERN_C muteState getMuteState();

#endif

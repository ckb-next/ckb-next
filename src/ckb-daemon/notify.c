#include "device.h"
#include "devnode.h"
#include "notify.h"

void nprintf(usbdevice* kb, usbprofile* profile, usbmode* mode, const char* format, ...){
    if(!kb && !profile)
        return;
    if(!profile)
        profile = &kb->profile;
    va_list va_args;
    char line = '\n';
    int fifo;
    // Write the string to the keyboard's FIFOs (if any)
    for(int i = 0; i < OUTFIFO_MAX; i++){
        va_start(va_args, format);
        if(kb && (fifo = kb->outfifo[i])){
            if(mode)
                dprintf(fifo, "mode %d ", INDEX_OF(mode, profile->mode) + 1);
            vdprintf(fifo, format, va_args);
            if(write(fifo, &line, 1) <= 0)
                printf("Write error: %s\n", strerror(errno));
        }
        // Write it to the root FIFOs and include the serial number
        va_start(va_args, format);
        if((fifo = keyboard[0].outfifo[i])){
            dprintf(fifo, "%s ", profile->serial);
            if(mode)
                dprintf(fifo, "mode %d ", INDEX_OF(mode, profile->mode) + 1);
            vdprintf(fifo, format, va_args);
            if(write(fifo, &line, 1) <= 0)
                printf("Write error: %s\n", strerror(errno));
        }
    }
}

void notifyconnect(usbdevice* kb, int connecting){
    int index = INDEX_OF(kb, keyboard);
    for(int i = 0; i < OUTFIFO_MAX; i++){
        if(keyboard[0].outfifo[i]){
            dprintf(keyboard[0].outfifo[i], "%s %s %s%d\n", kb->profile.serial, connecting ? "added at" : "removed from", devpath, index);
        }
    }
}

void cmd_notify(usbmode* mode, const key* keymap, int keyindex, const char* toggle){
    if(!strcmp(toggle, "on") || *toggle == 0)
        SET_KEYBIT(mode->notify, keyindex);
    else if(!strcmp(toggle, "off"))
        CLEAR_KEYBIT(mode->notify, keyindex);
}

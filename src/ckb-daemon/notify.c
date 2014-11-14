#include "device.h"
#include "devnode.h"
#include "led.h"
#include "notify.h"

void nprintf(usbdevice* kb, usbprofile* profile, usbmode* mode, const char* format, ...){
    if(!kb && !profile)
        return;
    if(!profile)
        profile = &kb->profile;
    va_list va_args;
    int fifo;
    // Write the string to the keyboard's FIFOs (if any)
    for(int i = 0; i < OUTFIFO_MAX; i++){
        va_start(va_args, format);
        if(kb && (fifo = kb->outfifo[i])){
            if(mode)
                dprintf(fifo, "mode %d ", INDEX_OF(mode, profile->mode) + 1);
            vdprintf(fifo, format, va_args);
        }
        // Write it to the root FIFOs and include the serial number
        va_start(va_args, format);
        if((fifo = keyboard[0].outfifo[i])){
            dprintf(fifo, "device %s ", profile->serial);
            if(mode)
                dprintf(fifo, "mode %d ", INDEX_OF(mode, profile->mode) + 1);
            vdprintf(fifo, format, va_args);
        }
    }
}

void nrprintf(const char* format, ...){
    va_list va_args;
    char line = '\n';
    int fifo;
    for(int i = 0; i < OUTFIFO_MAX; i++){
        va_start(va_args, format);
        if((fifo = keyboard[0].outfifo[i])){
            vdprintf(fifo, format, va_args);
        }
    }
}

void notifyconnect(usbdevice* kb, int connecting){
    int index = INDEX_OF(kb, keyboard);
    nrprintf("device %s %s %s%d\n", kb->profile.serial, connecting ? "added at" : "removed from", devpath, index);
}

void cmd_notify(usbmode* mode, const key* keymap, int keyindex, const char* toggle){
    if(!strcmp(toggle, "on") || *toggle == 0)
        SET_KEYBIT(mode->notify, keyindex);
    else if(!strcmp(toggle, "off"))
        CLEAR_KEYBIT(mode->notify, keyindex);
}

void getinfo(usbdevice* kb, usbmode* mode, const char* setting){
    if(!strcmp(setting, ":hello")){
        if(kb && mode)
            return;
        nrprintf("hello\n");
        return;
    } else if(!strcmp(setting, ":fps")){
        if(kb && mode)
            return;
        nrprintf("fps %d\n", fps);
        return;
    } else if(!strcmp(setting, ":layout")){
        if(kb && mode)
            nprintf(kb, 0, 0, "layout %s\n", kb->profile.keymap == keymap_uk ? "uk" : "us");
        else
            nrprintf("layout %s\n", keymap_system == keymap_uk ? "uk" : "us");
        return;
    } else if(!kb || !mode)
        // Only FPS and layout can be printed without an active mode
        return;
}

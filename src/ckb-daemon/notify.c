#include "device.h"
#include "devnode.h"
#include "notify.h"

void nprintf(usbdevice* kb, usbsetting* setting, usbmode* mode, const char* format, ...){
    if(!kb && !setting)
        return;
    if(!setting)
        setting = &kb->setting;
    va_list va_args;
    char line = '\n';
    int fifo;
    // Write the string to the keyboard's FIFOs (if any)
    for(int i = 0; i < OUTFIFO_MAX; i++){
        va_start(va_args, format);
        if(kb && (fifo = kb->outfifo[i])){
            if(mode)
                dprintf(fifo, "mode %d ", INDEX_OF(mode, setting->profile.mode) + 1);
            vdprintf(fifo, format, va_args);
            write(fifo, &line, 1);
        }
        // Write it to the root FIFOs and include the serial number
        va_start(va_args, format);
        if((fifo = keyboard[0].outfifo[i])){
            dprintf(fifo, "%s ", setting->serial);
            if(mode)
                dprintf(fifo, "mode %d ", INDEX_OF(mode, setting->profile.mode) + 1);
            vdprintf(fifo, format, va_args);
            write(fifo, &line, 1);
        }
    }
}

void notifyconnect(usbdevice* kb, int connecting){
    int index = INDEX_OF(kb, keyboard);
    for(int i = 0; i < OUTFIFO_MAX; i++){
        if(keyboard[0].outfifo[i]){
            dprintf(keyboard[0].outfifo[i], "%s %s %s%d\n", kb->setting.serial, connecting ? "added at" : "removed from", devpath, index);
        }
    }
}

void cmd_notify(usbmode* mode, int keycode, const char* toggle){
    if(!strcmp(toggle, "on") || *toggle == 0)
        SET_KEYBIT(mode->notify, keycode);
    else if(!strcmp(toggle, "off"))
        CLEAR_KEYBIT(mode->notify, keycode);
}

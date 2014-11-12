#include "notify.h"
#include "device.h"
#include "devnode.h"

void nprintf(usbdevice* kb, usbsetting* setting, usbmode* mode, const char* format, ...){
    if(!kb && !setting)
        return;
    if(!setting)
        setting = &kb->setting;
    va_list va_args;
    va_start(va_args, format);
    char line = '\n';
    int fifo;
    // Write the string to the keyboard's FIFO (if any)
    if(kb && (fifo = kb->outfifo)){
        if(mode)
            dprintf(fifo, "mode %d ", INDEX_OF(mode, setting->profile.mode) + 1);
        vdprintf(fifo, format, va_args);
        write(fifo, &line, 1);
    }
    // Write it to the root FIFO and include the serial number
    va_start(va_args, format);
    if((fifo = keyboard[0].outfifo)){
        dprintf(fifo, "%s ", setting->serial);
        if(mode)
            dprintf(fifo, "mode %d ", INDEX_OF(mode, setting->profile.mode) + 1);
        vdprintf(fifo, format, va_args);
        write(fifo, &line, 1);
    }
}

void notifyconnect(int index, int connecting){
    if(keyboard[0].outfifo){
        usbdevice* kb = keyboard + index;
        dprintf(keyboard[0].outfifo, "%s %s %s%d\n", kb->setting.serial, connecting ? "added at" : "removed from", devpath, index);
    }
}

void cmd_notify(usbmode* mode, int keycode, const char* toggle){
    if(!strcmp(toggle, "on") || *toggle == 0)
        SET_KEYBIT(mode->notify, keycode);
    else if(!strcmp(toggle, "off"))
        CLEAR_KEYBIT(mode->notify, keycode);
}

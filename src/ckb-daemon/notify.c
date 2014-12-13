#include "device.h"
#include "devnode.h"
#include "led.h"
#include "notify.h"
#include "profile.h"

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

#define HWMODE_OR_RETURN(kb, index) \
    switch((kb)->model){            \
    case 95:                        \
        if((index) >= HWMODE_K95)   \
            return;                 \
        break;                      \
    case 70:                        \
        if((index) >= HWMODE_K70)   \
            return;                 \
        break;                      \
    default:                        \
        return;                     \
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
            nprintf(kb, 0, 0, "layout %s\n", getmapname(kb->profile.keymap));
        else
            nrprintf("layout %s\n", getmapname(keymap_system));
        return;
    } else if(!kb || !mode)
        // Only FPS and layout can be printed without an active mode
        return;

    usbprofile* profile = &kb->profile;
    if(!strcmp(setting, ":mode")){
        // Get the current mode number
        nprintf(kb, 0, mode, "switch\n");
        return;
    } else if(!strcmp(setting, ":rgb")){
        // Get the current RGB settings
        char* rgb = printrgb(&mode->light, profile->keymap);
        nprintf(kb, 0, mode, "rgb %s\n", rgb);
        free(rgb);
        return;
    } else if(!strcmp(setting, ":rgbon")){
        // Get the current RGB status
        if(mode->light.enabled)
            nprintf(kb, 0, mode, "rgb on\n");
        else
            nprintf(kb, 0, mode, "rgb off\n");
        return;
    } else if(!strcmp(setting, ":hwrgb")){
        // Get the current hardware RGB settings
        if(!kb->hw)
            return;
        unsigned index = INDEX_OF(mode, profile->mode);
        // Make sure the mode number is valid
        HWMODE_OR_RETURN(kb, index);
        // Get the mode from the hardware store
        char* rgb = printrgb(kb->hw->light + index, profile->keymap);
        nprintf(kb, 0, mode, "hwrgb %s\n", rgb);
        free(rgb);
        return;
    } else if(!strcmp(setting, ":profilename")){
        // Get the current profile name
        char* name = getprofilename(profile);
        nprintf(kb, 0, 0, "profilename %s\n", name[0] ? name : "Unnamed");
        free(name);
    } else if(!strcmp(setting, ":name")){
        // Get the current mode name
        char* name = getmodename(mode);
        nprintf(kb, 0, mode, "name %s\n", name[0] ? name : "Unnamed");
        free(name);
    } else if(!strcmp(setting, ":hwprofilename")){
        // Get the current hardware profile name
        if(!kb->hw)
            return;
        char* name = gethwprofilename(kb->hw);
        nprintf(kb, 0, 0, "hwprofilename %s\n", name[0] ? name : "Unnamed");
        free(name);
    } else if(!strcmp(setting, ":hwname")){
        // Get the current hardware mode name
        if(!kb->hw)
            return;
        unsigned index = INDEX_OF(mode, profile->mode);
        HWMODE_OR_RETURN(kb, index);
        char* name = gethwmodename(kb->hw, index);
        nprintf(kb, 0, mode, "hwname %s\n", name[0] ? name : "Unnamed");
        free(name);
    } else if(!strcmp(setting, ":profileid")){
        // Get the current profile ID
        char* guid = getid(&profile->id);
        int modified;
        memcpy(&modified, &profile->id.modified, sizeof(modified));
        nprintf(kb, 0, 0, "profileid %s %x\n", guid, modified);
        free(guid);
    } else if(!strcmp(setting, ":id")){
        // Get the current mode ID
        char* guid = getid(&mode->id);
        int modified;
        memcpy(&modified, &mode->id.modified, sizeof(modified));
        nprintf(kb, 0, mode, "id %s %x\n", guid, modified);
        free(guid);
    } else if(!strcmp(setting, ":hwprofileid")){
        // Get the current hardware profile ID
        if(!kb->hw)
            return;
        char* guid = getid(&kb->hw->id[0]);
        int modified;
        memcpy(&modified, &kb->hw->id[0].modified, sizeof(modified));
        nprintf(kb, 0, 0, "hwprofileid %s %x\n", guid, modified);
        free(guid);
    } else if(!strcmp(setting, ":hwid")){
        // Get the current hardware mode ID
        if(!kb->hw)
            return;
        unsigned index = INDEX_OF(mode, profile->mode);
        HWMODE_OR_RETURN(kb, index);
        char* guid = getid(&kb->hw->id[index + 1]);
        int modified;
        memcpy(&modified, &kb->hw->id[index + 1].modified, sizeof(modified));
        nprintf(kb, 0, mode, "hwid %s %x\n", guid, modified);
        free(guid);
    }
}

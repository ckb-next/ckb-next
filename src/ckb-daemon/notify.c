#include "device.h"
#include "devnode.h"
#include "led.h"
#include "notify.h"
#include "profile.h"

void nprintf(usbdevice* kb, int nodenumber, usbmode* mode, const char* format, ...){
    if(!kb)
        return;
    usbprofile* profile = &kb->profile;
    va_list va_args;
    int fifo;
    if(nodenumber >= 0){
        // If node number was given, print to that node (if open)
        if((fifo = kb->outfifo[nodenumber])){
            va_start(va_args, format);
            if(mode)
                dprintf(fifo, "mode %d ", INDEX_OF(mode, profile->mode) + 1);
            vdprintf(fifo, format, va_args);
        }
        return;
    }
    // Otherwise, print to all nodes
    for(int i = 0; i < OUTFIFO_MAX; i++){
        if((fifo = kb->outfifo[i])){
            va_start(va_args, format);
            if(mode)
                dprintf(fifo, "mode %d ", INDEX_OF(mode, profile->mode) + 1);
            vdprintf(fifo, format, va_args);
        }
    }
}

void nrprintf(int nodenumber, const char* format, ...){
    if(nodenumber >= OUTFIFO_MAX)
        return;
    va_list va_args;
    int fifo;
    if(nodenumber >= 0){
        // If node number was given, print to that node (if open)
        if((fifo = keyboard[0].outfifo[nodenumber])){
            va_start(va_args, format);
            vdprintf(fifo, format, va_args);
        }
        return;
    }
    // Otherwise, print to all nodes
    for(int i = 0; i < OUTFIFO_MAX; i++){
        if((fifo = keyboard[0].outfifo[i])){
            va_start(va_args, format);
            vdprintf(fifo, format, va_args);
        }
    }
}

void notifyconnect(usbdevice* kb, int connecting){
    int index = INDEX_OF(kb, keyboard);
    nrprintf(-1, "device %s %s %s%d\n", kb->profile.serial, connecting ? "added at" : "removed from", devpath, index);
}

void cmd_notify(usbmode* mode, const key* keymap, int nnumber, int keyindex, const char* toggle){
    if(!strcmp(toggle, "on") || *toggle == 0)
        SET_KEYBIT(mode->notify[nnumber], keyindex);
    else if(!strcmp(toggle, "off"))
        CLEAR_KEYBIT(mode->notify[nnumber], keyindex);
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

void getinfo(usbdevice* kb, usbmode* mode, int nnumber, const char* setting){
    if(!strcmp(setting, ":hello")){
        if(kb && mode)
            return;
        nrprintf(nnumber, "hello\n");
        return;
    } else if(!strcmp(setting, ":fps")){
        if(kb && mode)
            return;
        nrprintf(nnumber, "fps %d\n", fps);
        return;
    } else if(!strcmp(setting, ":layout")){
        if(kb && mode)
            nprintf(kb, nnumber, 0, "layout %s\n", getmapname(kb->profile.keymap));
        else
            nrprintf(nnumber, "layout %s\n", getmapname(keymap_system));
        return;
    } else if(!kb || !mode)
        // Only FPS and layout can be printed without an active mode
        return;

    usbprofile* profile = &kb->profile;
    if(!strcmp(setting, ":mode")){
        // Get the current mode number
        nprintf(kb, nnumber, mode, "switch\n");
        return;
    } else if(!strcmp(setting, ":rgb")){
        // Get the current RGB settings
        char* rgb = printrgb(&mode->light, profile->keymap);
        nprintf(kb, nnumber, mode, "rgb %s\n", rgb);
        free(rgb);
        return;
    } else if(!strcmp(setting, ":rgbon")){
        // Get the current RGB status
        if(mode->light.enabled)
            nprintf(kb, nnumber, mode, "rgb on\n");
        else
            nprintf(kb, nnumber, mode, "rgb off\n");
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
        nprintf(kb, nnumber, mode, "hwrgb %s\n", rgb);
        free(rgb);
        return;
    } else if(!strcmp(setting, ":profilename")){
        // Get the current profile name
        char* name = getprofilename(profile);
        nprintf(kb, nnumber, 0, "profilename %s\n", name[0] ? name : "Unnamed");
        free(name);
    } else if(!strcmp(setting, ":name")){
        // Get the current mode name
        char* name = getmodename(mode);
        nprintf(kb, nnumber, mode, "name %s\n", name[0] ? name : "Unnamed");
        free(name);
    } else if(!strcmp(setting, ":hwprofilename")){
        // Get the current hardware profile name
        if(!kb->hw)
            return;
        char* name = gethwprofilename(kb->hw);
        nprintf(kb, nnumber, 0, "hwprofilename %s\n", name[0] ? name : "Unnamed");
        free(name);
    } else if(!strcmp(setting, ":hwname")){
        // Get the current hardware mode name
        if(!kb->hw)
            return;
        unsigned index = INDEX_OF(mode, profile->mode);
        HWMODE_OR_RETURN(kb, index);
        char* name = gethwmodename(kb->hw, index);
        nprintf(kb, nnumber, mode, "hwname %s\n", name[0] ? name : "Unnamed");
        free(name);
    } else if(!strcmp(setting, ":profileid")){
        // Get the current profile ID
        char* guid = getid(&profile->id);
        int modified;
        memcpy(&modified, &profile->id.modified, sizeof(modified));
        nprintf(kb, nnumber, 0, "profileid %s %x\n", guid, modified);
        free(guid);
    } else if(!strcmp(setting, ":id")){
        // Get the current mode ID
        char* guid = getid(&mode->id);
        int modified;
        memcpy(&modified, &mode->id.modified, sizeof(modified));
        nprintf(kb, nnumber, mode, "id %s %x\n", guid, modified);
        free(guid);
    } else if(!strcmp(setting, ":hwprofileid")){
        // Get the current hardware profile ID
        if(!kb->hw)
            return;
        char* guid = getid(&kb->hw->id[0]);
        int modified;
        memcpy(&modified, &kb->hw->id[0].modified, sizeof(modified));
        nprintf(kb, nnumber, 0, "hwprofileid %s %x\n", guid, modified);
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
        nprintf(kb, nnumber, mode, "hwid %s %x\n", guid, modified);
        free(guid);
    }
}

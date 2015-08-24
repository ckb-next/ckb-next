#include "device.h"
#include "devnode.h"
#include "dpi.h"
#include "led.h"
#include "notify.h"
#include "profile.h"

void nprintf(usbdevice* kb, int nodenumber, usbmode* mode, const char* format, ...){
    if(!kb)
        return;
    usbprofile* profile = kb->profile;
    va_list va_args;
    int fifo;
    if(nodenumber >= 0){
        // If node number was given, print to that node (if open)
        if((fifo = kb->outfifo[nodenumber] - 1) != -1){
            va_start(va_args, format);
            if(mode)
                dprintf(fifo, "mode %d ", INDEX_OF(mode, profile->mode) + 1);
            vdprintf(fifo, format, va_args);
        }
        return;
    }
    // Otherwise, print to all nodes
    for(int i = 0; i < OUTFIFO_MAX; i++){
        if((fifo = kb->outfifo[i] - 1) != -1){
            va_start(va_args, format);
            if(mode)
                dprintf(fifo, "mode %d ", INDEX_OF(mode, profile->mode) + 1);
            vdprintf(fifo, format, va_args);
        }
    }
}

void nprintkey(usbdevice* kb, int nnumber, int keyindex, int down){
    const key* map = keymap + keyindex;
    if(map->name)
        nprintf(kb, nnumber, 0, "key %c%s\n", down ? '+' : '-', map->name);
    else
        nprintf(kb, nnumber, 0, "key %c#%d\n", down ? '+' : '-', keyindex);
}

void nprintind(usbdevice* kb, int nnumber, int led, int on){
    const char* name = 0;
    switch(led){
    case I_NUM:
        name = "num";
        break;
    case I_CAPS:
        name = "caps";
        break;
    case I_SCROLL:
        name = "scroll";
        break;
    default:
        return;
    }
    nprintf(kb, nnumber, 0, "i %c%s\n", on ? '+' : '-', name);
}

void cmd_notify(usbdevice* kb, usbmode* mode, int nnumber, int keyindex, const char* toggle){
    if(keyindex >= N_KEYS_INPUT)
        return;
    pthread_mutex_lock(imutex(kb));
    if(!strcmp(toggle, "on") || *toggle == 0)
        SET_KEYBIT(mode->notify[nnumber], keyindex);
    else if(!strcmp(toggle, "off"))
        CLEAR_KEYBIT(mode->notify[nnumber], keyindex);
    pthread_mutex_unlock(imutex(kb));
}

// Check hardware mode, bail out if it doesn't exist
#define HWMODE_OR_RETURN(kb, index) \
    if(IS_K95(kb)){                 \
        if((index) >= HWMODE_K95)   \
            return;                 \
    } else {                        \
        if((index) >= HWMODE_K70)   \
            return;                 \
    }

// Standard check for hardware variables: get index, bail if index is too high or no HW profile
#define HW_STANDARD                                 \
    if(!kb->hw)                                     \
        return;                                     \
    unsigned index = INDEX_OF(mode, profile->mode); \
    /* Make sure the mode number is valid */        \
    HWMODE_OR_RETURN(kb, index)

static void _cmd_get(usbdevice* kb, usbmode* mode, int nnumber, const char* setting){
    usbprofile* profile = kb->profile;
    if(!strcmp(setting, ":mode")){
        // Get the current mode number
        nprintf(kb, nnumber, mode, "switch\n");
        return;
    } else if(!strcmp(setting, ":rgb")){
        // Get the current RGB settings
        char* rgb = printrgb(&mode->light, kb);
        nprintf(kb, nnumber, mode, "rgb %s\n", rgb);
        free(rgb);
        return;
    } else if(!strcmp(setting, ":hwrgb")){
        // Get the current hardware RGB settings
        HW_STANDARD;
        char* rgb = printrgb(kb->hw->light + index, kb);
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
        HW_STANDARD;
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
        HW_STANDARD;
        char* guid = getid(&kb->hw->id[index + 1]);
        int modified;
        memcpy(&modified, &kb->hw->id[index + 1].modified, sizeof(modified));
        nprintf(kb, nnumber, mode, "hwid %s %x\n", guid, modified);
        free(guid);
    } else if(!strcmp(setting, ":keys")){
        // Get the current state of all keys
        for(int i = 0; i < N_KEYS_INPUT; i++){
            if(!keymap[i].name)
                continue;
            int byte = i / 8, bit = 1 << (i & 7);
            uchar state = kb->input.keys[byte] & bit;
            if(state)
                nprintkey(kb, nnumber, i, 1);
        }
    } else if(!strcmp(setting, ":i")){
        // Get the current state of all indicator LEDs
        if(kb->hw_ileds & I_NUM) nprintind(kb, nnumber, I_NUM, 1);
        if(kb->hw_ileds & I_CAPS) nprintind(kb, nnumber, I_CAPS, 1);
        if(kb->hw_ileds & I_SCROLL) nprintind(kb, nnumber, I_SCROLL, 1);
    } else if(!strcmp(setting, ":dpi")){
        // Get the current DPI levels
        char* dpi = printdpi(&mode->dpi, kb);
        nprintf(kb, nnumber, mode, "dpi %s\n", dpi);
        free(dpi);
        return;
    } else if(!strcmp(setting, ":hwdpi")){
        // Get the current hardware DPI levels
        HW_STANDARD;
        char* dpi = printdpi(kb->hw->dpi + index, kb);
        nprintf(kb, nnumber, mode, "hwdpi %s\n", dpi);
        free(dpi);
        return;
    } else if(!strcmp(setting, ":dpisel")){
        // Get the currently-selected DPI
        nprintf(kb, nnumber, mode, "dpisel %d\n", mode->dpi.current);
    } else if(!strcmp(setting, ":hwdpisel")){
        // Get the currently-selected hardware DPI
        HW_STANDARD;
        nprintf(kb, nnumber, mode, "hwdpisel %d\n", kb->hw->dpi[index].current);
    } else if(!strcmp(setting, ":lift")){
        // Get the mouse lift height
        nprintf(kb, nnumber, mode, "lift %d\n", mode->dpi.lift);
    } else if(!strcmp(setting, ":hwlift")){
        // Get the hardware lift height
        HW_STANDARD;
        nprintf(kb, nnumber, mode, "hwlift %d\n", kb->hw->dpi[index].lift);
    } else if(!strcmp(setting, ":snap")){
        // Get the angle snap status
        nprintf(kb, nnumber, mode, "snap %s\n", mode->dpi.snap ? "on" : "off");
    } else if(!strcmp(setting, ":hwsnap")){
        // Get the hardware angle snap status
        HW_STANDARD;
        nprintf(kb, nnumber, mode, "hwsnap %s\n", kb->hw->dpi[index].snap ? "on" : "off");
    }
}

void cmd_get(usbdevice* kb, usbmode* mode, int nnumber, int dummy, const char* setting){
    pthread_mutex_lock(imutex(kb));
    _cmd_get(kb, mode, nnumber, setting);
    pthread_mutex_unlock(imutex(kb));
}

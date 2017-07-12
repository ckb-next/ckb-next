#include "command.h"
#include "led.h"
#include "profile.h"
#include "usb.h"

void cmd_rgb(usbdevice* kb, usbmode* mode, int dummy, int keyindex, const char* code){
    int index = keymap[keyindex].led;
    if(index < 0) {
        if (index == -2){     // Process strafe sidelights
            uchar sideshine;
            if (sscanf(code, "%2hhx",&sideshine)) // monochromatic
                mode->light.sidelight = sideshine;
        }
        return;
    }
    uchar r, g, b;
    if(sscanf(code, "%2hhx%2hhx%2hhx", &r, &g, &b) == 3){
        mode->light.r[index] = r;
        mode->light.g[index] = g;
        mode->light.b[index] = b;
    }
}

// Indicator bitfield from string
static uchar iselect(const char* led){
    int result = 0;
    if(!strncmp(led, "num", 3) || strstr(led, ",num"))
        result |= I_NUM;
    if(!strncmp(led, "caps", 4) || strstr(led, ",caps"))
        result |= I_CAPS;
    if(!strncmp(led, "scroll", 6) || strstr(led, ",scroll"))
        result |= I_SCROLL;
    if(!strncmp(led, "all", 3) || strstr(led, ",all"))
        result |= I_NUM | I_CAPS | I_SCROLL;
    return result;
}

void cmd_ioff(usbdevice* kb, usbmode* mode, int dummy1, int dummy2, const char* led){
    uchar bits = iselect(led);
    // Add the bits to ioff, remove them from ion
    mode->ioff |= bits;
    mode->ion &= ~bits;
    kb->vtable->updateindicators(kb, 0);
}

void cmd_ion(usbdevice* kb, usbmode* mode, int dummy1, int dummy2, const char* led){
    uchar bits = iselect(led);
    // Remove the bits from ioff, add them to ion
    mode->ioff &= ~bits;
    mode->ion |= bits;
    kb->vtable->updateindicators(kb, 0);
}

void cmd_iauto(usbdevice* kb, usbmode* mode, int dummy1, int dummy2, const char* led){
    uchar bits = iselect(led);
    // Remove the bits from both ioff and ion
    mode->ioff &= ~bits;
    mode->ion &= ~bits;
    kb->vtable->updateindicators(kb, 0);
}

void cmd_inotify(usbdevice* kb, usbmode* mode, int nnumber, int dummy, const char* led){
    uchar bits = iselect(led);
    if(strstr(led, ":off"))
        // Turn notifications for these bits off
        mode->inotify[nnumber] &= ~bits;
    else
        // Turn notifications for these bits on
        mode->inotify[nnumber] |= bits;
}

// Does a key exist in the current LED layout?
static int has_key(const char* name, const usbdevice* kb){
    if(!name)
        return 0;
    if(IS_MOUSE(kb->vendor, kb->product)){
        // Mice only have the RGB zones
        if((IS_SABRE(kb) || IS_SCIMITAR(kb)) && !strcmp(name, "wheel"))
            return 1;
        if(IS_SCIMITAR(kb) && !strcmp(name, "thumb"))
            return 1;
        if(strstr(name, "dpi") == name || !strcmp(name, "front") || !strcmp(name, "back"))
            return 1;
        return 0;
    } else {
        // But keyboards don't have them at all
        if(strstr(name, "dpi") == name || !strcmp(name, "front") || !strcmp(name, "back") || !strcmp(name, "wheel") || !strcmp(name, "thumb"))
            return 0;
        // Only K95 has G keys and M keys (G1 - G18, MR, M1 - M3)
        if(!IS_K95(kb) && ((name[0] == 'g' && name[1] >= '1' && name[1] <= '9') || (name[0] == 'm' && (name[1] == 'r' || name[1] == '1' || name[1] == '2' || name[1] == '3'))))
            return 0;
        // Only K65 has lights on VolUp/VolDn
        if(!IS_K65(kb) && (!strcmp(name, "volup") || !strcmp(name, "voldn")))
            return 0;
        // K65 lacks numpad and media buttons
        if(IS_K65(kb) && (strstr(name, "num") == name || !strcmp(name, "stop") || !strcmp(name, "prev") || !strcmp(name, "play") || !strcmp(name, "next")))
            return 0;
    }
    return 1;
}

char* printrgb(const lighting* light, const usbdevice* kb){
    uchar r[N_KEYS_EXTENDED], g[N_KEYS_EXTENDED], b[N_KEYS_EXTENDED];
    const uchar* mr = light->r;
    const uchar* mg = light->g;
    const uchar* mb = light->b;
    for(int i = 0; i < N_KEYS_EXTENDED; i++){
        // Translate the key index to an RGB index using the key map
        int k = keymap[i].led;
        if(k < 0)
            continue;
        r[i] = mr[k];
        g[i] = mg[k];
        b[i] = mb[k];
    }
    // Make a buffer to track key names and to filter out duplicates
    char names[N_KEYS_EXTENDED][11];
    for(int i = 0; i < N_KEYS_EXTENDED; i++){
        const char* name = keymap[i].name;
        if(keymap[i].led < 0 || !has_key(name, kb))
            names[i][0] = 0;
        else
            strncpy(names[i], name, 11);
    }
    // Check to make sure these aren't all the same color
    int same = 1;
    for(int i = 1; i < N_KEYS_EXTENDED; i++){
        if(!names[i][0])
            continue;
        if(r[i] != r[0] || g[i] != g[0] || b[i] != b[0]){
            same = 0;
            break;
        }
    }
    // If they are, just output that color
    if(same){
        char* buffer = malloc(7);
        snprintf(buffer, 7, "%02x%02x%02x", r[0], g[0], b[0]);
        return buffer;
    }
    const int BUFFER_LEN = 4096;    // Should be more than enough to fit all keys
    char* buffer = malloc(BUFFER_LEN);
    int length = 0;
    for(int i = 0; i < N_KEYS_EXTENDED; i++){
        if(!names[i][0])
            continue;
        // Print the key name
        int newlen = 0;
        snprintf(buffer + length, BUFFER_LEN - length, length == 0 ? "%s%n" : " %s%n", names[i], &newlen);
        length += newlen;
        // Look ahead to see if any other keys have this color. If so, print them here as well.
        uchar kr = r[i], kg = g[i], kb = b[i];
        for(int j = i + 1; j < N_KEYS_EXTENDED; j++){
            if(!names[j][0])
                continue;
            if(r[j] != kr || g[j] != kg || b[j] != kb)
                continue;
            snprintf(buffer + length, BUFFER_LEN - length, ",%s%n", names[j], &newlen);
            length += newlen;
            // Erase the key's name so it won't get printed later
            names[j][0] = 0;
        }
        // Print the color
        snprintf(buffer + length, BUFFER_LEN - length, ":%02x%02x%02x%n", kr, kg, kb, &newlen);
        length += newlen;
    }
    return buffer;
}

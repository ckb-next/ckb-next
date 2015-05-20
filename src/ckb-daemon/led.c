#include "led.h"
#include "profile.h"

void cmd_rgb(usbdevice* kb, usbmode* mode, int dummy, int keyindex, const char* code){
    int index = keymap[keyindex].led;
    if(index < 0)
        return;
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
}

void cmd_ion(usbdevice* kb, usbmode* mode, int dummy1, int dummy2, const char* led){
    uchar bits = iselect(led);
    // Remove the bits from ioff, add them to ion
    mode->ioff &= ~bits;
    mode->ion |= bits;
}

void cmd_iauto(usbdevice* kb, usbmode* mode, int dummy1, int dummy2, const char* led){
    uchar bits = iselect(led);
    // Remove the bits from both ioff and ion
    mode->ioff &= ~bits;
    mode->ion &= ~bits;
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

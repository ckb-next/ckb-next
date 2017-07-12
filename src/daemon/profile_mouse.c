#include "dpi.h"
#include "profile.h"
#include "usb.h"
#include "led.h"

int cmd_hwload_mouse(usbdevice* kb, usbmode* dummy1, int dummy2, int apply, const char* dummy3){
    DELAY_LONG(kb);
    hwprofile* hw = calloc(1, sizeof(hwprofile));
    // Ask for profile and mode IDs
    uchar data_pkt[2][MSG_SIZE] = {
        { 0x0e, 0x15, 0x01, 0 },
        { 0x0e, 0x16, 0x01, 0 }
    };
    uchar in_pkt[MSG_SIZE];
    for(int i = 0; i <= 1; i++){
        data_pkt[0][3] = i;
        if(!usbrecv(kb, data_pkt[0], in_pkt)){
            free(hw);
            return -1;
        }
        memcpy(hw->id + i, in_pkt + 4, sizeof(usbid));
    }
    // Ask for profile and mode names
    for(int i = 0; i <= 1; i++){
        data_pkt[1][3] = i;
        if(!usbrecv(kb, data_pkt[1],in_pkt)){
            free(hw);
            return -1;
        }
        memcpy(hw->name[i], in_pkt + 4, PR_NAME_LEN * 2);
    }

    // Load the RGB and DPI settings
    if(loadrgb_mouse(kb, hw->light, 0)
            || loaddpi(kb, hw->dpi, hw->light)){
        free(hw);
        return -1;
    }

    // Make the profile active (if requested)
    if(apply)
        hwtonative(kb->profile, hw, 1);
    // Free the existing profile (if any)
    free(kb->hw);
    kb->hw = hw;
    DELAY_LONG(kb);
    return 0;
}

int cmd_hwsave_mouse(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4){
    DELAY_LONG(kb);
    hwprofile* hw = kb->hw;
    if(!hw)
        hw = kb->hw = calloc(1, sizeof(hwprofile));
    nativetohw(kb->profile, hw, 1);
    // Save the profile and mode names
    uchar data_pkt[2][MSG_SIZE] = {
        { 0x07, 0x16, 0x01, 0 },
        { 0x07, 0x15, 0x01, 0 },
    };
    for(int i = 0; i <= 1; i++){
        data_pkt[0][3] = i;
        memcpy(data_pkt[0] + 4, hw->name[i], MD_NAME_LEN * 2);
        if(!usbsend(kb, data_pkt[0], 1))
            return -1;
    }
    // Save the IDs
    for(int i = 0; i <= 1; i++){
        data_pkt[1][3] = i;
        memcpy(data_pkt[1] + 4, hw->id + i, sizeof(usbid));
        if(!usbsend(kb, data_pkt[1], 1))
            return -1;
    }
    // Save the RGB data for the non-DPI zones
    if(savergb_mouse(kb, hw->light, 0))
        return -1;
    // Save the DPI data (also saves RGB for those states)
    if(savedpi(kb, hw->dpi, hw->light))
        return -1;
    DELAY_LONG(kb);
    return 0;
}

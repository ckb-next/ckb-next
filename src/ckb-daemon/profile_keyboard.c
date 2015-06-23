#include "profile.h"
#include "usb.h"
#include "led.h"

static int hwloadmode(usbdevice* kb, hwprofile* hw, int mode){
    // Ask for mode's name
    uchar data_pkt[MSG_SIZE] = { 0x0e, 0x16, 0x01, mode + 1, 0 };
    uchar in_pkt[MSG_SIZE];
    if(!usbrecv(kb, data_pkt, in_pkt))
        return -1;
    memcpy(hw->name[mode + 1], in_pkt + 4, MD_NAME_LEN * 2);
    // Load the RGB setting
    return loadrgb_kb(kb, hw->light + mode, mode);
}

int cmd_hwload_kb(usbdevice* kb, usbmode* dummy1, int dummy2, int apply, const char* dummy3){
    DELAY_LONG(kb);
    hwprofile* hw = calloc(1, sizeof(hwprofile));
    // Ask for profile and mode IDs
    uchar data_pkt[2][MSG_SIZE] = {
        { 0x0e, 0x15, 0x01, 0 },
        { 0x0e, 0x16, 0x01, 0 }
    };
    uchar in_pkt[MSG_SIZE];
    int modes = (IS_K95(kb) ? HWMODE_K95 : HWMODE_K70);
    for(int i = 0; i <= modes; i++){
        data_pkt[0][3] = i;
        if(!usbrecv(kb, data_pkt[0], in_pkt)){
            free(hw);
            return -1;
        }
        memcpy(hw->id + i, in_pkt + 4, sizeof(usbid));
    }
    // Ask for profile name
    if(!usbrecv(kb, data_pkt[1], in_pkt)){
        free(hw);
        return -1;
    }
    memcpy(hw->name[0], in_pkt + 4, PR_NAME_LEN * 2);
    // Load modes
    for(int i = 0; i < modes; i++){
        if(hwloadmode(kb, hw, i)){
            free(hw);
            return -1;
        }
    }
    // Make the profile active (if requested)
    if(apply)
        hwtonative(kb->profile, hw, modes);
    // Free the existing profile (if any)
    free(kb->hw);
    kb->hw = hw;
    DELAY_LONG(kb);
    return 0;
}

int cmd_hwsave_kb(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4){
    DELAY_LONG(kb);
    hwprofile* hw = kb->hw;
    if(!hw)
        hw = kb->hw = calloc(1, sizeof(hwprofile));
    int modes = (IS_K95(kb) ? HWMODE_K95 : HWMODE_K70);
    nativetohw(kb->profile, hw, modes);
    // Save the profile and mode names
    uchar data_pkt[2][MSG_SIZE] = {
        { 0x07, 0x16, 0x01, 0 },
        { 0x07, 0x15, 0x01, 0 },
    };
    // Save the mode names
    for(int i = 0; i <= modes; i++){
        data_pkt[0][3] = i;
        memcpy(data_pkt[0] + 4, hw->name[i], MD_NAME_LEN * 2);
        if(!usbsend(kb, data_pkt[0], 1))
            return -1;
    }
    // Save the IDs
    for(int i = 0; i <= modes; i++){
        data_pkt[1][3] = i;
        memcpy(data_pkt[1] + 4, hw->id + i, sizeof(usbid));
        if(!usbsend(kb, data_pkt[1], 1))
            return -1;
    }
    // Save the RGB data
    for(int i = 0; i < modes; i++){
        if(savergb_kb(kb, hw->light + i, i))
            return -1;
    }
    DELAY_LONG(kb);
    return 0;
}

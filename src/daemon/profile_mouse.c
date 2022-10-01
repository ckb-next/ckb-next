#include "dpi.h"
#include "profile.h"
#include "usb.h"
#include "led.h"
#include "nxp_proto.h"

#define HWLOAD_ERR_RET() { kb->usbdelay_ns = delay; free(hw); return -1; }
int cmd_hwload_mouse(usbdevice* kb, usbmode* dummy1, int dummy2, int apply, const char* dummy3){
    (void)dummy1;
    (void)dummy2;
    (void)dummy3;

    long delay = kb->usbdelay_ns;
    // Ensure delay of 10ms as the device can get overwhelmed otherwise
    kb->usbdelay_ns = 10000000L;

    DELAY_100MS();
    hwprofile* hw = calloc(1, sizeof(hwprofile));
    // Ask for profile and mode IDs
    uchar data_pkt[2][MSG_SIZE] = {
        { CMD_GET, FIELD_M_PROFID, 0x01, 0 },
        { CMD_GET, FIELD_M_PROFNM, 0x01, 0 }
    };
    uchar in_pkt[MSG_SIZE];
    for(int i = 0; i <= 1; i++){
        data_pkt[0][3] = i;
        if(!usbrecv(kb, data_pkt[0], MSG_SIZE, in_pkt))
            HWLOAD_ERR_RET();

        memcpy(hw->id + i, in_pkt + 4, sizeof(usbid));
    }
    // Ask for profile and mode names
    for(int i = 0; i <= 1; i++){
        data_pkt[1][3] = i;
        if(!usbrecv(kb, data_pkt[1], MSG_SIZE, in_pkt))
            HWLOAD_ERR_RET();
        memcpy(hw->name[i], in_pkt + 4, PR_NAME_LEN * 2);
    }

    // Load the RGB and DPI settings
    if(loadrgb_mouse(kb, hw->light, 0)
            || loaddpi(kb, hw->dpi, hw->light))
        HWLOAD_ERR_RET();

    // Make the profile active (if requested)
    if(apply)
        hwtonative(kb->profile, hw, 1);

    // Free the existing profile (if any)
    free(kb->hw);
    kb->hw = hw;
    DELAY_100MS();
    kb->usbdelay_ns = delay;
    return 0;
}

#define HWSAVE_RET(e) { kb->usbdelay_ns = delay; return e; }
int cmd_hwsave_mouse(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4){
    (void)dummy1;
    (void)dummy2;
    (void)dummy3;
    (void)dummy4;

    long delay = kb->usbdelay_ns;
    // Ensure delay of 10ms as the device can get overwhelmed otherwise
    kb->usbdelay_ns = 10000000L;

    DELAY_100MS();
    hwprofile* hw = kb->hw;
    if(!hw)
        hw = kb->hw = calloc(1, sizeof(hwprofile));
    nativetohw(kb->profile, hw, 1);
    // Save the profile and mode names
    uchar data_pkt[2][MSG_SIZE] = {
        { CMD_SET, FIELD_M_PROFNM, 0x01, 0 },
        { CMD_SET, FIELD_M_PROFID, 0x01, 0 },
    };
    for(int i = 0; i <= 1; i++){
        data_pkt[0][3] = i;
        memcpy(data_pkt[0] + 4, hw->name[i], MD_NAME_LEN * 2);
        if(!usbsend(kb, data_pkt[0], MSG_SIZE, 1))
            HWSAVE_RET(-1);
    }
    // Save the IDs
    for(int i = 0; i <= 1; i++){
        data_pkt[1][3] = i;
        memcpy(data_pkt[1] + 4, hw->id + i, sizeof(usbid));
        if(!usbsend(kb, data_pkt[1], MSG_SIZE, 1))
            HWSAVE_RET(-1);
    }
    // Save the RGB data for the non-DPI zones
    if(savergb_mouse(kb, hw->light, 0))
        HWSAVE_RET(-1);
    // Save the DPI data (also saves RGB for those states)
    if(savedpi(kb, hw->dpi, hw->light))
        HWSAVE_RET(-1);
    DELAY_100MS();
    HWSAVE_RET(0);
}

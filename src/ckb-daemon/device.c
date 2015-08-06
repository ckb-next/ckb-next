#include "command.h"
#include "device.h"
#include "firmware.h"
#include "profile.h"
#include "usb.h"

int hwload_mode = 1;

// Device list
usbdevice keyboard[DEV_MAX];
pthread_mutex_t devlistmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t devmutex[DEV_MAX] = { [0 ... DEV_MAX-1] = PTHREAD_MUTEX_INITIALIZER };
pthread_mutex_t inputmutex[DEV_MAX] = { [0 ... DEV_MAX-1] = PTHREAD_MUTEX_INITIALIZER };

int start_dev(usbdevice* kb, int makeactive){
    // Get the firmware version from the device
    if(kb->pollrate == 0){
        if(!hwload_mode || (HAS_FEATURES(kb, FEAT_HWLOAD) && getfwversion(kb))){
            if(hwload_mode == 2)
                // hwload=always. Report setup failure.
                return -1;
            else if(hwload_mode){
                // hwload=once. Log failure, prevent trying again, and continue.
                ckb_warn("Unable to load firmware version/poll rate\n");
                kb->features &= ~FEAT_HWLOAD;
            }
            kb->pollrate = 0;
            kb->features &= ~(FEAT_POLLRATE | FEAT_ADJRATE);
            if(kb->fwversion == 0)
                kb->features &= ~(FEAT_FWVERSION | FEAT_FWUPDATE);
        }
    }
    if(NEEDS_FW_UPDATE(kb)){
        // Device needs a firmware update. Finish setting up but don't do anything.
        ckb_info("Device needs a firmware update. Please issue a fwupdate command.\n");
        kb->features = FEAT_RGB | FEAT_FWVERSION | FEAT_FWUPDATE;
        kb->active = 1;
        return 0;
    }
    // Load profile from device
    if(!kb->hw && hwload_mode && HAS_FEATURES(kb, FEAT_HWLOAD)){
        if(hwloadprofile(kb, 1)){
            if(hwload_mode == 2)
                return -1;
            ckb_warn("Unable to load hardware profile\n");
            kb->features &= ~FEAT_HWLOAD;
        }
    }
    // Active software mode if requested
    if(makeactive)
        return setactive(kb, 1);
    return 0;
}

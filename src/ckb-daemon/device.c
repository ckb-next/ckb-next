#include "command.h"
#include "device.h"
#include "firmware.h"
#include "profile.h"
#include "usb.h"

// Device list
usbdevice keyboard[DEV_MAX];
pthread_mutex_t devlistmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t devmutex[DEV_MAX] = { [0 ... DEV_MAX-1] = PTHREAD_MUTEX_INITIALIZER };
pthread_mutex_t inputmutex[DEV_MAX] = { [0 ... DEV_MAX-1] = PTHREAD_MUTEX_INITIALIZER };

int start_dev(usbdevice* kb, int makeactive){
    // Get the firmware version from the device
    if(getfwversion(kb))
        return -1;
    if(NEEDS_FW_UPDATE(kb)){
        // Device needs a firmware update. Finish setting up but don't do anything.
        ckb_info("Device needs a firmware update. Please issue a fwupdate command.\n");
        kb->features = FEAT_RGB | FEAT_FWVERSION | FEAT_FWUPDATE;
        kb->active = 1;
        return 0;
    }
    // Load profile from device
    if(hwloadprofile(kb, 1))
        return -2;
    // Active software mode if requested
    if(makeactive)
        return setactive(kb, 1);
    return 0;
}

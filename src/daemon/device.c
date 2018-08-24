#include "command.h"
#include "device.h"
#include "firmware.h"
#include "profile.h"
#include "usb.h"

int hwload_mode = 1;        ///< hwload_mode = 1 means read hardware once. should be enough

// Device list
usbdevice keyboard[DEV_MAX];    ///< remember all usb devices. Needed for closeusb().
pthread_mutex_t devlistmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t devmutex[DEV_MAX] = { [0 ... DEV_MAX-1] = PTHREAD_MUTEX_INITIALIZER };      ///< Mutex for handling the usbdevice structure
pthread_mutex_t inputmutex[DEV_MAX] = { [0 ... DEV_MAX-1] = PTHREAD_MUTEX_INITIALIZER };    ///< Mutex for dealing with usb input frames
pthread_mutex_t macromutex[DEV_MAX] = { [0 ... DEV_MAX-1] = PTHREAD_MUTEX_INITIALIZER };    ///< Protecting macros against lightning: Both use usb_send
pthread_mutex_t macromutex2[DEV_MAX] = { [0 ... DEV_MAX-1] = PTHREAD_MUTEX_INITIALIZER };   ///< Protecting the single link list of threads and the macrovar
pthread_cond_t macrovar[DEV_MAX] = { [0 ... DEV_MAX-1] = PTHREAD_COND_INITIALIZER };        ///< This variable is used to stop and wakeup all macro threads which have to wait.
pthread_mutex_t interruptmutex[DEV_MAX] = { [0 ... DEV_MAX-1] = PTHREAD_MUTEX_INITIALIZER };///< Used for interrupt transfers
pthread_cond_t interruptcond[DEV_MAX] = { [0 ... DEV_MAX-1] = PTHREAD_COND_INITIALIZER };   ///< Same as above

/// \brief .
///
/// \brief _start_dev get fw-info and pollrate; if available, install new firmware; get all hardware profiles.
/// \param kb   the normal kb pointer to the usbdevice. Is also valid for mice.
/// \param makeactive if set to 1, activate the device via setactive()
/// \return 0 if success, other else
///
int _start_dev(usbdevice* kb, int makeactive){
    // Get the firmware version from the device
    if(kb->pollrate == 0){
        ///
        /// - This hacker code is tricky in mutliple aspects. What it means is:
        /// \n if hwload_mode == 0: just set pollrate to 0 and clear features in the bottom lines of the if-block.
        /// \n if hwload_mode == 1: if the device has FEAT_HWLOAD active, call getfwversion(). If it returns true, there was an error while detecting fw-version. Put error message, reset FEAT_HWLOAD and finalize as above.
        /// \n if hwload_mode == 2: if the device has FEAT_HWLOAD active, call getfwversion(). If it returns true, there was an error while detecting fw-version. Put error message and return directly from function with error.
        /// \n Why do not you just write it down?
        ///
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
    ///
    /// SINGLE EP devices do not have any input, thus do not supporting binding.
    ///
    if(IS_SINGLE_EP(kb))
        kb->features &= ~FEAT_BIND;
    
    ///
    /// The Polaris doesn't support hardware profiles, so remove the FEAT_HWLOAD bit.
    ///
    if(IS_POLARIS(kb))
        kb->features &= ~FEAT_HWLOAD;

    ///
    /// hwload isn't supported yet on this hardware format.
    ///
    if(USES_FILE_HWSAVE(kb))
        kb->features &= ~FEAT_HWLOAD;
    
    ///
    /// K66 has no backlight
    ///
    if(HAS_NO_LIGHTS(kb)) {
        kb->features &= ~FEAT_RGB;
        kb->features &= ~FEAT_HWLOAD; // no LED data to read
    }

    ///
    /// - Now check if device needs a firmware update.
    /// If so, set it up and leave the function without error.
    ///
    if(NEEDS_FW_UPDATE(kb)){
        /// - Device needs a firmware update. Finish setting up but don't do anything.
        ckb_info("Device needs a firmware update. Please issue a fwupdate command.\n");
        kb->features = FEAT_RGB | FEAT_FWVERSION | FEAT_FWUPDATE;
        kb->active = 1;
        return 0;
    }
    ///
    /// - Load profile from device if the hw-pointer is not set yet and hw-loading is possible and allowed.
    /// \n return error if mode == 2 (load always) and loading got an error. Else reset HWLOAD feature, because hwload must be 1.
    /// \n\n That is real Horror code.
    ///
    if(!kb->hw && hwload_mode && HAS_FEATURES(kb, FEAT_HWLOAD)){
        if(hwloadprofile(kb, 1)){
            if(hwload_mode == 2)
                return -1;
            ckb_warn("Unable to load hardware profile\n");
            kb->features &= ~FEAT_HWLOAD;
        }
    }
    // Activate software mode if requested
    if(makeactive)
        return setactive(kb, 1);
    #ifdef DEBUG
    // 12 for each device + null terminator
    char devlist[12*(DEV_MAX-1)+1];
    int devlistpos = 0;
    for(unsigned i = 1; i < DEV_MAX; i++){
        devlistpos += sprintf(&devlist[devlistpos], "%u: 0x%x; ", i, keyboard[i].product);
    }
    ckb_info("Attached Devices: %s\n", devlist);
    #endif
    return 0;
}

int start_dev(usbdevice* kb, int makeactive){
    // Force USB interval to 10ms during initial setup phase; return to nominal 5ms after setup completes.
    kb->usbdelay = 10;
    int res = _start_dev(kb, makeactive);
    kb->usbdelay = USB_DELAY_DEFAULT;
    return res;
}

#include "command.h"
#include "device.h"
#include "firmware.h"
#include "profile.h"
#include "usb.h"
#include "input.h"
#include "nxp_proto.h"

// Device list
usbdevice keyboard[DEV_MAX];    ///< remember all usb devices. Needed for closeusb().
queued_mutex_t devmutex[DEV_MAX] = { [0 ... DEV_MAX-1] = QUEUED_MUTEX_INITIALIZER };        ///< Mutex for handling the usbdevice structure
queued_mutex_t inputmutex[DEV_MAX] = { [0 ... DEV_MAX-1] = QUEUED_MUTEX_INITIALIZER };      ///< Mutex for dealing with usb input frames
queued_mutex_t macromutex[DEV_MAX] = { [0 ... DEV_MAX-1] = QUEUED_MUTEX_INITIALIZER };      ///< Protecting macros against lightning: Both use usb_send
pthread_mutex_t macromutex2[DEV_MAX] = { [0 ... DEV_MAX-1] = PTHREAD_MUTEX_INITIALIZER };   ///< Protecting the single link list of threads and the macrovar
pthread_cond_t macrovar[DEV_MAX] = { [0 ... DEV_MAX-1] = PTHREAD_COND_INITIALIZER };        ///< This variable is used to stop and wakeup all macro threads which have to wait.
pthread_cond_t macroint[DEV_MAX];                                                           ///< Should a macro thread's sleep be interrupted, due to repeated key press?
pthread_mutex_t interruptmutex[DEV_MAX] = { [0 ... DEV_MAX-1] = PTHREAD_MUTEX_INITIALIZER };///< Used for interrupt transfers
pthread_cond_t interruptcond[DEV_MAX];                                                      ///< Same as above
pthread_mutex_t childrenmutex[DEV_MAX] = { [0 ... DEV_MAX-1] = PTHREAD_MUTEX_INITIALIZER };

///
/// \brief cond_nanosleep matches semantics of pthread_cond_timedwait, but with a relative wake time
/// \param cond     as pthread_cond_timedwait, but must use CLOCK_MONOTONIC
/// \param mutex    as pthread_cond_timedwait
/// \param ns       the maximum duration of sleep, in nanoseconds
/// \return         as pthread_cond_timedwait. returns ENOTSUP if clock_gettime fails
///
int cond_nanosleep(pthread_cond_t *restrict cond,
                   pthread_mutex_t *restrict mutex, uint32_t ns) {
    struct timespec ts = { 0 };
#ifdef OS_MAC
    timespec_add(&ts, ns);
    return pthread_cond_timedwait_relative_np(cond, mutex, &ts);
#else
    if(clock_gettime(CLOCK_MONOTONIC, &ts))
        return ENOTSUP;
    timespec_add(&ts, ns);
    return pthread_cond_timedwait(cond, mutex, &ts);
#endif
}

void queued_mutex_lock(queued_mutex_t* mutex){
#ifdef NO_FAIR_MUTEX_QUEUEING
    pthread_mutex_lock(mutex);
#else
    pthread_mutex_lock(&mutex->mutex);
    unsigned long my_turn = mutex->next_waiting++;

    while(my_turn != mutex->next_in)
        pthread_cond_wait(&mutex->cond, &mutex->mutex);

    pthread_mutex_unlock(&mutex->mutex);
#endif
}

int queued_mutex_trylock(queued_mutex_t* mutex){
#ifdef NO_FAIR_MUTEX_QUEUEING
    return pthread_mutex_trylock(mutex);
#else
    int res = 0;
    pthread_mutex_lock(&mutex->mutex);

    if(mutex->next_waiting == mutex->next_in){
        mutex->next_waiting++;
    }
    else{
        res = -1;
    }

    pthread_mutex_unlock(&mutex->mutex);

    return res;
#endif
}

void queued_mutex_unlock(queued_mutex_t* mutex){
#ifdef NO_FAIR_MUTEX_QUEUEING
    pthread_mutex_unlock(mutex);
#else
    pthread_mutex_lock(&mutex->mutex);
    mutex->next_in++;
    pthread_mutex_unlock(&mutex->mutex);
    pthread_cond_broadcast(&mutex->cond);
#endif
}

///
/// \brief queued_cond_nanosleep matches semantics of cond_nanosleep, but accepts a queued_mutex_t instead of a mutex
/// \param cond     as cond_nanosleep
/// \param mutex    as cond_nanosleep, but is a queued_mutex_t
/// \param ns       as cond_nanosleep
/// \return         as queued_cond_timedwait
///
void queued_cond_nanosleep(pthread_cond_t *restrict cond,
                           queued_mutex_t *restrict mutex, const uint32_t ns) {
#ifdef NO_FAIR_MUTEX_QUEUEING
    cond_nanosleep(cond, mutex, ns);
#else
    pthread_mutex_lock(&mutex->mutex);

    // release mutex
    mutex->next_in++;
    pthread_cond_broadcast(&mutex->cond);

    // perform the sleep
    cond_nanosleep(cond, &mutex->mutex, ns);

    // reacquire mutex
    unsigned long my_turn = mutex->next_waiting++;

    while(my_turn != mutex->next_in)
        pthread_cond_wait(&mutex->cond, &mutex->mutex);

    pthread_mutex_unlock(&mutex->mutex);
#endif
}

/// Initialize pthread_cond's with a monotonic clock, if possible
int init_cond_monotonic(void) {
    pthread_condattr_t monotonic_condattr;

    if(pthread_condattr_init(&monotonic_condattr)
#ifndef OS_MAC
        || pthread_condattr_setclock(&monotonic_condattr, CLOCK_MONOTONIC)
#endif
    )
        return 1;

    // pthread_cond_init
    for(int i = 0 ; i < DEV_MAX ; i++) {
        if(pthread_cond_init(&interruptcond[i], &monotonic_condattr) ||
           pthread_cond_init(&macroint[i], &monotonic_condattr))
            return 1;
    }

    pthread_condattr_destroy(&monotonic_condattr);

    return 0;
}

/// \brief .
///
/// \brief _start_dev get fw-info and pollrate; if available, install new firmware; get all hardware profiles.
/// \param kb   the normal kb pointer to the usbdevice. Is also valid for mice.
/// \param makeactive if set to 1, activate the device via setactive()
/// \return 0 if success, other else
///
int _start_dev(usbdevice* kb, int makeactive){
    // Get the firmware version from the device
    if(getfwversion(kb)){
        ckb_warn("Unable to load firmware version/poll rate");
        kb->features &= ~(FEAT_POLLRATE | FEAT_ADJRATE | FEAT_HWLOAD);
        if(!kb->fwversion)
            kb->features &= ~(FEAT_FWVERSION | FEAT_FWUPDATE);
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

    if(kb->product == P_M95)
        kb->features &= ~FEAT_POLLRATE; // M95 doesn't support reading the pollrate through the protocol
    ///
    /// - Now check if device needs a firmware update.
    /// If so, set it up and leave the function without error.
    ///
    if(NEEDS_FW_UPDATE(kb)){
        /// - Device needs a firmware update. Finish setting up but don't do anything.
        ckb_info("Device needs a firmware update. Please issue a fwupdate command.");
        kb->features = FEAT_FWVERSION | FEAT_FWUPDATE;
        kb->active = 1;
        return 0;
    }
    ///
    /// - Load profile from device if the hw-pointer is not set yet and hw-loading is possible and allowed.
    /// \n return error if mode == 2 (load always) and loading got an error. Else reset HWLOAD feature, because hwload must be 1.
    /// \n\n That is real Horror code.
    ///
    if(!kb->hw && HAS_FEATURES(kb, FEAT_HWLOAD)){
        if(hwloadprofile(kb, 1)){
            ckb_warn("Unable to load hardware profile");
            kb->features &= ~FEAT_HWLOAD;
        }
    }
    // Activate software mode if requested
    if(makeactive)
        return setactive(kb, 1);
#ifndef NDEBUG
    // 12 for each device + null terminator
    char devlist[12*(DEV_MAX-1)+1];
    int devlistpos = 0;
    for(unsigned i = 1; i < DEV_MAX; i++){
        devlistpos += sprintf(&devlist[devlistpos], "%u: 0x%x; ", i, keyboard[i].product);
    }
    ckb_info("Attached Devices: %s", devlist);
#endif
    return 0;
}

int start_dev(usbdevice* kb, int makeactive){
    // Force USB interval to 10ms during initial setup phase; return to nominal 5ms after setup completes.
    kb->usbdelay = 10;
    kb->maxpollrate = POLLRATE_1MS;
    kb->pollrate = POLLRATE_UNKNOWN;
    int res = _start_dev(kb, makeactive);
    kb->usbdelay = USB_DELAY_DEFAULT;
    return res;
}

void nxp_reset(usbdevice* kb, usbmode* mode, int dummy1, int dummy2, const char* type){
    uchar pkt[64] = { 0x07, 0x02, 0xff };
    if(!strcmp(type, "apply_fw")){ // Also used to get out of BLD mode
        pkt[2] = 0xf0;
    } else if(!strcmp(type, "isp")) {
        pkt[2] = 0xaa;
    } else if(!strcmp(type, "fast")) {
        pkt[2] = 0x01;
    } else if(!strcmp(type, "medium")) {
        pkt[2] = 0x00;
    } else if(!strcmp(type, "bld")) { // Reboots to bootloader and forces an eeprom wipe
        pkt[2] = 0x03;
    }

    if(pkt[2] != 0xff){
        if(!usbsend(kb, pkt, sizeof(pkt), 1))
            ckb_err("%s reset failed", type);
    }
}

void clear_input_and_rgb(usbdevice* kb, const int active){
    queued_mutex_lock(imutex(kb));
    kb->active = !!active;
    kb->profile->lastlight.forceupdate = 1;
    // Clear input
    memset(&kb->input.keys, 0, sizeof(kb->input.keys));
    inputupdate(kb);
    queued_mutex_unlock(imutex(kb));
}

static const ushort nxp_battery_lut[5] = {
    0,
    15,
    30,
    50,
    100,
};

void nxp_get_battery_info(usbdevice* kb){
    uchar msg[MSG_SIZE] = { CMD_GET, FIELD_BATTERY };
    uchar in[MSG_SIZE] = { 0 };
    if(!usbrecv(kb, msg, sizeof(msg), in)){
        ckb_err("ckb%d: Failed to get battery info. Returning last known state.", INDEX_OF(kb, keyboard));
        return;
    }
    if(in[4] > (sizeof(nxp_battery_lut)/sizeof(*nxp_battery_lut)) - 1){
        ckb_err("ckb%d: Invalid battery level received 0x%hhx.", INDEX_OF(kb, keyboard), in[4]);
        return;
    }
    kb->battery_level = nxp_battery_lut[in[4]];
    kb->battery_status = in[5];
}

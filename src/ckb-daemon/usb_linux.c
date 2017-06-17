#include "device.h"
#include "devnode.h"
#include "input.h"
#include "notify.h"
#include "usb.h"

#ifdef OS_LINUX

/// \details
/// \brief all open usb devices have their system path names here in this array.
#define DEBUG

static char kbsyspath[DEV_MAX][FILENAME_MAX];

////
/// \brief os_usbsend sends a data packet (MSG_SIZE = 64) Bytes long
///
/// os_usbsend has two functions:
/// - if is_recv == false, it tries to send a given MSG_SIZE buffer via the usb interface given with kb.
/// - otherwise a request is sent via the usb device to initiate the receiving of a message from the remote device.
///
/// The functionality for sending distinguishes two cases,
/// depending on the version number of the firmware of the connected device:
/// \n If the firmware is less or equal 1.2, the transmission is done via an ioctl().
/// The ioctl() is given a struct usbdevfs_ctrltransfer, in which the relevant parameters are entered:
///
/// bRequestType | bRequest | wValue | EP | size | Timeout | data
/// ------------ | -------- | ------ | -- | ---- | ------- | ----
/// 0x21 | 0x09 | 0x0200 | endpoint / IF to be addressed from epcount-1 | MSG_SIZE | 5000 (=5ms) | the message buffer pointer
/// Host to Device, Type=Class, Recipient=Interface | 9 = Send data? | specific | last or pre-last device # | 64 | 5000 | out_msg
///
/// \n The ioctl command is USBDEVFS_CONTROL.
///
/// The same constellation is used if the device is requested to send its data (is_recv = true).
///
/// For a more recent firmware and is_recv = false,
/// the ioctl command USBDEVFS_CONTROL is not used
/// (this tells the bus to enter the control mode),
/// but the bulk method is used: USBDEVFS_BULK.
/// This is astonishing, because all of the endpoints are type Interrupt, not bulk.
///
/// Anyhow, forthis purpose a different structure is used for the ioctl() (struct \b usbdevfs_bulktransfer)
/// and this is also initialized differently:
/// \n The length and timeout parameters are given the same values as above.
/// The formal parameter out_msg is also passed as a buffer pointer.
/// For the endpoints, the firmware version is differentiated again:
/// \n For a firmware version between 1.3 and <2.0 endpoint 4 is used,
/// otherwise (it can only be >=2.0) endpoint 3 is used.
///
/// \todo Since the handling of endpoints has already led to problems elsewhere, this implementation is extremely hardware-dependent and critical!
/// \n Eg. the new keyboard K95PLATINUMRGB has a version number significantly less than 2.0 - will it run with this implementation?
///
/// The ioctl() - no matter what type -
/// returns the number of bytes sent.
/// Now comes the usual check:
/// - If the return value is -1 AND the error is a timeout (ETIMEOUT),
/// os_usbsend() will return -1 to indicate that it is probably a recoverable problem and a retry is recommended.
/// - For another negative value or other error identifier OR 0 bytes sent, 0 is returned as a heavy error identifier.
/// - In all other cases, the function returns the number of bytes sent.
///
/// If this is not the entire blocksize (MSG_SIZE bytes),
/// an error message is issued on the standard error channel
/// [warning "Wrote YY bytes (expected 64)"].
///
/// If DEBUG_USB is set during compilation,
/// the number of bytes sent and their representation are logged to the error channel.
///
int os_usbsend(usbdevice* kb, const uchar* out_msg, int is_recv, const char* file, int line) {
    int res;
    if (kb->fwversion >= 0x120 && !is_recv) {
        struct usbdevfs_bulktransfer transfer;
        memset(&transfer, 0, sizeof(transfer));
        transfer.ep = (kb->fwversion >= 0x130 && kb->fwversion < 0x200) ? 4 : 3;
        transfer.len = MSG_SIZE;
        transfer.timeout = 5000;
        transfer.data = (void*)out_msg;
        res = ioctl(kb->handle - 1, USBDEVFS_BULK, &transfer);
    } else {
        struct usbdevfs_ctrltransfer transfer = { 0x21, 0x09, 0x0200, kb->epcount - 1, MSG_SIZE, 5000, (void*)out_msg };
        res = ioctl(kb->handle - 1, USBDEVFS_CONTROL, &transfer);
    }

    if (res <= 0){
        ckb_err_fn(" %s, res = 0x%x\n", file, line, res ? strerror(errno) : "No data written", res);
        if(res == -1 && errno == ETIMEDOUT)
            return -1;
        else
            return 0;
    } else if (res != MSG_SIZE)
        ckb_warn_fn("Wrote %d bytes (expected %d)\n", file, line, res, MSG_SIZE);
#ifdef DEBUG_USB
    char converted[MSG_SIZE*3 + 1];
    for(int i=0;i<MSG_SIZE;i++)
        sprintf(&converted[i*3], "%02x ", out_msg[i]);
    ckb_warn_fn("Sent %s\n", file, line, converted);
#endif
    return res;
}

///
/// \brief os_usbrecv receives a max MSGSIZE long buffer from usb device

/// os_usbrecv does what its name says:
///
/// The comment at the beginning of the procedure
/// causes the suspicion that the firmware versionspecific distinction
/// is missing for receiving from usb endpoint 3 or 4.
/// The commented code contains only the reception from EP4,
/// but this may be wrong for a software version 2.0 or higher (see the code for os-usbsend ()).
///
/// \n So all the receiving is done via an ioctl() like in os_usbsend.
/// The ioctl() is given a struct usbdevfs_ctrltransfer, in which the relevant parameters are entered:
///
/// bRequestType | bRequest | wValue | EP | size | Timeout | data
/// ------------ | -------- | ------ | -- | ---- | ------- | ----
/// 0xA1 | 0x01 | 0x0200 | endpoint to be addressed from epcount - 1 | MSG_SIZE | 5ms | the message buffer pointer
/// Device to Host, Type=Class, Recipient=Interface | 1 = RECEIVE? | specific | Interface # | 64 | 5000 | in_msg
///
/// The ioctl() returns the number of bytes received.
/// Here is the usual check again:
/// - If the return value is -1 AND the error is a timeout (ETIMEOUT),
/// os_usbrecv() will return -1 to indicate that it is probably a recoverable problem and a retry is recommended.
/// - For another negative value or other error identifier OR 0 bytes are received, 0 is returned as an identifier for a heavy error.
/// - In all other cases, the function returns the number of bytes received.
///
/// If this is not the entire blocksize (MSG_SIZE bytes),
/// an error message is issued on the standard error channel
/// [warning "Read YY bytes (expected 64)"].
///
int os_usbrecv(usbdevice* kb, uchar* in_msg, const char* file, int line){
    int res;
    // This is what CUE does, but it doesn't seem to work on linux.
    /*if(kb->fwversion >= 0x130){
        struct usbdevfs_bulktransfer transfer;
        memset(&transfer, 0, sizeof(transfer));
        transfer.ep = 0x84;
        transfer.len = MSG_SIZE;
        transfer.timeout = 5000;
        transfer.data = in_msg;
        res = ioctl(kb->handle - 1, USBDEVFS_BULK, &transfer);
    } else {*/
        struct usbdevfs_ctrltransfer transfer = { 0xa1, 0x01, 0x0300, kb->epcount - 1, MSG_SIZE, 5000, in_msg };
        res = ioctl(kb->handle - 1, USBDEVFS_CONTROL, &transfer);
    //}
    if(res <= 0){
        ckb_err_fn("%s\n", file, line, res ? strerror(errno) : "No data read");
        if(res == -1 && errno == ETIMEDOUT)
            return -1;
        else
            return 0;
    } else if(res != MSG_SIZE)
        ckb_warn_fn("Read %d bytes (expected %d)\n", file, line, res, MSG_SIZE);
#ifdef DEBUG_USB_RECV
    char converted[MSG_SIZE*3 + 1];
    for(int i=0;i<MSG_SIZE;i++)
        sprintf(&converted[i*3], "%02x ", in_msg[i]);
    ckb_warn_fn("Recv %s\n", file, line, converted);
#endif
    return res;
}

///
/// \brief _nk95cmd If we control a non RGB keyboard, set the keyboard via ioctl with usbdevfs_ctrltransfer
///
/// To send control packets to a non RGB non color K95 Keyboard,
/// use this function. Normally it is called via the nk95cmd() macro.
///
/// If it is the wrong device for which the function is called, 0 is returned and nothing done.
/// Otherwise a usbdevfs_ctrltransfer structure is filled and an USBDEVFS_CONTROL ioctl() called.
///
/// bRequestType | bRequest | wValue | EP | size | Timeout | data
/// ------------ | -------- | ------ | -- | ---- | ------- | ----
/// 0x40 | see table below to switch hardware-modus at Keyboard | wValue | device | MSG_SIZE | 5ms | the message buffer pointer
/// Host to Device, Type=Vendor, Recipient=Device | bRequest parameter | given wValue Parameter | device 0 | 0 data to write | 5000 | null
///
/// If a 0 or a negative error number is returned by the ioctl, an error message is shown depending on the errno or "No data written" if retval was 0.
/// In either case 1 is returned to indicate the error.
/// If the ioctl returned a value > 0, 0 is returned to indicate no error.
///
/// Currently the following combinations for bRequest and wValue are used:
/// Device | what it might to do | constant | bRequest | wValue
/// ------ | ------------------- | -------- | -------- | ------
/// non RGB Keyboard | set HW-modus on (leave the ckb driver) | HWON | 0x0002 | 0x0030
/// non RGB Keyboard | set HW-modus off (initialize the ckb driver) | HWOFF | 0x0002 | 0x0001
/// non RGB Keyboard | set light modus M1 in single-color keyboards | NK95_M1 | 0x0014 | 0x0001
/// non RGB Keyboard | set light modus M2 in single-color keyboards | NK95_M2 | 0x0014 | 0x0002
/// non RGB Keyboard | set light modus M3 in single-color keyboards | NK95_M3 | 0x0014 | 0x0003
/// \see usb.h
///
int _nk95cmd(usbdevice* kb, uchar bRequest, ushort wValue, const char* file, int line){
    if(kb->product != P_K95_NRGB)
        return 0;
    struct usbdevfs_ctrltransfer transfer = { 0x40, bRequest, wValue, 0, 0, 5000, 0 };
    int res = ioctl(kb->handle - 1, USBDEVFS_CONTROL, &transfer);
    if(res <= 0){
        ckb_err_fn("%s\n", file, line, res ? strerror(errno) : "No data written");
        return 1;
    }
    return 0;
}

/// \brief .
///
/// \brief os_sendindicators update the indicators for the special keys (Numlock, Capslock and what else?)
///
/// Read the data from kb->ileds ans send them via ioctl() to the keyboard.
///
/// bRequestType | bRequest | wValue | EP | size | Timeout | data
/// ------------ | -------- | ------ | -- | ---- | ------- | ----
/// 0x21 | 0x09 | 0x0200 | Interface 0 | MSG_SIZE 1 Byte | timeout 0,5ms | the message buffer pointer
/// Host to Device, Type=Class, Recipient=Interface (why not endpoint?) | 9 = SEND? | specific | 0 | 1 | 500 | struct* kb->ileds
///
/// \n The ioctl command is USBDEVFS_CONTROL.
///
void os_sendindicators(usbdevice* kb) {
    static int countForReset = 0;
    struct usbdevfs_ctrltransfer transfer = { 0x21, 0x09, 0x0200, 0x00, 1, 500, &kb->ileds };
    int res = ioctl(kb->handle - 1, USBDEVFS_CONTROL, &transfer);
    if(res <= 0) {
        ckb_err("%s\n", res ? strerror(errno) : "No data written");
        if (usb_tryreset(kb) == 0 && countForReset++ < 3) {
            os_sendindicators(kb);
        }
    }
}

///
/// \brief os_inputmain This function is run in a separate thread and will be detached from the main thread, so it needs to clean up its own resources.
/// \todo This function is a collection of many tasks. It should be divided into several sub-functions for the sake of greater convenience:
///
/// 1. set up an URB (Userspace Ressource Buffer) to communicate with the USBDEVFS_* ioctl()s
/// 2. perform the ioctl()
/// 3. interpretate the information got into the URB buffer or handle error situations and retry operation or leave the endless loop
/// 4. inform the os about the data
/// 5. loop endless via 2.
/// 6. if endless loop has gone, deinitalize the interface, free buffers etc.
/// 7. return null
///

void* os_inputmain(void* context){
    usbdevice* kb = context;
    int fd = kb->handle - 1;
    short vendor = kb->vendor, product = kb->product;
    int index = INDEX_OF(kb, keyboard);
    ckb_info("Starting input thread for %s%d\n", devpath, index);

    /// Here the actions in detail:
    ///
    /// Monitor input transfers on all endpoints for non-RGB devices
    /// For RGB, monitor all but the last, as it's used for input/output
    int urbcount = IS_RGB(vendor, product) ? (kb->epcount - 1) : kb->epcount;
    if (urbcount == 0) {
        ckb_err("urbcount = 0, so there is nothing to claim in os_inputmain()\n");
        return 0;
    }

    /// Get an usbdevfs_urb data structure and clear it via memset()
    struct usbdevfs_urb urbs[urbcount + 1];
    memset(urbs, 0, sizeof(urbs));

    /// Hopefully the buffer lengths are equal for all devices with congruent types.
    /// You can find out the correctness for your device with lsusb --v or similar on macOS.
    /// Currently the following combinations are known and implemented:
    ///
    /// device | detect with macro combination | endpoint # | buffer-length
    /// ------ | ----------------------------- | ---------- | -------------
    /// each | none | 0 | 8
    /// RGB Mouse | IS_RGB && IS_MOUSE | 1 | 10
    /// RGB Keyboard | IS_RGB && !IS_MOUSE | 1 | 21
    /// RGB Mouse or Keyboard | IS_RGB | 2 | MSG_SIZE (64)
    /// non RGB Mouse or Keyboard | !IS_RGB | 1 | 4
    /// non RGB Mouse or Keyboard | !IS_RGB | 2 | 15
    ///
    urbs[0].buffer_length = 8;
    if(urbcount > 1 && IS_RGB(vendor, product)) {
        if(IS_MOUSE(vendor, product))
            urbs[1].buffer_length = 10;
        else
            urbs[1].buffer_length = 21;
        urbs[2].buffer_length = MSG_SIZE;
        if(urbcount != 3)
            urbs[urbcount - 1].buffer_length = MSG_SIZE;
    } else {
        urbs[1].buffer_length = 4;
        urbs[2].buffer_length = 15;
    }

    /// Now submit all the URBs via ioctl(USBDEVFS_SUBMITURB) with type USBDEVFS_URB_TYPE_INTERRUPT (the endpoints are defined as type interrupt).
    /// Endpoint number is 0x80..0x82 or 0x83, depending on the model.
    for(int i = 0; i < urbcount; i++){
        urbs[i].type = USBDEVFS_URB_TYPE_INTERRUPT;
        urbs[i].endpoint = 0x80 | (i + 1);
        urbs[i].buffer = malloc(urbs[i].buffer_length);
        ioctl(fd, USBDEVFS_SUBMITURB, urbs + i);
    }

    /// The userSpaceFS knows the URBs now, so start monitoring input
    while (1) {
        struct usbdevfs_urb* urb = 0;

        /// if the ioctl returns something != 0, let's have a deeper look what happened.
        /// Broken devices or shutting down the entire system leads to closing the device and finishing this thread.
        if (ioctl(fd, USBDEVFS_REAPURB, &urb)) {
            if (errno == ENODEV || errno == ENOENT || errno == ESHUTDOWN)
                // Stop the thread if the handle closes
                break;
            else if(errno == EPIPE && urb){
                /// If just an EPIPE ocurred, give the device a CLEAR_HALT and resubmit the URB.
                ioctl(fd, USBDEVFS_CLEAR_HALT, &urb->endpoint);
                // Re-submit the URB
                if(urb)
                    ioctl(fd, USBDEVFS_SUBMITURB, urb);
                urb = 0;
            }
            continue;
        }

        /// A correct REAPURB returns a Pointer to the URB which we now have a closer look into.
        /// Lock all following actions with imutex.
        ///
        if (urb) {

            /// Process the input depending on type of device. Interprete the actual size of the URB buffer
            ///
            /// device | detect with macro combination | seems to be endpoint # | actual buffer-length | function called
            /// ------ | ----------------------------- | ---------------------- | -------------------- | ---------------
            /// mouse (RGB and non RGB) | IS_MOUSE | nA | 8, 10 or 11 | hid_mouse_translate()
            /// mouse (RGB and non RGB) | IS_MOUSE | nA | MSG_SIZE (64) | corsair_mousecopy()
            /// RGB Keyboard | IS_RGB && !IS_MOUSE | 1 | 8 (BIOS Mode) | hid_kb_translate()
            /// RGB Keyboard | IS_RGB && !IS_MOUSE | 2 | 5 or 21, KB inactive! | hid_kb_translate()
            /// RGB Keyboard | IS_RGB && !IS_MOUSE | 3? | MSG_SIZE | corsair_kbcopy()
            /// non RGB Keyboard | !IS_RGB && !IS_MOUSE | nA | nA | hid_kb_translate()
            ///
            pthread_mutex_lock(imutex(kb));
            if(IS_MOUSE(vendor, product)){
                switch(urb->actual_length){
                case 8:
                case 10:
                case 11:
                    // HID mouse input
                    hid_mouse_translate(kb->input.keys, &kb->input.rel_x, &kb->input.rel_y, -(urb->endpoint & 0xF), urb->actual_length, urb->buffer);
                    break;
                case MSG_SIZE:
                    // Corsair mouse input
                    corsair_mousecopy(kb->input.keys, -(urb->endpoint & 0xF), urb->buffer);
                    break;
                }
            } else if(IS_RGB(vendor, product)){
                switch(urb->actual_length){
                case 8:
                    // RGB EP 1: 6KRO (BIOS mode) input
                    hid_kb_translate(kb->input.keys, -1, urb->actual_length, urb->buffer);
                    break;
                case 21:
                case 5:
                    // RGB EP 2: NKRO (non-BIOS) input. Accept only if keyboard is inactive
                    if(!kb->active)
                        hid_kb_translate(kb->input.keys, -2, urb->actual_length, urb->buffer);
                    break;
                case MSG_SIZE:
                    // RGB EP 3: Corsair input
                    corsair_kbcopy(kb->input.keys, -(urb->endpoint & 0xF), urb->buffer);
                    break;
                }
            } else {
                // Non-RGB input
                hid_kb_translate(kb->input.keys, urb->endpoint & 0xF, urb->actual_length, urb->buffer);
            }
            ///
            /// The input data is transformed and copied to the kb structure. Now give it to the OS and unlock the imutex afterwards.
            inputupdate(kb);
            pthread_mutex_unlock(imutex(kb));

            /// Re-submit the URB for the next run.
            ioctl(fd, USBDEVFS_SUBMITURB, urb);
            urb = 0;
        }
    }

    ///
    /// If the endless loop is terminated, clean up by discarding the URBs via ioctl(USBDEVFS_DISCARDURB),
    /// free the URB buffers and return a null pointer as thread exit code.
    ckb_info("Stopping input thread for %s%d\n", devpath, index);
    for(int i = 0; i < urbcount; i++){
        ioctl(fd, USBDEVFS_DISCARDURB, urbs + i);
        free(urbs[i].buffer);
    }
    return 0;
}

/// \brief .
///
/// \brief usbunclaim do an unclaiming of the usb device gicen by kb.
/// \param kb THE usbdevice*
/// \param resetting boolean flag: If resseting is true, the caller will perform a bus reset command after unclaiming the device.
/// \return always 0.
///
/// Unclaim all endpoints for a given device (remeber the decrementing of the file descriptor)
/// via ioctl(USBDEVFS_DISCARDURB).
///
/// Afterwards - if ressetting is false - do a USBDEVFS_CONNECT for EP 0 and 1.
/// If it is a non RGB device, connect EP 2 also.
/// The comment mentions RGB keyboards only, but as I understand the code, this is valid also for RGB mice.
///
/// There is no error handling yet.
/// Function is called  in usb_linux.c only, so it is declared as static now.
///
static int usbunclaim(usbdevice* kb, int resetting) {
    int handle = kb->handle - 1;
    int count = kb->epcount;
    for (int i = 0; i < count; i++) {
        ioctl(handle, USBDEVFS_RELEASEINTERFACE, &i);
    }
    // For RGB keyboards, the kernel driver should only be reconnected to interfaces 0 and 1 (HID), and only if we're not about to do a USB reset.
    // Reconnecting any of the others causes trouble.
    if (!resetting) {
        struct usbdevfs_ioctl ctl = { 0, USBDEVFS_CONNECT, 0 };
        ioctl(handle, USBDEVFS_IOCTL, &ctl);
        ctl.ifno = 1;
        ioctl(handle, USBDEVFS_IOCTL, &ctl);
        // Also reconnect iface #2 (HID) for non-RGB keyboards
        if(!HAS_FEATURES(kb, FEAT_RGB)){
            ctl.ifno = 2;
            ioctl(handle, USBDEVFS_IOCTL, &ctl);
        }
    }
    return 0;
}

/// \brief .
///
/// \brief os_closeusb is the linux specific implementation for closing an active usb port.
/// \n If a valid handle is given in the kb structure, the usb port is unclaimed (usbunclaim()).
/// \n The device in unrefenced via library function udev_device_unref().
/// \n handle, udev and the first char of kbsyspath are cleared to 0 (empty string for kbsyspath).
///
void os_closeusb(usbdevice* kb){
    if(kb->handle){
        usbunclaim(kb, 0);
        close(kb->handle - 1);
    }
    if(kb->udev)
        udev_device_unref(kb->udev);
    kb->handle = 0;
    kb->udev = 0;
    kbsyspath[INDEX_OF(kb, keyboard)][0] = 0;
}

/// \brief .
///
/// \brief usbclaim does claiming all EPs for the usb device gicen by kb.
/// \param kb THE usbdevice*
/// \return 0 on success, -1 otherwise.
///
/// Claim all endpoints for a given device (remeber the decrementing of the file descriptor)
/// via ioctl(USBDEVFS_DISCONNECT) and ioctl(USBDEVFS_CLAIMINTERFACE).
///
/// Error handling is done for the ioctl(USBDEVFS_CLAIMINTERFACE) only. If this fails, now an error message is thrown and -1 is returned.
/// Function is called  in usb_linux.c only, so it is declared as static now.
///
static int usbclaim(usbdevice* kb){
    int count = kb->epcount;
#ifdef DEBUG
    ckb_info("claiming %d endpoints\n", count);
#endif // DEBUG

    for(int i = 0; i < count; i++){
        struct usbdevfs_ioctl ctl = { i, USBDEVFS_DISCONNECT, 0 };
        ioctl(kb->handle - 1, USBDEVFS_IOCTL, &ctl);
        if(ioctl(kb->handle - 1, USBDEVFS_CLAIMINTERFACE, &i)) {
            ckb_err("Failed to claim interface %d: %s\n", i, strerror(errno));
            return -1;
        }
    }
    return 0;
}

///
/// \brief TEST_RESET doesa "try / catch" for resetting the usb interface
///
#define TEST_RESET(op)                                                      \
    if(op){                                                                 \
        ckb_err_fn("resetusb failed: %s\n", file, line, strerror(errno));   \
        if(errno == EINTR || errno == EAGAIN)                               \
            return -1;              /* try again if status code says so */  \
        return -2;                  /* else, remove device */               \
    }

/// \brief .
///
/// Try to reset an usb device in a linux user space driver.
/// 1. unclaim the device, but do not reconnect the system driver (second param resetting = true)
/// 2. reset the device via USBDEVFS_RESET command
/// 3. claim the device again.
/// Returns 0 on success, -2 if device should be removed and -1 if reset should by tried again
///
/// \todo it seems that no one wants to try the reset again. But I'v seen it somewhere...
///
int os_resetusb(usbdevice* kb, const char* file, int line) {
    TEST_RESET(usbunclaim(kb, 1));
    TEST_RESET(ioctl(kb->handle - 1, USBDEVFS_RESET));
    TEST_RESET(usbclaim(kb));
    // Success!
    return 0;
}

/// \brief .
///
/// \brief strtrim trims a string by removing leading and trailing spaces.
/// \param string
///
void strtrim(char* string){
    // Find last non-space
    char* last = string;
    for(char* c = string; *c != 0; c++){
        if(!isspace(*c))
            last = c;
    }
    last[1] = 0;
    // Find first non-space
    char* first = string;
    for(; *first != 0; first++){
        if(!isspace(*first))
            break;
    }
    if(first != string)
        memmove(string, first, last - first);
}

/// \brief .
///
/// Perform the operating system-specific opening of the interface in os_setupusb().
/// As a result, some parameters should be set in kb (name, serial, fwversion, epcount = number of usb endpoints),
/// and all endpoints should be claimed with usbclaim().
/// Claiming is the only point where os_setupusb() can produce an error (-1).
///
int os_setupusb(usbdevice* kb) {
    ///
    /// - Copy device description and serial
    struct udev_device* dev = kb->udev;
    const char* name = udev_device_get_sysattr_value(dev, "product");
    if(name)
        strncpy(kb->name, name, KB_NAME_LEN);
    strtrim(kb->name);
    const char* serial = udev_device_get_sysattr_value(dev, "serial");
    if(serial)
        strncpy(kb->serial, serial, SERIAL_LEN);
    strtrim(kb->serial);
    ///
    /// - Copy firmware version (needed to determine USB protocol)
    const char* firmware = udev_device_get_sysattr_value(dev, "bcdDevice");
    if(firmware)
        sscanf(firmware, "%hx", &kb->fwversion);
    else
        kb->fwversion = 0;
    int index = INDEX_OF(kb, keyboard);

    /// - Do some output about connecting interfaces
    ckb_info("Connecting %s at %s%d\n", kb->name, devpath, index);

    ///
    /// - Claim the USB interfaces
    ///
    /// \todo in these modules a pullrequest is outstanding
    ///
    const char* ep_str = udev_device_get_sysattr_value(dev, "bNumInterfaces");
#ifdef DEBUG
    ckb_info("claiming interfaces. name=%s, serial=%s, firmware=%s; Got >>%s<< as ep_str\n", name, serial, firmware, ep_str);
#endif //DEBUG
    kb->epcount = 0;
    if(ep_str)
        sscanf(ep_str, "%d", &kb->epcount);
    if(kb->epcount < 2){
        // IF we have an RGB KB with 0 or 1 endpoints, it will be in BIOS mode.
        ckb_err("Unable to read endpoint count from udev, assuming %d and reading >>%s<< or device is in BIOS mode\n", kb->epcount, ep_str);
        if (usb_tryreset(kb) == 0) { ///< Try to reset the device and recall the function
            static int retryCount = 0; ///< Don't do this endless in recursion
            if (retryCount++ < 5) {
                return os_setupusb(kb); ///< os_setupusb() has a return value (used as boolean)
            }
        }
        return -1;
        // ToDo are there special versions we have to detect? If there are, that was the old code to handle it:
        // This shouldn't happen, but if it does, assume EP count based onckb_warn what the device is supposed to have
        // kb->epcount = (HAS_FEATURES(kb, FEAT_RGB) ? 4 : 3);
        // ckb_warn("Unable to read endpoint count from udev, assuming %d and reading >>%s<<...\n", kb->epcount, ep_str);
    }
    if(usbclaim(kb)){
        ckb_err("Failed to claim interfaces: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

int usbadd(struct udev_device* dev, short vendor, short product) {
    const char* path = udev_device_get_devnode(dev);
    const char* syspath = udev_device_get_syspath(dev);
    if(!path || !syspath || path[0] == 0 || syspath[0] == 0){
        ckb_err("Failed to get device path\n");
        return -1;
    }
#ifdef DEBUG
    ckb_info(">>>vendor = 0x%x, product = 0x%x, path = %s, syspath = %s\n", vendor, product, path, syspath);
#endif // DEDBUG
    // Find a free USB slot
    for(int index = 1; index < DEV_MAX; index++){
        usbdevice* kb = keyboard + index;
        if(pthread_mutex_trylock(dmutex(kb))){
            // If the mutex is locked then the device is obviously in use, so keep going
            if(!strcmp(syspath, kbsyspath[index])){
                // Make sure this existing keyboard doesn't have the same syspath (this shouldn't happen)
                return 0;
            }
            continue;
        }
        if(!IS_CONNECTED(kb)){
            // Open the sysfs device
            kb->handle = open(path, O_RDWR) + 1;
            if(kb->handle <= 0){
                ckb_err("Failed to open USB device: %s\n", strerror(errno));
                kb->handle = 0;
                pthread_mutex_unlock(dmutex(kb));
                return -1;
            } else {
                // Set up device
                kb->udev = dev;
                kb->vendor = vendor;
                kb->product = product;
                strncpy(kbsyspath[index], syspath, FILENAME_MAX);
                // Mutex remains locked
                setupusb(kb);
                return 0;
            }
        }
        pthread_mutex_unlock(dmutex(kb));
    }
    ckb_err("No free devices\n");
    return -1;
}

static struct udev* udev;   ///< struct udef is defined in /usr/include/libudev.h

/// \todo These two thread vasriables seem to be unused: usbtread, udevthread
pthread_t usbthread, udevthread;

// String -> numeric model map
typedef struct {
    const char* name;
    short number;
} _model;
///
/// \attention when adding new hardware this file hat to be changed too.
///
/// In this structure array \a models[] for each device the name
/// (the device id as string in hex without leading 0x)
/// and its usb device id as short must be entered in this array.
static _model models[] = {
    // Keyboards
    { P_K65_STR, P_K65 },
    { P_K65_NRGB_STR, P_K65_NRGB },
    { P_K65_LUX_STR, P_K65_LUX },
    { P_K65_RFIRE_STR, P_K65_RFIRE },
    { P_K70_STR, P_K70 },
    { P_K70_NRGB_STR, P_K70_NRGB },
    { P_K70_LUX_STR, P_K70_LUX },
    { P_K70_LUX_NRGB_STR, P_K70_LUX_NRGB },
    { P_K70_RFIRE_STR, P_K70_RFIRE },
    { P_K70_RFIRE_NRGB_STR, P_K70_RFIRE_NRGB },
    { P_K95_STR, P_K95 },
    { P_K95_NRGB_STR, P_K95_NRGB },
    { P_K95_PLATINUM_STR, P_K95_PLATINUM },
    { P_STRAFE_STR, P_STRAFE },
    { P_STRAFE_NRGB_STR, P_STRAFE_NRGB },
    // Mice
    { P_M65_STR, P_M65 },
    { P_M65_PRO_STR, P_M65_PRO },
    { P_SABRE_O_STR, P_SABRE_O },
    { P_SABRE_L_STR, P_SABRE_L },
    { P_SABRE_N_STR, P_SABRE_N },
    { P_SCIMITAR_STR, P_SCIMITAR },
    { P_SCIMITAR_PRO_STR, P_SCIMITAR_PRO },
    { P_SABRE_O2_STR, P_SABRE_O2 }
};
#define N_MODELS (sizeof(models) / sizeof(_model))

///
/// \brief Add a udev device. Returns 0 if device was recognized/added.
/// \brief If the device id can be found, call usbadd() with the appropriate parameters.
/// \param dev the functions usb_*_device get a struct udev* with the neccessary hardware-related information.
/// \return the retval of usbadd() or 1 if either vendor is not corsair or product is not mentioned in model[].
///
/// First get the idVendor via udev_device_get_sysattr_value(). If this is equal to the ID-string of corsair ("1b1c"),
/// get the idProduct on the same way.
/// \n If we can find the model name in the model array,
/// call usbadd() with the model number.
/// \todo So why the hell not a transformation between the string and the short presentation? Lets check if the string representation is used elsewhere.
static int usb_add_device(struct udev_device* dev){
    const char* vendor = udev_device_get_sysattr_value(dev, "idVendor");
    if(vendor && !strcmp(vendor, V_CORSAIR_STR)){
        const char* product = udev_device_get_sysattr_value(dev, "idProduct");
        if(product){
            for(_model* model = models; model < models + N_MODELS; model++){
                if(!strcmp(product, model->name)){
                    return usbadd(dev, V_CORSAIR, model->number);
                }
            }
        }
    }
    return 1;
}

///
/// \brief usb_rm_device find the usb port to remove and close it via closeusb().
/// \param dev the functions usb_*_device get a struct udev* with the neccessary hardware-related information.
///
/// First try to find the system path of the device given in parameter dev.
/// The index where the name is found is the same index we need to address the global keyboard array.
/// That array holds all usbdevices.
/// \n Searching for the correct name in kbsyspath-array and closing the usb via closeusb()
/// are protected by lock..unlock of the corresponding devmutex arraymember.
///
static void usb_rm_device(struct udev_device* dev){
    // Device removed. Look for it in our list of keyboards
    const char* syspath = udev_device_get_syspath(dev);
    if(!syspath || syspath[0] == 0)
        return;
    for(int i = 1; i < DEV_MAX; i++){
        pthread_mutex_lock(devmutex + i);
        if(!strcmp(syspath, kbsyspath[i]))
            closeusb(keyboard + i);
        pthread_mutex_unlock(devmutex + i);
    }
}

///
/// \brief udev_enum use the udev_enumerate_add_match_subsystem() to get all you need but only that.
///
/// Reduce the hits of the enumeration by limiting to usb as technology and corsair as idVendor.
/// Then filter with udev_enumerate_scan_devices () all hits.
///
/// The following call to udev_enumerate_get_list_entry() fetches the entire hitlist as udev_list_entry *.
/// \n Use udev_list_entry_foreach() to iterate through the hit set.
/// \n If both the device name exists (udev_list_entry_get_name)
/// and the subsequent creation of a new udev_device (udev_device_new_from_syspath) is ok,
/// the new device is added to the list with usb_add_device().
///
/// If the latter does not work, the new device is released again (udev_device_unref ()).
/// \n After the last iteration, the enumerator is released with udev_enumerate_unref ().
static void udev_enum(){
    struct udev_enumerate* enumerator = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerator, "usb");
    udev_enumerate_add_match_sysattr(enumerator, "idVendor", V_CORSAIR_STR);
    udev_enumerate_scan_devices(enumerator);
    struct udev_list_entry* devices, *dev_list_entry;
    devices = udev_enumerate_get_list_entry(enumerator);

    udev_list_entry_foreach(dev_list_entry, devices){
        const char* path = udev_list_entry_get_name(dev_list_entry);
        if(!path)
            continue;
        struct udev_device* dev = udev_device_new_from_syspath(udev, path);
        if(!dev)
            continue;
        // If the device matches a recognized device ID, open it
        if(usb_add_device(dev))
            // Release device if not
            udev_device_unref(dev);
    }
    udev_enumerate_unref(enumerator);
}

/// \brief .
///
/// \brief usbmain is called by main() after setting up all other stuff.
/// \return 0 normally or -1 if fatal error occurs (up to now only if no new devices are available)
///
int usbmain(){
    /// First check whether the uinput module is loaded by the kernel.
    /// \todo Why isn't missing of uinput
    /// a fatal error?
    ///
    // Load the uinput module (if it's not loaded already)
    if(system("modprobe uinput") != 0)
        ckb_warn("Failed to load uinput module\n");

    ///
    /// Create the udev object with udev_new() (is a function from libudev.h)
    /// terminate -1 if error
    if(!(udev = udev_new())) {
        ckb_fatal("Failed to initialize udev in usbmain(), usb_linux.c\n");
        return -1;
    }

    ///
    /// Enumerate all currently connected devices
    udev_enum();

    /// \todo lae. here the work has to go on...
    ///
    // Done scanning. Enter a loop to poll for device updates
    struct udev_monitor* monitor = udev_monitor_new_from_netlink(udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(monitor, "usb", 0);
    udev_monitor_enable_receiving(monitor);
    // Get an fd for the monitor
    int fd = udev_monitor_get_fd(monitor);
    fd_set fds;
    while(udev){
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        // Block until an event is read
        if(select(fd + 1, &fds, 0, 0, 0) > 0 && FD_ISSET(fd, &fds)){
            struct udev_device* dev = udev_monitor_receive_device(monitor);
            if(!dev)
                continue;
            const char* action = udev_device_get_action(dev);
            if(!action){
                udev_device_unref(dev);
                continue;
            }
            // Add/remove device
            if(!strcmp(action, "add")){
                int res = usb_add_device(dev);
                if(res == 0)
                    continue;
                // If the device matched but the handle wasn't opened correctly, re-enumerate (this sometimes solves the problem)
                if(res == -1)
                    udev_enum();
            } else if(!strcmp(action, "remove"))
                usb_rm_device(dev);
            udev_device_unref(dev);
        }
    }
    udev_monitor_unref(monitor);
    return 0;
}

void usbkill(){
    udev_unref(udev);
    udev = 0;
}

#endif

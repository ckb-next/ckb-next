#include "device.h"
#include "devnode.h"
#include "input.h"
#include "notify.h"
#include "usb.h"

#ifdef OS_LINUX
#include <time.h>

// usb.c
extern _Atomic int reset_stop;

/// \details
/// \brief all open usb devices have their system path names here in this array.

static char kbsyspath[DEV_MAX][FILENAME_MAX];

int os_usbsend_control(usbdevice* kb, uchar* data, ushort len, uchar bRequest, ushort wValue, ushort wIndex, const char* file, int line) {
#ifdef DEBUG_USB_SEND
    int ckb = INDEX_OF(kb, keyboard);
    ckb_info("ckb%d Control (%s:%d): bmRequestType: 0x%02hhx, bRequest: %hhu, wValue: 0x%04hx, wIndex: %04hx, wLength: %hu", ckb, file, line, 0x40, bRequest, wValue, wIndex, len);
    if(len)
        print_urb_buffer("Control buffer:", data, len, file, line, __func__, ckb);
#endif

    struct usbdevfs_ctrltransfer transfer = { 0x40, bRequest, wValue, wIndex, len, 5000, data };
    int res = ioctl(kb->handle - 1, USBDEVFS_CONTROL, &transfer);
    if (res == -1){
        int ioctlerrno = errno;
        ckb_err_fn(" %s, res = 0x%x", file, line, strerror(ioctlerrno), res);
        if(ioctlerrno == ETIMEDOUT)
            return -1;
        else
            return 0;

    } else if (res != len)
        ckb_warn_fn("Wrote %d bytes (expected %d)", file, line, res, MSG_SIZE);

    return res;
}

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
/// If DEBUG_USB_SEND is set during compilation,
/// the number of bytes sent and their representation are logged to the error channel.
///
int os_usbsend(usbdevice* kb, const uchar* out_msg, int is_recv, const char* file, int line) {
    int res;
    if (kb->fwversion >= 0x120 || IS_V2_OVERRIDE(kb) || kb->protocol == PROTO_BRAGI){
        // If we need to read a response, lock the interrupt mutex
        if(is_recv)
            if(pthread_mutex_lock(intmutex(kb)))
                ckb_fatal("Error locking interrupt mutex in os_usbsend()");

        struct usbdevfs_bulktransfer transfer = {0};
        // All firmware versions for normal HID devices have the OUT endpoint at the end
        // Devices with no input, such as the Polaris, have it at the start.
        transfer.ep = (IS_SINGLE_EP(kb) ? 1 : kb->epcount);
        transfer.len = MSG_SIZE;
        transfer.timeout = 5000;
        transfer.data = (void*)out_msg;
        res = ioctl(kb->handle - 1, USBDEVFS_BULK, &transfer);
    } else {
        // Note, Ctrl Transfers require an index, not an endpoint, which is why kb->epcount - 1 works
        struct usbdevfs_ctrltransfer transfer = { 0x21, 0x09, 0x0200, kb->epcount - 1, MSG_SIZE, 5000, (void*)out_msg };
        res = ioctl(kb->handle - 1, USBDEVFS_CONTROL, &transfer);
    }

    if (res <= 0){
        int ioctlerrno = errno;
        ckb_err_fn("%s, res = 0x%x", file, line, res ? strerror(ioctlerrno) : "No data written", res);
        if(res == -1 && ioctlerrno == ETIMEDOUT){
            if(is_recv)
                pthread_mutex_unlock(intmutex(kb));
            return -1;
        } else {
            if(is_recv)
                pthread_mutex_unlock(intmutex(kb));
            return 0;
        }
    } else if (res != MSG_SIZE)
        ckb_warn_fn("Wrote %d bytes (expected %d)", file, line, res, MSG_SIZE);
#ifdef DEBUG_USB_SEND
    print_urb_buffer("Sent:", out_msg, MSG_SIZE, file, line, __func__, INDEX_OF(kb, keyboard));
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
    if(kb->fwversion >= 0x120 || IS_V2_OVERRIDE(kb) || kb->protocol == PROTO_BRAGI){
        // Wait for 2s
        struct timespec condwait = {0};
        condwait.tv_sec = time(NULL) + 2;
        int condret = pthread_cond_timedwait(intcond(kb), intmutex(kb), &condwait);
        if(condret != 0){
            if(pthread_mutex_unlock(intmutex(kb)))
                ckb_fatal("Error unlocking interrupt mutex in os_usbrecv()");
            if(condret == ETIMEDOUT)
                ckb_warn_fn("ckb%d: Timeout while waiting for response", file, line, INDEX_OF(kb, keyboard));
            else
                ckb_warn_fn("Interrupt cond error %i", file, line, condret);
            return -1;
        }
        memcpy(in_msg, kb->interruptbuf, MSG_SIZE);
        memset(kb->interruptbuf, 0, MSG_SIZE);
        res = MSG_SIZE;
        if(pthread_mutex_unlock(intmutex(kb)))
            ckb_fatal("Error unlocking interrupt mutex in os_usbrecv()");
    } else {
        struct usbdevfs_ctrltransfer transfer = { 0xa1, 0x01, 0x0300, kb->epcount - 1, MSG_SIZE, 5000, in_msg };
        res = ioctl(kb->handle - 1, USBDEVFS_CONTROL, &transfer);
        if(res <= 0){
            // This is done because ckb_err_fn can set errno itself
            int ioctlerrno = errno;
            ckb_err_fn("%s", file, line, res ? strerror(ioctlerrno) : "No data read");
            if(res == -1 && ioctlerrno == ETIMEDOUT)
                return -1;
            else
                return 0;
        } else if(res != MSG_SIZE)
            ckb_warn_fn("Read %d bytes (expected %d)", file, line, res, MSG_SIZE);
    }

#ifdef DEBUG_USB_RECV
    print_urb_buffer("Recv:", in_msg, MSG_SIZE, file, line, __func__, INDEX_OF(kb, keyboard));
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
    if(kb->product != P_K95_LEGACY)
        return 0;
    struct usbdevfs_ctrltransfer transfer = { 0x40, bRequest, wValue, 0, 0, 5000, 0 };
    int res = ioctl(kb->handle - 1, USBDEVFS_CONTROL, &transfer);
    if(res <= 0){
        ckb_err_fn("%s", file, line, res ? strerror(errno) : "No data written");
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
    void *ileds;
    ushort leds;
    if(kb->fwversion >= 0x300 || IS_V3_OVERRIDE(kb)) {
        leds = (kb->ileds << 8) | 0x0001;
        ileds = &leds;
    }
    else {
        ileds = &kb->ileds;
    }
    struct usbdevfs_ctrltransfer transfer = { 0x21, 0x09, 0x0200, 0x00, ((kb->fwversion >= 0x300 || IS_V3_OVERRIDE(kb)) ? 2 : 1), 500, ileds };
    queued_mutex_unlock(dmutex(kb));
    int res = ioctl(kb->handle - 1, USBDEVFS_CONTROL, &transfer);
    queued_mutex_lock(dmutex(kb));
    if(res <= 0) {
        ckb_err("%s", res ? strerror(errno) : "No data written");
        if (usb_tryreset(kb) == 0 && countForReset++ < 3) {
            os_sendindicators(kb);
        }
    }
}

static ushort check_endpoint_in_intf(struct udev* dev_udev, char* base, size_t baselen, int urbcount, ushort* size){
    for(int i = 0; i < urbcount; i++){
        ushort ep = 0x80 | (i + 1);
        // Append the endpoint
        char epstr[7];
        snprintf(epstr, 7, "/ep_%02x", ep & 0xFF);
        strcpy(base + baselen, epstr);
        // Access it
        struct udev_device* child = udev_device_new_from_syspath(dev_udev, base);
        const char* sizehex = udev_device_get_sysattr_value(child, "wMaxPacketSize");
        // Read its wMaxPacketSize
        if(sizehex && sscanf(sizehex, "%hx", size) == 1)
        {
//#ifdef DEBUG
            ckb_info("Found EP 0x%hx at %s", ep, base);
//#endif
            udev_device_unref(child);
            return ep;
        }
        udev_device_unref(child);
    }
    return 0;
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
    ushort vendor = kb->vendor, product = kb->product;
    int index = INDEX_OF(kb, keyboard);
    ckb_info("Starting input thread for %s%d", devpath, index);

    /// Here the actions in detail:
    ///
    /// Monitor input transfers on all endpoints for non-RGB devices
    /// For RGB, monitor all but the last, as it's used for input/output
    int urbcount = ((IS_LEGACY(vendor, product) || product == P_ST100 || kb->protocol == PROTO_BRAGI) ? kb->epcount : (kb->epcount - 1));
    if (urbcount == 0) {
        ckb_err("urbcount = 0, so there is nothing to claim in os_inputmain()");
        return 0;
    }

    /// Get an usbdevfs_urb data structure and clear it via memset()
    struct usbdevfs_urb urbs[urbcount];
    memset(urbs, 0, sizeof(urbs));

    /// Query udev for wMaxPacketSize on each endpoint, due to certain devices sending more data than the max defined, causing all sorts of issues.
    /// A syspath example would be:
    /// $ cat "/sys/devices/pci0000:00/0000:00:05.0/0000:03:00.0/usb8/8-2/8-2:1.2/ep_03/wMaxPacketSize"
    /// 0040
    /// Where 0x0040 == 64
    ///
    /// Submit all the URBs via ioctl(USBDEVFS_SUBMITURB) with type USBDEVFS_URB_TYPE_INTERRUPT (the endpoints are defined as type interrupt).
    /// Endpoint number is 0x80..0x82 or 0x83, depending on the model and FW version.

    // Enumerate the current device's children
    struct udev* dev_udev = udev_device_get_udev(kb->udev);
    struct udev_enumerate* enumerate = udev_enumerate_new(dev_udev);
    udev_enumerate_add_match_parent(enumerate, kb->udev);
    udev_enumerate_scan_devices(enumerate);

    // Create a list containing them
    struct udev_list_entry* udeventry = udev_enumerate_get_list_entry(enumerate);

    for(int i = 0; i < urbcount; i++){
        // Move to the next entry in the udev list (skipping the first one).
        struct udev_list_entry* nextentry = udev_list_entry_get_next(udeventry);
        const char* path = udev_list_entry_get_name(nextentry);
        // If there's an underscore, that means we are dealing with udev iterating through endpoints
        // usbX/X-X/X-X:1.0/ep_80
        // ~~~~~~~~~~~~~~~~~~~^
        if(path[strlen(path) - 3] == '_'){
            ckb_info("Applying udev endpoint workaround for %s", path);
            // Skip the current entry
            udeventry = nextentry;
            nextentry = udev_list_entry_get_next(udeventry);
            path = udev_list_entry_get_name(nextentry);
        }
        // Create the path to the endpoint
        size_t pathlen = strlen(path);
        char* finalpath = calloc(pathlen+7, 1);
        // Copy the base path
        strcpy(finalpath, path);
        // Try to find any of the wanted endpoints in the current interface
        ushort size = 64;
        ushort ep = check_endpoint_in_intf(dev_udev, finalpath, pathlen, urbcount, &size);
        if(!ep)
        {
            ep = 0x80 | (i + 1);
            ckb_warn("Unable to read wMaxPacketSize for %s, assuming 64 and ep 0x%hx", finalpath, ep);
        }

#ifdef DEBUG
        ckb_info("Endpoint path %s has wMaxPacketSize %i", epstr, size);
#endif
        // Increment the udev list pointer
        udeventry = nextentry;
        // Set the URB parameters
        urbs[i].buffer_length = size;
        urbs[i].type = USBDEVFS_URB_TYPE_INTERRUPT;
        urbs[i].endpoint = ep;
        urbs[i].buffer = malloc(urbs[i].buffer_length);
        ioctl(fd, USBDEVFS_SUBMITURB, urbs + i);
        // Clean up
        free(finalpath);
    }

    udev_enumerate_unref(enumerate);
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
            // If we're shutting down, don't submit another urb, or try to process the data on this one
            if(urb->status == -ESHUTDOWN && reset_stop)
                break;

            process_input_urb(kb, urb->buffer, urb->actual_length, urb->endpoint);

            /// Re-submit the URB for the next run.
            ioctl(fd, USBDEVFS_SUBMITURB, urb);
            urb = 0;
        }
    }

    ///
    /// If the endless loop is terminated, clean up by discarding the URBs via ioctl(USBDEVFS_DISCARDURB),
    /// free the URB buffers and return a null pointer as thread exit code.
    ckb_info("Stopping input thread for %s%d", devpath, index);
    for(int i = 0; i < urbcount; i++){
        ioctl(fd, USBDEVFS_DISCARDURB, urbs + i);
        free(urbs[i].buffer);
    }
    return 0;
}

/// \brief .
///
/// \brief usbunclaim do an unclaiming of the usb device given by kb.
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
    // Intentional unclean exit workaround, because usbhid hangs while initialising these devices.
    if (NEEDS_UNCLEAN_EXIT(kb)) {
        ckb_warn("Your %s is being uncleanly removed to speed up shutdown times.", kb->name);
        ckb_warn("If you still need the device, you will have to restart ckb-next-daemon.");
        return 0;
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
    ckb_info("claiming %d endpoints", count);
#endif // DEBUG

    for(int i = 0; i < count; i++){
        struct usbdevfs_ioctl ctl = { i, USBDEVFS_DISCONNECT, 0 };
        ioctl(kb->handle - 1, USBDEVFS_IOCTL, &ctl);
        if(ioctl(kb->handle - 1, USBDEVFS_CLAIMINTERFACE, &i)) {
            ckb_err("Failed to claim interface %d: %s", i, strerror(errno));
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
        ckb_err_fn("resetusb failed: %s", file, line, strerror(errno));   \
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
    // Find first non-space
    char* first = string;
    for(; *first != 0; first++){
        if(!isspace(*first))
            break;
    }

    // If we reached the end and didn't find anything, blank out the string
    if(first[1] == '\0'){
        string[0] = '\0';
        return;
    }

    // Find last non-space
    char* last = first;
    for(char* c = string; *c != 0; c++){
        if(!isspace(*c))
            last = c;
    }
    last[1] = '\0';

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
    ckb_info("Connecting %s at %s%d", kb->name, devpath, index);

    ///
    /// - Claim the USB interfaces
    ///
    /// \todo in these modules a pullrequest is outstanding
    ///
    const char* ep_str = udev_device_get_sysattr_value(dev, "bNumInterfaces");
#ifdef DEBUG
    ckb_info("Claiming interfaces. name=%s, firmware=%s, ep_str=%s", name, firmware, ep_str);
#endif //DEBUG
    kb->epcount = 0;
    if(ep_str)
        sscanf(ep_str, "%d", &kb->epcount);
    if(kb->epcount < 2 && !IS_SINGLE_EP(kb)){
        // If we have an RGB KB with 1 endpoint, it will be in BIOS mode.
        if(kb->epcount == 1){
            ckb_info("Device is in BIOS mode");
            return -1;
        }
        // Something probably went wrong if we got here
        ckb_err("Unable to read endpoint count from udev, assuming %d", kb->epcount);
        if (usb_tryreset(kb) == 0) { ///< Try to reset the device and recall the function
            static int retryCount = 0; ///< Don't do this endless in recursion
            if (retryCount++ < 5) {
                return os_setupusb(kb); ///< os_setupusb() has a return value (used as boolean)
            }
        }
        return -1;
        // ToDo are there special versions we have to detect? If there are, that was the old code to handle it:
        // This shouldn't happen, but if it does, assume EP count based on what the device is supposed to have
        // kb->epcount = (HAS_FEATURES(kb, FEAT_RGB) ? 4 : 3);
        // ckb_warn("Unable to read endpoint count from udev, assuming %d and reading >>%s<<...", kb->epcount, ep_str);
    }
    if(usbclaim(kb)){
        ckb_err("Failed to claim interfaces: %s", strerror(errno));
        return -1;
    }
    return 0;
}

int usbadd(struct udev_device* dev, ushort vendor, ushort product) {
    // Check if device should be ignored
    for(int i = 0; i < DEV_MAX; i++){
        if(ignored_devices[i].idVendor == vendor && ignored_devices[i].idProduct == product){
            ckb_info("Ignoring device 0x%hx:0x%hx as requested. If you're using this to work around a bug, please report it.", vendor, product);
            return 1;
        }
    }
    const char* path = udev_device_get_devnode(dev);
    const char* syspath = udev_device_get_syspath(dev);
    if(!path || !syspath || path[0] == 0 || syspath[0] == 0){
        ckb_err("Failed to get device path");
        return -1;
    }
#ifdef DEBUG
    ckb_info(">>>vendor = 0x%x, product = 0x%x, path = %s, syspath = %s", vendor, product, path, syspath);
#endif // DEDBUG
    // Find a free USB slot
    for(int index = 1; index < DEV_MAX; index++){
        usbdevice* kb = keyboard + index;
        if(queued_mutex_trylock(dmutex(kb))){
            // If the mutex is locked then the device is obviously in use, so keep going
            if(!strcmp(syspath, kbsyspath[index])){
                // Make sure this existing keyboard doesn't have the same syspath (this shouldn't happen)
                return 0;
            }
            continue;
        }
        // We can't use IS_CONNECTED() here.
        // If multiple devices are being opened at once, we will attempt to set up the second one before
        // we're done with the first one, which means the uinput handles might still be 0.
        // This will cause IS_CONNECTED to return 0, resulting in overwriting ckb1's kb fields with ckb2's,
        // leaving ckb1 in an unknown and possibly uninitialised state.
        //
        // This happened rarely because dmutex was most likely already locked, so we just skipped over the device
        // before getting here. (See trylock above.)
        if(!kb->handle){
            // Open the sysfs device
            kb->handle = open(path, O_RDWR) + 1;
            if(kb->handle <= 0){
                ckb_err("Failed to open USB device: %s", strerror(errno));
                kb->handle = 0;
                queued_mutex_unlock(dmutex(kb));
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
        queued_mutex_unlock(dmutex(kb));
    }
    ckb_err("No free devices");
    return -1;
}

static struct udev* udev;   ///< struct udef is defined in /usr/include/libudev.h

/// \todo These two thread vasriables seem to be unused: usbtread, udevthread
pthread_t usbthread, udevthread;

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
static int usb_add_device(struct udev_device* dev){
    const char* vendor = udev_device_get_sysattr_value(dev, "idVendor");
    const char* product = udev_device_get_sysattr_value(dev, "idProduct");
    if(vendor == NULL || product == NULL)
        return 1;

    ushort pid, vid;
    pid = vid = 0;

    if(!(sscanf(vendor, "%04hx", &vid) == 1 && vid))
        return 1;

    if(!(sscanf(product, "%04hx", &pid) == 1 && pid))
        return 1;

    for(size_t c = 0; c < N_MODELS; c++){
        if(models[c].idVendor == vid && models[c].idProduct == pid)
            return usbadd(dev, models[c].idVendor, models[c].idProduct);
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
        queued_mutex_lock(devmutex + i);
        if(!strcmp(syspath, kbsyspath[i]))
            closeusb(keyboard + i);
        queued_mutex_unlock(devmutex + i);
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

_Atomic int suspend_run = 1;

// While POSIX says we may need a runtime test, on Linux the clock
// was added immediately after the define was introduced, so a
// compile time check should be enough

// https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=70a08cca1227dc31c784ec930099a4417a06e7d0
// https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=7fdd7f89006dd5a4c702fa0ce0c272345fa44ae0

#ifndef CLOCK_BOOTTIME
#define CLOCK_BOOTTIME CLOCK_REALTIME
#endif

static time_t get_clock_monotonic_seconds() {
    struct timespec timespec_var = {0};

    if(clock_gettime(CLOCK_BOOTTIME, &timespec_var))
        ckb_err("Error in clock_gettime()");

    return timespec_var.tv_sec;
}

void* suspend_check() {
    time_t prev_time = get_clock_monotonic_seconds();
    if(!prev_time)
        return NULL;
    while(suspend_run){
        clock_nanosleep(CLOCK_MONOTONIC, 0, &(struct timespec) {.tv_sec = 2}, NULL);
        time_t current_time = get_clock_monotonic_seconds(NULL);

        if(prev_time + 4 < current_time)
            reactivate_devices();

        prev_time = current_time;
    }
    return NULL;
}

/// \brief .
///
/// \brief usbmain is called by main() after setting up all other stuff.
/// \return 0 normally or -1 if fatal error occurs (up to now only if no new devices are available)
///
int usbmain(){
    ///
    /// Create the udev object with udev_new() (is a function from libudev.h)
    /// terminate -1 if error
    if(!(udev = udev_new())) {
        ckb_fatal("Failed to initialize udev in usbmain(), usb_linux.c");
        return -1;
    }
    
    // Create thread that detects system suspend
    pthread_t suspend_thread;
    pthread_create(&suspend_thread, NULL, suspend_check, NULL);
    pthread_setname_np(suspend_thread, "suspend");
    
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
                if(res == -1){
                    ckb_warn("Handle wasn't opened correctly. Trying again");
                    udev_enum();
                }
            } else if(!strcmp(action, "remove"))
                usb_rm_device(dev);
            udev_device_unref(dev);
        } else {
            // if select returns -1 there is a chance that the waiting
            // was interrupted by a signal
            // check whether there is data available in the
            // sighandler_pipe, read if there is and manually call the
            // signal-handling routine
            int sighandler_msg;
            ioctl(sighandler_pipe[SIGHANDLER_RECEIVER], FIONREAD, &sighandler_msg);
            if (sighandler_msg > 0){
                int unused_result = read(sighandler_pipe[SIGHANDLER_RECEIVER], &sighandler_msg, sizeof(int));
                exithandler(sighandler_msg);

                // cast unused result to void to silence over-eager
                // warning about unused variables:
                // https://sourceware.org/bugzilla/show_bug.cgi?id=11959
                (void) unused_result;
            }
        }
    }
    udev_monitor_unref(monitor);
    suspend_run = 0;
    pthread_join(suspend_thread, NULL);
    
    return 0;
}

void usbkill(){
    udev_unref(udev);
    udev = 0;
}

#endif

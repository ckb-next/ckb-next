#include "device.h"
#include "devnode.h"
#include "input.h"
#include "notify.h"
#include "usb.h"
#include <sodium.h>

#ifdef OS_LINUX
#include <time.h>

// usb.c
extern _Atomic int reset_stop;
extern int enable_experimental;

/// \details
/// \brief all open usb devices have their system path names here in this array.
static char kbsyspath[DEV_MAX][FILENAME_MAX];

// USB IO functions
int os_usb_control(usbdevice* kb, ctrltransfer* transfer, const char* file, int line) {
#ifdef DEBUG_USB_SEND
    const int ckb = INDEX_OF(kb, keyboard);
    ckb_info("ckb%d Control (%s:%d): bmRequestType: 0x%02hhx, bRequest: %hhu, wValue: 0x%04hx, wIndex: %04hx, wLength: %hu", ckb, file, line, transfer->bRequestType, transfer->bRequest, transfer->wValue, transfer->wIndex, transfer->wLength);
    if(transfer->wLength)
        print_urb_buffer("Control buffer:", transfer->data, transfer->wLength, file, line, __func__, ckb, 0);
#endif

    const int res = ioctl(kb->handle - 1, USBDEVFS_CONTROL, transfer);
    if (res == -1){
        int ioctlerrno = errno;
        ckb_err_fn(" %s, res = 0x%x", file, line, strerror(ioctlerrno), res);
#ifndef NDEBUG
        if (ioctlerrno == EBUSY) {
            if ((transfer->bRequestType & 0x1f) == 1) {
                struct usbdevfs_getdriver query = {
                    .interface = transfer->wIndex,
                };
                if (!ioctl(kb->handle - 1, USBDEVFS_GETDRIVER, &query)) {
                    ckb_info("Directed at interface %d, which is claimed by %s", transfer->wIndex, query.driver);
                } else {
                    ckb_info("Directed at interface %d, GETDRIVER error %s", transfer->wIndex, strerror(errno));
                }
            } else {
                ckb_info("Directed at endpoint (called from where?)");
            }
        }
#endif
        if(ioctlerrno == ETIMEDOUT)
            return -1;
        else
            return 0;

    } else if (res != transfer->wLength) {
        ckb_warn_fn("Wrote %d bytes (expected %hu)", file, line, res, transfer->wLength);
    }

    return res;
}

int os_usb_interrupt_out(usbdevice* kb, unsigned int ep, unsigned int len, uchar* data, const char* file, int line)
{
#ifdef DEBUG_USB_SEND
    print_urb_buffer("Sending:", data, (len > MSG_SIZE ? len : MSG_SIZE), file, line, __func__, INDEX_OF(kb, keyboard), (uchar)ep);
#endif
    const struct usbdevfs_bulktransfer transfer = { .ep = ep, .len = len, .timeout = 5000, .data = data, };
    int res = ioctl(kb->handle - 1, USBDEVFS_BULK, &transfer);
    if (res <= 0){
        int ioctlerrno = errno;
        ckb_err_fn("%s, res = 0x%x", file, line, res ? strerror(ioctlerrno) : "No data written", res);
        if(res == -1 && ioctlerrno == ETIMEDOUT)
            return -1;
        else
            return 0;
    } else if ((unsigned int)res != len) {
        ckb_warn_fn("Wrote %d bytes (expected %d)", file, line, res, len);
    }
    return res;
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

static inline short udev_get_byte_hex(struct udev_device* d, const char* what) {
    const char* that = udev_device_get_sysattr_value(d, what);
    uchar ret;
    if(!that || sscanf(that, "%hhx", &ret) != 1)
        return -1;
    return ret;
}

static inline short udev_get_byte_dec(struct udev_device* d, const char* what) {
    const char* that = udev_device_get_sysattr_value(d, what);
    uchar ret;
    if(!that || sscanf(that, "%hhu", &ret) != 1)
        return -1;
    return ret;
}

int os_get_device_ifs_eps(usbdevice* kb) {
    short temp;
    if((temp = udev_get_byte_dec(kb->udev, "bNumInterfaces")) <= 0) {
        ckb_err("FAIL");
        return -1;
    }
    kb->bNumInterfaces = temp;

    ckb_info("%hhu", kb->bNumInterfaces);

    // Enumerate the current device's children
    struct udev* dev_udev = udev_device_get_udev(kb->udev);
    struct udev_enumerate* enumerate = udev_enumerate_new(dev_udev);
    udev_enumerate_add_match_parent(enumerate, kb->udev);
    udev_enumerate_add_match_property(enumerate, "DEVTYPE", "usb_interface");
    udev_enumerate_scan_devices(enumerate);

    // Create a list containing them
    struct udev_list_entry* udeventry;

    uchar ifcount = 0;
    udev_list_entry_foreach(udeventry, udev_enumerate_get_list_entry(enumerate)) {
        // Move to the next entry in the udev list (skipping the first one).
        const char* path = udev_list_entry_get_name(udeventry);
        ckb_info("path %s", path);

        // If there's an underscore, that means we are dealing with udev iterating through endpoints
        // FIXME Is the above needed with the usb_interface devtype filter? I removed it for now

        struct udev_device* udev_if = udev_device_new_from_syspath(dev_udev, path);

        // Check if it's an HID interface
        if((temp = udev_get_byte_hex(udev_if, "bInterfaceClass")) < 0) {
            udev_device_unref(udev_if);
            ckb_err("FAIL");
            return -2;
        }

        if(temp != 3) { // HID
            udev_device_unref(udev_if);
            ckb_info("NOT HID %d", temp);
            // Do not increment ifcount here
            continue;
        }

        // Fill in the interface information
        if((temp = udev_get_byte_hex(udev_if, "bInterfaceNumber")) < 0) {
            udev_device_unref(udev_if);
            ckb_err("FAIL");
            return -3;
        }
        kb->hid_interfaces[ifcount].bInterfaceNumber = temp;

        if((temp = udev_get_byte_hex(udev_if, "bNumEndpoints")) < 0) {
            udev_device_unref(udev_if);
            ckb_err("FAIL");
            return -4;
        }
        kb->hid_interfaces[ifcount].bNumEndpoints = temp;

        // Iterate through the endpoints
        // Sadly modern udev doesn't seem to let us do it, so we'll use fs access
        DIR* dir = opendir(path);
        if (!dir) {
            udev_device_unref(udev_if);
            ckb_err("DIR");
            return -5;
        }

        uchar epcount = 0;
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if(entry->d_type != DT_DIR)
                continue;

            if(strncmp(entry->d_name, "ep_", 3))
                continue;

            unsigned char bEndpointAddress;
            if(sscanf(entry->d_name, "ep_%hhx", &bEndpointAddress) != 1)
                continue;
            ckb_info("EP %hhx", bEndpointAddress);
            kb->hid_interfaces[ifcount].endpoints[epcount].bEndpointAddress = bEndpointAddress;

            kb->hid_interfaces[ifcount].endpoints[epcount].wMaxPacketSize = 64;
            char* wMaxPacketSizePath = malloc(strlen(path) + 1 + strlen(entry->d_name) + strlen("/wMaxPacketSize") + 1);
            sprintf(wMaxPacketSizePath, "%s/%s/wMaxPacketSize", path, entry->d_name);
            ckb_info("%s", wMaxPacketSizePath);
            FILE* wMaxPacketSizeFile = fopen(wMaxPacketSizePath, "r");
            if(wMaxPacketSizeFile) {
                ushort wMaxPacketSize;
                if(fscanf(wMaxPacketSizeFile, "%hx", &wMaxPacketSize) == 1)
                    kb->hid_interfaces[ifcount].endpoints[epcount].wMaxPacketSize = wMaxPacketSize;
                else
                    ckb_err("FAILED TO PARSE");
                fclose(wMaxPacketSizeFile);
            } else {
                ckb_err("FAILED TO FOPEN wMaxPacketSize");
            }
            free(wMaxPacketSizePath);

            if(bEndpointAddress & 0x80)
                kb->num_endpoints_in++;
            ckb_info("Found EP %02hhx in IF %02hhx with wMaxPacketSize %hx at %s", bEndpointAddress, kb->hid_interfaces[ifcount].bInterfaceNumber, kb->hid_interfaces[ifcount].endpoints[epcount].wMaxPacketSize, path);
            epcount++;
        }

        closedir(dir);

        if(epcount != kb->hid_interfaces[ifcount].bNumEndpoints) {
            ckb_warn("epcount != bNumEndpoints. Assuming epcount.");
            kb->hid_interfaces[ifcount].bNumEndpoints = epcount;
        }

        udev_device_unref(udev_if);
        ifcount++;
    }

    udev_enumerate_unref(enumerate);

    if(ifcount != kb->bNumInterfaces) {
        // This is expected on certain devices since we're filtering for HID interfaces only
#ifndef NDEBUG
        ckb_info("ifcount != bNumInterfaces. Assuming ifcount.");
#endif
        kb->bNumInterfaces = ifcount;
    }

    return 0;
}

void* os_inputmain(void* context){
    usbdevice* kb = context;
    int fd = kb->handle - 1;
    int index = INDEX_OF(kb, keyboard);
    ckb_info("Starting input thread for %s%d", devpath, index);

    if (kb->num_endpoints_in == 0) {
        ckb_err("No endpoints claimed in inputmain");
        return NULL;
    }

    struct usbdevfs_urb* urbs = calloc(kb->num_endpoints_in, sizeof(struct usbdevfs_urb));
    if(!urbs) {
        ckb_fatal("Can't allocate memory for usbdevfs_urb");
        return NULL;
    }

    for(uchar i = 0, ep = 0; i < kb->bNumInterfaces && ep < kb->num_endpoints_in; i++) {
        for(uchar j = 0; j < kb->hid_interfaces[i].bNumEndpoints; j++) {
            const uchar bEndpointAddress = kb->hid_interfaces[i].endpoints[j].bEndpointAddress;
            if(!(bEndpointAddress & 0x80))
                continue;

            const ushort wMaxPacketSize = kb->hid_interfaces[i].endpoints[j].wMaxPacketSize;
            // Set the URB parameters
            urbs[ep].buffer_length = wMaxPacketSize;
            urbs[ep].type = USBDEVFS_URB_TYPE_INTERRUPT;
            urbs[ep].endpoint = bEndpointAddress;
            urbs[ep].buffer = malloc(wMaxPacketSize);
            ioctl(fd, USBDEVFS_SUBMITURB, urbs + ep);
            ep++;
        }
    }

    // Start monitoring input
    while (1) {
        struct usbdevfs_urb* urb = NULL;

        /// if the ioctl returns something != 0, let's have a deeper look what happened.
        /// Broken devices or shutting down the entire system leads to closing the device and finishing this thread.
        int res = ioctl(fd, USBDEVFS_REAPURB, &urb);
        if (res) {
            int ioctlerrno = errno;
            wait_until_suspend_processed();
            if (ioctlerrno == ENODEV || ioctlerrno == ENOENT || ioctlerrno == ESHUTDOWN)
                // Stop the thread if the handle closes
                break;
            else if(ioctlerrno == EPIPE && urb){
                /// If just an EPIPE ocurred, give the device a CLEAR_HALT and resubmit the URB.
                ioctl(fd, USBDEVFS_CLEAR_HALT, &urb->endpoint);
                // Re-submit the URB
                if(urb)
                    ioctl(fd, USBDEVFS_SUBMITURB, urb);
                urb = NULL;
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
            if (ioctl(fd, USBDEVFS_SUBMITURB, urb)) {
                wait_until_suspend_processed();
            }
            urb = NULL;
        }
    }

    ///
    /// If the endless loop is terminated, clean up by discarding the URBs via ioctl(USBDEVFS_DISCARDURB),
    /// free the URB buffers and return a null pointer as thread exit code.
    ckb_info("Stopping input thread for %s%d", devpath, index);
    for(int i = 0; i < kb->num_endpoints_in; i++){
        ioctl(fd, USBDEVFS_DISCARDURB, urbs + i);
        free(urbs[i].buffer);
    }

    free(urbs);

    return NULL;
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
    for(uchar i = 0; i < kb->bNumInterfaces; i++){
        if(kb->hid_interfaces[i].dont_claim)
            continue;

        const uchar bInterfaceNumber = kb->hid_interfaces[i].bInterfaceNumber;

        ioctl(handle, USBDEVFS_RELEASEINTERFACE, &bInterfaceNumber);
    }

    // Intentional unclean exit workaround, because usbhid hangs while initialising these devices.
    if (NEEDS_UNCLEAN_EXIT(kb)) {
        ckb_warn("Your %s is being uncleanly removed to speed up shutdown times.", kb->name);
        ckb_warn("If you still need the device, you will have to restart ckb-next-daemon.");
        return 0;
    }

    // The kernel driver should only be reconnected if we're not about to do a USB reset.
    if (resetting)
        return 0;

    struct usbdevfs_ioctl ctl = { 0, USBDEVFS_CONNECT, 0 };
    for(int i = 0; i < kb->bNumInterfaces; i++){
        ctl.ifno = kb->hid_interfaces[i].bInterfaceNumber;
        ioctl(handle, USBDEVFS_IOCTL, &ctl);
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
    int retries = 0;
    for(uchar i = 0; i < kb->bNumInterfaces; i++){
        if(kb->hid_interfaces[i].dont_claim)
            continue;

        const uchar bInterfaceNumber = kb->hid_interfaces[i].bInterfaceNumber;

#ifndef NDEBUG
        ckb_info("ckb%d: Claiming interface %02hhx", INDEX_OF(kb, keyboard), bInterfaceNumber);
#endif // DEBUG

        while (1) {
            struct usbdevfs_ioctl ctl = { bInterfaceNumber, USBDEVFS_DISCONNECT, 0 };
            ioctl(kb->handle - 1, USBDEVFS_IOCTL, &ctl);
            if(ioctl(kb->handle - 1, USBDEVFS_CLAIMINTERFACE, &bInterfaceNumber)) {
                if (errno == EBUSY && retries < 50) {
                    retries++;
                    struct timespec sleep_for = { .tv_nsec = 100 * 1000000 };
                    clock_nanosleep(CLOCK_MONOTONIC, 0, &sleep_for, NULL);
                    continue;
                }
#ifndef NDEBUG
                if (errno == EBUSY)
                    ckb_info("Retry timeout after %d seconds", retries / 10);
#endif // DEBUG
                ckb_err("Failed to claim interface %d: %s", i, strerror(errno));
                return -1;
            } else {
                break;
            }
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
    while(*first){
        if(!isspace(*first))
            break;
        first++;
    }

    // If we reached the end and didn't find anything, blank out the string
    if(*first == '\0'){
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
        memmove(string, first, last - first + 2);
}

/// \brief .
///
/// Perform the operating system-specific opening of the interface in os_setupusb().
/// As a result, some parameters should be set in kb (name, serial, fwversion, epcount = number of usb endpoints),
/// and all endpoints should be claimed with usbclaim().
/// Claiming is the only point where os_setupusb() can produce an error (-1).
///
int os_setupusb(usbdevice* kb) {
    struct udev_device* dev = kb->udev;

    // Fill in product, serial and fwversion
    const char* name = udev_device_get_sysattr_value(dev, "product");
    if(name)
        snprintf(kb->name, KB_NAME_LEN, "%s", name);
    strtrim(kb->name);

    const char* serial = udev_device_get_sysattr_value(dev, "serial");
    if(serial)
        snprintf(kb->serial, SERIAL_LEN, "%s", serial);
    strtrim(kb->serial);

    const char* firmware = udev_device_get_sysattr_value(dev, "bcdDevice");
    if(firmware)
        sscanf(firmware, "%"SCNx32, &kb->fwversion);
    else
        kb->fwversion = 0;
    int index = INDEX_OF(kb, keyboard);

    ckb_info("Connecting %s with bcdDevice %s at %s%d", kb->name, firmware, devpath, index);

    // FIXME: There has to be a better way to detect this
    // FIXME: Make this a vtable function
    // Bragi devices have more than 1 interface in BIOS mode
    if(kb->protocol == PROTO_NXP && kb->bNumInterfaces < 2 && !IS_SINGLE_EP(kb)){
        if(kb->bNumInterfaces == 1){
            ckb_info("Device is in BIOS mode");
            return -1;
        }
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
#ifndef NDEBUG
    ckb_info("Adding device with vendor = 0x%x, product = 0x%x, path = %s, syspath = %s", vendor, product, path, syspath);
#endif // DEDBUG

    for(int i = 0; i < DEV_MAX; i++){
        // Make sure any existing keyboard doesn't have the same syspath (this shouldn't happen, but it does happen...)
        // We have to do it separately in case a device was unplugged and there's an empty usbdevice
        if(!strcmp(syspath, kbsyspath[i])){
            ckb_warn("Attempted to add device with duplicate syspath %s (path %s)", syspath, path);
            return 1;
        }
    }

    // Find a free USB slot
    for(int index = 1; index < DEV_MAX; index++){
        usbdevice* kb = keyboard + index;
        if(queued_mutex_trylock(dmutex(kb))){
            // If the mutex is locked then the device is obviously in use, so keep going
            continue;
        }
        if(kb->status == DEV_STATUS_DISCONNECTED){
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
                snprintf(kbsyspath[index], FILENAME_MAX, "%s", syspath);
                kb->status = DEV_STATUS_CONNECTING;
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
        if(models[c].idVendor == vid && models[c].idProduct == pid){
            if(IS_EXPERIMENTAL(vid, pid) && !enable_experimental)
                continue;
            return usbadd(dev, models[c].idVendor, models[c].idProduct);
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

///
/// \brief Gracefully resume from suspend.
static void graceful_suspend_resume() {
    // Wait for the kernel to finish reattaching drivers. Avoids race
    // conditions (kernel bug) and situations where the usbhid driver is
    // attached after we already believe to be in control of the device.
    // Retry for at most 5 seconds.
    // XXX: Is 5s too much? It might be possible for interfaces to be left
    // unclaimed indefinitely.
    for (int try = 0; suspend_run && try < 50; try++) {
        bool drivers_attached = true;
        for (int i = 1; i < DEV_MAX; i++) {
            usbdevice* kb = keyboard + i;
            queued_mutex_lock(dmutex(kb));
            if (kb->status == DEV_STATUS_CONNECTED) {
                // Replicates the logic from usbunclaim()
                for(uchar j = 0; j < kb->bNumInterfaces; j++){
                    if(kb->hid_interfaces[i].dont_claim)
                        continue;

                    const uchar bInterfaceNumber = kb->hid_interfaces[j].bInterfaceNumber;

                    struct usbdevfs_getdriver query = {
                        .interface = bInterfaceNumber,
                    };
                    if (ioctl(kb->handle - 1, USBDEVFS_GETDRIVER, &query)) {
                        if (errno == ENODATA) {
                            // No driver attached yet, so keep waiting.
                            drivers_attached = false;
                        }
                        // Ignore other errors.
                    }
                }
            }
            queued_mutex_unlock(dmutex(kb));
        }
        if (drivers_attached) {
#ifndef NDEBUG
            ckb_info("graceful_suspend_resume: drivers_attached");
#endif // DEBUG
            break;
        } else {
            struct timespec sleep_for = { .tv_nsec = 100 * 1000000 };
            clock_nanosleep(CLOCK_MONOTONIC, 0, &sleep_for, NULL);
        }
    }
    // If some interfaces are now claimed by a driver other than usbfs (or
    // none), attemt to reclaim them.
    for (int i = 1; i < DEV_MAX; i++) {
        usbdevice* kb = keyboard + i;
        queued_mutex_lock(dmutex(kb));
        if (kb->status == DEV_STATUS_CONNECTED) {
            bool needs_reclaim = false;
            for(uchar j = 0; j < kb->bNumInterfaces; j++){
                if(kb->hid_interfaces[j].dont_claim)
                    continue;

                const uchar bInterfaceNumber = kb->hid_interfaces[j].bInterfaceNumber;
                struct usbdevfs_getdriver query = {
                    .interface = bInterfaceNumber,
                };
                if (!ioctl(kb->handle - 1, USBDEVFS_GETDRIVER, &query)) {
                    if (strcmp("usbfs", query.driver)) {
#ifndef NDEBUG
                        ckb_info("ckb%d: USBDEVFS_GETDRIVER on interface %d gave %s, reclaiming", i, bInterfaceNumber, query.driver);
#endif // DEBUG
                        needs_reclaim = true;
                        break;
                    }
                } else {
                    if (errno == ENODATA) {
#ifndef NDEBUG
                        ckb_info("ckb%d: USBDEVFS_GETDRIVER on interface %d gave ENODATA, reclaiming", i, bInterfaceNumber);
#endif // DEBUG
                        needs_reclaim = true;
                        break;
                    }
                }
            }
            if (needs_reclaim) {
                usbclaim(kb);
                // Ignore errors for now. reactivate_devices() should
                // try to reset the device.
            }
        }
        queued_mutex_unlock(dmutex(kb));
    }
}

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

static pthread_mutex_t suspend_check_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t suspend_check_cond = PTHREAD_COND_INITIALIZER;
static time_t prev_suspend_check_time;

void* suspend_check() {
    time_t current_time = get_clock_monotonic_seconds();
    if(!current_time)
        return NULL;
    pthread_mutex_lock(&suspend_check_mutex);
    prev_suspend_check_time = current_time;
    pthread_mutex_unlock(&suspend_check_mutex);
    bool woke_up = false;
    while(suspend_run){
        clock_nanosleep(CLOCK_MONOTONIC, 0, &(struct timespec) {.tv_sec = 2}, NULL);
        current_time = get_clock_monotonic_seconds();

        if (prev_suspend_check_time + 4 < current_time) {
            graceful_suspend_resume();
            // Some time might have passed, get a fresh timestamp so other
            // threads can continue
            current_time = get_clock_monotonic_seconds();
            woke_up = true;
        }

        pthread_mutex_lock(&suspend_check_mutex);
        prev_suspend_check_time = current_time;
        pthread_cond_broadcast(&suspend_check_cond);
        pthread_mutex_unlock(&suspend_check_mutex);

        if(woke_up){
            reactivate_devices();
            woke_up = false;
        }
    }
    return NULL;
}

void wait_until_suspend_processed() {
    // Prevent other threads from doing anything after waking from suspend and
    // before the suspend thread has finished reactivate_devices().
    // If necessary, this could be optimized by first checking an _Atomic
    // representing prev_suspend_check_time instead of locking the shared mutex
    // directly.

    time_t current_time = get_clock_monotonic_seconds();
    pthread_mutex_lock(&suspend_check_mutex);
    while (suspend_run && prev_suspend_check_time + 4 < current_time) {
        pthread_cond_wait(&suspend_check_cond, &suspend_check_mutex);
        current_time = get_clock_monotonic_seconds();
    }
    pthread_mutex_unlock(&suspend_check_mutex);
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

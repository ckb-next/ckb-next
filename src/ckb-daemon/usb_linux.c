#include "device.h"
#include "devnode.h"
#include "input.h"
#include "notify.h"
#include "usb.h"

#ifdef OS_LINUX

int os_usbsend(usbdevice* kb, uchar* messages, int count, const char* file, int line){
    for(int i = 0; i < count; i++){
        DELAY_SHORT(kb);
        int res;
        if(kb->fwversion >= 0x120){
#if 1       // Change to #if 1 if using valgrind (4 padding bytes between timeout/data; valgrind thinks they're uninit'd and complains)
            struct usbdevfs_bulktransfer transfer;
            memset(&transfer, 0, sizeof(transfer));
            transfer.ep = 3; transfer.len = MSG_SIZE; transfer.timeout = 5000; transfer.data = messages + MSG_SIZE * i;
#else
            struct usbdevfs_bulktransfer transfer = { 3, MSG_SIZE, 5000, messages + MSG_SIZE * i };
#endif
            res = ioctl(kb->handle, USBDEVFS_BULK, &transfer);
        } else {
            struct usbdevfs_ctrltransfer transfer = { 0x21, 0x09, 0x0300, 0x03, MSG_SIZE, 5000, messages + MSG_SIZE * i };
            res = ioctl(kb->handle, USBDEVFS_CONTROL, &transfer);
        }
        if(res <= 0){
            ckb_err_fn("%s\n", file, line, res ? strerror(-res) : "No data written");
            return 0;
        }
        if(res != MSG_SIZE)
            ckb_err_fn("Wrote %d bytes (expected %d)\n", file, line, res, MSG_SIZE);
    }
    return MSG_SIZE * count;
}

int os_usbrecv(usbdevice* kb, uchar* message, const char* file, int line){
    DELAY_MEDIUM(kb);
    struct usbdevfs_ctrltransfer transfer = { 0xa1, 0x01, 0x0300, 0x03, MSG_SIZE, 5000, message };
    int res = ioctl(kb->handle, USBDEVFS_CONTROL, &transfer);
    if(res <= 0){
        ckb_err_fn("%s\n", file, line, res ? strerror(-res) : "No data read");
        return 0;
    }
    if(res != MSG_SIZE)
        ckb_err_fn("Read %d bytes (expected %d)\n", file, line, res, MSG_SIZE);
    return res;
}

int _nk95cmd(usbdevice* kb, uchar bRequest, ushort wValue, const char* file, int line){
    if(kb->product != P_K95_NRGB)
        return 0;
    struct usbdevfs_ctrltransfer transfer = { 0x40, bRequest, wValue, 0, 0, 5000, 0 };
    int res = ioctl(kb->handle, USBDEVFS_CONTROL, &transfer);
    if(res < 0){
        ckb_err_fn("%s\n", file, line, strerror(-res));
        return 1;
    }
    return 0;
}

void* os_inputmain(void* context){
    usbdevice* kb = context;
    int fd = kb->handle;
    short vendor = kb->vendor, product = kb->product;
    int index = INDEX_OF(kb, keyboard);
    ckb_info("Starting input thread for %s%d\n", devpath, index);

    // Monitor input transfers on all endpoints
    int urbcount = IS_RGB(vendor, product) ? 4 : 3;
    struct usbdevfs_urb urbs[urbcount];
    memset(urbs, 0, sizeof(urbs));
    urbs[0].buffer_length = 8;
    if(IS_RGB(vendor, product)){
        if(IS_MOUSE(vendor, product))
            urbs[1].buffer_length = 10;
        else
            urbs[1].buffer_length = 21;
        urbs[2].buffer_length = MSG_SIZE;
        urbs[3].buffer_length = MSG_SIZE;
    } else {
        urbs[1].buffer_length = 4;
        urbs[2].buffer_length = 15;
    }
    // Submit URBs
    for(int i = 0; i < urbcount; i++){
        urbs[i].type = USBDEVFS_URB_TYPE_INTERRUPT;
        urbs[i].endpoint = 0x80 | (i + 1);
        urbs[i].buffer = malloc(urbs[i].buffer_length);
        ioctl(fd, USBDEVFS_SUBMITURB, urbs + i);
    }
    // Start monitoring input
    while(1){
        struct usbdevfs_urb* urb = 0;
        if(ioctl(fd, USBDEVFS_REAPURB, &urb)){
            if(errno == ENODEV || errno == ENOENT || errno == ESHUTDOWN)
                // Stop the thread if the handle closes
                break;
            else if(errno == EPIPE && urb){
                // On EPIPE, clear halt on the endpoint
                ioctl(fd, USBDEVFS_CLEAR_HALT, &urb->endpoint);
                // Re-submit the URB
                if(urb)
                    ioctl(fd, USBDEVFS_SUBMITURB, urb);
                urb = 0;
            }
        }
        if(urb){
            // Process input (if any)
            pthread_mutex_lock(imutex(kb));
            if(IS_MOUSE(vendor, product)){
                if(urb->endpoint == 0x82){
                    // RGB mouse input
                    hid_mouse_translate(kb->input.keys, &kb->input.rel_x, &kb->input.rel_y, -(urb->endpoint & 0xF), urb->actual_length, urb->buffer);
                }
            } else if(IS_RGB(vendor, product)){
                switch(urb->endpoint){
                case 0x81:
                    // RGB EP 1: 6KRO (BIOS mode) input
                    hid_kb_translate(kb->input.keys, -1, urb->actual_length, urb->buffer);
                    break;
                case 0x82:
                    // RGB EP 2: NKRO (non-BIOS) input. Accept only if keyboard is inactive
                    if(!kb->active)
                        hid_kb_translate(kb->input.keys, -2, urb->actual_length, urb->buffer);
                    break;
                case 0x83:
                    // RGB EP 3: Corsair input
                    memcpy(kb->input.keys, urb->buffer, N_KEYBYTES_KB);
                    break;
                }
            } else
                // Non-RGB input
                hid_kb_translate(kb->input.keys, urb->endpoint & 0xF, urb->actual_length, urb->buffer);
            inputupdate(kb);
            pthread_mutex_unlock(imutex(kb));
            // Re-submit the URB
            ioctl(fd, USBDEVFS_SUBMITURB, urb);
            urb = 0;
        }
    }
    // Clean up
    ckb_info("Stopping input thread for %s%d\n", devpath, index);
    for(int i = 0; i < urbcount; i++){
        ioctl(fd, USBDEVFS_DISCARDURB, urbs + i);
        free(urbs[i].buffer);
    }
    return 0;
}

int usbunclaim(usbdevice* kb, int resetting, int rgb){
    int count = (rgb) ? 4 : 3;
    for(int i = 0; i < count; i++)
        ioctl(kb->handle, USBDEVFS_RELEASEINTERFACE, &i);
    // For RGB keyboards, the kernel driver should only be reconnected to interfaces 0 and 1 (HID), and only if we're not about to do a USB reset.
    // Reconnecting any of the others causes trouble.
    if(!resetting){
        struct usbdevfs_ioctl ctl = { 0, USBDEVFS_CONNECT, 0 };
        ioctl(kb->handle, USBDEVFS_IOCTL, &ctl);
        ctl.ifno = 1;
        ioctl(kb->handle, USBDEVFS_IOCTL, &ctl);
        // Reconnect all handles for non-RGB keyboards
        if(!HAS_FEATURES(kb, FEAT_RGB)){
            ctl.ifno = 0;
            ioctl(kb->handle, USBDEVFS_IOCTL, &ctl);
            ctl.ifno = 2;
            ioctl(kb->handle, USBDEVFS_IOCTL, &ctl);
        }
    }
    return 0;
}

void os_closeusb(usbdevice* kb){
    usbunclaim(kb, 0, HAS_FEATURES(kb, FEAT_RGB));
    close(kb->handle);
    udev_device_unref(kb->udev);
    kb->handle = 0;
    kb->udev = 0;
}

int usbclaim(usbdevice* kb, int rgb){
    // 0 is for BIOS mode/non-RGB key input
    // 1 is for standard HID key input
    // 2 is for Corsair input
    // 3 is for the LED and board controller (not present on non-RGB models)
    int count = (rgb) ? 4 : 3;
    for(int i = 0; i < count; i++){
        struct usbdevfs_ioctl ctl = { i, USBDEVFS_DISCONNECT, 0 };
        if((ioctl(kb->handle, USBDEVFS_IOCTL, &ctl) && errno != ENODATA)
                || ioctl(kb->handle, USBDEVFS_CLAIMINTERFACE, &i))
            return -1;
    }
    return 0;
}

#define TEST_RESET(op)                                                      \
    if(op){                                                                 \
        ckb_err_fn("usbunclaim failed: %s\n", file, line, strerror(errno)); \
        if(errno == EINTR || errno == EAGAIN)                               \
            return -1;              /* try again if status code says so */  \
        return -2;                  /* else, remove device */               \
    }

int os_resetusb(usbdevice* kb, const char* file, int line){
    TEST_RESET(usbunclaim(kb, 1, HAS_FEATURES(kb, FEAT_RGB)));
    TEST_RESET(ioctl(kb->handle, USBDEVFS_RESET));
    TEST_RESET(usbclaim(kb, HAS_FEATURES(kb, FEAT_RGB)));
    // Success!
    return 0;
}

int os_setupusb(usbdevice* kb){
    // Copy device description and serial
    struct udev_device* dev = kb->udev;
    const char* name = udev_device_get_sysattr_value(dev, "product");
    if(name)
        strncpy(kb->name, name, KB_NAME_LEN);
    const char* serial = udev_device_get_sysattr_value(dev, "serial");
    if(serial)
        strncpy(kb->serial, serial, SERIAL_LEN);
    else
        snprintf(kb->serial, SERIAL_LEN, "%04x:%04x-NoID", kb->vendor, kb->product);
    // Copy firmware version (needed to determine USB protocol)
    const char* firmware = udev_device_get_sysattr_value(dev, "bcdDevice");
    if(firmware)
        sscanf(firmware, "%hx", &kb->fwversion);
    else
        kb->fwversion = 0;
    ckb_info("Connecting %s (S/N: %s)\n", kb->name, kb->serial);

    // Claim the USB interfaces
    if(usbclaim(kb, HAS_FEATURES(kb, FEAT_RGB))){
        ckb_err("Failed to claim interface: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

int usbadd(struct udev_device* dev, short vendor, short product){
    const char* path = udev_device_get_devnode(dev);
    // Find a free USB slot
    for(int index = 1; index < DEV_MAX; index++){
        usbdevice* kb = keyboard + index;
        if(pthread_mutex_trylock(dmutex(kb)))
            // If the mutex is locked then the device is obviously in use, so keep going
            continue;
        if(!IS_CONNECTED(kb)){
            // Open the sysfs device
            kb->handle = open(path, O_RDWR);
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
                setupusb(kb);
                return 0;
            }
        }
        pthread_mutex_unlock(dmutex(kb));
    }
    ckb_err("No free devices\n");
    return -1;
}

static struct udev* udev;
pthread_t usbthread, udevthread;

// String -> numeric model map
typedef struct {
    const char* name;
    short number;
} _model;
static _model models[] = {
    // Keyboards
    { P_K65_STR, P_K65 },
    { P_K70_STR, P_K70 },
    { P_K70_NRGB_STR, P_K70_NRGB },
    { P_K95_STR, P_K95 },
    { P_K95_NRGB_STR, P_K95_NRGB },
    // Mice
    { P_M65_STR, P_M65 }
};
#define N_MODELS (sizeof(models) / sizeof(_model))

// Add a udev device. Returns 0 if device was recognized/added.
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

// Remove a udev device.
static void usb_rm_device(struct udev_device* dev){
    // Device removed. Look for it in our list of keyboards
    const char* path = udev_device_get_syspath(dev);
    for(int i = 1; i < DEV_MAX; i++){
        pthread_mutex_lock(devmutex + i);
        if(keyboard[i].udev && !strcmp(path, udev_device_get_syspath(keyboard[i].udev)))
            closeusb(keyboard + i);
        pthread_mutex_unlock(devmutex + i);
    }
}

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

int usbmain(){
    // Load the uinput module (if it's not loaded already)
    if(system("modprobe uinput") != 0)
        ckb_warn("Failed to load uinput module\n");

    // Create the udev object
    if(!(udev = udev_new())){
        ckb_fatal("Failed to initialize udev\n");
        return -1;
    }

    // Enumerate all currently connected devices
    udev_enum();

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

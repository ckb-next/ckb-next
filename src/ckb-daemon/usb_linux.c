#include "device.h"
#include "devnode.h"
#include "input.h"
#include "notify.h"
#include "usb.h"

#ifdef OS_LINUX

int _usbdequeue(usbdevice* kb, const char* file, int line){
    if(kb->queuecount == 0 || !kb->handle)
        return -1;
    struct usbdevfs_ctrltransfer transfer = { 0x21, 0x09, 0x0300, 0x03, MSG_SIZE, 500, kb->queue[0] };
    int res = ioctl(kb->handle, USBDEVFS_CONTROL, &transfer);
    if(res <= 0){
        printf("usbdequeue (%s:%d): %s\n", file, line, res ? strerror(-res) : "No data written");
        return 0;
    }
    if(res != MSG_SIZE)
        printf("usbdequeue (%s:%d): Wrote %d bytes (expected %d)\n", file, line, res, MSG_SIZE);
    // Rotate queue
    uchar* first = kb->queue[0];
    for(int i = 1; i < QUEUE_LEN; i++)
        kb->queue[i - 1] = kb->queue[i];
    kb->queue[QUEUE_LEN - 1] = first;
    kb->queuecount--;
    return res;
}

int _usbinput(usbdevice* kb, uchar* message, const char* file, int line){
    if(!IS_CONNECTED(kb))
        return -1;
    struct usbdevfs_ctrltransfer transfer = { 0xa1, 0x01, 0x0300, 0x03, MSG_SIZE, 500, message };
    int res = ioctl(kb->handle, USBDEVFS_CONTROL, &transfer);
    if(res <= 0){
        printf("usbinput (%s:%d): %s\n", file, line, res ? strerror(-res) : "No data read");
        return 0;
    }
    if(res != MSG_SIZE)
        printf("usbinput (%s:%d): Read %d bytes (expected %d)\n", file, line, res, MSG_SIZE);
    return res;
}

void* intreap(void* context){
    usbdevice* kb = context;
    int fd = kb->handle;
    while(1){
        struct usbdevfs_urb* urb = 0;
        if(ioctl(fd, USBDEVFS_REAPURB, &urb)){
            if(errno == ENODEV || errno == ENOENT || errno == ESHUTDOWN)
                // Stop the thread if the handle closes
                return 0;
            else if(errno == EPIPE && urb){
                // On EPIPE, clear halt on the endpoint
                ioctl(fd, USBDEVFS_CLEAR_HALT, &urb->endpoint);
                // Re-submit the URB
                ioctl(fd, USBDEVFS_SUBMITURB, urb);
                urb = 0;
            }
        }
        if(urb){
            // Process input (if any)
            if(kb->INPUT_READY){
                if(HAS_FEATURES(kb, FEAT_RGB)){
                    switch(urb->endpoint){
                    case 0x81:
                        // RGB EP 1: BIOS HID input
                        hid_translate(kb->kbinput, -1, 8, kb->urbinput);
                        inputupdate(kb);
                        break;
                    case 0x82:
                        // RGB EP 2: non-BIOS HID input (accept only if keyboard is inactive)
                        if(!kb->active){
                            hid_translate(kb->kbinput, -2, 21, kb->urbinput + 8);
                            inputupdate(kb);
                        }
                        break;
                    case 0x83:
                        // RGB EP 3: Corsair input
                        inputupdate(kb);
                        break;
                    }
                } else {
                    switch(urb->endpoint){
                    case 0x81:
                        // Non-RGB EP 1: input
                        hid_translate(kb->kbinput, 1, 8, kb->urbinput);
                        inputupdate(kb);
                        break;
                    case 0x82:
                        // Non-RGB EP 2: media keys
                        hid_translate(kb->kbinput, 2, 4, kb->urbinput + 8);
                        inputupdate(kb);
                        break;
                    }
                }
            }
            // Mark the keyboard as having received input
            kb->INPUT_TEST = 1;
            // Re-submit the URB
            ioctl(fd, USBDEVFS_SUBMITURB, urb);
            urb = 0;
        }
    }
    return 0;
}

void setint(usbdevice* kb, short vendor, short product){
    // Monitor input transfers on all endpoints
    struct usbdevfs_urb* urb = kb->urb;
    urb->type = USBDEVFS_URB_TYPE_INTERRUPT;
    urb->endpoint = 0x81;
    urb->buffer = kb->urbinput;
    urb->buffer_length = 8;
    urb->usercontext = kb;
    ioctl(kb->handle, USBDEVFS_SUBMITURB, urb);

    urb++;
    urb->type = USBDEVFS_URB_TYPE_INTERRUPT;
    urb->endpoint = 0x82;
    urb->buffer = kb->urbinput + 8;
    if(IS_RGB(vendor, product))
        urb->buffer_length = 21;
    else
        urb->buffer_length = 4;
    urb->usercontext = kb;
    ioctl(kb->handle, USBDEVFS_SUBMITURB, urb);

    urb++;
    urb->type = USBDEVFS_URB_TYPE_INTERRUPT;
    urb->endpoint = 0x83;
    if(IS_RGB(vendor, product)){
        urb->buffer = kb->kbinput;
        urb->buffer_length = MSG_SIZE;
    } else {
        urb->buffer = kb->urbinput + 8 + 4;
        urb->buffer_length = 15;
    }
    urb->usercontext = kb;
    ioctl(kb->handle, USBDEVFS_SUBMITURB, urb);

    // Launch a thread to reap transfers
    pthread_create(&kb->usbthread, 0, intreap, kb);
    pthread_detach(kb->usbthread);
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

void closehandle(usbdevice* kb){
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

int os_resetusb(usbdevice* kb, const char* file, int line){
    int res = usbunclaim(kb, 1, HAS_FEATURES(kb, FEAT_RGB));
    if(res){
        printf("resetusb (%s:%d): usbunclaim failed: %s\n", file, line, strerror(errno));
        if(errno == EINTR || errno == EAGAIN)
            return -1;
        return -2;
    }
    res = ioctl(kb->handle, USBDEVFS_RESET, 0);
    if(res){
        printf("resetusb (%s:%d): USBDEVFS_RESET ioctl failed: %s\n", file, line, strerror(errno));
        if(errno == EINTR || errno == EAGAIN)
            return -1;
        return -2;
    }
    res = usbclaim(kb, HAS_FEATURES(kb, FEAT_RGB));
    if(res){
        printf("resetusb (%s:%d): usbclaim failed: %s\n", file, line, strerror(errno));
        if(errno == EINTR || errno == EAGAIN)
            return -1;
        return -2;
    }
    return 0;
}

int openusb(struct udev_device* dev, short vendor, short product){
    // Make sure it's not connected yet
    const char* path = udev_device_get_devnode(dev);
    if(!path)
        return -1;
    for(int i = 1; i < DEV_MAX; i++){
        if(keyboard[i].udev && !strcmp(path, udev_device_get_devnode(keyboard[i].udev)))
            return 0;
    }
    // Find a free USB slot
    for(int index = 1; index < DEV_MAX; index++){
        usbdevice* kb = keyboard + index;
        if(!IS_CONNECTED(kb)){
            // Open the sysfs device
            kb->udev = dev;
            kb->handle = open(path, O_RDWR);
            if(kb->handle <= 0){
                printf("Error: Failed to open USB device: %s\n", strerror(errno));
                udev_device_unref(kb->udev);
                kb->handle = 0;
                kb->udev = 0;
                return -1;
            }

            // Copy device description and serial
            const char* name = udev_device_get_sysattr_value(dev, "product");
            if(name)
                strncpy(kb->name, name, NAME_LEN);
            const char* serial = udev_device_get_sysattr_value(dev, "serial");
            if(serial)
                strncpy(kb->profile.serial, serial, SERIAL_LEN);
            else
                snprintf(kb->profile.serial, SERIAL_LEN, "%04x:%x04-NoID", vendor, product);
            printf("Connecting %s (S/N: %s)\n", kb->name, kb->profile.serial);
            // If the keyboard is a non-RGB model, the FW version needs to be copied here too
            if(!IS_RGB(vendor, product)){
                const char* firmware = udev_device_get_sysattr_value(dev, "bcdDevice");
                if(firmware)
                    // Scan the version number as a hex constant for consistency with RGB devices
                    sscanf(firmware, "%hx", &kb->fwversion);
                else
                    kb->fwversion = 0;
            }

            // Claim the USB interfaces
            if(usbclaim(kb, IS_RGB(vendor, product))){
                printf("Error: Failed to claim interface: %s\n", strerror(errno));
                closehandle(kb);
                return -1;
            }

            // Set up the interrupt transfers.
            // This should be done before setting up the keyboard as the device may freeze if inputs aren't processed.
            kb->INPUT_TEST = 0;
            kb->INPUT_READY = 0;
            setint(kb, vendor, product);

            // Set up the device.
            int setup = setupusb(kb, vendor, product);
            if(setup == -1){
                // -1 indicates a software failure. Give up.
                printf("Failed to set up device.\n");
                closehandle(kb);
                return -1;
            } else if(setup && usb_tryreset(kb)){
                // Any other failure is hardware based. Reset and try again.
                closehandle(kb);
                pthread_mutex_unlock(&kb->mutex);
                pthread_mutex_destroy(&kb->mutex);
                pthread_mutex_destroy(&kb->keymutex);
                DELAY_LONG;
                return -1;
            }

            kb->INPUT_READY = 1;
            updateconnected();
            notifyconnect(kb, 1);
            int index = INDEX_OF(kb, keyboard);
            printf("Device ready at %s%d\n", devpath, index);
            pthread_mutex_unlock(&kb->mutex);
            return 0;
        }
    }
    printf("Error: No free devices\n");
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
    { P_K65_STR, P_K65 },
    { P_K70_STR, P_K70 },
    { P_K70_NRGB_STR, P_K70_NRGB },
    { P_K95_STR, P_K95 },
};
#define N_MODELS (sizeof(models) / sizeof(_model))

void* udevmain(void* context){
    // Enumerate all currently connected devices
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
        const char* product = udev_device_get_sysattr_value(dev, "idProduct");
        int found = 0;
        if(product){
            for(_model* model = models; model < models + N_MODELS; model++){
                if(!strcmp(product, model->name)){
                    found = 1;
                    pthread_mutex_lock(&kblistmutex);
                    openusb(dev, V_CORSAIR, model->number);
                    pthread_mutex_unlock(&kblistmutex);
                    break;
                }
            }
        }
        // Free the device if it wasn't used
        if(!found)
            udev_device_unref(dev);
    }
    udev_enumerate_unref(enumerator);

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
            if(!strcmp(action, "add")){
                // Device added. Check vendor and product ID and add the device if it matches.
                const char* vendor = udev_device_get_sysattr_value(dev, "idVendor");
                if(vendor && !strcmp(vendor, V_CORSAIR_STR)){
                    const char* product = udev_device_get_sysattr_value(dev, "idProduct");
                    int found = 0;
                    if(product){
                        for(_model* model = models; model < models + N_MODELS; model++){
                            if(!strcmp(product, model->name)){
                                found = 1;
                                pthread_mutex_lock(&kblistmutex);
                                openusb(dev, V_CORSAIR, model->number);
                                pthread_mutex_unlock(&kblistmutex);
                                break;
                            }
                        }
                    }
                    // Don't free the device if it's now in use
                    if(found)
                        continue;
                }
            } else if(!strcmp(action, "remove")){
                // Device removed. Look for it in our list of keyboards
                pthread_mutex_lock(&kblistmutex);
                const char* path = udev_device_get_syspath(dev);
                for(int i = 1; i < DEV_MAX; i++){
                    if(keyboard[i].udev && !strcmp(path, udev_device_get_syspath(keyboard[i].udev))){
                        pthread_mutex_lock(&keyboard[i].mutex);
                        closeusb(keyboard + i);
                        break;
                    }
                }
                pthread_mutex_unlock(&kblistmutex);
            }
            udev_device_unref(dev);
        }
    }
    udev_monitor_unref(monitor);
    return 0;
}

int usbinit(){
    // Load the uinput module (if it's not loaded already)
    if(system("modprobe uinput") != 0)
        printf("Warning: Failed to load module uinput\n");

    // Create the udev object
    if(!(udev = udev_new())){
        printf("Fatal: Failed to initialize udev\n");
        return -1;
    }

    // Launch the udev thread
    if(pthread_create(&udevthread, 0, udevmain, 0)){
        printf("Fatal: Failed to start USB thread\n");
        return -1;
    }
    return 0;
}

void usbdeinit(){
    udev_unref(udev);
    udev = 0;
}

#endif

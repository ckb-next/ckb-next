#include "device.h"
#include "devnode.h"
#include "input.h"
#include "notify.h"
#include "usb.h"

#ifdef OS_LINUX

// Number used to track whether or not a device connects successfully
volatile int connectstatus = 0;

int _usbdequeue(usbdevice* kb, const char* file, int line){
    if(kb->queuecount == 0 || !kb->handle)
        return -1;
    struct usbdevfs_ctrltransfer transfer = { 0x21, 0x09, 0x0300, 0x03, MSG_SIZE, 50, kb->queue[0] };
    int res = ioctl(kb->handle, USBDEVFS_CONTROL, &transfer);
    if(res <= 0){
        printf("Error: usbdequeue (%s:%d): %s\n", file, line, res ? strerror(-res) : "No data written");
        return 0;
    }
    if(res != MSG_SIZE)
        printf("Warning: usbdequeue (%s:%d): Wrote %d bytes (expected %d)\n", file, line, res, MSG_SIZE);
    // Rotate queue
    uchar* first = kb->queue[0];
    for(int i = 1; i < QUEUE_LEN; i++)
        kb->queue[i - 1] = kb->queue[i];
    kb->queue[QUEUE_LEN - 1] = first;
    kb->queuecount--;
    return res;
}

int _usbinput(usbdevice* kb, uchar* message, const char* file, int line){
    if(!IS_ACTIVE(kb))
        return -1;
    struct usbdevfs_ctrltransfer transfer = { 0xa1, 0x01, 0x0300, 0x03, MSG_SIZE, 50, message };
    int res = ioctl(kb->handle, USBDEVFS_CONTROL, &transfer);
    if(res <= 0){
        printf("Error: usbinput (%s:%d): %s\n", file, line, res ? strerror(-res) : "No data read");
        return 0;
    }
    if(res != MSG_SIZE)
        printf("Warning: usbinput (%s:%d): Read %d bytes (expected %d)\n", file, line, res, MSG_SIZE);
    return res;
}

void* intreap(void* context){
    usbdevice* kb = context;
    int fd = kb->handle;
    int rgb = HAS_FEATURES(kb, FEAT_RGB);
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
                ioctl(fd, USBDEVFS_SUBMITURB, urb);
                urb = 0;
            }
        }
        if(urb){
            // Process input (if any)
            if(kb->INPUT_READY){
                if(!rgb){
                    // For non RGB keyboards, translate input first
                    switch(urb->endpoint){
                    case 0x81:
                        hid_translate(kb->kbinput, 1, 8, kb->urbinput);
                        break;
                    case 0x82:
                        hid_translate(kb->kbinput, 2, 4, kb->urbinput + 8);
                        break;
                    case 0x83:
                        hid_translate(kb->kbinput, 3, 15, kb->urbinput + 8 + 4);
                        break;
                    }
                    inputupdate(kb);
                } else if(urb->actual_length == MSG_SIZE)
                    // For RGB keyboards, process any input of the correct size
                    inputupdate(kb);
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
    // EPs 1, 2, and 3 are capable of generating input transfers and the keyboard can lock up if they're not received.
    // Only EP 3 (corsair input) is actually interesting, however
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
    // The kernel driver should only be reconnected to interface 1 (HID), and only if we're not about to do a USB reset
    // Reconnecting any of the others causes trouble
    if(!resetting){
        struct usbdevfs_ioctl ctl = { 1, USBDEVFS_CONNECT, 0 };
        ioctl(kb->handle, USBDEVFS_IOCTL, &ctl);
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
    // 0 is useless (but claim it anyway for completeness)
    // 1 is for HID inputs
    // 2 is for Corsair inputs
    // 3 is for the LED and board controller (not present on non-RGB models)
    int count = (rgb) ? 4 : 3;
    for(int i = 0; i < count; i++){
        struct usbdevfs_ioctl ctl = { i, USBDEVFS_DISCONNECT, 0 };
        if((ioctl(kb->handle, USBDEVFS_IOCTL, &ctl) && errno != ENODATA))
            return -1;
    }
    for(int i = 0; i < count; i++){
        if(ioctl(kb->handle, USBDEVFS_CLAIMINTERFACE, &i))
            return -1;
    }
    return 0;
}

int os_resetusb(usbdevice* kb, const char* file, int line){
    int res = usbunclaim(kb, 1, HAS_FEATURES(kb, FEAT_RGB));
    if(res){
        printf("Error: resetusb (%s:%d): usbunclaim failed: %s\n", file, line, strerror(errno));
        if(errno == ENODEV)
            return -2;
        return -1;
    }
    res = ioctl(kb->handle, USBDEVFS_RESET, 0);
    if(res){
        printf("Error: resetusb (%s:%d): USBDEVFS_RESET ioctl failed: %s\n", file, line, strerror(errno));
        if(errno == ENODEV)
            return -2;
        return -1;
    }
    res = usbclaim(kb, HAS_FEATURES(kb, FEAT_RGB));
    if(res){
        printf("Error: resetusb (%s:%d): usbclaim failed: %s\n", file, line, strerror(errno));
        if(errno == ENODEV)
            return -2;
        return -1;
    }
    return 0;
}

int openusb(struct udev_device* dev, short vendor, short product){
    // Make sure it's not connected yet
    const char* path = udev_device_get_devnode(dev);
    for(int i = 1; i < DEV_MAX; i++){
        if(keyboard[i].udev && !strcmp(path, udev_device_get_devnode(keyboard[i].udev)))
            return 0;
    }
    // Find a free USB slot
    for(int index = 1; index < DEV_MAX; index++){
        usbdevice* kb = keyboard + index;
        if(!IS_ACTIVE(kb)){
            // Open the sysfs device
            kb->udev = dev;
            kb->handle = open(path, O_RDWR);
            if(kb->handle <= 0){
                printf("Error: Failed to open USB device: %s\n", strerror(errno));
                // Don't try again if it failed with ENOENT or EINVAL
                if(errno != ENOENT && errno != ENODEV && errno != EINVAL)
                    connectstatus |= 2;
                udev_device_unref(kb->udev);
                kb->handle = 0;
                kb->udev = 0;
                return -1;
            }

            // Copy device description and serial
            strncpy(kb->name, udev_device_get_sysattr_value(dev, "product"), NAME_LEN);
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
                connectstatus |= 2;
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
                connectstatus |= 2;
                pthread_mutex_unlock(&kb->mutex);
                pthread_mutex_destroy(&kb->mutex);
                pthread_mutex_destroy(&kb->keymutex);
                DELAY_LONG;
                return -1;
            }

            kb->INPUT_READY = 1;
            if(IS_RGB(vendor, product)){
                // We should receive an interrupt transfer shortly after setting them up. If it doesn't happen, the device
                // isn't working correctly and needs to be reset
                int received = 0;
                for(int wait = 0; wait < 10; wait++){
                    DELAY_LONG;
                    if(kb->INPUT_TEST){
                        received = 1;
                        break;
                    }
                }
                if(!received && kb->fwversion > 0){
                    printf("Didn't get input, trying to reset...\n");
                    if(usb_tryreset(kb)){
                        closehandle(kb);
                        connectstatus |= 2;
                        pthread_mutex_unlock(&kb->mutex);
                        pthread_mutex_destroy(&kb->mutex);
                        pthread_mutex_destroy(&kb->keymutex);
                        DELAY_LONG;
                        return -1;
                    }
                }
            }

            updateconnected();
            notifyconnect(kb, 1);
            int index = INDEX_OF(kb, keyboard);
            printf("Device ready at %s%d\n", devpath, index);
            connectstatus |= 1;
            pthread_mutex_unlock(&kb->mutex);
            return 0;
        }
    }
    printf("Error: No free devices\n");
    return -1;
}

static struct udev* udev;
pthread_t usbthread, udevthread;

void udevenum(){
    struct udev_enumerate* enumerator = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerator, "usb");
    udev_enumerate_add_match_sysattr(enumerator, "idVendor", V_CORSAIR_STR);
    udev_enumerate_scan_devices(enumerator);
    struct udev_list_entry* devices, *dev_list_entry;
    devices = udev_enumerate_get_list_entry(enumerator);

    udev_list_entry_foreach(dev_list_entry, devices){
        const char* path = udev_list_entry_get_name(dev_list_entry);
        struct udev_device* dev = udev_device_new_from_syspath(udev, path);
        // If the device matches a recognized device ID, open it
        const char* product = udev_device_get_sysattr_value(dev, "idProduct");
        if(!strcmp(product, P_K70_STR)){
            pthread_mutex_lock(&kblistmutex);
            openusb(dev, V_CORSAIR, P_K70);
            pthread_mutex_unlock(&kblistmutex);
            continue;
        }
        if(!strcmp(product, P_K70_NRGB_STR)){
            pthread_mutex_lock(&kblistmutex);
            openusb(dev, V_CORSAIR, P_K70_NRGB);
            pthread_mutex_unlock(&kblistmutex);
            continue;
        }
        if(!strcmp(product, P_K95_STR)){
            pthread_mutex_lock(&kblistmutex);
            openusb(dev, V_CORSAIR, P_K95);
            pthread_mutex_unlock(&kblistmutex);
            continue;
        }
        // Free the device if it wasn't used
        udev_device_unref(dev);
    }
    udev_enumerate_unref(enumerator);
}

void* udevmain(void* context){
    // Enumerate all currently connected devices
    udevenum();
    int tries = 0;
    while((connectstatus & 2) && ++tries < 5){
        // If a device failed to connect, enumerate again to make sure we reconnect it (if possible)
        printf("Trying again...\n");
        connectstatus = 0;
        udevenum();
    }

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
            if(dev){
                const char* action = udev_device_get_action(dev);
                if(action && !strcmp(action, "add")){
                    // Device added. Check vendor and product ID and add the device if it matches.
                    const char* vendor = udev_device_get_sysattr_value(dev, "idVendor");
                    if(vendor && !strcmp(vendor, V_CORSAIR_STR)){
                        const char* product = udev_device_get_sysattr_value(dev, "idProduct");
                        if(product && !strcmp(product, P_K70_STR)){
                            pthread_mutex_lock(&kblistmutex);
                            openusb(dev, V_CORSAIR, P_K70);
                            pthread_mutex_unlock(&kblistmutex);
                        } else if(product && !strcmp(product, P_K70_NRGB_STR)){
                            pthread_mutex_lock(&kblistmutex);
                            openusb(dev, V_CORSAIR, P_K70_NRGB);
                            pthread_mutex_unlock(&kblistmutex);
                        } else if(product && !strcmp(product, P_K95_STR)){
                            pthread_mutex_lock(&kblistmutex);
                            openusb(dev, V_CORSAIR, P_K95);
                            pthread_mutex_unlock(&kblistmutex);
                        } else
                            // Free the device if it wasn't used
                            udev_device_unref(dev);
                    } else
                        udev_device_unref(dev);
                    // Wait a little bit and then re-enumerate devices. Sometimes the keyboard doesn't get picked up right away.
                    DELAY_LONG;
                    connectstatus = 0;
                    udevenum();
                    tries = 0;
                    while((connectstatus & 2) && ++tries < 5){
                        // If a device failed to connect, enumerate again to make sure we reconnect it (if possible)
                        printf("Trying again...\n");
                        connectstatus = 0;
                        udevenum();
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
            }
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

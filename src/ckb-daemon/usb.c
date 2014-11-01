#include "usb.h"
#include "io.h"
#include "led.h"

#include <linux/uinput.h>

usbdevice keyboard[DEV_MAX];
usbstore* store = 0;
int storecount = 0;

usbstore* findstore(const char* serial){
    for(int i = 0; i < storecount; i++){
        usbstore* res = store + i;
        if(!strcmp(res->serial, serial))
            return res;
    }
    return 0;
}

usbstore* addstore(const char* serial){
    // Try to find the device before adding it
    usbstore* res = findstore(serial);
    if(res)
        return res;
    // Add device to the list
    store = realloc(store, ++storecount * sizeof(usbstore));
    res = store + storecount - 1;
    memset(res, 0, sizeof(usbstore));
    strcpy(res->serial, serial);
    return res;
}

int usbqueue(usbdevice* kb, char* messages, int count){
    // Don't add messages unless the queue has enough room for all of them
    if(kb->queuelength + count > QUEUE_LEN)
        return -1;
    for(int i = 0; i < count; i++)
        memcpy(kb->queue[kb->queuelength + i], messages + MSG_SIZE * i, MSG_SIZE);
    kb->queuelength += count;
    return 0;
}

int usbdequeue(usbdevice* kb){
    if(kb->queuelength == 0 || !kb->handle)
        return 0;
    int count = libusb_control_transfer(kb->handle, 0x21, 0x09, 0x0300, 0x03, kb->queue[0], MSG_SIZE, 0);
    // Rotate queue
    char* first = kb->queue[0];
    for(int i = 1; i < QUEUE_LEN; i++)
        kb->queue[i - 1] = kb->queue[i];
    kb->queue[QUEUE_LEN - 1] = first;
    kb->queuelength--;
    return count;
}

void intcallback(struct libusb_transfer* transfer){
    usbdevice* kb = transfer->user_data;
    // If the transfer didn't finish successfully, free it
    if(transfer->status != LIBUSB_TRANSFER_COMPLETED){
        libusb_free_transfer(transfer);
        if(kb->keyint == transfer)
            kb->keyint = 0;
        return;
    }

    // Process key data
    if(transfer->actual_length < MSG_SIZE){
        libusb_submit_transfer(transfer);
        return;
    }
    if(kb->uinput){
        for(int byte = 0; byte < N_KEYS / 8; byte++){
            char oldb = kb->previntinput[byte], newb = kb->intinput[byte];
            if(oldb == newb)
                continue;
            for(int bit = 0; bit < 8; bit++){
                key* map = keymap + byte * 8 + bit;
                char mask = 1 << bit;
                char old = oldb & mask, new = newb & mask;
                if(old != new && map->scan != 0){
                    // If the key state changed, send it to the uinput device
                    struct input_event event;
                    memset(&event, 0, sizeof(event));
                    event.type = EV_KEY;
                    event.code = map->scan;
                    event.value = !!new;
                    if(write(kb->uinput, &event, sizeof(event)) <= 0)
                        printf("Write error: %s\n", strerror(errno));
                    // The volume wheel doesn't generate keyups, so create them automatically
                    if(map->scan == KEY_VOLUMEUP || map->scan == KEY_VOLUMEDOWN){
                        kb->intinput[byte] &= ~mask;
                        event.type = EV_KEY;
                        event.code = map->scan;
                        event.value = 0;
                        if(write(kb->uinput, &event, sizeof(event)) <= 0)
                            printf("Write error: %s\n", strerror(errno));
                    }
                    event.type = EV_SYN;
                    event.code = SYN_REPORT;
                    event.value = 0;
                    if(write(kb->uinput, &event, sizeof(event)) <= 0)
                        printf("Write error: %s\n", strerror(errno));
                }
            }
        }
        memcpy(kb->previntinput, kb->intinput, N_KEYS / 8);
    }

    // Re-submit the transfer
    libusb_submit_transfer(transfer);
}

void setint(usbdevice* kb){
    kb->keyint = libusb_alloc_transfer(0);
    libusb_fill_interrupt_transfer(kb->keyint, kb->handle, 0x83, kb->intinput, MSG_SIZE, intcallback, kb, 0);
    libusb_submit_transfer(kb->keyint);
}

int uinputopen(int index, const struct libusb_device_descriptor* descriptor){
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if(fd <= 0){
        // If that didn't work, try /dev/input/uinput instead
        fd = open("/dev/input/uinput", O_WRONLY | O_NONBLOCK);
        if(fd <= 0){
            printf("Failed to open uinput: %s\n", strerror(errno));
            return fd;
        }
    }
    printf("Setting up uinput device %d\n", index);
    // Set up as a keyboard device and enable all keys
    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    ioctl(fd, UI_SET_EVBIT, EV_SYN);
    for(int i = 0; i < 256; i++)
        ioctl(fd, UI_SET_KEYBIT, i);
    // Create the new input device
    struct uinput_user_dev indev;
    memset(&indev, 0, sizeof(indev));
    snprintf(indev.name, UINPUT_MAX_NAME_SIZE, "ckb%d", index);
    indev.id.bustype = BUS_USB;
    indev.id.vendor = descriptor->idVendor;
    indev.id.product = descriptor->idProduct;
    indev.id.version = 4;
    if(write(fd, &indev, sizeof(indev)) <= 0)
        printf("Write error: %s\n", strerror(errno));
    if(ioctl(fd, UI_DEV_CREATE)){
        printf("Failed to create uinput device: %s\n", strerror(errno));
        close(fd);
        return -1;
    }
    return fd;
}

void uinputclose(int index){
    usbdevice* kb = keyboard + index;
    if(kb->uinput <= 0)
        return;
    printf("Closing uinput device %d\n", index);
    // Set all keys released
    struct input_event event;
    memset(&event, 0, sizeof(event));
    event.type = EV_KEY;
    for(int key = 0; key < 256; key++){
        event.code = key;
        if(write(kb->uinput, &event, sizeof(event)) <= 0)
            printf("Write error: %s\n", strerror(errno));
    }
    event.type = EV_SYN;
    event.code = SYN_REPORT;
    if(write(kb->uinput, &event, sizeof(event)) <= 0)
        printf("Write error: %s\n", strerror(errno));
    // Close the device
    ioctl(kb->uinput, UI_DEV_DESTROY);
    close(kb->uinput);
    kb->uinput = 0;
}

void setinput(usbdevice* kb, int input){
    // Set input mode on the keys. 0x80 generates a normal HID interrupt, 0x40 generates a proprietary interrupt. 0xc0 generates both.
    // NOTE: I observed the windows driver setting a key to 0x49; it seems there are other bits used in this message. I doubt that
    // they're useful, though. Additionally, the windows driver omits some of the key indices, but there seems to be no harm in
    // including all of them.
    char datapkt[6][MSG_SIZE] = { };
    for(int i = 0; i < 4; i++){
        datapkt[i][0] = 0x07;
        datapkt[i][1] = 0x40;
        datapkt[i][2] = 0x1e;
    }
    datapkt[4][0] = 0x07;
    datapkt[4][1] = 0x40;
    datapkt[4][2] = 0x0d;
    datapkt[5][0] = 0x07;
    datapkt[5][1] = 0x05;
    datapkt[5][2] = 0x02;
    datapkt[5][4] = 0x03;
    // The special corsair keys don't have any HID scancode, so don't allow them to generate HID interrupts no matter what
#define IMASK(key) ~(!keymap[key].scan << 7)
    for(int i = 0; i < 30; i++){
        int key = i;
        datapkt[0][i * 2 + 4] = key;
        datapkt[0][i * 2 + 5] = input & IMASK(key);
    }
    for(int i = 0; i < 30; i++){
        int key = i + 30;
        datapkt[1][i * 2 + 4] = key;
        datapkt[1][i * 2 + 5] = input & IMASK(key);
    }
    for(int i = 0; i < 30; i++){
        int key = i + 60;
        datapkt[2][i * 2 + 4] = key;
        datapkt[2][i * 2 + 5] = input & IMASK(key);
    }
    for(int i = 0; i < 30; i++){
        int key = i + 90;
        datapkt[3][i * 2 + 4] = key;
        datapkt[3][i * 2 + 5] = input & IMASK(key);
    }
    for(int i = 0; i < 24; i++){
        int key = i + 120;
        datapkt[4][i * 2 + 4] = key;
        datapkt[4][i * 2 + 5] = input & IMASK(key);
    }
#undef IMASK
    usbqueue(kb, datapkt[0], 6);
}

int usbcmp(libusb_device* dev1, libusb_device* dev2){
    uint8_t num1[7], num2[7];
    int len1, len2;
    if(!dev1 || !dev2 || (len1 = libusb_get_port_numbers(dev1, num1, 7)) <= 0 || (len2 = libusb_get_port_numbers(dev2, num2, 7)) <= 0)
        return -1;
    if(len1 != len2)
        return -2;
    for(int i = 0; i < len1; i++){
        if(num1[i] != num2[i])
            return 1;
    }
    return 0;
}

int openusb(libusb_device* device){
    // Get info and check the manufacturer/product ID
    struct libusb_device_descriptor descriptor;
    if(libusb_get_device_descriptor(device, &descriptor))
        return -1;
    int model;
    if(descriptor.idVendor == V_CORSAIR){
        if(descriptor.idProduct == P_K70){
            model = 70;
            printf("Detected K70 keyboard\n");
        } else if(descriptor.idProduct == P_K95){
            model = 95;
            printf("Detected K95 keyboard\n");
        } else
            return 0;
    } else
        return 0;
    // Make sure it's not connected yet
    for(int i = 1; i < DEV_MAX; i++){
        if(!usbcmp(keyboard[i].dev, device)){
            printf("Already connected\n");
            return 0;
        }
    }
    // Find a free USB slot
    libusb_ref_device(device);
    int devreset = 0;
    for(int index = 1; index < DEV_MAX; index++){
        usbdevice* kb = keyboard + index;
        if(!kb->handle){
            // Open device
            memcpy(&kb->descriptor, &descriptor, sizeof(descriptor));
            kb->dev = device;
            kb->model = model;
            if(libusb_open(device, &kb->handle)){
                printf("Error: Failed to open USB device\n");
                kb->dev = 0;
                return -1;
            }
            // Claim the USB interfaces.
            libusb_set_auto_detach_kernel_driver(kb->handle, 1);
            // 0 is useless (but claim it anyway for completeness)
            if(libusb_claim_interface(kb->handle, 0))
                printf("Warning: Failed to claim interface 0\n");
            // 1 is for HID inputs
            if(libusb_claim_interface(kb->handle, 1)){
                printf("Error: Failed to claim interface 1\n");
                libusb_release_interface(kb->handle, 0);
                libusb_close(kb->handle);
                kb->dev = 0;
                kb->handle = 0;
                return -1;
            }
            // 2 is for Corsair inputs
            if(libusb_claim_interface(kb->handle, 2)){
                printf("Error: Failed to claim interface 2\n");
                libusb_release_interface(kb->handle, 0);
                libusb_release_interface(kb->handle, 1);
                libusb_close(kb->handle);
                kb->dev = 0;
                kb->handle = 0;
                return -1;
            }
            // 3 is for the LED and board controller
            if(libusb_claim_interface(kb->handle, 3)){
                printf("Error: Failed to claim interface 3\n");
                libusb_release_interface(kb->handle, 0);
                libusb_release_interface(kb->handle, 1);
                libusb_release_interface(kb->handle, 2);
                libusb_close(kb->handle);
                kb->dev = 0;
                kb->handle = 0;
                return -1;
            }
            // Reset the device. It acts weird if you don't.
            if(devreset < 2){
                printf("Resetting device\n");
                int reset = libusb_reset_device(kb->handle);
                if(reset){
                    libusb_release_interface(kb->handle, 0);
                    libusb_release_interface(kb->handle, 1);
                    libusb_release_interface(kb->handle, 2);
                    libusb_release_interface(kb->handle, 3);
                    libusb_close(kb->handle);
                    kb->dev = 0;
                    kb->handle = 0;
                    if(reset != LIBUSB_ERROR_NOT_FOUND){
                        printf("Error: Reset failed\n");
                        return -1;
                    }
                    index--;
                    devreset++;
                    continue;
                }
            }
            // Get device description and serial
            if(libusb_get_string_descriptor_ascii(kb->handle, descriptor.iProduct, (unsigned char*)kb->name, NAME_LEN) <= 0
                    || libusb_get_string_descriptor_ascii(kb->handle, descriptor.iSerialNumber, (unsigned char*)kb->serial, SERIAL_LEN) <= 0){
                // If it fails, try to reset the device again
                printf("%s: Failed to get device info%s\n", devreset >= 2 ? "Error" : "Warning", devreset >= 2 ? "" : ", trying another reset...");
                if(devreset < 2){
                    int reset = libusb_reset_device(kb->handle);
                    if(reset){
                        libusb_release_interface(kb->handle, 0);
                        libusb_release_interface(kb->handle, 1);
                        libusb_release_interface(kb->handle, 2);
                        libusb_release_interface(kb->handle, 3);
                        libusb_close(kb->handle);
                        kb->dev = 0;
                        kb->handle = 0;
                        if(reset != LIBUSB_ERROR_NOT_FOUND){
                            printf("Error: Reset failed\n");
                            return -1;
                        }
                        index--;
                        devreset++;
                        continue;
                    }
                    if(libusb_get_string_descriptor_ascii(kb->handle, descriptor.iProduct, (unsigned char*)kb->name, NAME_LEN) <= 0
                            || libusb_get_string_descriptor_ascii(kb->handle, descriptor.iSerialNumber, (unsigned char*)kb->serial, SERIAL_LEN) <= 0){
                        printf("Error: Reset failed\n");
                        return -1;
                    }
                    printf("Reset success\n");
                } else
                    return -1;\
            }
            printf("Connecting %s (S/N: %s)\n", kb->name, kb->serial);

            // Set up a uinput device for key events
            if((kb->uinput = uinputopen(index, &descriptor)) <= 0){
                libusb_release_interface(kb->handle, 0);
                libusb_release_interface(kb->handle, 1);
                libusb_release_interface(kb->handle, 2);
                libusb_release_interface(kb->handle, 3);
                libusb_close(kb->handle);
                kb->dev = 0;
                kb->handle = 0;
                kb->uinput = 0;
                return -1;
            }

            // Make /dev path
            if(makedevpath(index)){
                libusb_release_interface(kb->handle, 0);
                libusb_release_interface(kb->handle, 1);
                libusb_release_interface(kb->handle, 2);
                libusb_release_interface(kb->handle, 3);
                libusb_close(kb->handle);
                kb->dev = 0;
                kb->handle = 0;
                return -1;
            }

            // Create the USB queue
            for(int q = 0; q < QUEUE_LEN; q++)
                kb->queue[q] = malloc(MSG_SIZE);

            // Put the M-keys (K95) as well as the Brightness/Lock keys into software-controlled mode. This packet disables their
            // hardware-based functions.
            char datapkt[64] = { 0x07, 0x04, 0x02 };
            usbqueue(kb, datapkt, 1);
            // Set all keys to use the Corsair input. HID input is unused.
            setinput(kb, IN_CORSAIR);

            // Setup the interrupt handler. These have to be processed asychronously so as not to lock up the animation
            setint(kb);

            // Restore profile (if any)
            usbstore* store = findstore(kb->serial);
            if(store){
                kb->rgb = store->rgb;
                if((kb->rgbon = store->rgbon))
                    updateleds(kb, kb->rgb);
                else
                    updateleds(kb, 0);
            } else {
                // If no profile, set all LEDs to white
                // TODO: Load factory calibration instead
                initrgb(&kb->rgb, &kb->rgbon);
                updateleds(kb, kb->rgb);
            }
            updateconnected();

            printf("Device ready at %s%d\n", devpath, index);
            return 0;
        }
    }
    // No free devices
    printf("Can't connect USB device: No free entries\n");
    return -1;
}

int closeusb(int index){
    // Close file handles
    usbdevice* kb = keyboard + index;
    if(!kb->ledfifo)
        return 0;
    close(kb->ledfifo);
    kb->ledfifo = 0;
    if(kb->handle){
        printf("Disconnecting %s (S/N: %s)\n", kb->name, kb->serial);
        uinputclose(index);
        // Delete USB queue
        for(int i = 0; i < QUEUE_LEN; i++)
            free(kb->queue[i]);
        // Move the RGB data into the device store
        usbstore* store = addstore(kb->serial);
        store->rgb = kb->rgb;
        store->rgbon = kb->rgbon;
        // Close USB device
        libusb_release_interface(kb->handle, 0);
        libusb_release_interface(kb->handle, 1);
        libusb_release_interface(kb->handle, 2);
        libusb_release_interface(kb->handle, 3);
        libusb_close(kb->handle);
        kb->handle = 0;
        kb->dev = 0;
        updateconnected();
    }
    // Delete the control path
    char path[strlen(devpath) + 2];
    snprintf(path, sizeof(path), "%s%d", devpath, index);
    if(rm_recursive(path) != 0 && errno != ENOENT)
        printf("Unable to delete %s: %s\n", path, strerror(errno));
    else
        printf("Removed device path %s\n", path);
    memset(kb, 0, sizeof(*kb));
    return 0;
}

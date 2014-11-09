#include "usb.h"
#include "device.h"
#include "devnode.h"
#include "led.h"
#include "input.h"

#ifdef OS_LINUX

int usbdequeue(usbdevice* kb){
    if(kb->queuecount == 0 || !kb->handle)
        return 0;
    int count = libusb_control_transfer(kb->handle, 0x21, 0x09, 0x0300, 0x03, kb->queue[0], MSG_SIZE, 500);
    // Rotate queue
    unsigned char* first = kb->queue[0];
    for(int i = 1; i < QUEUE_LEN; i++)
        kb->queue[i - 1] = kb->queue[i];
    kb->queue[QUEUE_LEN - 1] = first;
    kb->queuecount--;
    return count;
}

int usbinput(usbdevice* kb, unsigned char* message){
    if(!kb->handle)
        return 0;
    return libusb_control_transfer(kb->handle, 0xa1, 1, 0x0300, 0x03, message, MSG_SIZE, 500);
}

void icorcallback(struct libusb_transfer* transfer){
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
    inputupdate(kb);

    // Re-submit the transfer
    libusb_submit_transfer(transfer);
}

void ihidcallback(struct libusb_transfer* transfer){
    // If the transfer didn't finish successfully, free it
    if(transfer->status != LIBUSB_TRANSFER_COMPLETED){
        free(transfer->buffer);
        libusb_free_transfer(transfer);
        return;
    }
    // All useful inputs come from the corsair interface, so don't bother processing this
    // Re-submit the transfer
    libusb_submit_transfer(transfer);
}

void setint(usbdevice* kb){
    kb->keyint = libusb_alloc_transfer(0);
    libusb_fill_interrupt_transfer(kb->keyint, kb->handle, 0x83, kb->intinput, MSG_SIZE, icorcallback, kb, 0);
    libusb_submit_transfer(kb->keyint);
    struct libusb_transfer* transfer = libusb_alloc_transfer(0);
    libusb_fill_interrupt_transfer(transfer, kb->handle, 0x82, malloc(MSG_SIZE), MSG_SIZE, ihidcallback, kb, 0);
    libusb_submit_transfer(transfer);
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

void closehandle(usbdevice* kb){
    libusb_release_interface(kb->handle, 0);
    libusb_release_interface(kb->handle, 1);
    libusb_release_interface(kb->handle, 2);
    libusb_release_interface(kb->handle, 3);
    libusb_close(kb->handle);
    kb->handle = 0;
    kb->dev = 0;
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
                closehandle(kb);
                return -1;
            }
            // 2 is for Corsair inputs
            if(libusb_claim_interface(kb->handle, 2)){
                printf("Error: Failed to claim interface 2\n");
                closehandle(kb);
                return -1;
            }
            // 3 is for the LED and board controller
            if(libusb_claim_interface(kb->handle, 3)){
                printf("Error: Failed to claim interface 3\n");
                closehandle(kb);
                return -1;
            }
            // Reset the device. It acts weird if you don't.
            if(devreset < 2){
                printf("Resetting device\n");
                int reset = libusb_reset_device(kb->handle);
                if(reset){
                    closehandle(kb);
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
                    || libusb_get_string_descriptor_ascii(kb->handle, descriptor.iSerialNumber, (unsigned char*)kb->setting.serial, SERIAL_LEN) <= 0){
                // If it fails, try to reset the device again
                printf("%s: Failed to get device info%s\n", devreset >= 2 ? "Error" : "Warning", devreset >= 2 ? "" : ", trying to reset...");
                if(devreset < 2){
                    int reset = libusb_reset_device(kb->handle);
                    if(reset){
                        closehandle(kb);
                        if(reset != LIBUSB_ERROR_NOT_FOUND){
                            printf("Error: Reset failed\n");
                            return -1;
                        }
                        index--;
                        devreset++;
                        continue;
                    }
                    if(libusb_get_string_descriptor_ascii(kb->handle, descriptor.iProduct, (unsigned char*)kb->name, NAME_LEN) <= 0
                            || libusb_get_string_descriptor_ascii(kb->handle, descriptor.iSerialNumber, (unsigned char*)kb->setting.serial, SERIAL_LEN) <= 0){
                        printf("Error: Reset failed\n");
                        return -1;
                    }
                    printf("Reset success\n");
                } else
                    return -1;
            }
            printf("Connecting %s (S/N: %s)\n", kb->name, kb->setting.serial);

            // Set up the device
            if(setupusb(index))
                closehandle(kb);

            // Setup the interrupt handler. These have to be processed asychronously so as not to lock up the animation
            setint(kb);

            updateconnected();

            printf("Device ready at %s%d\n", devpath, index);
            return 0;
        }
    }
    printf("Error: No free devices\n");
    return -1;
}

int usbhotplug(struct libusb_context* ctx, struct libusb_device* device, libusb_hotplug_event event, void* user_data){
    pthread_mutex_lock(&kblistmutex);
    printf("Got hotplug event\n");
    int ret = 0;
    if(event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED){
        // Device connected: parse device
        ret = openusb(device);
    } else if(event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT){
        // Device disconnected: look for it in the device list
        for(int i = 1; i < DEV_MAX; i++){
            if(!usbcmp(keyboard[i].dev, device)){
                ret = closeusb(i);
                break;
            }
        }
    }
    pthread_mutex_unlock(&kblistmutex);
    return ret;
}

int usbinit(){
    // Load the uinput module (if it's not loaded already)
    if(system("modprobe uinput") != 0)
        printf("Warning: Failed to load module uinput\n");

    // Start libusb
    if(libusb_init(0)){
        printf("Fatal: Failed to initialize libusb\n");
        return -1;
    }
    libusb_set_debug(0, LIBUSB_LOG_LEVEL_NONE);
    // Enumerate connected devices
    printf("Scanning devices\n");
    libusb_device** devices = 0;
    if(libusb_get_device_list(0, &devices) > 0){
        for(libusb_device** dev = devices; *dev != 0; dev++)
            openusb(*dev);
        libusb_free_device_list(devices, 1);
    } else
        printf("Warning: Failed to scan USB devices\n");
    // Set hotplug callback
    libusb_hotplug_callback_handle hphandle;
    if(libusb_hotplug_register_callback(0, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, 0, V_CORSAIR, LIBUSB_HOTPLUG_MATCH_ANY, LIBUSB_HOTPLUG_MATCH_ANY, usbhotplug, 0, &hphandle) != LIBUSB_SUCCESS)
        printf("Warning: Failed to activate hot plug callback\n");
    printf("Device scan finished\n");
    return 0;
}

void usbmainloop(){
    // Run hotplug callback
    struct timeval tv = { 0, 0 };
    libusb_handle_events_timeout_completed(0, &tv, 0);
}

void usbdeinit(){
    libusb_exit(0);
}

#endif

#include "usb.h"
#include "io.h"
#include "led.h"

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
    // TODO: Actually do something with this data
    printf("Got interrupt on %s\n", kb->serial);
    // If the transfer didn't finish successfully, free it
    if(transfer->status != LIBUSB_TRANSFER_COMPLETED){
        libusb_free_transfer(transfer);
        if(kb->keyint == transfer)
            kb->keyint = 0;
        return;
    }
    // Else, re-submit the transfer
    libusb_submit_transfer(transfer);
}

void setint(usbdevice* kb){
    kb->keyint = libusb_alloc_transfer(0);
    libusb_fill_interrupt_transfer(kb->keyint, kb->handle, 0x83, kb->intinput, MSG_SIZE, intcallback, kb, 0);
    libusb_submit_transfer(kb->keyint);
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
    int devreset = 0;
    for(int index = 1; index < DEV_MAX; index++){
        usbdevice* kb = keyboard + index;
        if(!kb->handle){
            // Open device
            memcpy(&kb->descriptor, &descriptor, sizeof(descriptor));
            kb->dev = device;
            kb->model = model;
            if(libusb_open(device, &kb->handle)){
                printf("Failed to open USB device\n");
                kb->dev = 0;
                return -1;
            }
            // Claim the USB interfaces. 2 is needed for key interrupts, 3 for the LED display.
            libusb_set_auto_detach_kernel_driver(kb->handle, 1);
            if(libusb_claim_interface(kb->handle, 0))
                printf("Warning: Failed to claim interface 0\n");
            if(libusb_claim_interface(kb->handle, 1))
                printf("Warning: Failed to claim interface 1\n");
            if(libusb_claim_interface(kb->handle, 2)){
                printf("Error: Failed to claim interface 2\n");
                libusb_release_interface(kb->handle, 0);
                libusb_release_interface(kb->handle, 1);
                libusb_close(kb->handle);
                kb->dev = 0;
                kb->handle = 0;
                return -1;
            }
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
            // Get device description and serial
            if(libusb_get_string_descriptor_ascii(kb->handle, descriptor.iProduct, (unsigned char*)kb->name, NAME_LEN) <= 0
                    || libusb_get_string_descriptor_ascii(kb->handle, descriptor.iSerialNumber, (unsigned char*)kb->serial, SERIAL_LEN) <= 0){
                printf("%s: Failed to get device info%s\n", devreset ? "Error" : "Warning", devreset ? "" : ", trying reset...");
                int reset = libusb_reset_device(kb->handle);
                if(reset){
                    libusb_release_interface(kb->handle, 0);
                    libusb_release_interface(kb->handle, 1);
                    libusb_release_interface(kb->handle, 2);
                    libusb_release_interface(kb->handle, 3);
                    libusb_close(kb->handle);
                    kb->dev = 0;
                    kb->handle = 0;
                    if(reset != LIBUSB_ERROR_NOT_FOUND)
                        return -1;
                    index--;
                    devreset = 1;
                    continue;
                }
                if(libusb_get_string_descriptor_ascii(kb->handle, descriptor.iProduct, (unsigned char*)kb->name, NAME_LEN) <= 0
                        || libusb_get_string_descriptor_ascii(kb->handle, descriptor.iSerialNumber, (unsigned char*)kb->serial, SERIAL_LEN) <= 0){
                    printf("Error: Reset failed\n");
                    return -1;
                }
                printf("Reset success\n");
            }
            printf("Connecting %s (S/N: %s)\n", kb->name, kb->serial);
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
            printf("Device ready at %s%d\n", devpath, index);

            // Create the USB queue
            for(int q = 0; q < QUEUE_LEN; q++)
                kb->queue[q] = malloc(MSG_SIZE);

            // Put the M-keys (K95) as well as the Brightness/Lock keys into software-controlled mode. This packet disables their
            // hardware-based functions.
            char datapkt[7][64] = { { 0x07, 0x04, 0x02, 0} };

            // Set interrupt mode on the keys. 0x80 generates a normal HID interrupt, 0x40 generates a custom interrupt. 0xc0 generates both.
            // NOTE: I observed the windows driver setting a key to 0x49. Seems there are other bits in use here. I'm also a bit uncertain
            // about the order of keys here; I got the 0x40 entries by looking at the Windows packet. It seems the K95 keys are all at the end,
            // but the rest of them don't correspond to the LED indices in any apparent manner. Might just be totally different mapping.

            // For now, set the extra keys to use the special interrupt and everything else to use regular. This is the board's default
            // behavior anyway. Special interrupts will be activated for other keys later, for animation/rebinding purposes.

            // TODO: It would probably be a good idea to reset everything to 0x80 when quitting the driver so that the keys won't lock up if
            // you press them afterward.

            for(int i = 1; i < 5; i++){
                datapkt[i][0] = 0x07;
                datapkt[i][1] = 0x40;
                datapkt[i][2] = 0x1e;
            }
            datapkt[5][0] = 0x07;
            datapkt[5][1] = 0x40;
            datapkt[5][2] = 0x0d;
            datapkt[6][0] = 0x07;
            datapkt[6][1] = 0x05;
            datapkt[6][2] = 0x02;
            datapkt[6][4] = 0x03;
            for(int i = 0; i < 30; i++){
                datapkt[1][i * 2 + 4] = i;
                datapkt[1][i * 2 + 5] = 0x80;
            }
            for(int i = 0; i < 30; i++){
                // Skip 0x31
                int val = i + 0x1e;
                if(val > 0x30)
                    val++;
                datapkt[2][i * 2 + 4] = val;
                datapkt[2][i * 2 + 5] = 0x80;
            }
            for(int i = 0; i < 30; i++){
                // Skip 3f, 41, 42, 46, 51, 53, 55
                int val = i + 0x3d;
                if(val > 0x3e)
                    val++;
                if(val > 0x40)
                    val += 2;
                if(val > 0x45)
                    val++;
                if(val > 0x50)
                    val++;
                if(val > 0x52)
                    val++;
                if(val > 0x54)
                    val++;
                datapkt[3][i * 2 + 4] = val;
                datapkt[3][i * 2 + 5] = (val == 0x47 || val == 0x60) ? 0x40 : 0x80;
            }
            for(int i = 0; i < 30; i++){
                // Skip 6f
                int val = i + 0x62;
                if(val > 0x6e)
                    val++;
                datapkt[4][i * 2 + 4] = val;
                datapkt[4][i * 2 + 5] = (val > 0x77) ? 0x40 : 0x80;
            }
            for(int i = 0; i < 13; i++){
                // Skip 82, 83
                int val = i + 0x81;
                if(val > 0x81)
                    val += 2;
                datapkt[5][i * 2 + 4] = val;
                datapkt[5][i * 2 + 5] = 0x40;
            }
            // Send out the messages.
            usbqueue(kb, datapkt[0], 7);

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

            libusb_release_interface(kb->handle, 0);
            libusb_release_interface(kb->handle, 1);
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
        // Delete USB queue
        for(int i = 0; i < QUEUE_LEN; i++)
            free(kb->queue[i]);
        // Move the RGB data into the device store
        usbstore* store = addstore(kb->serial);
        store->rgb = kb->rgb;
        store->rgbon = kb->rgbon;
        // Close USB device
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

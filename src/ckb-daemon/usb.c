#include "usb.h"
#include "devnode.h"
#include "led.h"
#include "input.h"

usbdevice keyboard[DEV_MAX];
usbsetting* store = 0;
int storecount = 0;

usbdevice* findusb(const char* serial){
    for(int i = 0; i < DEV_MAX; i++){
        if(keyboard[i].fifo && !strcmp(serial, keyboard[i].setting.serial))
            return keyboard + i;
    }
    return 0;
}

usbsetting* findstore(const char* serial){
    for(int i = 0; i < storecount; i++){
        usbsetting* res = store + i;
        if(!strcmp(res->serial, serial))
            return res;
    }
    return 0;
}

usbsetting* addstore(const char* serial){
    // Try to find the device before adding it
    usbsetting* res = findstore(serial);
    if(res)
        return res;
    // Add device to the list
    store = realloc(store, ++storecount * sizeof(usbsetting));
    res = store + storecount - 1;
    // Initialize device
    memset(&res->profile, 0, sizeof(res->profile));
    strcpy(res->serial, serial);
    genid(&res->profile.id);
    return res;
}

usbmode* getusbmode(int id, usbprofile* profile){
    if(id < profile->modecount)
        return profile->mode + id;
    int cap = id / 4 * 4 + 4;
    if(cap > profile->modecap){
        profile->mode = realloc(profile->mode, cap * sizeof(usbmode));
        profile->modecap = cap;
    }
    // Initialize any newly-created modes
    for(int i = profile->modecount; i <= id; i++){
        initrgb(&profile->mode[i].light);
        initbind(&profile->mode[i].bind);
        memset(profile->mode[i].name, 0, sizeof(profile->mode[i].name));
        genid(&profile->mode[i].id);
    }
    profile->modecount = id + 1;
    return profile->mode + id;
}

iconv_t utf8to16 = 0;

void urldecode2(char *dst, const char *src){
    char a, b;
    while (*src) {
        if ((*src == '%') &&
                ((a = src[1]) && (b = src[2])) &&
                (isxdigit(a) && isxdigit(b))) {
            if (a >= 'a')
                a -= 'a'-'A';
            if (a >= 'A')
                a -= ('A' - 10);
            else
                a -= '0';
            if (b >= 'a')
                b -= 'a'-'A';
            if (b >= 'A')
                b -= ('A' - 10);
            else
                b -= '0';
            *dst++ = 16*a+b;
            src+=3;
        } else {
            *dst++ = *src++;
        }
    }
    *dst++ = '\0';
}

void setmodename(usbmode* mode, const char* name){
    if(!utf8to16)
        utf8to16 = iconv_open("UTF-16LE", "UTF-8");
    memset(mode->name, 0, sizeof(mode->name));
    char decoded[strlen(name) + 1];
    urldecode2(decoded, name);
    size_t srclen = strlen(decoded), dstlen = MD_NAME_LEN * 2 - 2;
    char* in = decoded, *out = (char*)mode->name;
    iconv(utf8to16, &in, &srclen, &out, &dstlen);
}

void setprofilename(usbprofile* profile, const char* name){
    if(!utf8to16)
        utf8to16 = iconv_open("UTF-16LE", "UTF-8");
    memset(profile->name, 0, sizeof(profile->name));
    char decoded[strlen(name) + 1];
    urldecode2(decoded, name);
    size_t srclen = strlen(decoded), dstlen = PR_NAME_LEN * 2 - 2;
    char* in = decoded, *out = (char*)profile->name;
    iconv(utf8to16, &in, &srclen, &out, &dstlen);
}

void erasemode(usbmode *mode){
    closebind(&mode->bind);
    memset(mode, 0, sizeof(*mode));
    initrgb(&mode->light);
    initbind(&mode->bind);
    genid(&mode->id);
}

void eraseprofile(usbprofile* profile){
    // Clear all mode data
    for(int i = 0; i < profile->modecount; i++)
        closebind(&profile->mode[i].bind);
    free(profile->mode);
    memset(profile, 0, sizeof(*profile));
    genid(&profile->id);
}

void genid(usbid* id){
    static int seeded = 0;
    if(!seeded){
        srand(time(NULL));
        seeded = 1;
    }
    // Generate a random ID
    int numbers[4] = { rand(), rand(), rand(), rand() };
    memcpy(id->guid, numbers, sizeof(id->guid));
    memset(id->modified, 0, sizeof(id->modified));
}

void updatemod(usbid* id){
    struct timeval tv;
    gettimeofday(&tv, 0);
    short new = (short)tv.tv_usec, old;
    memcpy(&old, id->modified, 2);
    if(new == old)
        new++;
    memcpy(id->modified, &new, 2);
}

void hwloadmode(usbdevice* kb, int mode){
    // Ask for mode's name
    usbmode* kbmode = kb->setting.profile.mode + mode;
    unsigned char data_pkt[MSG_SIZE] = { 0x0e, 0x16, 0x01, mode + 1, 0 };
    usbqueue(kb, data_pkt, 1);
    usleep(3333);
    usbdequeue(kb);
    // Wait for the response
    libusb_control_transfer(kb->handle, 0xa1, 1, 0x0300, 0x03, data_pkt, MSG_SIZE, 500);
    if(data_pkt[0] == 0x0e && data_pkt[1] == 0x01)
        memcpy(kbmode->name, data_pkt + 4, MD_NAME_LEN * 2);
    // Load the RGB setting
    loadleds(kb, mode);
}

void hwloadprofile(usbdevice* kb){
    if(!kb || !kb->handle)
        return;
    // Empty the board's USB queue
    while(kb->queuecount > 0){
        usleep(3333);
        usbdequeue(kb);
    }
    // Ask for profile ID
    usbprofile* profile = &kb->setting.profile;
    unsigned char data_pkt[2][MSG_SIZE] = {
        { 0x0e, 0x15, 0x01, 0 },
        { 0x0e, 0x16, 0x01, 0 }
    };
    unsigned char in_pkt[MSG_SIZE];
    usbqueue(kb, data_pkt[0], 1);
    usleep(3333);
    usbdequeue(kb);
    // Wait for the response
    libusb_control_transfer(kb->handle, 0xa1, 1, 0x0300, 0x03, in_pkt, MSG_SIZE, 500);
    memcpy(&profile->id, in_pkt + 4, sizeof(usbid));
    // Ask for mode IDs
    int modes = (kb->model == 95 ? 3 : 1);
    for(int i = 0; i < modes; i++){
        data_pkt[0][3] = i + 1;
        usbqueue(kb, data_pkt[0], 1);
        usleep(3333);
        usbdequeue(kb);
        // Wait for the response
        libusb_control_transfer(kb->handle, 0xa1, 1, 0x0300, 0x03, in_pkt, MSG_SIZE, 500);
        memcpy(&profile->mode[i].id, in_pkt + 4, sizeof(usbid));
    }
    // Ask for profile name
    usbqueue(kb, data_pkt[1], 1);
    usleep(3333);
    usbdequeue(kb);
    // Wait for the response
    libusb_control_transfer(kb->handle, 0xa1, 1, 0x0300, 0x03, in_pkt, MSG_SIZE, 500);
    memcpy(kb->setting.profile.name, in_pkt + 4, PR_NAME_LEN * 2);
    // Load modes
    for(int i = 0; i < modes; i++)
        hwloadmode(kb, i);
}

void hwsaveprofile(usbdevice* kb){
    if(!kb || !kb->handle)
        return;
    // Save the profile name
    usbprofile* profile = &kb->setting.profile;
    unsigned char data_pkt[2][MSG_SIZE] = {
        {0x07, 0x16, 0x01, 0 },
        {0x07, 0x15, 0x01, 0, 1, 2, 3, 4, 5 },
    };
    memcpy(data_pkt[0] + 4, profile->name, PR_NAME_LEN * 2);
    usbqueue(kb, data_pkt[0], 1);
    // Save the mode names
    int modes = (kb->model == 95 ? 3 : 1);
    for(int i = 0; i < modes; i++){
        data_pkt[0][3] = i + 1;
        memcpy(data_pkt[0] + 4, profile->mode[i].name, MD_NAME_LEN * 2);
        usbqueue(kb, data_pkt[0], 1);
    }
    // Save the profile ID
    memcpy(data_pkt[1] + 4, &profile->id, sizeof(usbid));
    usbqueue(kb, data_pkt[1], 1);
    // Save the mode IDs
    for(int i = 0; i < modes; i++){
        data_pkt[1][3] = i + 1;
        memcpy(data_pkt[1] + 4, &profile->mode[i].id, sizeof(usbid));
        usbqueue(kb, data_pkt[1], 1);
    }
    // Save the RGB data
    for(int i = 0; i < modes; i++)
        saveleds(kb, i);
}

int usbqueue(usbdevice* kb, unsigned char* messages, int count){
    // Don't add messages unless the queue has enough room for all of them
    if(kb->queuecount + count > QUEUE_LEN)
        return -1;
    for(int i = 0; i < count; i++)
        memcpy(kb->queue[kb->queuecount + i], messages + MSG_SIZE * i, MSG_SIZE);
    kb->queuecount += count;
    return 0;
}

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

void setinput(usbdevice* kb, int input){
    // Set input mode on the keys. 0x80 generates a normal HID interrupt, 0x40 generates a proprietary interrupt. 0xc0 generates both.
    // NOTE: I observed the windows driver setting a key to 0x49; it seems there are other bits used in this message. I doubt that
    // they're useful, though. Additionally, the windows driver omits some of the key indices, but there seems to be no harm in
    // including all of them.
    unsigned char datapkt[6][MSG_SIZE] = { };
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
#define IMASK(key) ~((keymap[key].scan == -1) << 7)
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
#ifdef OS_LINUX
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
#endif
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
#ifdef OS_MAC
            // OSX has some problems sending packets to the device immediately
            sleep(1);
#endif

            // Set up an input device for key events
            if(!inputopen(index, &descriptor)){
                closehandle(kb);
                return -1;
            }
            updateindicators(kb, 1);

            // Make /dev path
            if(makedevpath(index)){
                inputclose(index);
                closehandle(kb);
                return -1;
            }

            // Create the USB queue
            for(int q = 0; q < QUEUE_LEN; q++)
                kb->queue[q] = malloc(MSG_SIZE);

            // Put the M-keys (K95) as well as the Brightness/Lock keys into software-controlled mode. This packet disables their
            // hardware-based functions.
            unsigned char datapkt[64] = { 0x07, 0x04, 0x02 };
            usbqueue(kb, datapkt, 1);
            // Set all keys to use the Corsair input. HID input is unused.
#ifdef OS_LINUX
            setinput(kb, IN_CORSAIR);
#else
            setinput(kb, IN_HID);
#endif

            // Setup the interrupt handler. These have to be processed asychronously so as not to lock up the animation
            setint(kb);

            // Restore profile (if any)
            usbsetting* store = findstore(kb->setting.serial);
            if(store){
                memcpy(&kb->setting.profile, &store->profile, sizeof(store->profile));
            } else {
                // If there is no profile, load it from the device
                kb->setting.profile.currentmode = getusbmode(0, &kb->setting.profile);
                getusbmode(1, &kb->setting.profile);
                getusbmode(2, &kb->setting.profile);
                hwloadprofile(kb);
            }
            updateleds(kb);

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
    if(!kb->fifo)
        return 0;
    close(kb->fifo);
    kb->fifo = 0;
    if(kb->handle){
        printf("Disconnecting %s (S/N: %s)\n", kb->name, kb->setting.serial);
        inputclose(index);
        // Delete USB queue
        for(int i = 0; i < QUEUE_LEN; i++)
            free(kb->queue[i]);
        // Move the profile data into the device store
        usbsetting* store = addstore(kb->setting.serial);
        memcpy(&store->profile, &kb->setting.profile, sizeof(kb->setting.profile));
        // Reset and close USB device
        libusb_reset_device(kb->handle);
        closehandle(kb);
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

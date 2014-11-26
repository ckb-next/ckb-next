#include "devnode.h"
#include "firmware.h"
#include "notify.h"
#include "usb.h"

#define FW_OK       0
#define FW_NOFILE   -1
#define FW_WRONGDEV -2
#define FW_USBFAIL  -3

#define FW_MAXSIZE  (64 * 1024 - 256)

int getfwversion(usbdevice* kb){
    // Empty the board's USB queue
    while(kb->queuecount > 0){
        DELAY_SHORT;
        if(!usbdequeue(kb))
            return -1;
    }
    // Ask board for firmware info
    uchar data_pkt[MSG_SIZE] = { 0x0e, 0x01, 0 };
    usbqueue(kb, data_pkt, 1);
    if(!usbdequeue(kb))
        return -1;
    // Wait for the response
    DELAY_MEDIUM;
    uchar in_pkt[MSG_SIZE];
    if(!usbinput(kb, in_pkt) || in_pkt[0] != 0x0e || in_pkt[1] != 0x01)
        return -1;
    short vendor, product, version;
    // Copy the vendor ID, product ID, and version from the firmware data
    memcpy(&version, in_pkt + 8, 2);
    memcpy(&vendor, in_pkt + 12, 2);
    memcpy(&product, in_pkt + 14, 2);
    // Print a warning if the vendor or product isn't what it should be
    if(vendor != V_CORSAIR)
        printf("Warning: Got vendor ID %04x (expected %04x)", vendor, V_CORSAIR);
    int eproduct = (kb->model == 95) ? P_K95 : P_K70;
    if(product != eproduct)
        printf("Warning: Got product ID %04x (expected %04x)", product, eproduct);
    // Set firmware version
    kb->fwversion = version;
    writefwnode(kb);
    return 0;
}

// Updates the device's firmware with the specified file. Returns one of the FW_ constants.
// Lock the keyboard's main mutex before calling this and unlock it when done.
int fwupdate(usbdevice* kb, const char* path){
    // Read the firmware from the given path
    char* fwdata = calloc(1, 64 * 1024);
    int fd = open(path, O_RDONLY);
    if(fd <= 0){
        printf("Error: Failed to open firmware file %s: %s\n", path, strerror(errno));
        return FW_NOFILE;
    }
    ssize_t length = read(fd, fwdata, FW_MAXSIZE);
    if(length <= 0x108 || length >= FW_MAXSIZE){
        printf("Error: Failed to read firmware file %s: %s\n", path, length <= 0 ? strerror(errno) : "Wrong size");
        close(fd);
        return FW_NOFILE;
    }
    close(fd);

    short vendor, product, version;
    // Copy the vendor ID, product ID, and version from the firmware file
    memcpy(&vendor, fwdata + 0x102, 2);
    memcpy(&product, fwdata + 0x104, 2);
    memcpy(&version, fwdata + 0x106, 2);
    // Check against the actual device
    if(vendor != V_CORSAIR || !((kb->model == 70 && product == P_K70) || (kb->model == 95 && product == P_K95))){
        printf("Error: Firmware file %s doesn't match device (V: %04x P: %04x)\n", path, vendor, product);
        return FW_WRONGDEV;
    }
    printf("Loading firmware version %04x from %s\n", version, path);
    // Empty the board's USB queue
    while(kb->queuecount > 0){
        DELAY_SHORT;
        if(!usbdequeue(kb)){
            printf("Error: Firmware update failed\n");
            return FW_USBFAIL;
        }
    }
    // Send the firmware messages
    uchar data_pkt[7][MSG_SIZE] = {
        { 0x07, 0x0c, 0xf0, 0x01, 0 },
        { 0x07, 0x0d, 0xf0, 0 },
        { 0x7f, 0x01, 0x3c, 0 },
        { 0x7f, 0x02, 0x3c, 0 },
        { 0x7f, 0x03, 0x3c, 0 },
        { 0x7f, 0x04, 0x3c, 0 },
        { 0x7f, 0x05, 0x10, 0 }
    };
    int output = 0, last = 0;
    int index = 0;
    while(output < length){
        int npackets = 2;
        // Packet 1: data position
        data_pkt[1][6] = index++;
        // Packets 2-5: 60 bytes of data
        memcpy(data_pkt[2] + 4, fwdata + output, 60);
        last = output;
        output += 60;
        if(output < length){
            npackets++;
            memcpy(data_pkt[3] + 4, fwdata + output, 60);
            last = output;
            output += 60;
            if(output < length){
                npackets++;
                memcpy(data_pkt[4] + 4, fwdata + output, 60);
                last = output;
                output += 60;
                if(output < length){
                    npackets++;
                    memcpy(data_pkt[5] + 4, fwdata + output, 60);
                    last = output;
                    output += 60;
                    if(output < length){
                        npackets++;
                        // Packet 6: 16 bytes
                        memcpy(data_pkt[6] + 4, fwdata + output, 16);
                        last = output;
                        output += 16;
                    }
                }
            }
        }
        if(index == 1){
            usbqueue(kb, data_pkt[0], 1);
            DELAY_MEDIUM;
            if(!usbdequeue(kb)){
                printf("Error: Firmware update failed\n");
                return FW_USBFAIL;
            }
            // The above packet can take a lot longer to process, so wait for a while
            sleep(3);
            usbqueue(kb, data_pkt[2], npackets - 1);
        } else {
            // If the output ends here, set the length byte appropriately
            if(output >= length)
                data_pkt[npackets][2] = length - last;
            usbqueue(kb, data_pkt[1], npackets);
        }
        // Run the queue
        while(kb->queuecount > 0){
            DELAY_MEDIUM;
            if(!usbdequeue(kb)){
                printf("Error: Firmware update failed\n");
                return FW_USBFAIL;
            }
        }
    }
    // Send the final pair of messages
    uchar data_pkt2[2][MSG_SIZE] = {
        { 0x07, 0x0d, 0xf0, 0x00, 0x00, 0x00, index },
        { 0x07, 0x02, 0xf0, 0 }
    };
    usbqueue(kb, data_pkt2[0], 2);
    while(kb->queuecount > 0){
        DELAY_MEDIUM;
        if(!usbdequeue(kb)){
            printf("Error: Firmware update failed\n");
            return FW_USBFAIL;
        }
    }
    // Updated successfully
    kb->fwversion = version;
    writefwnode(kb);
    printf("Firmware update complete\n");
    return FW_OK;
}

void cmd_fwupdate(usbdevice* kb, const char* path){
    // Lock the keyboard's mutex first
    pthread_mutex_lock(&kb->mutex);
    // Update the firmware
    int ret = fwupdate(kb, path);
    pthread_mutex_unlock(&kb->mutex);
    switch(ret){
    case FW_OK:
        nprintf(kb, 0, 0, "fwupdate %s ok\n", path);
        break;
    case FW_NOFILE:
    case FW_WRONGDEV:
        nprintf(kb, 0, 0, "fwupdate %s invalid\n", path);
        break;
    case FW_USBFAIL:
        nprintf(kb, 0, 0, "fwupdate %s fail\n", path);
        break;
    }
}

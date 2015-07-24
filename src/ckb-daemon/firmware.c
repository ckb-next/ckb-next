#include "devnode.h"
#include "firmware.h"
#include "notify.h"
#include "usb.h"

#define FW_OK       0
#define FW_NOFILE   -1
#define FW_WRONGDEV -2
#define FW_USBFAIL  -3

int getfwversion(usbdevice* kb){
    // Ask board for firmware info
    uchar data_pkt[MSG_SIZE] = { 0x0e, 0x01, 0 };
    uchar in_pkt[MSG_SIZE];
    if(!usbrecv(kb, data_pkt, in_pkt))
        return -1;
    if(in_pkt[0] != 0x0e || in_pkt[1] != 0x01){
        ckb_err("Bad input header\n");
        return -1;
    }
    short vendor, product, version, bootloader;
    // Copy the vendor ID, product ID, version, and poll rate from the firmware data
    memcpy(&version, in_pkt + 8, 2);
    memcpy(&bootloader, in_pkt + 10, 2);
    memcpy(&vendor, in_pkt + 12, 2);
    memcpy(&product, in_pkt + 14, 2);
    char poll = in_pkt[16];
    if(poll <= 0){
        poll = -1;
        kb->features &= ~FEAT_POLLRATE;
    }
    // Print a warning if the message didn't match the expected data
    if(vendor != kb->vendor)
        ckb_warn("Got vendor ID %04x (expected %04x)\n", vendor, kb->vendor);
    if(product != kb->product)
        ckb_warn("Got product ID %04x (expected %04x)\n", product, kb->product);
    // Set firmware version and poll rate
    if(version == 0 || bootloader == 0){
        // Needs firmware update
        kb->fwversion = 0;
        kb->pollrate = -1;
    } else {
        if(version != kb->fwversion && kb->fwversion != 0)
            ckb_warn("Got firmware version %04x (expected %04x)\n", version, kb->fwversion);
        kb->fwversion = version;
        kb->pollrate = poll;
    }
    return 0;
}

#define FW_MAXSIZE  (255 * 256)

// Updates the device's firmware with the specified file. Returns one of the FW_ constants.
// Lock the keyboard's main mutex before calling this and unlock it when done.
int fwupdate(usbdevice* kb, const char* path, int nnumber){
    // Read the firmware from the given path
    char* fwdata = calloc(1, FW_MAXSIZE + 256);
    int fd = open(path, O_RDONLY);
    if(fd == -1){
        ckb_err("Failed to open firmware file %s: %s\n", path, strerror(errno));
        return FW_NOFILE;
    }
    ssize_t length = read(fd, fwdata, FW_MAXSIZE + 1);
    if(length <= 0x108 || length > FW_MAXSIZE){
        ckb_err("Failed to read firmware file %s: %s\n", path, length <= 0 ? strerror(errno) : "Wrong size");
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
    if(vendor != kb->vendor || product != kb->product){
        ckb_err("Firmware file %s doesn't match device (V: %04x P: %04x)\n", path, vendor, product);
        return FW_WRONGDEV;
    }
    ckb_info("Loading firmware version %04x from %s\n", version, path);
    nprintf(kb, nnumber, 0, "fwupdate %s 0/%d\n", path, (int)length);
    // Force the device to 10ms delay (we need to deliver packets very slowly to make sure it doesn't get overwhelmed)
    kb->usbdelay = 10;
    // Send the firmware messages (256 bytes at a time)
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
        int npackets = 1;
        // Packet 1: data position
        data_pkt[1][6] = index++;
        while(output < length){
            npackets++;
            if(npackets != 6){
                // Packets 2-5: 60 bytes of data
                memcpy(data_pkt[npackets] + 4, fwdata + output, 60);
                last = output;
                output += 60;
            } else {
                // Packet 6: 16 bytes
                memcpy(data_pkt[npackets] + 4, fwdata + output, 16);
                last = output;
                output += 16;
                break;
            }
        }
        if(index == 1){
            if(!usbsend(kb, data_pkt[0], 1)){
                ckb_err("Firmware update failed\n");
                return FW_USBFAIL;
            }
            // The above packet can take a lot longer to process, so wait for a while
            sleep(3);
            if(!usbsend(kb, data_pkt[2], npackets - 1)){
                ckb_err("Firmware update failed\n");
                return FW_USBFAIL;
            }
        } else {
            // If the output ends here, set the length byte appropriately
            if(output >= length)
                data_pkt[npackets][2] = length - last;
            if(!usbsend(kb, data_pkt[1], npackets)){
                ckb_err("Firmware update failed\n");
                return FW_USBFAIL;
            }
        }
        nprintf(kb, nnumber, 0, "fwupdate %s %d/%d\n", path, output, (int)length);
    }
    // Send the final pair of messages
    uchar data_pkt2[2][MSG_SIZE] = {
        { 0x07, 0x0d, 0xf0, 0x00, 0x00, 0x00, index },
        { 0x07, 0x02, 0xf0, 0 }
    };
    if(!usbsend(kb, data_pkt2[0], 2)){
        ckb_err("Firmware update failed\n");
        return FW_USBFAIL;
    }
    // Updated successfully
    kb->fwversion = version;
    mkfwnode(kb);
    ckb_info("Firmware update complete\n");
    return FW_OK;
}

int cmd_fwupdate(usbdevice* kb, usbmode* dummy1, int nnumber, int dummy2, const char* path){
    if(!HAS_FEATURES(kb, FEAT_FWUPDATE))
        return 0;
    // Update the firmware
    int ret = fwupdate(kb, path, nnumber);
    while(ret == FW_USBFAIL){
        // Try to reset the device if it fails
        if(usb_tryreset(kb))
            break;
        ret = fwupdate(kb, path, nnumber);
    }
    switch(ret){
    case FW_OK:
        nprintf(kb, nnumber, 0, "fwupdate %s ok\n", path);
        break;
    case FW_NOFILE:
    case FW_WRONGDEV:
        nprintf(kb, nnumber, 0, "fwupdate %s invalid\n", path);
        break;
    case FW_USBFAIL:
        nprintf(kb, nnumber, 0, "fwupdate %s fail\n", path);
        return -1;
    }
    return 0;
}

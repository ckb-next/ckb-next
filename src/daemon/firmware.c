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
    uchar data_pkt[MSG_SIZE] = { CMD_GET, FIELD_IDENT, 0 };
    uchar in_pkt[MSG_SIZE];
    // Mice and mousepads don't have layouts, while the K55 can't report them.
    // If any other devices have the CUE attribute `supportsGetHardwareLayout` set to false,
    // please add them here.
    if(IS_MOUSE_DEV(kb) || IS_MOUSEPAD_DEV(kb) || IS_K55(kb)){
        kb->layout = LAYOUT_NONE;
    } else {
        kb->layout = LAYOUT_UNKNOWN;
    }

    if(!usbrecv(kb, data_pkt, in_pkt))
        return -1;
    if(in_pkt[0] != CMD_GET || in_pkt[1] != FIELD_IDENT){
        ckb_err("Bad input header");
        return -1;
    }
    char ident_str[3*MSG_SIZE+1] = "";
    memset(ident_str, 0, 3 * MSG_SIZE + 1);
    for (int i = 0; i < MSG_SIZE; i++) {
        sprintf(ident_str + (3 * i), "%02hhx ", in_pkt[i]);
    }
    ckb_info("Received identification packet: %s", ident_str);
    ushort vendor, product, version, bootloader;
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
        ckb_warn("Got vendor ID %04x (expected %04x)", vendor, kb->vendor);
    if(product != kb->product)
        ckb_warn("Got product ID %04x (expected %04x)", product, kb->product);
    // Set firmware version and poll rate
    if(version == 0 || bootloader == 0){
        // Needs firmware update
        kb->fwversion = 0;
        kb->pollrate = -1;
    } else {
        if(version != kb->fwversion && kb->fwversion != 0)
            ckb_warn("Got firmware version %04x (expected %04x)", version, kb->fwversion);
        kb->fwversion = version;
        kb->pollrate = poll;
    }
    // Physical layout detection.
    if (kb->layout == LAYOUT_UNKNOWN) {
        kb->layout = in_pkt[23] + 1;
        if (kb->layout > LAYOUT_DUBEOLSIK) {
            ckb_warn("Got unknown physical layout byte value %d, please file a bug report mentioning your keyboard's physical layout", in_pkt[23]);
            kb->layout = LAYOUT_UNKNOWN;
        }
    }
    // Wireless requires extra handshake packets.
    if(IS_WIRELESS(kb)){
        uchar wireless_pkt[5][MSG_SIZE] = { 
            { CMD_GET, 0xae, 0 },
            { CMD_GET, 0x4a, 0 },
            { CMD_GET, 0x50, 0 },
            { CMD_SET, 0xad, 0x00, 0x00, 100 }, // Opacity packet.
            { CMD_SET, 0xaa, 0 },               // Create blank colour profiles.
        };
        if(!usbrecv(kb, wireless_pkt[0], in_pkt))
            return -1;
        memcpy(&vendor, in_pkt + 4, 2);
        memcpy(&product, in_pkt + 6, 2);
        memcpy(&version, in_pkt + 8, 2);
        if(vendor != kb->vendor)
            ckb_warn("Got wireless vendor ID %04x (expected %04x)\n", vendor, kb->vendor);
        if(product != kb->product)
            ckb_warn("Got wireless product ID %04x (expected %04x)\n", product, kb->product);
        // More handshake packets.
        for(int i = 1; i < 3; i++)
            if(!usbrecv(kb, wireless_pkt[i], in_pkt))
                return -1;
        if(!usbsend(kb, wireless_pkt[3], 1))
            return -1;
        // Generate blank colour profiles.
        for(int profile = 1; profile < 6; profile++) {
            wireless_pkt[4][5] = 0xff; // Blank profile command.
            wireless_pkt[4][15] = profile;
            if(!usbsend(kb, wireless_pkt[4], 1))
                return -1;
        }
        /// !!! REMOVE THIS WHEN HARDWARE PROFILES AND WIRELESS FW UPDATE ARE ADDED
        kb->features &= ~(FEAT_HWLOAD | FEAT_FWUPDATE);
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
        ckb_err("Failed to open firmware file %s: %s", path, strerror(errno));
        free(fwdata);
        return FW_NOFILE;
    }
    ssize_t length = read(fd, fwdata, FW_MAXSIZE + 1);
    if(length <= 0x108 || length > FW_MAXSIZE){
        ckb_err("Failed to read firmware file %s: %s", path, length <= 0 ? strerror(errno) : "Wrong size");
        close(fd);
        free(fwdata);
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
        ckb_err("Firmware file %s doesn't match device (V: %04x P: %04x)", path, vendor, product);
        free(fwdata);
        return FW_WRONGDEV;
    }
    ckb_info("Loading firmware version %04x from %s", version, path);
    nprintf(kb, nnumber, 0, "fwupdate %s 0/%d\n", path, (int)length);
    // Force the device to 10ms delay (we need to deliver packets very slowly to make sure it doesn't get overwhelmed)
    kb->usbdelay = 10;
    // Send the firmware messages (256 bytes at a time)
    uchar data_pkt[7][MSG_SIZE] = {
        { CMD_SET, FIELD_FW_START, 0xf0, 0x01, 0 },
        { CMD_SET, FIELD_FW_DATA, 0xf0, 0 },
        { CMD_WRITE_BULK, 0x01, 0x3c, 0 },
        { CMD_WRITE_BULK, 0x02, 0x3c, 0 },
        { CMD_WRITE_BULK, 0x03, 0x3c, 0 },
        { CMD_WRITE_BULK, 0x04, 0x3c, 0 },
        { CMD_WRITE_BULK, 0x05, 0x10, 0 }
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
                ckb_err("Firmware update failed");
                free(fwdata);
                return FW_USBFAIL;
            }
            // The above packet can take a lot longer to process, so wait for a while
            sleep(3);
            if(!usbsend(kb, data_pkt[2], npackets - 1)){
                ckb_err("Firmware update failed");
                free(fwdata);
                return FW_USBFAIL;
            }
        } else {
            // If the output ends here, set the length byte appropriately
            if(output >= length)
                data_pkt[npackets][2] = length - last;
            if(!usbsend(kb, data_pkt[1], npackets)){
                ckb_err("Firmware update failed");
                free(fwdata);
                return FW_USBFAIL;
            }
        }
        nprintf(kb, nnumber, 0, "fwupdate %s %d/%d\n", path, output, (int)length);
    }
    // Send the final pair of messages
    uchar data_pkt2[2][MSG_SIZE] = {
        { CMD_SET, FIELD_FW_DATA, 0xf0, 0x00, 0x00, 0x00, index },
        { CMD_SET, FIELD_RESET, RESET_SLOW, 0 }
    };
    if(!usbsend(kb, data_pkt2[0], 2)){
        ckb_err("Firmware update failed");
        free(fwdata);
        return FW_USBFAIL;
    }
    // Updated successfully
    kb->fwversion = version;
    mkfwnode(kb);
    ckb_info("Firmware update complete");
    free(fwdata);
    return FW_OK;
}

int cmd_fwupdate(usbdevice* kb, usbmode* dummy1, int nnumber, int dummy2, const char* path){
    (void)dummy1;
    (void)dummy2;

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

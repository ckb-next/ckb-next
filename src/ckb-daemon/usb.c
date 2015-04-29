#include "device.h"
#include "devnode.h"
#include "firmware.h"
#include "input.h"
#include "led.h"
#include "notify.h"
#include "profile.h"
#include "usb.h"

// Vendor/product string representations
const char* vendor_str(short vendor){
    if(vendor == V_CORSAIR)
        return "corsair";
    return "";
}

const char* product_str(short product){
    if(product == P_K95 || product == P_K95_NRGB)
        return "k95";
    if(product == P_K70 || product == P_K70_NRGB)
        return "k70";
    if(product == P_K65)
        return "k65";
    return "";
}

// Mask of features to exclude from all devices
int features_mask = -1;

// OS-specific USB reset
extern int os_resetusb(usbdevice* kb, const char* file, int line);

// USB device main loop
static void* devmain(void* _kb){
    usbdevice* kb = _kb;
    while(1){
        pthread_mutex_lock(&kb->mutex);
        // End thread when the handle is removed
        if(!IS_CONNECTED(kb)){
            pthread_mutex_unlock(&kb->mutex);
            break;
        }
        // Read from FIFO
        if(kb->infifo){
            const char* line;
            if(readlines(kb->infifo, &line))
                readcmd(kb, line);
        }
        // Update indicator LEDs for this keyboard. These are polled rather than processed during events because they don't update
        // immediately and may be changed externally by the OS.
        // (also, they can lock the keyboard if they're sent at the wrong time, at least on some firmwares)
        updateindicators(kb, 0);
        // Wait a little bit and then read again
        pthread_mutex_unlock(&kb->mutex);
        DELAY_SHORT;
    }
    return 0;
}

int setupusb(usbdevice* kb, short vendor, short product){
    kb->vendor = vendor;
    kb->product = product;
    kb->features = (IS_RGB(vendor, product) ? FEAT_STD_RGB : FEAT_STD_NRGB) & features_mask;
    // Make up a device name if one wasn't assigned
    if(!kb->name[0])
        snprintf(kb->name, NAME_LEN, "Corsair K%d%s", (product == P_K65) ? 65 : (product == P_K70 || product == P_K70_NRGB) ? 70 : 95, HAS_FEATURES(kb, FEAT_RGB) ? " RGB" : "");
    pthread_mutex_init(&kb->mutex, 0);
    pthread_mutex_init(&kb->keymutex, 0);
    pthread_mutex_lock(&kb->mutex);

    // Make /dev path
    if(makedevpath(kb)){
        pthread_mutex_unlock(&kb->mutex);
        pthread_mutex_destroy(&kb->mutex);
        pthread_mutex_destroy(&kb->keymutex);
        return -1;
    }

    // Set up an input device for key events
    if(!inputopen(kb)){
        rmdevpath(kb);
        pthread_mutex_unlock(&kb->mutex);
        pthread_mutex_destroy(&kb->mutex);
        pthread_mutex_destroy(&kb->keymutex);
        return -1;
    }

    // Set indicator LEDs
    updateindicators(kb, 1);

    // Put non-RGB K95 into software mode. Nothing else needs to be done for non-RGB boards
    if(!HAS_FEATURES(kb, FEAT_RGB)){
        nk95cmd(kb, NK95_HWOFF);
        kb->active = 1;
        writefwnode(kb);
        // Fill out RGB features for consistency, even though the keyboard doesn't have them
        kb->pollrate = -1;
        kb->profile.currentmode = getusbmode(0, &kb->profile);
        if(IS_K95(kb)){
            getusbmode(1, &kb->profile);
            getusbmode(2, &kb->profile);
        }
        return 0;
    }

    // Get the firmware version from the device
    int fail = !!getfwversion(kb);

    if(!fail && NEEDS_FW_UPDATE(kb)){
        // Device needs a firmware update. Finish setting up but don't do anything.
        printf("Device needs a firmware update. Please issue a fwupdate command.\n");
        kb->features = FEAT_RGB | FEAT_FWVERSION | FEAT_FWUPDATE;
        kb->active = 1;
        kb->profile.currentmode = getusbmode(0, &kb->profile);
        getusbmode(1, &kb->profile);
        getusbmode(2, &kb->profile);
        return 0;
    }

    // Restore profile (if any)
    DELAY_LONG;
    usbprofile* store = findstore(kb->profile.serial);
    if(store){
        memcpy(&kb->profile, store, sizeof(usbprofile));
        if(IS_K95(kb)){
            // On the K95, make sure at least 3 modes are available
            getusbmode(1, &kb->profile);
            getusbmode(2, &kb->profile);
        }
        if(fail || hwloadprofile(kb, 0))
            return -2;
    } else {
        // If there is no profile, load it from the device
        kb->profile.currentmode = getusbmode(0, &kb->profile);
        if(IS_K95(kb)){
            getusbmode(1, &kb->profile);
            getusbmode(2, &kb->profile);
        }
        if(fail || hwloadprofile(kb, 1))
            return -2;
    }

    // Start main thread
    DELAY_SHORT;
    if(pthread_create(&kb->thread, 0, devmain, kb))
        return -1;
    return 0;
}

int revertusb(usbdevice* kb){
    if(NEEDS_FW_UPDATE(kb))
        return 0;
    if(!HAS_FEATURES(kb, FEAT_RGB)){
        nk95cmd(kb, NK95_HWON);
        return 0;
    }
    if(setactive(kb, 0))
        return -1;
    return 0;
}

int _resetusb(usbdevice* kb, const char* file, int line){
    // Perform a USB reset
    DELAY_LONG;
    int res = os_resetusb(kb, file, line);
    if(res)
        return res;
    DELAY_LONG;
    // Re-initialize the device.
    if(!HAS_FEATURES(kb, FEAT_RGB))
        return 0;
    if(getfwversion(kb))
        return -1;
    if(NEEDS_FW_UPDATE(kb))
        return 0;
    if(setactive(kb, kb->active))
        return -1;
    // If the hardware profile hasn't been loaded yet, load it here
    res = 0;
    if(!kb->hw){
        if(findstore(kb->profile.serial))
            res = hwloadprofile(kb, 0);
        else
            res = hwloadprofile(kb, 1);
    }
    updatergb(kb, 1);
    return res ? -1 : 0;
}

int usb_tryreset(usbdevice* kb){
    printf("Attempting reset...\n");
    while(1){
        DELAY_LONG;
        int res = resetusb(kb);
        if(!res){
            printf("Reset success\n");
            return 0;
        }
        if(res == -2)
            break;
    }
    printf("Reset failed. Disconnecting.\n");
    return -1;
}

int closeusb(usbdevice* kb){
    // Close file handles
    if(!kb->infifo)
        return 0;
    pthread_mutex_lock(&kb->keymutex);
    if(kb->handle){
        printf("Disconnecting %s (S/N: %s)\n", kb->name, kb->profile.serial);
        inputclose(kb);
        updateconnected();
        // Move the profile data into the device store (unless it wasn't set due to needing a firmware update)
        if(kb->fwversion == 0)
            freeprofile(&kb->profile);
        else {
            usbprofile* store = addstore(kb->profile.serial, 0);
            memcpy(store, &kb->profile, sizeof(usbprofile));
        }
        // Close USB device
        closehandle(kb);
        notifyconnect(kb, 0);
    } else
        updateconnected();

    // Wait for thread to close
    pthread_mutex_unlock(&kb->keymutex);
    pthread_mutex_unlock(&kb->mutex);
    pthread_join(kb->thread, 0);

    // Delete the control path
    rmdevpath(kb);

    pthread_mutex_destroy(&kb->keymutex);
    pthread_mutex_destroy(&kb->mutex);
    memset(kb, 0, sizeof(usbdevice));
    return 0;
}

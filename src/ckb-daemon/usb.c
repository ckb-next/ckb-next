#include "command.h"
#include "device.h"
#include "devnode.h"
#include "firmware.h"
#include "input.h"
#include "led.h"
#include "notify.h"
#include "profile.h"
#include "usb.h"

pthread_mutex_t usbmutex = PTHREAD_MUTEX_INITIALIZER;

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
    if(product == P_M65)
        return "m65";
    return "";
}

// Vtable selector
static const devcmd* get_vtable(short vendor, short product){
    return IS_MOUSE(vendor, product) ? &vtable_mouse : IS_RGB(vendor, product) ? &vtable_keyboard : &vtable_keyboard_nonrgb;
}

// Mask of features to exclude from all devices
int features_mask = -1;

// USB device main loop
static void* devmain(usbdevice* kb){
    while(1){
        pthread_mutex_lock(dmutex(kb));
        // End thread when the handle is removed
        if(!IS_CONNECTED(kb))
            break;
        // Read from FIFO
        const char* line;
        if(readlines(kb->infifo, &line)){
            if(readcmd(kb, line)){
                // USB transfer failed; destroy device
                closeusb(kb);
                break;
            }
        }
        // Update indicator LEDs for this keyboard. These are polled rather than processed during events because they don't update
        // immediately and may be changed externally by the OS.
        // (also, they can lock the keyboard if they're sent at the wrong time, at least on some firmwares)
        kb->vtable->updateindicators(kb, 0);
        // Wait a little bit and then read again
        pthread_mutex_unlock(dmutex(kb));
        DELAY_SHORT;
    }
    pthread_mutex_unlock(dmutex(kb));
    return 0;
}

static void* _setupusb(void* context){
    usbdevice* kb = context;
    // Set standard fields
    short vendor = kb->vendor, product = kb->product;
    const devcmd* vt = kb->vtable = get_vtable(vendor, product);
    kb->features = (IS_RGB(vendor, product) ? FEAT_STD_RGB : FEAT_STD_NRGB) & features_mask;

    // Perform OS-specific setup
    if(os_setupusb(kb))
        goto fail;
    if(pthread_create(&kb->inputthread, 0, os_inputmain, kb))
        goto fail;
    pthread_detach(kb->inputthread);

    // Make up a device name if one wasn't assigned
    if(!kb->name[0])
        snprintf(kb->name, KB_NAME_LEN, "Corsair K%d%s", (product == P_K65) ? 65 : (product == P_K70 || product == P_K70_NRGB) ? 70 : 95, HAS_FEATURES(kb, FEAT_RGB) ? " RGB" : "");

    // Set up an input device for key events
    if(os_inputopen(kb))
        goto fail;

    // Set up device
    vt->allocprofile(kb);
    vt->updateindicators(kb, 1);
    if(vt->start(kb, 0) && usb_tryreset(kb))
        goto fail;

    // Make /dev path
    if(mkdevpath(kb))
        goto fail;

    // Finished. Enter main loop
    int index = INDEX_OF(kb, keyboard);
    ckb_info("%s ready at %s%d\n", kb->name, devpath, index);
    updateconnected();
    notifyconnect(kb, 1);
    pthread_mutex_unlock(imutex(kb));
    pthread_mutex_unlock(dmutex(kb));
    return devmain(kb);

    fail:
    pthread_mutex_unlock(imutex(kb));
    closeusb(kb);
    pthread_mutex_unlock(dmutex(kb));
    return 0;
}

void setupusb(usbdevice* kb){
    pthread_mutex_lock(imutex(kb));
    if(pthread_create(&kb->thread, 0, _setupusb, kb))
        ckb_err("Failed to create USB thread");
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
    // Re-initialize the device
    kb->vtable->start(kb, kb->active);
    // If the hardware profile hasn't been loaded yet, load it here
    res = 0;
    if(!kb->hw)
        res = hwloadprofile(kb, 1);
    kb->vtable->updatergb(kb, 1);
    return res ? -1 : 0;
}

int usb_tryreset(usbdevice* kb){
    ckb_info("Attempting reset...\n");
    while(1){
        DELAY_LONG;
        int res = resetusb(kb);
        if(!res){
            ckb_info("Reset success\n");
            return 0;
        }
        if(res == -2)
            break;
    }
    ckb_err("Reset failed. Disconnecting.\n");
    return -1;
}

int closeusb(usbdevice* kb){
    // Close file handles
    pthread_mutex_lock(imutex(kb));
    if(kb->handle){
        ckb_info("Disconnecting %s (S/N: %s)\n", kb->name, kb->serial);
        os_inputclose(kb);
        updateconnected();
        // Close USB device
        os_closeusb(kb);
        notifyconnect(kb, 0);
    } else
        updateconnected();

    // Wait for thread to close
    pthread_mutex_unlock(imutex(kb));
    pthread_mutex_unlock(dmutex(kb));
    pthread_join(kb->thread, 0);
    pthread_mutex_lock(dmutex(kb));

    // Delete the profile and the control path
    kb->vtable->freeprofile(kb);
    rmdevpath(kb);
    memset(kb, 0, sizeof(usbdevice));
    return 0;
}

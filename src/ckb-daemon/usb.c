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

// Reset stopper for when the program shuts down
volatile int reset_stop = 0;

// Mask of features to exclude from all devices
int features_mask = -1;

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
    if(product == P_STRAFE || product == P_STRAFE_NRGB)
        return "strafe";
    if(product == P_M65)
        return "m65";
    if(product == P_SABRE_O || product == P_SABRE_L)
        return "sabre";
    if(product == P_SCIMITAR)
        return "scimitar";
    return "";
}

// Vtable selector
static const devcmd* get_vtable(short vendor, short product){
    return IS_MOUSE(vendor, product) ? &vtable_mouse : IS_RGB(vendor, product) ? &vtable_keyboard : &vtable_keyboard_nonrgb;
}

// USB device main loop
static void* devmain(usbdevice* kb){
    // dmutex should still be locked when this is called
    int kbfifo = kb->infifo - 1;
    readlines_ctx linectx;
    readlines_ctx_init(&linectx);
    while(1){
        pthread_mutex_unlock(dmutex(kb));
        // Read from FIFO
        const char* line;
        int lines = readlines(kbfifo, linectx, &line);
        pthread_mutex_lock(dmutex(kb));
        // End thread when the handle is removed
        if(!IS_CONNECTED(kb))
            break;
        if(lines){
            if(readcmd(kb, line)){
                // USB transfer failed; destroy device
                closeusb(kb);
                break;
            }
        }
    }
    pthread_mutex_unlock(dmutex(kb));
    readlines_ctx_free(linectx);
    return 0;
}

static void* _setupusb(void* context){
    usbdevice* kb = context;
    // Set standard fields
    short vendor = kb->vendor, product = kb->product;
    const devcmd* vt = kb->vtable = get_vtable(vendor, product);
    kb->features = (IS_RGB(vendor, product) ? FEAT_STD_RGB : FEAT_STD_NRGB) & features_mask;
    if(IS_MOUSE(vendor, product)) kb->features |= FEAT_ADJRATE;
    if(IS_MONOCHROME(vendor, product)) kb->features |= FEAT_MONOCHROME;
    kb->usbdelay = USB_DELAY_DEFAULT;

    // Perform OS-specific setup
    DELAY_LONG(kb);
    if(os_setupusb(kb))
        goto fail;
    // Make up a device name and serial if they weren't assigned
    if(!kb->serial[0])
        snprintf(kb->serial, SERIAL_LEN, "%04x:%04x-NoID", kb->vendor, kb->product);
    if(!kb->name[0])
        snprintf(kb->name, KB_NAME_LEN, "%s %s", vendor_str(kb->vendor), product_str(kb->product));

    // Set up an input device for key events
    if(os_inputopen(kb))
        goto fail;
    if(pthread_create(&kb->inputthread, 0, os_inputmain, kb))
        goto fail;
    pthread_detach(kb->inputthread);
    if(os_setupindicators(kb))
        goto fail;

    // Set up device
    vt->allocprofile(kb);
    vt->updateindicators(kb, 1);
    pthread_mutex_unlock(imutex(kb));
    if(vt->start(kb, 0) && usb_tryreset(kb))
        goto fail_noinput;

    // Make /dev path
    if(mkdevpath(kb))
        goto fail_noinput;

    // Finished. Enter main loop
    int index = INDEX_OF(kb, keyboard);
    ckb_info("Setup finished for %s%d\n", devpath, index);
    updateconnected();
    return devmain(kb);

    fail:
    pthread_mutex_unlock(imutex(kb));
    fail_noinput:
    closeusb(kb);
    pthread_mutex_unlock(dmutex(kb));
    return 0;
}

void setupusb(usbdevice* kb){
    pthread_mutex_lock(imutex(kb));
    if(pthread_create(&kb->thread, 0, _setupusb, kb))
        ckb_err("Failed to create USB thread\n");
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
    DELAY_LONG(kb);
    int res = os_resetusb(kb, file, line);
    if(res)
        return res;
    DELAY_LONG(kb);
    // Re-initialize the device
    if(kb->vtable->start(kb, kb->active) != 0)
        return -1;
    if(kb->vtable->updatergb(kb, 1) != 0)
        return -1;
    return 0;
}

int usb_tryreset(usbdevice* kb){
    if(reset_stop)
        return -1;
    ckb_info("Attempting reset...\n");
    while(1){
        int res = resetusb(kb);
        if(!res){
            ckb_info("Reset success\n");
            return 0;
        }
        if(res == -2 || reset_stop)
            break;
    }
    ckb_err("Reset failed. Disconnecting.\n");
    return -1;
}

// device.c
extern int hwload_mode;

int _usbsend(usbdevice* kb, const uchar* messages, int count, const char* file, int line){
    int total_sent = 0;
    for(int i = 0; i < count; i++){
        // Send each message via the OS function
        while(1){
            DELAY_SHORT(kb);
            int res = os_usbsend(kb, messages + i * MSG_SIZE, 0, file, line);
            if(res == 0)
                return 0;
            else if(res != -1){
                total_sent += res;
                break;
            }
            // Stop immediately if the program is shutting down or hardware load is set to tryonce
            if(reset_stop || hwload_mode != 2)
                return 0;
            // Retry as long as the result is temporary failure
            DELAY_LONG(kb);
        }
    }
    return total_sent;
}

int _usbrecv(usbdevice* kb, const uchar* out_msg, uchar* in_msg, const char* file, int line){
    // Try a maximum of 3 times
    for(int try = 0; try < 5; try++){
        // Send the output message
        DELAY_SHORT(kb);
        int res = os_usbsend(kb, out_msg, 1, file, line);
        if(res == 0)
            return 0;
        else if(res == -1){
            // Retry on temporary failure
            if(reset_stop)
                return 0;
            DELAY_LONG(kb);
            continue;
        }
        // Wait for the response
        DELAY_MEDIUM(kb);
        res = os_usbrecv(kb, in_msg, file, line);
        if(res == 0)
            return 0;
        else if(res != -1)
            return res;
        if(reset_stop || hwload_mode != 2)
            return 0;
        DELAY_LONG(kb);
    }
    // Give up
    ckb_err_fn("Too many send/recv failures. Dropping.\n", file, line);
    return 0;
}

int closeusb(usbdevice* kb){
    pthread_mutex_lock(imutex(kb));
    if(kb->handle){
        int index = INDEX_OF(kb, keyboard);
        ckb_info("Disconnecting %s%d\n", devpath, index);
        os_inputclose(kb);
        updateconnected();
        // Close USB device
        os_closeusb(kb);
    } else
        updateconnected();
    rmdevpath(kb);

    // Wait for thread to close
    pthread_mutex_unlock(imutex(kb));
    pthread_mutex_unlock(dmutex(kb));
    pthread_join(kb->thread, 0);
    pthread_mutex_lock(dmutex(kb));

    // Delete the profile and the control path
    if(!kb->vtable)
        return 0;
    kb->vtable->freeprofile(kb);
    memset(kb, 0, sizeof(usbdevice));
    return 0;
}

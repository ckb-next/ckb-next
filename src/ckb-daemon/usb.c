#include "device.h"
#include "devnode.h"
#include "input.h"
#include "led.h"
#include "notify.h"
#include "usb.h"

int usbqueue(usbdevice* kb, uchar* messages, int count){
    // Don't add messages unless the queue has enough room for all of them
    if(!kb->handle || kb->queuecount + count > QUEUE_LEN)
        return -1;
    for(int i = 0; i < count; i++)
        memcpy(kb->queue[kb->queuecount + i], messages + MSG_SIZE * i, MSG_SIZE);
    kb->queuecount += count;
    return 0;
}

int setupusb(usbdevice* kb){
    pthread_mutex_init(&kb->mutex, 0);
    pthread_mutex_lock(&kb->mutex);

    // Make /dev path
    if(makedevpath(kb)){
        pthread_mutex_unlock(&kb->mutex);
        pthread_mutex_destroy(&kb->mutex);
        return -1;
    }

    // Set up an input device for key events
    if(!inputopen(kb)){
        rmdevpath(kb);
        pthread_mutex_unlock(&kb->mutex);
        pthread_mutex_destroy(&kb->mutex);
        return -1;
    }
    updateindicators(kb, 1);

    // Create the USB queue
    for(int q = 0; q < QUEUE_LEN; q++)
        kb->queue[q] = malloc(MSG_SIZE);

    // Set all keys to use the Corsair input. HID input is unused.
    setinput(kb, IN_CORSAIR);

    // Restore profile (if any)
    usbsetting* store = findstore(kb->setting.serial);
    if(store){
        memcpy(&kb->setting, store, sizeof(*store));
    } else {
        // If there is no profile, load it from the device
        kb->setting.keymap = keymap_system;
        kb->setting.profile.currentmode = getusbmode(0, &kb->setting.profile, keymap_system);
        getusbmode(1, &kb->setting.profile, keymap_system);
        getusbmode(2, &kb->setting.profile, keymap_system);
        hwloadprofile(kb);
    }
    updatergb(kb);
    pthread_mutex_unlock(&kb->mutex);
    return 0;
}

int closeusb(usbdevice* kb){
    // Close file handles
    if(!kb->infifo)
        return 0;
    if(kb->handle){
        printf("Disconnecting %s (S/N: %s)\n", kb->name, kb->setting.serial);
        inputclose(kb);
        // Delete USB queue
        for(int i = 0; i < QUEUE_LEN; i++)
            free(kb->queue[i]);
        // Move the profile data into the device store
        usbsetting* store = addstore(kb->setting.serial);
        memcpy(&store->profile, &kb->setting.profile, sizeof(kb->setting.profile));
        // Close USB device
        closehandle(kb);
        updateconnected();
        notifyconnect(kb, 0);
    }
    // Delete the control path
    rmdevpath(kb);

    pthread_mutex_unlock(&kb->mutex);
    pthread_mutex_destroy(&kb->mutex);
    memset(kb, 0, sizeof(*kb));
    return 0;
}

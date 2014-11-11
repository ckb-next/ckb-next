#include "usb.h"
#include "input.h"
#include "devnode.h"
#include "device.h"
#include "led.h"

int usbqueue(usbdevice* kb, uchar* messages, int count){
    // Don't add messages unless the queue has enough room for all of them
    if(kb->queuecount + count > QUEUE_LEN)
        return -1;
    for(int i = 0; i < count; i++)
        memcpy(kb->queue[kb->queuecount + i], messages + MSG_SIZE * i, MSG_SIZE);
    kb->queuecount += count;
    return 0;
}

int setupusb(int index){
    usbdevice* kb = keyboard + index;
    pthread_mutex_init(&kb->mutex, 0);
    pthread_mutex_lock(&kb->mutex);
    // Set up an input device for key events
    if(!inputopen(index)){
        pthread_mutex_unlock(&kb->mutex);
        return -1;
    }
    updateindicators(kb, 1);

    // Make /dev path
    if(makedevpath(index)){
        pthread_mutex_unlock(&kb->mutex);
        return -1;
    }

    // Create the USB queue
    for(int q = 0; q < QUEUE_LEN; q++)
        kb->queue[q] = malloc(MSG_SIZE);

    // Set all keys to use the Corsair input. HID input is unused.
    setinput(kb, IN_CORSAIR);

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
    pthread_mutex_unlock(&kb->mutex);
    return 0;
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
        // Close USB device
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

    pthread_mutex_unlock(&kb->mutex);
    pthread_mutex_destroy(&kb->mutex);
    memset(kb, 0, sizeof(*kb));
    return 0;
}

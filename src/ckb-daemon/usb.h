#ifndef USB_H
#define USB_H

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif
#include <features.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>
#include "keyboard.h"

// Vendor/product codes
#define V_CORSAIR   0x1b1c
#define P_K70       0x1b13
#define P_K95       0x1b11

// Structure for tracking keyboard devices
#define DEV_MAX     10
#define NAME_LEN 33
#define SERIAL_LEN   33
#define RGB_SIZE (N_KEYS * sizeof(short))
#define QUEUE_LEN 12
#define MSG_SIZE 64
typedef struct {
    short* rgb;
    char* queue[QUEUE_LEN];
    struct libusb_transfer* keyint;
    char intinput[MSG_SIZE];
    int ledfifo;
    int queuelength;
    struct libusb_device_descriptor descriptor;
    libusb_device* dev;
    libusb_device_handle* handle;
    int rgbon;
    int model;
    char name[NAME_LEN];
    char serial[SERIAL_LEN];
} usbdevice;
extern usbdevice keyboard[DEV_MAX];

// Structure to store RGB settings for a keyboard not currently connected
typedef struct {
    short* rgb;
    int rgbon;
    char serial[SERIAL_LEN];
} usbstore;
extern usbstore* store;
extern int storecount;

// USB device compare. Returns 0 if devices are the same
int usbcmp(libusb_device* dev1, libusb_device* dev2);
// Open a USB device and create a new device entry. Returns 0 on success
int openusb(libusb_device* device);
// Close a USB device and remove device entry. Returns 0 on success
int closeusb(int index);

// Add a message to a USB device to be sent to the device. Returns 0 on success.
int usbqueue(usbdevice* kb, char* messages, int count);
// Output a message from the USB queue to the device, if any. Returns number of bytes written.
int usbdequeue(usbdevice* kb);

// Find a USB device from storage. Returns 0 if not found
usbstore* findstore(const char* serial);
// Add a USB device to storage. Returns an existing device if found or a new one if not.
usbstore* addstore(const char* serial);

#endif

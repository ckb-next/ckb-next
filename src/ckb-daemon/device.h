#ifndef DEVICE_H
#define DEVICE_H

#include "includes.h"
#include "keyboard.h"

// Connected devices
#define DEV_MAX     10
extern usbdevice keyboard[DEV_MAX];
// Is a device active?
#define IS_CONNECTED(kb) ((kb) && (kb)->handle && (kb)->event)
// A mutex used when accessing the device table. This mutex must be locked
// during any operation that could add or remove a device, or during any
// operation that accesses the devices as a list.
extern pthread_mutex_t kblistmutex;

// Activates/deactives a keyboard
void setactive(usbdevice* kb, int active);

// Find a connected USB device. Returns 0 if not found
usbdevice* findusb(const char* serial);
// Find a USB device from storage. Returns 0 if not found
usbprofile* findstore(const char* serial);
// Add a USB device to storage. Returns an existing device if found or a new one if not.
usbprofile* addstore(const char* serial, int autosetup);

#endif

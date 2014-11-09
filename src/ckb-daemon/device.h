#ifndef DEVICE_H
#define DEVICE_H

#include "includes.h"
#include "keyboard.h"

// Keyboard devices
#define DEV_MAX     10
extern usbdevice keyboard[DEV_MAX];
extern pthread_mutex_t kblistmutex;

// Set input mode on a device
#define IN_CORSAIR  0x40
#define IN_HID      0x80
void setinput(usbdevice* kb, int input);

// Find a connected USB device. Returns 0 if not found
usbdevice* findusb(const char* serial);
// Find a USB device from storage. Returns 0 if not found
usbsetting* findstore(const char* serial);
// Add a USB device to storage. Returns an existing device if found or a new one if not.
usbsetting* addstore(const char* serial);

// Get a mode from a profile. The mode will be created if it didn't already exist.
usbmode* getusbmode(int id, usbprofile* profile);

// Sets a mode's name
void setmodename(usbmode* mode, const char* name);
// Sets a profile's name
void setprofilename(usbprofile* profile, const char* name);
// Resets a mode to its default settings
void erasemode(usbmode* mode);
// Erases a profile, deleting all of its modes.
void eraseprofile(usbprofile* profile);

// Generates a new ID
void genid(usbid* id);
// Updates an ID's modification
void updatemod(usbid* id);

// Loads the profile name from hardware
void hwloadprofile(usbdevice* kb);
// Saves the profile name to hardware
void hwsaveprofile(usbdevice* kb);

#endif

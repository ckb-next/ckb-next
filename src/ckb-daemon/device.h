#ifndef DEVICE_H
#define DEVICE_H

#include "includes.h"
#include "keyboard.h"

// Connected devices
#define DEV_MAX     10
extern usbdevice keyboard[DEV_MAX];
// Is a device active?
#define IS_ACTIVE(kb) ((kb) && (kb)->handle && (kb)->event)
// A mutex used when accessing the device table. This mutex must be locked
// during any operation that could add or remove a device, or during any
// operation that accesses the devices as a list.
extern pthread_mutex_t kblistmutex;

// Set input mode on a device
#define IN_CORSAIR  0x40
#define IN_HID      0x80
void setinput(usbdevice* kb, int input);

// Find a connected USB device. Returns 0 if not found
usbdevice* findusb(const char* serial);
// Find a USB device from storage. Returns 0 if not found
usbprofile* findstore(const char* serial);
// Add a USB device to storage. Returns an existing device if found or a new one if not.
usbprofile* addstore(const char* serial, int autosetup);

// Get a mode from a profile. The mode will be created if it didn't already exist.
usbmode* getusbmode(int id, usbprofile* profile, const key* keymap);

// Sets a mode's name
void cmd_setmodename(usbmode* mode, const key* keymap, int zero, const char* name);
// Sets a profile's name
void setprofilename(usbprofile* profile, const char* name);
// Resets a mode to its default settings
void erasemode(usbmode* mode, const key* keymap);
// Erases a profile, deleting all of its modes.
void eraseprofile(usbprofile* profile);
// Frees a profile's memory
void freeprofile(usbprofile* profile);

// Generates a new ID
void genid(usbid* id);
// Updates an ID's modification
void updatemod(usbid* id);

// Loads the profile name from hardware. apply = 1 to apply/activate hardware profile, 1 to simply store it.
int hwloadprofile(usbdevice* kb, int apply);
// Saves the profile name to hardware
void hwsaveprofile(usbdevice* kb);

#endif

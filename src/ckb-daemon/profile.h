#ifndef PROFILE_H
#define PROFILE_H

#include "includes.h"
#include "device.h"

// Note: Lock dmutex before using profile commands (see device.h).
// Allocating/deallocating a profile additionally requires imutex to be locked

// Allocate a native profile
void allocprofile(usbdevice* kb);
// Allocate/load a hardware profile and copy it to native. Returns 0 on success.
int loadprofile(usbdevice* kb);
// Free a native profile and the corresponding HW profile
void freeprofile(usbdevice* kb);

// Command: Reset a mode to its default settings
void cmd_erase(usbdevice* kb, usbmode* mode, int dummy1, int dummy2, const char* dummy3);
// Command: Erase a profile, deleting all of its modes.
void cmd_eraseprofile(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4);

// Sets a mode's name
void cmd_name(usbdevice* kb, usbmode* mode, int dummy1, int dummy2, const char* name);
// Sets a profile's name
void cmd_profilename(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* name);
// Get a mode's name. Returns a URL-encoded UTF-8 buffer that needs to be freed later.
char* getmodename(usbmode* mode);
// Get a profile's name. See above.
char* getprofilename(usbprofile* profile);
// Get hardware names.
char* gethwmodename(hwprofile* profile, int index);
char* gethwprofilename(hwprofile* profile);

// Updates a profile/mode with the supplied GUID. Returns 1 if valid or 0 if invalid.
int setid(usbid* id, const char* guid);
// Generates a GUID from the given ID. Returns a string which must be freed later.
char* getid(usbid* id);

// Converts a hardware profile to a native profile
void hwtonative(usbprofile* profile, hwprofile* hw, int modecount);
// Converts a native profile to a hardware profile
void nativetohw(usbprofile* profile, hwprofile* hw, int modecount);

// Command: Set mode ID
void cmd_id(usbdevice* kb, usbmode* mode, int dummy1, int dummy2, const char* id);
// Command: Set profile ID
void cmd_profileid(usbdevice* kb, usbmode* mode, int dummy1, int dummy2, const char* id);

// Command: Load profile from hardware. apply = 1 to apply/activate hardware profile, 0 to simply store it. Returns 0 on success.
int cmd_hwload_kb(usbdevice* kb, usbmode* dummy1, int dummy2, int apply, const char* dummy3);
int cmd_hwload_mouse(usbdevice* kb, usbmode* dummy1, int dummy2, int apply, const char* dummy3);
#define hwloadprofile(kb, apply) (kb)->vtable->hwload(kb, 0, 0, apply, 0)
// Command: Saves the profile name to hardware. Returns 0 on success.
int cmd_hwsave_kb(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4);
int cmd_hwsave_mouse(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4);

#endif  // PROFILE_H

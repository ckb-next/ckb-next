#ifndef PROFILE_H
#define PROFILE_H

#include "includes.h"

// Get a mode from a profile. The mode will be created if it didn't already exist.
usbmode* getusbmode(int id, usbprofile* profile, const key* keymap);

// Sets a mode's name
void cmd_setmodename(usbmode* mode, const key* keymap, int zero, const char* name);
// Sets a profile's name
void setprofilename(usbprofile* profile, const char* name);
// Gets a mode's name. Returns a URL-encoded UTF-8 buffer that needs to be freed later.
char* getmodename(usbmode* mode);
// Gets a profile's name. See above.
char* getprofilename(usbprofile* profile);

// Resets a mode to its default settings
void erasemode(usbmode* mode, const key* keymap);
// Erases a profile, deleting all of its modes.
void eraseprofile(usbprofile* profile, int modecount);
// Frees a profile's memory
void freeprofile(usbprofile* profile);

// Generates a new ID
void genid(usbid* id);
// Updates an ID's modification
void updatemod(usbid* id);
// Updates a profile/mode with the supplied GUID. Returns 1 if valid or 0 if invalid.
int setid(usbid* id, const char* guid);
// Generates a GUID from the given ID. Returns a string which must be freed later.
char* getid(usbid* id);

// Loads the profile name from hardware. apply = 1 to apply/activate hardware profile, 1 to simply store it.
int hwloadprofile(usbdevice* kb, int apply);
// Saves the profile name to hardware
void hwsaveprofile(usbdevice* kb);

#endif

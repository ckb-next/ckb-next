#include "device.h"
#include "input.h"
#include "led.h"
#include "profile.h"

usbdevice keyboard[DEV_MAX];
pthread_mutex_t kblistmutex = PTHREAD_MUTEX_INITIALIZER;
usbprofile* store = 0;
int storecount = 0;

usbdevice* findusb(const char* serial){
    for(int i = 0; i < DEV_MAX; i++){
        if(keyboard[i].infifo && !strcmp(serial, keyboard[i].profile.serial))
            return keyboard + i;
    }
    return 0;
}

usbprofile* findstore(const char* serial){
    for(int i = 0; i < storecount; i++){
        usbprofile* res = store + i;
        if(!strcmp(res->serial, serial))
            return res;
    }
    return 0;
}

usbprofile* addstore(const char* serial, int autosetup){
    // Try to find the device before adding it
    usbprofile* res = findstore(serial);
    if(res)
        return res;
    // Add device to the list
    store = realloc(store, ++storecount * sizeof(usbprofile));
    res = store + storecount - 1;
    // Initialize device
    memset(res, 0, sizeof(*res));
    if(autosetup)
        eraseprofile(res, 1);
    strcpy(res->serial, serial);
    genid(&res->id);
    return res;
}

// Per-key input settings and hardware actions
// 0x80 generates a normal HID interrupt, 0x40 generates a proprietary interrupt. 0xc0 generates both.
// The exceptions are the proprietary Corsair keys, which only report HID input in BIOS mode and only report Corsair input in non-BIOS mode.
// In BIOS mode, the Corsair input is disabled no matter what.
#define IN_HID          0x80
#define IN_CORSAIR      0x40

// The lower nybble controls various hardware actions
#define ACT_LIGHT       1
#define ACT_NEXT        3
#define ACT_NEXT_NOWRAP 5
#define ACT_LOCK        8
#define ACT_MR_RING     9
#define ACT_M1          10
#define ACT_M2          11
#define ACT_M3          12

void setactive(usbdevice* kb, int active){
    if(!HAS_FEATURES(kb, FEAT_RGB) || NEEDS_FW_UPDATE(kb))
        return;
    // Empty the board's USB queue first
    while(kb->queuecount > 0){
        DELAY_SHORT;
        if(!usbdequeue(kb))
            return;
    }
    uchar msg[3][MSG_SIZE] = {
        { 0x07, 0x04, 0 },                  // Disables or enables HW control for top row
        { 0x07, 0x40, 0 },                  // Selects key input
        { 0x07, 0x05, 2, 0, 0x03, 0x00 }    // Commits key input selection
    };
    if(active){
        kb->active = 1;
        // Put the M-keys (K95) as well as the Brightness/Lock keys into software-controlled mode.
        msg[0][2] = 2;
        usbqueue(kb, msg[0], 1);
        // Set input mode on the keys. They must be grouped into packets of 60 bytes (+ 4 bytes header)
        // Keys are referenced in byte pairs, with the first byte representing the key and the second byte representing the mode.
        for(int key = 0; key < N_KEYS; ){
            int pair;
            for(pair = 0; pair < 30 && key < N_KEYS; pair++, key++){
                // Select both standard and Corsair input. The standard input will be ignored except in BIOS mode.
                uchar action = IN_HID | IN_CORSAIR;
                // Additionally, make MR activate the MR ring
                //if(keymap_system[key].name && !strcmp(keymap_system[key].name, "mr"))
                //    action |= ACT_MR_RING;
                msg[1][4 + pair * 2] = key;
                msg[1][5 + pair * 2] = action;
            }
            // Byte 2 = pair count (usually 30, less on final message)
            msg[1][2] = pair;
            usbqueue(kb, msg[1], 1);
        }
        // Commit new input settings
        usbqueue(kb, msg[2], 1);
    } else {
        kb->active = 0;
        // Set the M-keys back into hardware mode, restore hardware RGB profile. It has to be sent twice for some reason.
        msg[0][2] = 1;
        usbqueue(kb, msg[0], 1);
        usbqueue(kb, msg[0], 1);
#ifdef OS_LINUX
        // On OSX the default key mappings are fine. On Linux, the G keys will freeze the keyboard. Set the keyboard entirely to HID input.
        for(int key = 0; key < N_KEYS; ){
            int pair;
            for(pair = 0; pair < 30 && key < N_KEYS; pair++, key++){
                uchar action = IN_HID;
                // Enable hardware actions
                if(keymap_system[key].name){
                    if(!strcmp(keymap_system[key].name, "mr"))
                        action = ACT_MR_RING;
                    else if(!strcmp(keymap_system[key].name, "m1"))
                        action = ACT_M1;
                    else if(!strcmp(keymap_system[key].name, "m2"))
                        action = ACT_M2;
                    else if(!strcmp(keymap_system[key].name, "m3"))
                        action = ACT_M3;
                    else if(!strcmp(keymap_system[key].name, "light"))
                        action = ACT_LIGHT;
                    else if(!strcmp(keymap_system[key].name, "lock"))
                        action = ACT_LOCK;
                }
                msg[1][4 + pair * 2] = key;
                msg[1][5 + pair * 2] = action;
            }
            // Byte 2 = pair count (usually 30, less on final message)
            msg[1][2] = pair;
            usbqueue(kb, msg[1], 1);
        }
        // Commit new input settings
        usbqueue(kb, msg[2], 1);
#endif
    }
}

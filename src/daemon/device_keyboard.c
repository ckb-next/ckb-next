#include "command.h"
#include "device.h"
#include "devnode.h"
#include "firmware.h"
#include "input.h"
#include "profile.h"
#include "usb.h"
#include "nxp_proto.h"
#include "legacykb_proto.h"

int start_kb_legacy(usbdevice* kb, int makeactive){
    (void)makeactive;

    // Put the non-RGB K95 into software mode. Nothing else needs to be done hardware wise
    nk95cmd(kb, NK95_HWOFF);
    // Fill out RGB features for consistency, even though the keyboard doesn't have them
    kb->active = 1;
    kb->maxpollrate = kb->pollrate = POLLRATE_UNKNOWN;
    return 0;
}

int setactive_kb(usbdevice* kb, int active){
    if(NEEDS_FW_UPDATE(kb))
        return 0;

    clear_input_and_rgb(kb, active);

    uchar msg[3][MSG_SIZE] = {
        { CMD_SET, FIELD_SPECIAL, 0 },                // Disables or enables HW control for top row
        { CMD_SET, FIELD_KEYINPUT, 0 },               // Selects key input
        { CMD_SET, FIELD_LIGHTING, 2, 0, 0x03, 0x00 } // Commits key input selection
    };
    if(active){
        // Put the M-keys (K95) as well as the Brightness/Lock keys into software-controlled mode.
        msg[0][2] = MODE_SOFTWARE;
        if(!usbsend(kb, msg[0], MSG_SIZE, 1))
            return -1;
        DELAY_30MS();
        // Set input mode on the keys. They must be grouped into packets of 60 bytes (+ 4 bytes header)
        // Keys are referenced in byte pairs, with the first byte representing the key and the second byte representing the mode.
        for(int k = 0; k < N_KEYS_HW; ){
            int pair;
            for(pair = 0; pair < 30 && k < N_KEYS_HW; pair++, k++){
                // Select both standard and Corsair input. The standard input will be ignored except in BIOS mode.
                uchar action = IN_HID | IN_CORSAIR;
                // Additionally, make MR activate the MR ring (this is disabled for now, may be back later)
                //if(kb->kmap[k].name && !strcmp(kb->kmap[k].name, "mr"))
                //    action |= ACT_MR_RING;
                msg[1][4 + pair * 2] = k;
                msg[1][5 + pair * 2] = action;
            }
            // Byte 2 = pair count (usually 30, less on final message)
            msg[1][2] = pair;
            if(!usbsend(kb, msg[1], MSG_SIZE, 1))
                return -1;
        }
        // Commit new input settings
        if(!usbsend(kb, msg[2], MSG_SIZE, 1))
            return -1;
        DELAY_30MS();
    } else {
        // Set the M-keys back into hardware mode, restore hardware RGB profile. It has to be sent twice for some reason.
        msg[0][2] = MODE_HARDWARE;
        if(!usbsend(kb, msg[0], MSG_SIZE, 1))
            return -1;
        DELAY_30MS();
        if(!usbsend(kb, msg[0], MSG_SIZE, 1))
            return -1;
        DELAY_30MS();
#ifdef OS_LINUX
        // On OSX the default key mappings are fine. On Linux, the G keys will freeze the keyboard. Set the keyboard entirely to HID input.
        for(int k = 0; k < N_KEYS_HW; ){
            int pair;
            for(pair = 0; pair < 30 && k < N_KEYS_HW; pair++, k++){
                uchar action = IN_HID;
                // Enable hardware actions
                if(kb->keymap[k].name){
                    if(!strcmp(kb->keymap[k].name, "mr"))
                        action = ACT_MR_RING;
                    else if(!strcmp(kb->keymap[k].name, "m1"))
                        action = ACT_M1;
                    else if(!strcmp(kb->keymap[k].name, "m2"))
                        action = ACT_M2;
                    else if(!strcmp(kb->keymap[k].name, "m3"))
                        action = ACT_M3;
                    else if(!strcmp(kb->keymap[k].name, "light"))
                        action = ACT_LIGHT;
                    else if(!strcmp(kb->keymap[k].name, "lock"))
                        action = ACT_LOCK;
                }
                msg[1][4 + pair * 2] = k;
                msg[1][5 + pair * 2] = action;
            }
            // Byte 2 = pair count (usually 30, less on final message)
            msg[1][2] = pair;
            if(!usbsend(kb, msg[1], MSG_SIZE, 1))
                return -1;
        }
        // Commit new input settings
        if(!usbsend(kb, msg[2], MSG_SIZE, 1))
            return -1;
        DELAY_30MS();
#endif
    }
    // Update indicator LEDs if the profile contains settings for them
    kb->vtable.updateindicators(kb, 0);
    return 0;
}

int cmd_active_kb(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4){
    (void)dummy1;
    (void)dummy2;
    (void)dummy3;
    (void)dummy4;

    return setactive_kb(kb, 1);
}

int cmd_idle_kb(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4){
    (void)dummy1;
    (void)dummy2;
    (void)dummy3;
    (void)dummy4;

    return setactive_kb(kb, 0);
}

void setmodeindex_legacy(usbdevice* kb, int index){
    switch(index % 3){
    case 0:
        nk95cmd(kb, NK95_M1);
        break;
    case 1:
        nk95cmd(kb, NK95_M2);
        break;
    case 2:
        nk95cmd(kb, NK95_M3);
        break;
    }
}

static const struct timespec legacy_delay_ts = {.tv_nsec = 30000000000};
void legacy_delay(usbdevice* kb, delay_type_t type){
    // Fixed 30ms delay should be fine
    clock_nanosleep(CLOCK_MONOTONIC, 0, &legacy_delay_ts, NULL);
}

void nxp_kb_setfps(usbdevice* kb, int fps){
    // This is all guesswork
    //fullrange:
    //  750us at 60Hz
    //  800us at 55Hz
    //  830us at 50Hz
    //  830us at 45Hz
    // 1150us at 40Hz
    // 1500us at 35Hz
    // 1800us at 30Hz

    if(IS_FULLRANGE(kb)){
        // Not sure if it's worth handling IS_MONOCHROME_DEV(kb)

        // The keyboards get really sensitive between 40~45FPS.
        // 1150us is fine for 40FPS but 900us is not enough for 41FPS
        if(fps >= 55)
            kb->usbdelay_ns = 750000L;
        else if(fps >= 47)
            kb->usbdelay_ns = 800000L;
        else if(fps > 40)
            kb->usbdelay_ns = 850000L;
        else if(fps > 35)
            kb->usbdelay_ns = 1150000L;
        else if(fps > 30)
            kb->usbdelay_ns = 1500000L;
        else if(fps > 25)
            kb->usbdelay_ns = 1800000L;
        else
            kb->usbdelay_ns = 2500000L;
    } else {
        if(fps >= 50)
            kb->usbdelay_ns = 2650000L;
        else if(fps >= 40)
            kb->usbdelay_ns = 2800000L;
        else if(fps >= 30)
            kb->usbdelay_ns = 3700000L;
        else if(fps >= 15)
            kb->usbdelay_ns = 5500000L;
        else
            kb->usbdelay_ns = 10000000L;
    }
}

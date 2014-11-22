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

void setinput(usbdevice* kb, int input){
    // Set input mode on the keys. 0x80 generates a normal HID interrupt, 0x40 generates a proprietary interrupt. 0xc0 generates both.
    // NOTE: I observed the windows driver setting a key to 0x49; it seems there are other bits used in this message. I doubt that
    // they're useful, though. Additionally, the windows driver omits some of the key indices, but there seems to be no harm in
    // including all of them.
    uchar datapkt[6][MSG_SIZE] = { };
    for(int i = 0; i < 5; i++){
        datapkt[i][0] = 0x07;
        datapkt[i][1] = 0x40;
        datapkt[i][2] = 30;
    }
    datapkt[4][2] = 24;
    datapkt[5][0] = 0x07;
    datapkt[5][1] = 0x05;
    datapkt[5][2] = 0x02;
    datapkt[5][4] = 0x03;
    // The special corsair keys don't have any HID scancode, so don't allow them to generate HID interrupts no matter what.
    // (these should have the same key index regardless of layout)
#define IMASK(key) ~((keymap_system[key].scan == -2) << 7)
    for(int i = 0; i < 30; i++){
        int key = i;
        datapkt[0][i * 2 + 4] = key;
        datapkt[0][i * 2 + 5] = input & IMASK(key);
    }
    for(int i = 0; i < 30; i++){
        int key = i + 30;
        datapkt[1][i * 2 + 4] = key;
        datapkt[1][i * 2 + 5] = input & IMASK(key);
    }
    for(int i = 0; i < 30; i++){
        int key = i + 60;
        datapkt[2][i * 2 + 4] = key;
        datapkt[2][i * 2 + 5] = input & IMASK(key);
    }
    for(int i = 0; i < 30; i++){
        int key = i + 90;
        datapkt[3][i * 2 + 4] = key;
        datapkt[3][i * 2 + 5] = input & IMASK(key);
    }
    for(int i = 0; i < 24; i++){
        int key = i + 120;
        datapkt[4][i * 2 + 4] = key;
        datapkt[4][i * 2 + 5] = input & IMASK(key);
    }
#undef IMASK
    usbqueue(kb, datapkt[0], 6);
}

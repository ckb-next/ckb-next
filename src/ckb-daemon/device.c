#include "device.h"
#include "input.h"
#include "led.h"

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
        eraseprofile(res);
    strcpy(res->serial, serial);
    genid(&res->id);
    return res;
}

usbmode* getusbmode(int id, usbprofile* profile, const key* keymap){
    if(id < profile->modecount)
        return profile->mode + id;
    int cap = id / 4 * 4 + 4;
    if(cap > profile->modecap){
        profile->mode = realloc(profile->mode, cap * sizeof(usbmode));
        profile->modecap = cap;
    }
    // Initialize any newly-created modes
    for(int i = profile->modecount; i <= id; i++){
        memset(profile->mode + i, 0, sizeof(profile->mode[i]));
        initrgb(&profile->mode[i].light);
        initbind(&profile->mode[i].bind, keymap);
        genid(&profile->mode[i].id);
    }
    profile->modecount = id + 1;
    return profile->mode + id;
}

iconv_t utf8to16 = 0;

void urldecode2(char *dst, const char *src){
    char a, b;
    while (*src) {
        if ((*src == '%') &&
                ((a = src[1]) && (b = src[2])) &&
                (isxdigit(a) && isxdigit(b))) {
            if (a >= 'a')
                a -= 'a'-'A';
            if (a >= 'A')
                a -= ('A' - 10);
            else
                a -= '0';
            if (b >= 'a')
                b -= 'a'-'A';
            if (b >= 'A')
                b -= ('A' - 10);
            else
                b -= '0';
            *dst++ = 16*a+b;
            src+=3;
        } else {
            *dst++ = *src++;
        }
    }
    *dst++ = '\0';
}

void cmd_setmodename(usbmode* mode, const key* keymap, int zero, const char* name){
    if(!utf8to16)
        utf8to16 = iconv_open("UTF-16LE", "UTF-8");
    memset(mode->name, 0, sizeof(mode->name));
    char decoded[strlen(name) + 1];
    urldecode2(decoded, name);
    size_t srclen = strlen(decoded), dstlen = MD_NAME_LEN * 2 - 2;
    char* in = decoded, *out = (char*)mode->name;
    iconv(utf8to16, &in, &srclen, &out, &dstlen);
}

void setprofilename(usbprofile* profile, const char* name){
    if(!utf8to16)
        utf8to16 = iconv_open("UTF-16LE", "UTF-8");
    memset(profile->name, 0, sizeof(profile->name));
    char decoded[strlen(name) + 1];
    urldecode2(decoded, name);
    size_t srclen = strlen(decoded), dstlen = PR_NAME_LEN * 2 - 2;
    char* in = decoded, *out = (char*)profile->name;
    iconv(utf8to16, &in, &srclen, &out, &dstlen);
}

void erasemode(usbmode *mode, const key* keymap){
    closebind(&mode->bind);
    memset(mode, 0, sizeof(*mode));
    initrgb(&mode->light);
    initbind(&mode->bind, keymap);
    genid(&mode->id);
}

void eraseprofile(usbprofile* profile){
    // Clear all mode data
    for(int i = 0; i < profile->modecount; i++)
        closebind(&profile->mode[i].bind);
    free(profile->mode);
    memset(profile, 0, sizeof(*profile));
    genid(&profile->id);
    // There need to be at least three profiles or some functions might crash
    profile->currentmode = getusbmode(0, profile, profile->keymap);
    getusbmode(1, profile, profile->keymap);
    getusbmode(2, profile, profile->keymap);
}

void freeprofile(usbprofile* profile){
    // Clear all mode data
    for(int i = 0; i < profile->modecount; i++)
        closebind(&profile->mode[i].bind);
    free(profile->mode);
    memset(profile, 0, sizeof(*profile));
}

void genid(usbid* id){
    static int seeded = 0;
    if(!seeded){
        srand(time(NULL));
        seeded = 1;
    }
    // Generate a random ID
    int numbers[4] = { rand(), rand(), rand(), rand() };
    memcpy(id->guid, numbers, sizeof(id->guid));
    memset(id->modified, 0, sizeof(id->modified));
}

void updatemod(usbid* id){
    struct timeval tv;
    gettimeofday(&tv, 0);
    short new = (short)tv.tv_usec, old;
    memcpy(&old, id->modified, 2);
    if(new == old)
        new++;
    memcpy(id->modified, &new, 2);
}

// Converts a hardware profile to a native profile
void hwtonative(usbprofile* profile, hwprofile* hw, int modes){
    // Copy the profile and mode names
    memcpy(profile->name, hw->name[0], PR_NAME_LEN * 2);
    for(int i = 0; i < modes; i++)
        memcpy(profile->mode[i].name, hw->name[i + 1], MD_NAME_LEN * 2);
    // Copy the profile and mode IDs
    memcpy(&profile->id, hw->id, sizeof(*hw->id));
    for(int i = 0; i < modes; i++)
        memcpy(&profile->mode[i].id, hw->id + i + 1, sizeof(*hw->id));
    // Copy the key lighting
    for(int i = 0; i < modes; i++)
        memcpy(&profile->mode[i].light, hw->light + i, sizeof(*hw->light));
}

// Converts a native profile to a hardware profile
void nativetohw(usbprofile* profile, hwprofile* hw, int modes){
    // Copy the profile and mode names
    memcpy(hw->name[0], profile->name, PR_NAME_LEN * 2);
    for(int i = 0; i < modes; i++)
        memcpy(hw->name[i + 1], profile->mode[i].name, MD_NAME_LEN * 2);
    // Copy the profile and mode IDs
    memcpy(hw->id, &profile->id, sizeof(*hw->id));
    for(int i = 0; i < modes; i++)
        memcpy(hw->id + i + 1, &profile->mode[i].id, sizeof(*hw->id));
    // Copy the key lighting
    for(int i = 0; i < modes; i++)
        memcpy(hw->light + i, &profile->mode[i].light, sizeof(*hw->light));
}

int hwloadmode(usbdevice* kb, hwprofile* hw, int mode){
    // Ask for mode's name
    uchar data_pkt[MSG_SIZE] = { 0x0e, 0x16, 0x01, mode + 1, 0 };
    usbqueue(kb, data_pkt, 1);
    usleep(3000);
    if(!usbdequeue(kb))
        return -1;
    // Wait for the response
    if(!usbinput(kb, data_pkt))
        return -1;
    memcpy(hw->name + mode + 1, data_pkt + 4, MD_NAME_LEN * 2);
    // Load the RGB setting
    return loadrgb(kb, hw->light + mode, mode);
}

int hwloadprofile(usbdevice* kb, int apply){
    if(!IS_ACTIVE(kb))
        return 0;
    // Empty the board's USB queue
    while(kb->queuecount > 0){
        usleep(3000);
        if(!usbdequeue(kb))
            return -1;
    }
    hwprofile* hw = calloc(1, sizeof(hwprofile));
    // Ask for profile and mode IDs
    uchar data_pkt[2][MSG_SIZE] = {
        { 0x0e, 0x15, 0x01, 0 },
        { 0x0e, 0x16, 0x01, 0 }
    };
    uchar in_pkt[MSG_SIZE];
    int modes = (kb->model == 95 ? HWMODE_K95 : HWMODE_K70);
    for(int i = 0; i <= modes; i++){
        data_pkt[0][3] = i;
        usbqueue(kb, data_pkt[0], 1);
        usleep(3000);
        if(!usbdequeue(kb)){
            free(hw);
            return -1;
        }
        // Wait for the response
        if(!usbinput(kb, in_pkt)){
            free(hw);
            return -1;
        }
        memcpy(hw->id + i, in_pkt + 4, sizeof(usbid));
    }
    // Ask for profile name
    usbqueue(kb, data_pkt[1], 1);
    usleep(3000);
    if(!usbdequeue(kb)){
        free(hw);
        return -1;
    }
    // Wait for the response
    if(!usbinput(kb, in_pkt)){
        free(hw);
        return -1;
    }
    memcpy(hw->name[0], in_pkt + 4, PR_NAME_LEN * 2);
    // Load modes
    for(int i = 0; i < modes; i++){
        if(hwloadmode(kb, hw, i)){
            free(hw);
            return -1;
        }
    }
    // Make the profile active (if requested)
    if(apply)
        hwtonative(&kb->profile, hw, modes);
    // Free the existing profile (if any)
    if(kb->profile.hw)
        free(kb->profile.hw);
    kb->profile.hw = hw;
    return 0;
}

void hwsaveprofile(usbdevice* kb){
    if(!IS_ACTIVE(kb))
        return;
    hwprofile* hw = kb->profile.hw;
    if(!hw)
        hw = kb->profile.hw = calloc(1, sizeof(hwprofile));
    int modes = (kb->model == 95 ? HWMODE_K95 : HWMODE_K70);
    nativetohw(&kb->profile, hw, modes);
    // Save the profile and mode names
    uchar data_pkt[2][MSG_SIZE] = {
        {0x07, 0x16, 0x01, 0 },
        {0x07, 0x15, 0x01, 0 },
    };
    // Save the mode names
    for(int i = 0; i <= modes; i++){
        data_pkt[0][3] = i;
        memcpy(data_pkt[0] + 4, hw->name[i], MD_NAME_LEN * 2);
        usbqueue(kb, data_pkt[0], 1);
    }
    // Save the IDs
    for(int i = 0; i <= modes; i++){
        data_pkt[1][3] = i;
        memcpy(data_pkt[1] + 4, hw->id + i, sizeof(usbid));
        usbqueue(kb, data_pkt[1], 1);
    }
    // Save the RGB data
    for(int i = 0; i < modes; i++)
        savergb(kb, i);
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

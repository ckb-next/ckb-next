#include "device.h"
#include "usb.h"
#include "led.h"
#include "input.h"

usbdevice keyboard[DEV_MAX];
pthread_mutex_t kblistmutex = PTHREAD_MUTEX_INITIALIZER;
usbsetting* store = 0;
int storecount = 0;

usbdevice* findusb(const char* serial){
    for(int i = 0; i < DEV_MAX; i++){
        if(keyboard[i].fifo && !strcmp(serial, keyboard[i].setting.serial))
            return keyboard + i;
    }
    return 0;
}

usbsetting* findstore(const char* serial){
    for(int i = 0; i < storecount; i++){
        usbsetting* res = store + i;
        if(!strcmp(res->serial, serial))
            return res;
    }
    return 0;
}

usbsetting* addstore(const char* serial){
    // Try to find the device before adding it
    usbsetting* res = findstore(serial);
    if(res)
        return res;
    // Add device to the list
    store = realloc(store, ++storecount * sizeof(usbsetting));
    res = store + storecount - 1;
    // Initialize device
    memset(&res->profile, 0, sizeof(res->profile));
    strcpy(res->serial, serial);
    genid(&res->profile.id);
    return res;
}

usbmode* getusbmode(int id, usbprofile* profile){
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
        initbind(&profile->mode[i].bind);
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

void cmd_setmodename(usbmode* mode, int zero, const char* name){
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

void erasemode(usbmode *mode){
    closebind(&mode->bind);
    memset(mode, 0, sizeof(*mode));
    initrgb(&mode->light);
    initbind(&mode->bind);
    genid(&mode->id);
}

void eraseprofile(usbprofile* profile){
    // Clear all mode data
    for(int i = 0; i < profile->modecount; i++)
        closebind(&profile->mode[i].bind);
    free(profile->mode);
    memset(profile, 0, sizeof(*profile));
    genid(&profile->id);
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

void hwloadmode(usbdevice* kb, int mode){
    // Ask for mode's name
    usbmode* kbmode = kb->setting.profile.mode + mode;
    uchar data_pkt[MSG_SIZE] = { 0x0e, 0x16, 0x01, mode + 1, 0 };
    usbqueue(kb, data_pkt, 1);
    usleep(3333);
    usbdequeue(kb);
    // Wait for the response
    usbinput(kb, data_pkt);
    if(data_pkt[0] == 0x0e && data_pkt[1] == 0x01)
        memcpy(kbmode->name, data_pkt + 4, MD_NAME_LEN * 2);
    // Load the RGB setting
    loadleds(kb, mode);
}

void hwloadprofile(usbdevice* kb){
    if(!IS_ACTIVE(kb))
        return;
    // Empty the board's USB queue
    while(kb->queuecount > 0){
        usleep(3333);
        usbdequeue(kb);
    }
    // Ask for profile ID
    usbprofile* profile = &kb->setting.profile;
    uchar data_pkt[2][MSG_SIZE] = {
        { 0x0e, 0x15, 0x01, 0 },
        { 0x0e, 0x16, 0x01, 0 }
    };
    uchar in_pkt[MSG_SIZE];
    usbqueue(kb, data_pkt[0], 1);
    usleep(3333);
    usbdequeue(kb);
    // Wait for the response
    usbinput(kb, in_pkt);
    memcpy(&profile->id, in_pkt + 4, sizeof(usbid));
    // Ask for mode IDs
    int modes = (kb->model == 95 ? 3 : 1);
    for(int i = 0; i < modes; i++){
        data_pkt[0][3] = i + 1;
        usbqueue(kb, data_pkt[0], 1);
        usleep(3333);
        usbdequeue(kb);
        // Wait for the response
        usbinput(kb, in_pkt);
        memcpy(&profile->mode[i].id, in_pkt + 4, sizeof(usbid));
    }
    // Ask for profile name
    usbqueue(kb, data_pkt[1], 1);
    usleep(3333);
    usbdequeue(kb);
    // Wait for the response
    usbinput(kb, in_pkt);
    memcpy(kb->setting.profile.name, in_pkt + 4, PR_NAME_LEN * 2);
    // Load modes
    for(int i = 0; i < modes; i++)
        hwloadmode(kb, i);
}

void hwsaveprofile(usbdevice* kb){
    if(!IS_ACTIVE(kb))
        return;
    // Save the profile name
    usbprofile* profile = &kb->setting.profile;
    uchar data_pkt[2][MSG_SIZE] = {
        {0x07, 0x16, 0x01, 0 },
        {0x07, 0x15, 0x01, 0, 1, 2, 3, 4, 5 },
    };
    memcpy(data_pkt[0] + 4, profile->name, PR_NAME_LEN * 2);
    usbqueue(kb, data_pkt[0], 1);
    // Save the mode names
    int modes = (kb->model == 95 ? 3 : 1);
    for(int i = 0; i < modes; i++){
        data_pkt[0][3] = i + 1;
        memcpy(data_pkt[0] + 4, profile->mode[i].name, MD_NAME_LEN * 2);
        usbqueue(kb, data_pkt[0], 1);
    }
    // Save the profile ID
    memcpy(data_pkt[1] + 4, &profile->id, sizeof(usbid));
    usbqueue(kb, data_pkt[1], 1);
    // Save the mode IDs
    for(int i = 0; i < modes; i++){
        data_pkt[1][3] = i + 1;
        memcpy(data_pkt[1] + 4, &profile->mode[i].id, sizeof(usbid));
        usbqueue(kb, data_pkt[1], 1);
    }
    // Save the RGB data
    for(int i = 0; i < modes; i++)
        saveleds(kb, i);
}

void setinput(usbdevice* kb, int input){
    // Set input mode on the keys. 0x80 generates a normal HID interrupt, 0x40 generates a proprietary interrupt. 0xc0 generates both.
    // NOTE: I observed the windows driver setting a key to 0x49; it seems there are other bits used in this message. I doubt that
    // they're useful, though. Additionally, the windows driver omits some of the key indices, but there seems to be no harm in
    // including all of them.
    uchar datapkt[6][MSG_SIZE] = { };
    for(int i = 0; i < 4; i++){
        datapkt[i][0] = 0x07;
        datapkt[i][1] = 0x40;
        datapkt[i][2] = 0x1e;
    }
    datapkt[4][0] = 0x07;
    datapkt[4][1] = 0x40;
    datapkt[4][2] = 0x0d;
    datapkt[5][0] = 0x07;
    datapkt[5][1] = 0x05;
    datapkt[5][2] = 0x02;
    datapkt[5][4] = 0x03;
    // The special corsair keys don't have any HID scancode, so don't allow them to generate HID interrupts no matter what
#define IMASK(key) ~((keymap[key].scan == -1) << 7)
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

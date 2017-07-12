#include "command.h"
#include "device.h"
#include "input.h"
#include "led.h"
#include "profile.h"

// Percent-enconding conversions
void urldecode2(char* dst, const char* src){
    char a, b;
    char s;
    while((s = *src)){
        if((s == '%') &&
                ((a = src[1]) && (b = src[2])) &&
                (isxdigit(a) && isxdigit(b))){
            if(a >= 'a')
                a -= 'a'-'A';
            if(a >= 'A')
                a -= 'A' - 10;
            else
                a -= '0';
            if(b >= 'a')
                b -= 'a'-'A';
            if(b >= 'A')
                b -= 'A' - 10;
            else
                b -= '0';
            *dst++ = 16 * a + b;
            src += 3;
        } else {
            *dst++ = s;
            src++;
        }
    }
    *dst = '\0';
}

void urlencode2(char* dst, const char* src){
    char s;
    while((s = *src++)){
        if(s <= ',' || s == '/' ||
                (s >= ':' && s <= '@') ||
                s == '[' || s == ']' ||
                s >= 0x7F){
            char a = s >> 4, b = s & 0xF;
            if(a >= 10)
                a += 'A' - 10;
            else
                a += '0';
            if(b >= 10)
                b += 'A' - 10;
            else
                b += '0';
            dst[0] = '%';
            dst[1] = a;
            dst[2] = b;
            dst += 3;
        } else
            *dst++ = s;
    }
    *dst = '\0';
}

// GUID conversions
int setid(usbid* id, const char* guid){
    int32_t data1;
    int16_t data2, data3, data4a;
    char data4b[6];
    if(sscanf(guid, "{%08X-%04hX-%04hX-%04hX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
              &data1, &data2, &data3, &data4a, data4b, data4b + 1, data4b + 2, data4b + 3, data4b + 4, data4b + 5) != 10)
        return 0;
    memcpy(id->guid + 0x0, &data1, 4);
    memcpy(id->guid + 0x4, &data2, 2);
    memcpy(id->guid + 0x6, &data3, 2);
    memcpy(id->guid + 0x8, &data4a, 2);
    memcpy(id->guid + 0xA, data4b, 6);
    return 1;
}

char* getid(usbid* id){
    int32_t data1;
    int16_t data2, data3, data4a;
    char data4b[6];
    memcpy(&data1, id->guid + 0x0, 4);
    memcpy(&data2, id->guid + 0x4, 2);
    memcpy(&data3, id->guid + 0x6, 2);
    memcpy(&data4a, id->guid + 0x8, 2);
    memcpy(data4b, id->guid + 0xA, 6);
    char* guid = malloc(39);
    snprintf(guid, 39, "{%08X-%04hX-%04hX-%04hX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
             data1, data2, data3, data4a, data4b[0], data4b[1], data4b[2], data4b[3], data4b[4], data4b[5]);
    return guid;
}

// UTF-8/UTF-16 conversions (srclen and dstlen in characters - 1 byte UTF8, 2 bytes UTF16)
static iconv_t utf8to16 = 0, utf16to8 = 0;

void u16enc(char* in, ushort* out, size_t* srclen, size_t* dstlen){
    if(!utf8to16)
        utf8to16 = iconv_open("UTF-16LE", "UTF-8");
    memset(out, 0, *dstlen * 2);
    *dstlen = *dstlen * 2 - 2;
    iconv(utf8to16, &in, srclen, (char**)&out, dstlen);
}

void u16dec(ushort* in, char* out, size_t* srclen, size_t* dstlen){
    if(!utf16to8)
        utf16to8 = iconv_open("UTF-8", "UTF-16LE");
    size_t srclen2 = 0, srclenmax = *srclen;
    for(; srclen2 < srclenmax; srclen2++){
        if(!in[srclen2])
            break;
    }
    *srclen = srclen2 * 2;
    iconv(utf16to8, (char**)&in, srclen, &out, dstlen);
}

void cmd_name(usbdevice* kb, usbmode* mode, int dummy1, int dummy2, const char* name){
    char decoded[strlen(name) + 1];
    urldecode2(decoded, name);
    size_t srclen = strlen(decoded), dstlen = MD_NAME_LEN;
    u16enc(decoded, mode->name, &srclen, &dstlen);
}

void cmd_profilename(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* name){
    usbprofile* profile = kb->profile;
    char decoded[strlen(name) + 1];
    urldecode2(decoded, name);
    size_t srclen = strlen(decoded), dstlen = PR_NAME_LEN;
    u16enc(decoded, profile->name, &srclen, &dstlen);
}

char* printname(ushort* name, int length){
    // Convert the name to UTF-8
    char* buffer = calloc(1, length * 4 - 3);
    size_t srclen = length, dstlen = length * 4 - 4;
    u16dec(name, buffer, &srclen, &dstlen);
    // URL-encode it
    char* buffer2 = malloc(strlen(buffer) * 3 + 1);
    urlencode2(buffer2, buffer);
    free(buffer);
    return buffer2;
}

char* getmodename(usbmode* mode){
    return printname(mode->name, MD_NAME_LEN);
}

char* getprofilename(usbprofile* profile){
    return printname(profile->name, PR_NAME_LEN);
}

char* gethwmodename(hwprofile* profile, int index){
    return printname(profile->name[index + 1], MD_NAME_LEN);
}

char* gethwprofilename(hwprofile* profile){
    return printname(profile->name[0], MD_NAME_LEN);
}

void cmd_id(usbdevice* kb, usbmode* mode, int dummy1, int dummy2, const char* id){
    // ID is either a GUID or an 8-digit hex number
    int newmodified;
    if(!setid(&mode->id, id) && sscanf(id, "%08x", &newmodified) == 1)
        memcpy(mode->id.modified, &newmodified, sizeof(newmodified));
}

void cmd_profileid(usbdevice* kb, usbmode* mode, int dummy1, int dummy2, const char* id){
    usbprofile* profile = kb->profile;
    int newmodified;
    if(!setid(&profile->id, id) && sscanf(id, "%08x", &newmodified) == 1)
        memcpy(profile->id.modified, &newmodified, sizeof(newmodified));

}

static void initmode(usbmode* mode){
    memset(mode, 0, sizeof(*mode));
    mode->light.forceupdate = 1;
    mode->dpi.forceupdate = 1;
    initbind(&mode->bind);
}

void allocprofile(usbdevice* kb){
    if(kb->profile)
        return;
    usbprofile* profile = kb->profile = calloc(1, sizeof(usbprofile));
    for(int i = 0; i < MODE_COUNT; i++)
        initmode(profile->mode + i);
    profile->currentmode = profile->mode;
    profile->lastlight.forceupdate = profile->lastdpi.forceupdate = 1;
}

int loadprofile(usbdevice* kb){
    if(hwloadprofile(kb, 1))
        return -1;
    return 0;
}

static void freemode(usbmode* mode){
    freebind(&mode->bind);
    memset(mode, 0, sizeof(*mode));
}

void cmd_erase(usbdevice* kb, usbmode* mode, int dummy1, int dummy2, const char* dummy3){
    pthread_mutex_lock(imutex(kb));
    freemode(mode);
    initmode(mode);
    pthread_mutex_unlock(imutex(kb));
}

static void _freeprofile(usbdevice* kb){
    usbprofile* profile = kb->profile;
    if(!profile)
        return;
    // Clear all mode data
    for(int i = 0; i < MODE_COUNT; i++)
        freemode(profile->mode + i);
    free(profile);
    kb->profile = 0;
}

void cmd_eraseprofile(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4){
    pthread_mutex_lock(imutex(kb));
    _freeprofile(kb);
    allocprofile(kb);
    pthread_mutex_unlock(imutex(kb));
}

void freeprofile(usbdevice* kb){
    _freeprofile(kb);
    // Also free HW profile
    free(kb->hw);
    kb->hw = 0;
}

void hwtonative(usbprofile* profile, hwprofile* hw, int modecount){
    // Copy the profile name and ID
    memcpy(profile->name, hw->name[0], PR_NAME_LEN * 2);
    memcpy(&profile->id, hw->id, sizeof(usbid));
    // Copy the mode settings
    for(int i = 0; i < modecount; i++){
        usbmode* mode = profile->mode + i;
        memcpy(mode->name, hw->name[i + 1], MD_NAME_LEN * 2);
        memcpy(&mode->id, hw->id + i + 1, sizeof(usbid));
        memcpy(&mode->light, hw->light + i, sizeof(lighting));
        memcpy(&mode->dpi, hw->dpi + i, sizeof(dpiset));
        // Set a force update on the light/DPI since they've been overwritten
        mode->light.forceupdate = mode->dpi.forceupdate = 1;
    }
    profile->lastlight.forceupdate = profile->lastdpi.forceupdate = 1;
}

void nativetohw(usbprofile* profile, hwprofile* hw, int modecount){
    // Copy name and ID
    memcpy(hw->name[0], profile->name, PR_NAME_LEN * 2);
    memcpy(hw->id, &profile->id, sizeof(usbid));
    // Copy the mode settings
    for(int i = 0; i < modecount; i++){
        usbmode* mode = profile->mode + i;
        memcpy(hw->name[i + 1], mode->name, MD_NAME_LEN * 2);
        memcpy(hw->id + i + 1, &mode->id, sizeof(usbid));
        memcpy(hw->light + i, &mode->light, sizeof(lighting));
        memcpy(hw->dpi + i, &mode->dpi, sizeof(dpiset));
    }
}

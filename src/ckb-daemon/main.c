#define _DEFAULT_SOURCE
#include <features.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/signal.h>
#include <libusb-1.0/libusb.h>

#define N_KEYS 144
typedef struct {
    const char* name;
    int code;
} key;
key keymap[] = {
    { "esc", 0 },
    { "f1", 12 },
    { "f2", 24 },
    { "f3", 36 },
    { "f4", 48 },
    { "f5", 60 },
    { "f6", 72 },
    { "f7", 84 },
    { "f8", 96 },
    { "f9", 108 },
    { "f10", 120 },
    { "f11", 132 },
    { "f12", 6 },
    { "tilde", 1 },
    { "1", 13 },
    { "2", 25 },
    { "3", 37 },
    { "4", 49 },
    { "5", 61 },
    { "6", 73 },
    { "7", 85 },
    { "8", 97 },
    { "9", 109 },
    { "0", 121 },
    { "minus", 133 },
    { "plus", 7 },
    { "bspace", 31 },
    { "tab", 2 },
    { "q", 14 },
    { "w", 26 },
    { "e", 38 },
    { "r", 50 },
    { "t", 62 },
    { "y", 74 },
    { "u", 86 },
    { "i", 98},
    { "o",  110 },
    { "p", 122 },
    { "lbracket", 134 },
    { "rbracket", 90 },
    { "bslash", 102 },
    { "caps", 3 },
    { "a", 15 },
    { "s", 27 },
    { "d", 39 },
    { "f", 51 },
    { "g", 63 },
    { "h", 75 },
    { "j", 87 },
    { "k", 99 },
    { "l", 111 },
    { "colon", 123 },
    { "quote", 135 },
    { "enter", 126 },
    { "lshift", 4 },
    { "z", 28 },
    { "x", 40 },
    { "c", 52 },
    { "v", 64 },
    { "b", 76 },
    { "n", 88 },
    { "m", 100 },
    { "comma", 112 },
    { "dot", 124 },
    { "slash", 136 },
    { "rshift", 79 },
    { "lctrl", 5 },
    { "lwin", 17 },
    { "lalt", 29 },
    { "space", 53 },
    { "ralt", 89 },
    { "rwin", 101 },
    { "rmenu", 113 },
    { "rctrl", 91 },
    { "prtscn", 18 },
    { "scroll", 30 },
    { "pause", 42 },
    { "ins", 54 },
    { "home", 66 },
    { "pgup", 78 },
    { "del", 43 },
    { "end", 55 },
    { "pgdown", 67 },
    { "up", 103},
    { "left", 115 },
    { "down", 127 },
    { "right", 139 },
    { "numlock", 80 },
    { "numslash", 92 },
    { "numtimes", 104 },
    { "numminus", 116 },
    { "num7", 9 },
    { "num8", 21 },
    { "num9", 33 },
    { "numplus", 128 },
    { "num4", 57 },
    { "num5", 69 },
    { "num6", 81 },
    { "num1", 93 },
    { "num2", 105 },
    { "num3", 117 },
    { "numenter", 140 },
    { "num0", 129 },
    { "numdot", 141 },
    { "stop", 32 },
    { "prev", 44 },
    { "play", 56 },
    { "next", 68 },
    { "mute", 20 },
    { "light", 137 },
    { "lock", 8 },

    // k95 buttons
    { "g1", 10 },
    { "g2", 22 },
    { "g3", 34 },
    { "g4", 46 },
    { "g5", 58 },
    { "g6", 70 },
    { "g7", 82 },
    { "g8", 94 },
    { "g9", 106 },
    { "g10", 118 },
    { "g11", 59 },
    { "g12", 71 },
    { "g13", 83 },
    { "g14", 95 },
    { "g15", 107 },
    { "g16", 119 },
    { "g17", 131 },
    { "g18", 143 },
    { "mr", 11 },
    { "m1", 23 },
    { "m2", 35 },
    { "m3", 47 },
};
#define N_KEYMAP (sizeof(keymap) / sizeof(key))


// Structure for tracking keyboard devices
#define DEV_MAX     10
#define NAME_LEN 33
#define SERIAL_LEN   33
#define RGB_SIZE (N_KEYS * sizeof(short))
#define QUEUE_LEN 6
#define MSG_SIZE 64
typedef struct {
    short* rgb;
    char* queue[QUEUE_LEN];
    int ledfifo;
    int queuelength;
    struct libusb_device_descriptor descriptor;
    libusb_device* dev;
    libusb_device_handle* handle;
    int rgbon;
    int model;
    char name[NAME_LEN];
    char serial[SERIAL_LEN];
} usbdevice;
usbdevice keyboard[DEV_MAX];

// Structure to store RGB settings for a keyboard not currently connected
typedef struct {
    short* rgb;
    int rgbon;
    char serial[SERIAL_LEN];
} usbstore;
usbstore* store = 0;
int storecount = 0;

void initrgb(short** rgb, int* rgbon){
    if(!*rgb){
        // Allocate colors if they're not set already. Default to all white.
        *rgb = malloc(RGB_SIZE);
        *rgbon = 1;
        memset(*rgb, 0xFF, RGB_SIZE);
    }
}

usbstore* findstore(const char* serial){
    for(int i = 0; i < storecount; i++){
        usbstore* res = store + i;
        if(!strcmp(res->serial, serial))
            return res;
    }
    return 0;
}

usbstore* addstore(const char* serial){
    // Try to find the device before adding it
    usbstore* res = findstore(serial);
    if(res)
        return res;
    // Add device to the list
    store = realloc(store, ++storecount * sizeof(usbstore));
    res = store + storecount - 1;
    memset(res, 0, sizeof(usbstore));
    strcpy(res->serial, serial);
    return res;
}

int usbqueue(usbdevice* kb, char* messages, int count){
    // Don't add messages unless the queue has enough room for all of them
    if(kb->queuelength + count > QUEUE_LEN)
        return -1;
    for(int i = 0; i < count; i++)
        memcpy(kb->queue[kb->queuelength + i], messages + MSG_SIZE * i, MSG_SIZE);
    kb->queuelength += count;
    return 0;
}

int usbdequeue(usbdevice* kb){
    if(kb->queuelength == 0 || !kb->handle)
        return 0;
    int count = libusb_control_transfer(kb->handle, 0x21, 0x09, 0x0300, 0x03, kb->queue[0], MSG_SIZE, 0);
    // Rotate queue
    char* first = kb->queue[0];
    for(int i = 1; i < QUEUE_LEN; i++)
        kb->queue[i - 1] = kb->queue[i];
    kb->queue[QUEUE_LEN - 1] = first;
    kb->queuelength--;
    return count;
}

#define red(i) (7 - (rgb[i] >> 6 & 0x7))
#define green(i) (7 - (rgb[i] >> 3 & 0x7))
#define blue(i) (7 - (rgb[i] & 0x7))

void updateleds(usbdevice* kb, short* rgb){
    if(!kb)
        return;
    int freemem = 0;
    if(!rgb){
        rgb = malloc(RGB_SIZE);
        memset(rgb, 0, RGB_SIZE);
        freemem = 1;
    }
    char data_pkt[5][MSG_SIZE] = { { 0 } };
    data_pkt[0][0] = 0x7F;
    data_pkt[0][1] = 0x01;
    data_pkt[0][2] = 0x3C;

    data_pkt[1][0] = 0x7F;
    data_pkt[1][1] = 0x02;
    data_pkt[1][2] = 0x3C;

    data_pkt[2][0] = 0x7F;
    data_pkt[2][1] = 0x03;
    data_pkt[2][2] = 0x3C;

    data_pkt[3][0] = 0x7F;
    data_pkt[3][1] = 0x04;
    data_pkt[3][2] = 0x24;

    data_pkt[4][0] = 0x07;
    data_pkt[4][1] = 0x27;
    data_pkt[4][4] = 0xD8;

    for(int i = 0; i < 60; i++)
        data_pkt[0][i+4] = red(i*2+1) << 4 | red(i*2);
    for(int i = 0; i < 12; i++)
        data_pkt[1][i+4] = red(i*2+121) << 4 | red(i*2+120);
    for(int i = 0; i < 48; i++)
        data_pkt[1][i+16] = green(i*2+1) << 4 | green(i*2);
    for(int i = 0; i < 24; i++)
        data_pkt[2][i+4] = green(i*2+97) << 4 | green(i*2+96);
    for(int i = 0; i < 36; i++)
        data_pkt[2][i+28] = blue(i*2+1) << 4 | blue(i*2);
    for(int i = 0; i < 36; i++)
        data_pkt[3][i+4] = blue(i*2+73) << 4 | blue(i*2+72);

    usbqueue(kb, data_pkt[0], 5);

    if(freemem)
        free(rgb);
}

int rm_recursive(const char* path){
    DIR* dir = opendir(path);
    if(!dir)
        return remove(path);
    struct dirent* file;
    while((file = readdir(dir)))
    {
        if(!strcmp(file->d_name, ".") || !strcmp(file->d_name, ".."))
            continue;
        char path2[FILENAME_MAX];
        snprintf(path2, FILENAME_MAX, "%s/%s", path, file->d_name);
        int stat = rm_recursive(path2);
        if(stat != 0)
            return stat;
    }
    closedir(dir);
    return remove(path);
}

const char *const devpath = "/dev/input/ckb";
#define S_READDIR (S_IRWXU | S_IRGRP | S_IROTH | S_IXGRP | S_IXOTH)
#define S_READ (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR)
#define S_READWRITE (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP | S_IWOTH)

void updateconnected(){
    char cpath[strlen(devpath) + 12];
    snprintf(cpath, sizeof(cpath), "%s0/connected", devpath);
    FILE* cfile = fopen(cpath, "w");
    if(!cfile){
        printf("Warning: Unable to update %s: %s\n", cpath, strerror(errno));
        return;
    }
    int written = 0;
    for(int i = 1; i < DEV_MAX; i++){
        if(keyboard[i].handle){
            written = 1;
            fprintf(cfile, "%s%d %s %s\n", devpath, i, keyboard[i].serial, keyboard[i].name);
        }
    }
    if(!written)
        fputc('\n', cfile);
    fclose(cfile);
    chmod(cpath, S_READ);
}

int makedevpath(int index){
    usbdevice* kb = keyboard + index;
    // Create the control path
    char path[strlen(devpath) + 2];
    snprintf(path, sizeof(path), "%s%d", devpath, index);
    if(rm_recursive(path) != 0 && errno != ENOENT){
        printf("Unable to delete %s: %s\n", path, strerror(errno));
        return -1;
    }
    if(mkdir(path, S_READDIR) != 0){
        rm_recursive(path);
        printf("Unable to create %s: %s\n", path, strerror(errno));
        return -1;
    }
    // Create FIFO for the LEDs
    char ledpath[sizeof(path) + 4];
    snprintf(ledpath, sizeof(ledpath), "%s/led", path);
    if(mkfifo(ledpath, S_READWRITE) != 0 || (kb->ledfifo = open(ledpath, O_RDONLY | O_NONBLOCK)) <= 0){
        rm_recursive(path);
        printf("Unable to create %s: %s\n", ledpath, strerror(errno));
        return -1;
    }
    if(kb->model == -1){
        // Root keyboard: write a list of devices
        updateconnected();
    } else {
        // Write the model and serial to files (doesn't apply to root keyboard)
        char mpath[sizeof(path) + 6], spath[sizeof(path) + 7];
        snprintf(mpath, sizeof(mpath), "%s/model", path);
        snprintf(spath, sizeof(spath), "%s/serial", path);
        FILE* mfile = fopen(mpath, "w");
        if(mfile){
            fputs(kb->name, mfile);
            fputc('\n', mfile);
            fclose(mfile);
            chmod(mpath, S_READ);
        } else {
            printf("Warning: Unable to create %s: %s\n", mpath, strerror(errno));
        }
        FILE* sfile = fopen(spath, "w");
        if(sfile){
            fputs(kb->serial, sfile);
            fputc('\n', sfile);
            fclose(sfile);
            chmod(spath, S_READ);
        } else {
            printf("Warning: Unable to create %s: %s\n", spath, strerror(errno));
        }
    }
    return 0;
}

#define V_CORSAIR   0x1b1c
#define P_K70       0x1b13
#define P_K95       0x1b11

int usbcmp(libusb_device* dev1, libusb_device* dev2){
    uint8_t num1[7], num2[7];
    int len1, len2;
    if(!dev1 || !dev2 || (len1 = libusb_get_port_numbers(dev1, num1, 7)) <= 0 || (len2 = libusb_get_port_numbers(dev2, num2, 7)) <= 0)
        return -1;
    if(len1 != len2)
        return -2;
    for(int i = 0; i < len1; i++){
        if(num1[i] != num2[i])
            return 1;
    }
    return 0;
}

int openusb(libusb_device* device){
    // Get info and check the manufacturer/product ID
    struct libusb_device_descriptor descriptor;
    if(libusb_get_device_descriptor(device, &descriptor))
        return -1;
    int model;
    if(descriptor.idVendor == V_CORSAIR){
        if(descriptor.idProduct == P_K70){
            model = 70;
            printf("Detected K70 keyboard\n");
        } else if(descriptor.idProduct == P_K95){
            model = 95;
            printf("Detected K95 keyboard\n");
        } else
            return 0;
    } else
        return 0;
    // Make sure it's not connected yet
    for(int i = 1; i < DEV_MAX; i++){
        if(!usbcmp(keyboard[i].dev, device)){
            printf("Already connected\n");
            return 0;
        }
    }
    // Find a free USB slot
    for(int i = 1; i < DEV_MAX; i++){
        usbdevice* kb = keyboard + i;
        if(!kb->handle){
            // Open device
            memcpy(&kb->descriptor, &descriptor, sizeof(descriptor));
            kb->dev = device;
            kb->model = model;
            if(libusb_open(device, &kb->handle)){
                printf("Failed to open USB device\n");
                kb->dev = 0;
                return -1;
            }
            libusb_set_auto_detach_kernel_driver(kb->handle, 1);
            if(libusb_claim_interface(kb->handle, 3)){
                printf("Failed to claim interface\n");
                libusb_release_interface(kb->handle, 3);
                kb->dev = 0;
                kb->handle = 0;
                return -1;
            }
            if(libusb_get_string_descriptor_ascii(kb->handle, descriptor.iProduct, (unsigned char*)kb->name, NAME_LEN) <= 0
                    || libusb_get_string_descriptor_ascii(kb->handle, descriptor.iSerialNumber, (unsigned char*)kb->serial, SERIAL_LEN) <= 0){
                printf("Failed to get device info\n");
                libusb_release_interface(kb->handle, 3);
                libusb_close(kb->handle);
                kb->dev = 0;
                kb->handle = 0;
                return -1;
            }
            printf("Connecting %s (S/N: %s)\n", kb->name, kb->serial);
            // Make /dev path
            if(makedevpath(i)){
                libusb_release_interface(kb->handle, 3);
                libusb_close(kb->handle);
                kb->dev = 0;
                kb->handle = 0;
                return -1;
            }
            printf("Device ready at %s%d\n", devpath, i);

            // Create the USB queue
            for(int q = 0; q < QUEUE_LEN; q++)
                kb->queue[q] = malloc(MSG_SIZE);

            // The default behavior is to highlight the M-buttons (K95) as well as Win Lock with an inverted color when active.
            // Additionally, pressing an M-button will switch to a different memory state. This message will remove both behaviors,
            // at the cost of the buttons causing the keyboard to freeze instead of switch mode. Need more research here...
            char datapkt[64] = { 0x07, 0x04, 0x02, 0 };
            usbqueue(kb, datapkt, 1);

            // Restore profile (if any)
            usbstore* store = findstore(kb->serial);
            if(store){
                kb->rgb = store->rgb;
                if((kb->rgbon = store->rgbon))
                    updateleds(kb, kb->rgb);
                else
                    updateleds(kb, 0);
            } else {
                // If no profile, set all LEDs to white
                // TODO: Load factory calibration instead
                initrgb(&kb->rgb, &kb->rgbon);
                updateleds(kb, kb->rgb);
            }
            updateconnected();

            return 0;
        }
    }
    // No free devices
    printf("Can't connect USB device: No free entries\n");
    return -1;
}

int closeusb(int index){
    // Close file handles
    usbdevice* kb = keyboard + index;
    if(!kb->ledfifo)
        return 0;
    close(kb->ledfifo);
    kb->ledfifo = 0;
    if(kb->handle){
        printf("Disconnecting %s (S/N: %s)\n", kb->name, kb->serial);
        // Delete USB queue
        for(int i = 0; i < QUEUE_LEN; i++)
            free(kb->queue[i]);
        // Move the RGB data into the device store
        usbstore* store = addstore(kb->serial);
        store->rgb = kb->rgb;
        store->rgbon = kb->rgbon;
        // Close USB device
        libusb_release_interface(kb->handle, 3);
        libusb_close(kb->handle);
        kb->handle = 0;
        kb->dev = 0;
        updateconnected();
    }
    // Delete the control path
    char path[strlen(devpath) + 2];
    snprintf(path, sizeof(path), "%s%d", devpath, index);
    if(rm_recursive(path) != 0 && errno != ENOENT)
        printf("Unable to delete %s: %s\n", path, strerror(errno));
    else
        printf("Removed device path %s\n", path);
    memset(kb, 0, sizeof(*kb));
    return 0;
}

int usbhotplug(struct libusb_context* ctx, struct libusb_device* device, libusb_hotplug_event event, void* user_data){
    if(event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED){
        // Device connected: parse device
        return openusb(device);
    } else if(event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT){
        // Device disconnected: look for it in the device list
        for(int i = 1; i < DEV_MAX; i++){
            if(!usbcmp(keyboard[i].dev, device))
                return closeusb(i);
        }
    }
    return 0;
}

#define MAX_WORDS 3
#define MAKERGB(r, g, b) (((((r) > 255 ? 255 : (r)) >> 5) << 6) | ((((g) > 255 ? 255 : (g)) >> 5) << 3) | (((b) > 255 ? 255 : (b)) >> 5))

void readled(usbdevice* kb, char* line){
    short* colors = kb->rgb;
    int* enable = &kb->rgbon;

    // Split the message up into words
    char* word[MAX_WORDS] = { line };
    int wpos = 0;
    for(int i = 1; i < MAX_WORDS; i++){
        for(; line[wpos] != '\0'; wpos++){
            if(line[wpos] == ' '){
                line[wpos] = '\0';
                wpos++;
                break;
            }
        }
        word[i] = line + wpos;
    }
    // Remove the newline at the end (or cut the string after the third word)
    line[wpos - 1] = '\0';
    // See if there's a serial number here. If so, switch keyboards.
    if(strlen(word[0]) == SERIAL_LEN - 1){
        int found = 0;
        for(int i = 1; i < DEV_MAX; i++){
            if(keyboard[i].ledfifo && !strcmp(word[0], keyboard[i].serial)){
                kb = keyboard + i;
                found = 1;
                initrgb(&kb->rgb, &kb->rgbon);
                colors = kb->rgb;
                enable = &kb->rgbon;
                break;
            }
        }
        // If it wasn't found in the device list, put it in the store.
        if(!found){
            usbstore* stored = addstore(word[0]);
            initrgb(&stored->rgb, &stored->rgbon);
            colors = stored->rgb;
            enable = &stored->rgbon;
            kb = 0;
        }
        // Parse the rest of the message
        for(int i = 0; i < MAX_WORDS - 1; i++)
            word[i] = word[i + 1];
    } else if(kb->model == -1)
        // Root controller *must* be given a serial number
        return;
    // Parse commands
    if(!strcmp(word[0], "off") && word[1][0] == '\0'){
        // Turn all keys off. Set LEDs to black.
        *enable = 0;
        updateleds(kb, 0);
    } else if(!strcmp(word[0], "on") && word[1][0] == '\0'){
        // Turn all keys on. Restore previous color settings.
        *enable = 1;
        updateleds(kb, colors);
    } else if(!strcmp(word[0], "rgb")){
        // Set the RGB color of key(s).
        *enable = 1;
        unsigned int r, g, b;
        if(sscanf(word[1], "%2x%2x%2x", &r, &g, &b) == 3){
            // Single color for all keys
            short rgb = MAKERGB(r, g, b);
            for(int i = 0; i < N_KEYS; i++)
                colors[i] = rgb;
        } else {
            // Scan a list of key/color pairs
            int length = strlen(word[1]);
            int position = 0, fieldleft = 0, field = 0;
            char* keynames = malloc(length);
            while(position < length && sscanf(word[1] + position, "%[^:]%n:%2x%2x%2x%n", keynames, &fieldleft, &r, &g, &b, &field) == 4){
                short rgb = MAKERGB(r, g, b);
                // Scan keys
                fieldleft += position;
                int position2 = position, field2 = 0;
                char keyname[11];
                while(position2 < fieldleft && sscanf(word[1] + position2, "%10[^:,]%n", keyname, &field2) == 1){
                    int keycode;
                    if(!strcmp(keyname, "all")){
                        // Set all keys
                        for(int i = 0; i < N_KEYS; i++)
                            colors[i] = rgb;
                    } else if(sscanf(keyname, "#%d", &keycode) && keycode >= 0 && keycode < N_KEYS){
                        // Set a key numerically
                        colors[keycode] = rgb;
                    } else {
                        // Find this key in the keymap
                        for(unsigned i = 0; i < N_KEYMAP; i++){
                            if(!strcmp(keyname, keymap[i].name)){
                                colors[keymap[i].code] = rgb;
                                break;
                            }
                        }
                    }
                    if(word[1][position2 += field2] == ',')
                        position2++;
                }
                if(word[1][position += field] == ',')
                    position++;
            }
            free(keynames);
        }
        updateleds(kb, colors);
    }
}

// Custom readline is needed for FIFOs. fopen()/getline() will die if the data is sent in too fast.
#define MAX_LINES 512
int readlines(int fd, char*** lines){
    int buffersize = 4095;
    char* buffer = malloc(buffersize + 1);
    ssize_t length = read(fd, buffer, buffersize);
    if(length <= 0){
        free(buffer);
        *lines = 0;
        return 0;
    }
    // Continue buffering until all available input is read
    while(length == buffersize){
        int oldsize = buffersize;
        buffersize += 4096;
        buffer = realloc(buffer, buffersize + 1);
        ssize_t length2 = read(fd, buffer + oldsize, buffersize - oldsize);
        if(length2 <= 0)
            break;
        length += length2;
    }
    buffer[length] = 0;
    // Break the input into lines
    char** linebuffer = malloc(MAX_LINES * sizeof(char*));
    char* line = buffer;
    int nlines = 0;
    while(1){
        char* nextline = strchr(line, '\n');
        if(!nextline || nlines == MAX_LINES - 1){
            int linesize = length - (line - buffer) + 1;
            char* output = malloc(linesize);
            memcpy(output, line, linesize);
            linebuffer[nlines++] = output;
            break;
        }
        // Include the \n in the output
        nextline++;
        int linesize = nextline - line;
        char* output = malloc(linesize + 1);
        memcpy(output, line, linesize);
        output[linesize] = 0;
        linebuffer[nlines++] = output;
        line = nextline;
    }
    // Clean up
    free(buffer);
    *lines = linebuffer;
    return nlines;
}

void quit(){
    for(int i = 1; i < DEV_MAX; i++)
        closeusb(i);
    closeusb(0);
    libusb_exit(0);
}

void sighandler(int type){
    printf("Caught signal %d\n", type);
    quit();
    exit(0);
}

int main(void){
    printf("ckb Corsair Keyboard RGB driver v0.1\n");

    // Start libusb
    libusb_init(0);
    libusb_device** devices = 0;
    if(libusb_init(0)){
        printf("Failed to initialize libusb\n");
        return -1;
    }
    libusb_set_debug(0, LIBUSB_LOG_LEVEL_INFO);
    // Make root keyboard
    umask(0);
    memset(keyboard, 0, sizeof(keyboard));
    keyboard[0].model = -1;
    if(!makedevpath(0))
        printf("Root controller ready at %s0\n", devpath);
    // Enumerate connected devices
    if(libusb_get_device_list(0, &devices) > 0){
        for(libusb_device** dev = devices; *dev != 0; dev++)
            openusb(*dev);
        libusb_free_device_list(devices, 1);
    } else
        printf("Failed to scan USB devices\n");
    // Set hotplug callback
    libusb_hotplug_callback_handle hphandle;
    if(libusb_hotplug_register_callback(0, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, 0, V_CORSAIR, LIBUSB_HOTPLUG_MATCH_ANY, LIBUSB_HOTPLUG_MATCH_ANY, usbhotplug, 0, &hphandle) != LIBUSB_SUCCESS)
        printf("Failed to activate hot plug callback\n");

    // Set up signal handlers for quitting the service.
    signal(SIGTERM, sighandler);
    signal(SIGINT, sighandler);
    signal(SIGQUIT, sighandler);

    while(1){
        // Run hotplug callback
        struct timeval tv = { 0 };
        libusb_handle_events_timeout_completed(0, &tv, 0);
        // Process FIFOs
        for(int i = 0; i < DEV_MAX; i++){
            if(keyboard[i].ledfifo){
                char** lines;
                int nlines = readlines(keyboard[i].ledfifo, &lines);
                for(int j = 0; j < nlines; j++){
                    if(strlen(lines[j]) > 1)
                        readled(keyboard + i, lines[j]);
                    free(lines[j]);
                }
                free(lines);
            }
        }
        // Run the USB queue. Messages must be queued because sending multiple messages at the same time can cause the interface to freeze
        for(int i = 1; i < DEV_MAX; i++){
            if(keyboard[i].handle)
                usbdequeue(keyboard + i);
        }
        // Sleep for 3ms. This delay seems to be enough to prevent the device from stopping and achieves a throughput of 60FPS.
        usleep(3333);
    }
    quit();
    return 0;
}

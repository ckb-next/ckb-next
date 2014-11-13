#include "device.h"
#include "devnode.h"
#include "input.h"
#include "led.h"
#include "notify.h"

// OSX doesn't like putting FIFOs in /dev for some reason
#ifndef OS_MAC
const char *const devpath = "/dev/input/ckb";
#else
const char *const devpath = "/tmp/ckb";
#endif

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
        if(IS_ACTIVE(keyboard + i)){
            written = 1;
            fprintf(cfile, "%s%d %s %s\n", devpath, i, keyboard[i].setting.serial, keyboard[i].name);
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
        printf("Error: Unable to delete %s: %s\n", path, strerror(errno));
        return -1;
    }
    if(mkdir(path, S_READDIR) != 0){
        printf("Error: Unable to create %s: %s\n", path, strerror(errno));
        rm_recursive(path);
        return -1;
    }
    // Create command FIFO
    char inpath[sizeof(path) + 4];
    snprintf(inpath, sizeof(inpath), "%s/cmd", path);
    if(mkfifo(inpath, S_READWRITE) != 0 || (kb->infifo = open(inpath, O_RDONLY | O_NONBLOCK)) <= 0){
        printf("Error: Unable to create %s: %s\n", inpath, strerror(errno));
        rm_recursive(path);
        kb->infifo = 0;
        return -1;
    }
    // Create notification FIFO
    mknotifynode(kb, 0);
    if(kb->model == -1){
        // Root keyboard: write a list of devices
        updateconnected();
    } else {
        // Write the model and serial to files
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
            fputs(kb->setting.serial, sfile);
            fputc('\n', sfile);
            fclose(sfile);
            chmod(spath, S_READ);
        } else {
            printf("Warning: Unable to create %s: %s\n", spath, strerror(errno));
        }
    }
    return 0;
}

int rmdevpath(int index){
    usbdevice* kb = keyboard + index;
    close(kb->infifo);
    kb->infifo = 0;
    for(int i = 0; i < OUTFIFO_MAX; i++)
        rmnotifynode(kb, i);
    char path[strlen(devpath) + 2];
    snprintf(path, sizeof(path), "%s%d", devpath, index);
    if(rm_recursive(path) != 0 && errno != ENOENT){
        printf("Unable to delete %s: %s\n", path, strerror(errno));
        return -1;
    }
    printf("Removed device path %s\n", path);
    return 0;
}


int mknotifynode(usbdevice* kb, int notify){
    if(notify < 0 || notify >= OUTFIFO_MAX)
        return -1;
    // Create the notification node
    int index = INDEX_OF(kb, keyboard);
    char outpath[strlen(devpath) + 10];
    snprintf(outpath, sizeof(outpath), "%s%d/notify%d", devpath, index, notify);
    if(mkfifo(outpath, S_READWRITE) != 0 || (kb->outfifo[notify] = open(outpath, O_RDWR | O_NONBLOCK)) <= 0){
        printf("Warning: Unable to create %s: %s\n", outpath, strerror(errno));
        kb->outfifo[notify] = 0;
        return -1;
    }
    return 0;
}

int rmnotifynode(usbdevice* kb, int notify){
    if(notify < 0 || notify >= OUTFIFO_MAX)
        return -1;
    int index = INDEX_OF(kb, keyboard);
    char outpath[strlen(devpath) + 10];
    snprintf(outpath, sizeof(outpath), "%s%d/notify%d", devpath, index, notify);
    // Close FIFO
    close(kb->outfifo[notify]);
    kb->outfifo[notify] = 0;
    // Delete node
    return remove(outpath);
}

#define MAX_LINES 512
#define MAX_BUFFER (16 * 1024 - 1)
int readlines(int fd, const char*** lines){
    // Allocate static buffers to store data
    static int buffersize = 4095;
    static int leftover = 0, leftoverlen = 0;
    static char* buffer = 0;
    static const char** linebuffer = 0;
    if(!buffer){
        buffer = malloc(buffersize + 1);
        linebuffer = malloc(MAX_LINES * sizeof(const char**));
    }
    // Move any data left over from a previous read to the start of the buffer
    if(leftover)
        memcpy(buffer, buffer + leftover, leftoverlen);
    ssize_t length = read(fd, buffer + leftoverlen, buffersize - leftoverlen);
    length = (length < 0 ? 0 : length) + leftoverlen;
    leftover = leftoverlen = 0;
    if(length <= 0){
        *lines = 0;
        return 0;
    }
    // Continue buffering until all available input is read or there's no room left
    while(length == buffersize || buffersize >= MAX_BUFFER){
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
    char* line = buffer;
    int nlines = 0;
    while(1){
        char* nextline = memchr(line, '\n', buffer + length - line);
        if(!nextline || nlines == MAX_LINES - 1){
            // Process any left over bytes next time
            leftover = line - buffer;
            leftoverlen = length - leftover;
            break;
        }
        // Replace the \n with \0 and insert the line into the line buffer
        *nextline = 0;
        linebuffer[nlines++] = line;
        line = ++nextline;
        if(line == buffer + length)
            break;
    }
    *lines = linebuffer;
    return nlines;
}

void readcmd(usbdevice* kb, const char* line){
    char word[strlen(line) + 1];
    int wordlen;
    // See if the first word is a serial number. If so, switch devices and skip to the next word.
    usbsetting* set = (IS_ACTIVE(kb) ? &kb->setting : 0);
    usbprofile* profile = (set ? &set->profile : 0);
    usbmode* mode = (profile ? profile->currentmode : 0);
    cmd command = NONE;
    cmdhandler handler = 0;
    int rgbchange = 0;
    // Read words from the input
    while(sscanf(line, "%s%n", word, &wordlen) == 1){
        line += wordlen;
        // Check for a command word
        if(!strcmp(word, "device")){
            command = DEVICE;
            handler = 0;
            continue;
        } else if(!strcmp(word, "mode")){
            command = MODE;
            handler = 0;
            continue;
        } else if(!strcmp(word, "switch")){
            command = NONE;
            handler = 0;
            if(profile)
                profile->currentmode = mode;
            rgbchange = 1;
            continue;
        } else if(!strcmp(word, "hwload")){
            command = NONE;
            handler = 0;
            if(profile)
                hwloadprofile(kb);
            rgbchange = 1;
        } else if(!strcmp(word, "hwsave")){
            command = NONE;
            handler = 0;
            if(profile)
                hwsaveprofile(kb);
        } else if(!strcmp(word, "erase")){
            command = NONE;
            handler = 0;
            if(mode)
                erasemode(mode);
            rgbchange = 1;
            continue;
        } else if(!strcmp(word, "eraseprofile")){
            command = NONE;
            handler = 0;
            if(profile){
                eraseprofile(profile);
                mode = profile->currentmode = getusbmode(0, profile);
            }
            rgbchange = 1;
            continue;
        } else if(!strcmp(word, "name")){
            command = NAME;
            handler = cmd_setmodename;
            if(mode)
                updatemod(&mode->id);
            continue;
        } else if(!strcmp(word, "profilename")){
            command = PROFILENAME;
            handler = 0;
            if(profile)
                updatemod(&profile->id);
            continue;
        } else if(!strcmp(word, "bind")){
            command = BIND;
            handler = cmd_bind;
            continue;
        } else if(!strcmp(word, "unbind")){
            command = UNBIND;
            handler = cmd_unbind;
            continue;
        } else if(!strcmp(word, "rebind")){
            command = REBIND;
            handler = cmd_rebind;
            continue;
        } else if(!strcmp(word, "macro")){
            command = MACRO;
            handler = 0;
            continue;
        } else if(!strcmp(word, "rgb")){
            command = RGB;
            handler = cmd_rgb;
            rgbchange = 1;
            if(mode)
                updatemod(&mode->id);
            continue;
        } else if(!strcmp(word, "ioff")){
            command = IOFF;
            handler = cmd_ioff;
            continue;
        } else if(!strcmp(word, "ion")){
            command = ION;
            handler = cmd_ion;
            continue;
        } else if(!strcmp(word, "iauto")){
            command = IAUTO;
            handler = cmd_iauto;
            continue;
        } else if(!strcmp(word, "notify")){
            command = NOTIFY;
            handler = cmd_notify;
            continue;
        } else if(!strcmp(word, "notifyon")){
            command = NOTIFYON;
            handler = 0;
            continue;
        } else if(!strcmp(word, "notifyoff")){
            command = NOTIFYOFF;
            handler = 0;
            continue;
        }
        if(command == NONE)
            continue;
        else if(command == DEVICE){
            if(strlen(word) == SERIAL_LEN - 1){
                usbdevice* found = findusb(word);
                if(found){
                    kb = found;
                    set = &kb->setting;
                } else {
                    // If the device isn't plugged in, find (or add) it to storage
                    kb = 0;
                    set = addstore(word);
                }
                profile = (set ? &set->profile : 0);
                mode = (profile ? profile->currentmode : 0);
            }
            continue;
        } else if(command == NOTIFYON){
            int notify;
            if(kb && sscanf(word, "%u", &notify) == 1)
                mknotifynode(kb, notify);
            continue;
        } else if(command == NOTIFYOFF){
            int notify;
            if(kb && sscanf(word, "%u", &notify) == 1 && notify != 0)
                rmnotifynode(kb, notify);
            continue;
        }
        // Only the DEVICE or NOTIFYON/OFF commands are valid without an existing mode
        if(!mode)
            continue;
        if(command == MODE){
            int newmode;
            if(sscanf(word, "%u", &newmode) == 1 && newmode > 0 && newmode < MODE_MAX)
                mode = getusbmode(newmode - 1, profile);
            continue;
        } else if(command == NAME || command == IOFF || command == ION || command == IAUTO){
            // All of the above just parse the whole word
            handler(mode, 0, word);
            continue;
        } else if(command == PROFILENAME){
            // Profile name is the same, but takes a different parameter
            setprofilename(profile, word);
            continue;
        } else if(command == RGB){
            // RGB command has a special response for "on", "off", and a hex constant
            int r, g, b;
            if(!strcmp(word, "on")){
                cmd_rgbon(mode);
                continue;
            } else if(!strcmp(word, "off")){
                cmd_rgboff(mode);
                continue;
            } else if(sscanf(word, "%02x%02x%02x", &r, &g, &b) == 3){
                for(int i = 0; i < N_KEYS; i++)
                    cmd_rgb(mode, i, word);
                continue;
            }
        } else if(command == MACRO && !strcmp(word, "clear")){
            // Macro has a special clear command
            cmd_macroclear(mode);
            continue;
        }
        // Split the parameter at the colon
        int left = -1;
        sscanf(word, "%*[^:]%n", &left);
        if(left <= 0)
            continue;
        const char* right = word + left;
        if(right[0] == ':')
            right++;
        // Macros have a separate left-side handler
        if(command == MACRO){
            word[left] = 0;
            cmd_macro(mode, word, right);
            continue;
        }
        // Scan the left side for key names and run the request command
        int position = 0, field = 0;
        char keyname[11];
        while(position < left && sscanf(word + position, "%10[^:,]%n", keyname, &field) == 1){
            int keycode;
            if(!strcmp(keyname, "all")){
                // Set all keys
                for(int i = 0; i < N_KEYS; i++)
                    handler(mode, i, right);
            } else if((sscanf(keyname, "#%d", &keycode) && keycode >= 0 && keycode < N_KEYS)
                      || (sscanf(keyname, "#x%x", &keycode) && keycode >= 0 && keycode < N_KEYS)){
                // Set a key numerically
                handler(mode, keycode, right);
            } else {
                // Find this key in the keymap
                for(unsigned i = 0; i < N_KEYS; i++){
                    if(keymap[i].name && !strcmp(keyname, keymap[i].name)){
                        handler(mode, i, right);
                        break;
                    }
                }
            }
            if(word[position += field] == ',')
                position++;
        }
    }
    if(mode && rgbchange)
        updatergb(kb);
}

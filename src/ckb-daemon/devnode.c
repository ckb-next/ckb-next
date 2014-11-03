#include "devnode.h"
#include "usb.h"
#include "input.h"
#include "led.h"

const char *const devpath = "/dev/input/ckb";

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
    // Create command FIFO
    char fifopath[sizeof(path) + 4];
    snprintf(fifopath, sizeof(fifopath), "%s/cmd", path);
    if(mkfifo(fifopath, S_READWRITE) != 0 || (kb->fifo = open(fifopath, O_RDONLY | O_NONBLOCK)) <= 0){
        rm_recursive(path);
        printf("Unable to create %s: %s\n", fifopath, strerror(errno));
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

void readcmd(usbdevice* kb, const char* line){
    char word[strlen(line) + 1];
    int wordlen;
    // See if the first word is a serial number. If so, switch devices and skip to the next word.
    if(sscanf(line, "%s%n", word, &wordlen) == 1 && strlen(word) == SERIAL_LEN - 1){
        usbdevice* found = findusb(word);
        if(found)
            kb = found;
        line += wordlen;
        sscanf(line, "%s%n", word, &wordlen);
    }
    if(!kb->handle)
        return;
    cmd mode = NONE;
    cmdhandler handler = 0;
    int rgbchange = 0;
    // Read words from the input
    do {
        line += wordlen;
        // Check for a command word
        if(!strcmp(word, "bind")){
            mode = BIND;
            handler = cmd_bind;
            continue;
        } else if(!strcmp(word, "unbind")){
            mode = UNBIND;
            handler = cmd_unbind;
            continue;
        } else if(!strcmp(word, "reset")){
            mode = RESET;
            handler = cmd_reset;
            continue;
        }/* else if(!strcmp(word, "macro")){
            mode = MACRO;
            handler = cmd_macro;
            continue;
        }*/ else if(!strcmp(word, "rgb")){
            mode = RGB;
            handler = cmd_ledrgb;
            rgbchange = 1;
            continue;
        }
        if(mode == NONE)
            continue;
        else if(mode == RGB){
            // RGB command has a special response for "on", "off", and a hex constant
            int r, g, b;
            if(!strcmp(word, "on")){
                cmd_ledon(kb);
                continue;
            } else if(!strcmp(word, "off")){
                cmd_ledoff(kb);
                continue;
            } else if(sscanf(word, "%02x%02x%02x", &r, &g, &b) == 3){
                for(int i = 0; i < N_KEYS; i++)
                    cmd_ledrgb(kb, i, word);
                continue;
            }
        }
        // Split the parameter at the colon
        int left = -1;
        sscanf(word, "%*[^:]%n", &left);
        if(left <= 0)
            continue;
        const char* right = word + left;
        if(right[0] == ':')
            right++;
        // Scan the left side for key names and run the request command
        int position = 0, field = 0;
        char keyname[11];
        while(position < left && sscanf(word + position, "%10[^:,]%n", keyname, &field) == 1){
            int keycode;
            if(!strcmp(keyname, "all")){
                // Set all keys
                for(int i = 0; i < N_KEYS; i++)
                    handler(kb, i, right);
            } else if((sscanf(keyname, "#%d", &keycode) && keycode >= 0 && keycode < N_KEYS)
                      || (sscanf(keyname, "#x%x", &keycode) && keycode >= 0 && keycode < N_KEYS)){
                // Set a key numerically
                handler(kb, keycode, right);
            } else {
                // Find this key in the keymap
                for(unsigned i = 0; i < N_KEYS; i++){
                    if(keymap[i].name && !strcmp(keyname, keymap[i].name)){
                        handler(kb, i, right);
                        break;
                    }
                }
            }
            if(word[position += field] == ',')
                position++;
        }
    } while(sscanf(line, "%s%n", word, &wordlen) == 1);
    if(rgbchange)
        updateleds(kb, kb->rgbon ? kb->rgb : 0);
}

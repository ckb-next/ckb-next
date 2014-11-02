#include "devnode.h"
#include "usb.h"

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

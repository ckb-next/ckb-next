#include <ckb-next/animation.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

void ckb_info(){
    // Plugin info
    CKB_NAME("Pipe");
    CKB_VERSION("1.0");
    CKB_COPYRIGHT("2015-2019", "Tasos Sahanidis");
    CKB_LICENSE("GPLv2+");
    CKB_GUID("{64A824CD-3E12-4973-8F80-AA5E6DA807FA}");
    CKB_DESCRIPTION("Control lights using named pipes\nplaced in /tmp/.\nData format is RRGGBBAA.\nWARNING: It doesn't clean up the nodes.\nUsage Examples:\necho \"rgb ff00ffff\" > /tmp/ckbpipe000\necho \"rgb r:ff0000ff\" > /tmp/ckbpipe000\nEach rgb command must be terminated with a newline (\\n).");

    // Effect parameters
    CKB_PARAM_LONG("fifonum", "/tmp/ckbpipe", "", 0, 0, 200);

    // Timing/input parameters
    CKB_KPMODE(CKB_KP_POSITION);
    CKB_TIMEMODE(CKB_TIME_ABSOLUTE);
    CKB_LIVEPARAMS(FALSE);
    CKB_PREEMPT(TRUE);
    CKB_REPEAT(FALSE);

    // Presets
    CKB_PRESET_START("Pipe");
    CKB_PRESET_END;
}

long fifonum = 0;
int fd = -1;
#define FIFO_NAME_LEN 16
char fifoname[FIFO_NAME_LEN] = { 0 }; // /tmp/ckbpipeNNN
#define MAX_INPUT 4096

void ckb_init(ckb_runctx* context){

}

void ckb_parameter(ckb_runctx* context, const char* name, const char* value){
    CKB_PARSE_LONG("fifonum", &fifonum){
        sprintf(fifoname, "/tmp/ckbpipe%03hhu", (unsigned char)fifonum);
        unlink(fifoname);
        // Create named pipe in tmp
        mkfifo(fifoname, 0600);
        fd = open(fifoname, O_RDONLY | O_NONBLOCK);
    }
}

void ckb_keypress(ckb_runctx* context, ckb_key* key, int x, int y, int state){
}

void ckb_start(ckb_runctx* context, int state){
}

void ckb_time(ckb_runctx* context, double delta){
}

int ckb_frame(ckb_runctx* context){
    if(fd == -1)
        return 0;

    char input[MAX_INPUT];

    ssize_t ret = read(fd, input, MAX_INPUT - 1);

    // read() read 0 bytes
    if(ret < 1)
        return 0;

    // NULL terminate the string
    input[ret] = '\0';

    unsigned char r = 0, g = 0, b = 0, a = 0;

    // Start parsing loop
    char* strptr = input;
    while(strptr < input + ret){
        // Minimum length of a valid rgb command
        size_t len = strlen(strptr);
        if(len < 13)
            break;

        if(strncmp(strptr, "rgb ", 4))
            break;

        // Skip over the "rgb "
        strptr += 4;
        len -= 4;

        // Find the first newline and null terminate the string there
        char* strptrend;
        for(strptrend = strptr; strptrend < strptr + len; strptrend++){
            if(*strptrend == '\n'){
                *strptrend = '\0';
                break;
            }
        }

        // Update the length
        len = strptrend - strptr;

        // Try to parse
        // len > 8 means it's a per-key command
        if (len > 8) {
            while(strptr < strptrend) {
                char key_name[MAX_INPUT] = { 0 };
                int tmppos = -1;
                if(sscanf(strptr, "%[^:]:%02hhx%02hhx%02hhx%02hhx%n", key_name, &r, &g, &b, &a, &tmppos) != 5)
                    break;
                strptr += tmppos + 1;
                for(unsigned i = 0; i < context->keycount; i++){
                    ckb_key* key = context->keys + i;
                    if(strcmp(key_name, key->name))
                        continue;
                    key->a = a;
                    key->r = r;
                    key->g = g;
                    key->b = b;
                }
            }
        } else if (sscanf(strptr, "%02hhx%02hhx%02hhx%02hhx", &r, &g, &b, &a) == 4) {
            for(unsigned i = 0; i < context->keycount; i++){
                ckb_key* key = context->keys + i;
                key->a = a;
                key->r = r;
                key->g = g;
                key->b = b;
            }
        }

        // Move to the next command now
        // The while loop checks if strptr is valid
        strptr = strptrend + 1;
    }

    return 0;
}

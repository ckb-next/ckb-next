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
    CKB_DESCRIPTION("Control lights using named pipes\nplaced in /tmp/.\nData format is RRGGBBAA.\nWARNING: It doesn't clean up the nodes.\nUsage Examples:\necho \"rgb ff00ffff\" > /tmp/ckbpipe000\necho \"rgb r:ff0000ff\" > /tmp/ckbpipe000");

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
unsigned char a = 0, r = 0, g = 0, b = 0;
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

    char input[MAX_INPUT] = { 0 };

    ssize_t ret = read(fd, input, MAX_INPUT);

    // read() read 0 bytes
    if(!ret)
        return 0;
    
    // Something may have gone wrong
    if(ret == -1){
        // This is expected and should not be treated as failure
        if(errno == EWOULDBLOCK)
            return 0;

        int err = errno;
        fprintf(stderr, "read failed with %d (%s)\n", err, strerror(err));
        return -ret;
    }

    size_t len = strlen(input);
    unsigned char r = 0, g = 0, b = 0, a = 0;

    if(len < 8 + 4)
        return 0;

    if(strncmp(input, "rgb ", 4))
        return 0;

    char* strptr = input + 4;

    if(sscanf(strptr, "%02hhx%02hhx%02hhx%02hhx", &r, &g, &b, &a) == 4) {
        for(unsigned i = 0; i < context->keycount; i++){
            ckb_key* key = context->keys + i;
            key->a = a;
            key->r = r;
            key->g = g;
            key->b = b;
        }
    } else if (len > 8 + 4) {
        int pos = 0;
        while(pos < (long int)strlen(strptr)) {
            char key_name[MAX_INPUT] = { 0 };
            int tmppos = -1;
            if(sscanf(strptr + pos, "%[^:]:%02hhx%02hhx%02hhx%02hhx%n", key_name, &r, &g, &b, &a, &tmppos) != 5)
                break;
            pos += tmppos + 1;
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
    }

    return 0;
}

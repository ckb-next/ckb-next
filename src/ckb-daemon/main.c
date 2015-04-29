#include "device.h"
#include "devnode.h"
#include "input.h"
#include "led.h"
#include "notify.h"

extern int features_mask;

// Timespec utility function
void timespec_add(struct timespec* timespec, long nanoseconds){
    nanoseconds += timespec->tv_nsec;
    timespec->tv_sec += nanoseconds / 1000000000;
    timespec->tv_nsec = nanoseconds % 1000000000;
}

// Not supported on OSX...
#ifdef OS_MAC
#define pthread_mutex_timedlock(mutex, timespec) pthread_mutex_trylock(mutex)
#endif

void quit(){
    // Wait at most 1s for mutex locks. Better to crash than to freeze shutting down.
    struct timespec timeout = { 1, 0 };
    pthread_mutex_timedlock(&kblistmutex, &timeout);
    for(int i = 1; i < DEV_MAX; i++){
        // Before closing, set all keyboards back to HID input mode so that the stock driver can still talk to them
        if(IS_CONNECTED(keyboard + i)){
            pthread_mutex_timedlock(&keyboard[i].mutex, &timeout);
            // Stop the uinput device now to ensure no keys get stuck
            inputclose(keyboard + i);
            revertusb(keyboard + i);
            closeusb(keyboard + i);
        }
    }
    rmdevpath(keyboard);
    usbkill();
    pthread_mutex_unlock(&kblistmutex);
}

void sighandler2(int type){
    printf("\nIgnoring signal %d (already shutting down)\n", type);
}

void sighandler(int type){
    signal(SIGTERM, sighandler2);
    signal(SIGINT, sighandler2);
    signal(SIGQUIT, sighandler2);
    printf("\nCaught signal %d\n", type);
    quit();
    exit(0);
}

void localecase(char* dst, size_t length, const char* src){
    char* ldst = dst + length;
    char s;
    while((s = *src++)){
        if(s == '_')
            s = '-';
        else
            s = tolower(s);
        *dst++ = s;
        if(dst == ldst){
            dst--;
            break;
        }
    }
    *dst = 0;
}

int main(int argc, char** argv){
    printf("ckb Corsair Keyboard RGB driver %s\n", CKB_VERSION_STR);

    // Check PID, quit if already running
    char pidpath[strlen(devpath) + 6];
    snprintf(pidpath, sizeof(pidpath), "%s0/pid", devpath);
    FILE* pidfile = fopen(pidpath, "r");
    if(pidfile){
        pid_t pid;
        fscanf(pidfile, "%d", &pid);
        fclose(pidfile);
        if(pid > 0){
            // kill -s 0 checks if the PID is active but doesn't send a signal
            if(!kill(pid, 0)){
                printf("ckb-daemon is already running (PID %d). Try killing the existing process first.\n(If this is an error, delete %s and try again)\n", pid, pidpath);
                return 0;
            }
        }
    }

    // Read parameters
    int forceroot = 1;
    for(int i = 1; i < argc; i++){
        char* argument = argv[i];
        char layout[10];
        unsigned newgid;
        if(sscanf(argument, "--gid=%u", &newgid) == 1){
            // Set dev node GID
            gid = newgid;
            printf("Setting /dev node gid: %u\n", newgid);
        } else if(!strcmp(argument, "--nobind")){
            // Disable key notifications and rebinding
            features_mask &= ~FEAT_BIND & ~FEAT_NOTIFY;
            printf("Key binding and key notifications are disabled\n");
        } else if(!strcmp(argument, "--nonotify")){
            // Disable key notifications
            features_mask &= ~FEAT_NOTIFY;
            printf("Key notifications are disabled\n");
        } else if(!strcmp(argument, "--nonroot")){
            // Allow running as a non-root user
            forceroot = 0;
        }
    }

    // Check UID
    if(getuid() != 0){
        if(forceroot){
            printf("Fatal: ckb-daemon must be run as root. Try `sudo %s`\n", argv[0]);
            exit(0);
        } else
            printf("Warning: not running as root, allowing anyway per command-line parameter...\n");
    }

    // Make root keyboard
    umask(0);
    memset(keyboard, 0, sizeof(keyboard));
    if(!makedevpath(keyboard))
        printf("Root controller ready at %s0\n", devpath);

    // Set signals
    sigset_t signals;
    sigfillset(&signals);
    sigdelset(&signals, SIGTERM);
    sigdelset(&signals, SIGINT);
    sigdelset(&signals, SIGQUIT);
    // Set up signal handlers for quitting the service.
    pthread_sigmask(SIG_SETMASK, &signals, 0);
    signal(SIGTERM, sighandler);
    signal(SIGINT, sighandler);
    signal(SIGQUIT, sighandler);

    // Start the USB system
    int result = usbmain();
    quit();
    return result;
}

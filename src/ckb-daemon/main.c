#include "device.h"
#include "devnode.h"
#include "input.h"
#include "led.h"
#include "notify.h"

// usb.c
extern volatile int reset_stop;
extern int features_mask;

// Timespec utility function
void timespec_add(struct timespec* timespec, long nanoseconds){
    nanoseconds += timespec->tv_nsec;
    timespec->tv_sec += nanoseconds / 1000000000;
    timespec->tv_nsec = nanoseconds % 1000000000;
}

void quit(){
    // Abort any USB resets in progress
    reset_stop = 1;
    for(int i = 1; i < DEV_MAX; i++){
        // Before closing, set all keyboards back to HID input mode so that the stock driver can still talk to them
        pthread_mutex_lock(devmutex + i);
        if(IS_CONNECTED(keyboard + i)){
            revertusb(keyboard + i);
            closeusb(keyboard + i);
        }
        pthread_mutex_unlock(devmutex + i);
    }
    rmdevpath(keyboard);
    usbkill();
}

void sighandler2(int type){
    // Don't use ckb_warn, we want an extra \n at the beginning
    printf("\n[W] Ignoring signal %d (already shutting down)\n", type);
}

void sighandler(int type){
    signal(SIGTERM, sighandler2);
    signal(SIGINT, sighandler2);
    signal(SIGQUIT, sighandler2);
    printf("\n[I] Caught signal %d\n", type);
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
    printf("    ckb: Corsair RGB driver %s\n", CKB_VERSION_STR);

    // If --help occurs anywhere in the command-line, don't launch the program but instead print usage
    for(int i = 1; i < argc; i++){
        if(!strcmp(argv[i], "--help")){
            printf(
#ifdef OS_MAC
                        "Usage: ckb-daemon [--gid=<gid>] [--nonotify] [--nobind] [--nomouseaccel] [--nonroot]\n"
#else
                        "Usage: ckb-daemon [--gid=<gid>] [--nonotify] [--nobind] [--nonroot]\n"
#endif
                        "\n"
                        "See https://github.com/ccMSC/ckb/blob/master/DAEMON.md for full instructions.\n"
                        "\n"
                        "Command-line parameters:\n"
                        "    --gid=<gid>\n"
                        "        Restrict access to %s* nodes to users in group <gid>.\n"
                        "        (Ordinarily they are accessible to anyone)\n"
                        "    --nonotify\n"
                        "        Disables key monitoring/notifications.\n"
                        "        Note that this makes reactive lighting impossible.\n"
                        "    --nobind\n"
                        "        Disables all key rebinding, macros, and notifications. Implies --nonotify.\n"
#ifdef OS_MAC
                        "    --nomouseaccel\n"
                        "        Completely disables mouse acceleration, even if the system preferences enable it.\n"
#endif
                        "    --nonroot\n"
                        "        Allows running ckb-daemon as a non root user.\n"
                        "        This will almost certainly not work. Use only if you know what you're doing.\n"
                        "\n", devpath);
            exit(0);
        }
    }

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
                ckb_fatal_nofile("ckb-daemon is already running (PID %d). Try `killall ckb-daemon`.\n", pid);
                ckb_fatal_nofile("(If you're certain the process is dead, delete %s and try again)\n", pidpath);
                return 0;
            }
        }
    }

    // Read parameters
    int forceroot = 1;
    for(int i = 1; i < argc; i++){
        char* argument = argv[i];
        unsigned newgid;
        if(sscanf(argument, "--gid=%u", &newgid) == 1){
            // Set dev node GID
            gid = newgid;
            ckb_info_nofile("Setting /dev node gid: %u\n", newgid);
        } else if(!strcmp(argument, "--nobind")){
            // Disable key notifications and rebinding
            features_mask &= ~FEAT_BIND & ~FEAT_NOTIFY;
            ckb_info_nofile("Key binding and key notifications are disabled\n");
        } else if(!strcmp(argument, "--nonotify")){
            // Disable key notifications
            features_mask &= ~FEAT_NOTIFY;
            ckb_info_nofile("Key notifications are disabled\n");
        } else if(!strcmp(argument, "--nonroot")){
            // Allow running as a non-root user
            forceroot = 0;
        }
#ifdef OS_MAC
        else if(!strcmp(argument, "--nomouseaccel")){
            // On OSX, provide an option to disable mouse acceleration
            features_mask &= ~FEAT_MOUSEACCEL;
            ckb_info_nofile("Mouse acceleration disabled\n");
        }
#endif
    }

    // Check UID
    if(getuid() != 0){
        if(forceroot){
            ckb_fatal_nofile("ckb-daemon must be run as root. Try `sudo %s`\n", argv[0]);
            exit(0);
        } else
            ckb_warn_nofile("Warning: not running as root, allowing anyway per command-line parameter...\n");
    }

    // Make root keyboard
    umask(0);
    memset(keyboard, 0, sizeof(keyboard));
    if(!mkdevpath(keyboard))
        ckb_info("Root controller ready at %s0\n", devpath);

    // Set signals
    sigset_t signals;
    sigfillset(&signals);
    sigdelset(&signals, SIGTERM);
    sigdelset(&signals, SIGINT);
    sigdelset(&signals, SIGQUIT);
    // Set up signal handlers for quitting the service.
    sigprocmask(SIG_SETMASK, &signals, 0);
    signal(SIGTERM, sighandler);
    signal(SIGINT, sighandler);
    signal(SIGQUIT, sighandler);

    // Start the USB system
    int result = usbmain();
    quit();
    return result;
}

#include "device.h"
#include "devnode.h"
#include "input.h"
#include "led.h"
#include "notify.h"
#include "request_hid_mac.h"
#include <ckbnextconfig.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "keymap_patch.h"

// usb.c
extern _Atomic int reset_stop;
extern int features_mask;
extern int enable_experimental;
// device.c
extern int hwload_mode;

int sighandler_pipe[2] = { 0, 0 };

// Timespec utility function
void timespec_add(struct timespec* timespec, int64_t nanoseconds){
    nanoseconds += timespec->tv_nsec;
    timespec->tv_sec += nanoseconds / 1000000000;
    timespec->tv_nsec = nanoseconds % 1000000000;
}

///
/// \brief quit
/// Stop working the daemon.
/// function is called if the daemon received a sigterm
/// In this case, locking the device-mutex is ok.
static void quit() {
    // Abort any USB resets in progress
    reset_stop = 1;
    for(int i = 1; i < DEV_MAX; i++){
        // Before closing, set all keyboards back to HID input mode so that the stock driver can still talk to them
        queued_mutex_lock(devmutex + i);
        if(IS_CONNECTED(keyboard + i)){
            revertusb(keyboard + i);
            closeusb(keyboard + i);
        }
        queued_mutex_unlock(devmutex + i);
    }
    ckb_info("Closing root controller");
    rmdevpath(keyboard);
    usbkill();
}

static inline void safe_write(const char* const str) {
    int unused_result = write(1, str, strlen(str));
    // cast unused result to void to silence over-eager
    // warning about unused variables:
    // https://sourceware.org/bugzilla/show_bug.cgi?id=11959
    (void) unused_result;
}

///
/// \brief ignore_signal
/// Nested signal handler for previously received signals.
/// \param type received signal type
void ignore_signal(int type){
    // Use signal-safe(7) write(3) call to print warning
    safe_write("\n[W] Ignoring signal ");
    switch (type) {
        case SIGTERM:
            safe_write("SIGTERM");
            break;
        case SIGINT:
            safe_write("SIGINT");
            break;
        case SIGQUIT:
            safe_write("SIGQUIT");
            break;
        default:
            safe_write("UNKNOWN");
            break;
    }
    safe_write(" (already shutting down)\n");
}

///
/// \brief exithandler
/// Main signal handler to catch further signals and call shutdown
/// sequence of daemon. This function is allowed to call unsafe
/// (signal-safe(7)) function calls, as it is itself executed in another
/// process via the socket handler.
/// \param type received signal type
void exithandler(int type){
    signal(SIGTERM, ignore_signal);
    signal(SIGINT, ignore_signal);
    signal(SIGQUIT, ignore_signal);

    printf("\n[I] Caught signal %d\n", type);
    quit();
    exit(0);
}

///
/// \brief signalhandler
/// Write the received signal type to the socketpair for signal-safe(7)
/// signal handling.
/// \param type received signal type
void sighandler(int type){
    int unused_result = write(sighandler_pipe[SIGHANDLER_SENDER], &type, sizeof(int));

    // cast unused result to void to silence over-eager
    // warning about unused variables:
    // https://sourceware.org/bugzilla/show_bug.cgi?id=11959
    (void) unused_result;
}

void nullhandler(int s){
    safe_write("[I] Caught internal signal SIGUSR2\n");
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
    // Set output pipes to buffer on newlines, if they weren't set that way already
    setlinebuf(stdout);
    setlinebuf(stderr);

    printf("ckb-next: Corsair RGB driver %s\n", CKB_NEXT_VERSION_STR);
    // If --help occurs anywhere in the command-line, don't launch the program but instead print usage
    for(int i = 1; i < argc; i++){
        if(!strcmp(argv[i], "--help")){
            printf(
#ifdef OS_MAC_LEGACY
                        "Usage: ckb-next-daemon [--version] [--gid=<gid>] [--hwload=<always|try|never>] [--nonotify] [--nobind] [--nomouseaccel] [--nonroot]\n"
#else
                        "Usage: ckb-next-daemon [--version] [--gid=<gid>] [--hwload=<always|try|never>] [--nonotify] [--nobind] [--nonroot]\n"
#endif
                        "\n"
                        "See https://github.com/ckb-next/ckb-next/wiki/CKB-Daemon-Manual for full instructions.\n"
                        "\n"
                        "Command-line parameters:\n"
                        "    --version\n"
                        "        Print version string to stdout and quit.\n"
                        "    --gid=<gid>\n"
                        "        Restrict access to %s* nodes to users in group <gid>.\n"
                        "        (Ordinarily they are accessible to anyone)\n"
                        "    --hwload=<always|try|never>\n"
                        "        --hwload=always will force loading of stored hardware profiles on compatible devices. May result in long start up times.\n"
                        "        --hwload=try will try to load the profiles, but give up if not immediately successful (default).\n"
                        "        --hwload=never will ignore hardware profiles completely.\n"
                        "    --nonotify\n"
                        "        Disables key monitoring/notifications.\n"
                        "        Note that this makes reactive lighting impossible.\n"
                        "    --nobind\n"
                        "        Disables all key rebinding, macros, and notifications. Implies --nonotify.\n"
#ifdef OS_MAC_LEGACY
                        "    --nomouseaccel\n"
                        "        Disables mouse acceleration, even if the system preferences enable it.\n"
#endif
                        "    --nonroot\n"
                        "        Allows running ckb-next-daemon as a non root user.\n"
                        "        This will almost certainly not work. Use only if you know what you're doing.\n"
                        "\n", devpath);
            exit(0);
        } else if (!strcmp(argv[i], "--version")){
            return 0;
        }
    }

#ifdef OS_MAC
    if(argc == 2 && getuid() != 0 && !(strcmp(argv[1], "--request-hid-permission-because-it-doesnt-work-as-root-thanks-apple")))
        return request_hid_access_mac();
#endif

    // Check PID, quit if already running
    char pidpath[strlen(devpath) + 6];
    snprintf(pidpath, sizeof(pidpath), "%s0/pid", devpath);
    FILE* pidfile = fopen(pidpath, "r");
    if(pidfile){
        pid_t pid;
        if(fscanf(pidfile, "%d", &pid) == EOF)
            ckb_err("PID fscanf returned EOF (%s)", strerror(errno));
        fclose(pidfile);
        if(pid > 0){
            // kill -s 0 checks if the PID is active but doesn't send a signal
            if(!kill(pid, 0)){
                ckb_fatal_nofile("ckb-next-daemon is already running (PID %d). Try `killall ckb-next-daemon`.", pid);
                ckb_fatal_nofile("(If you're certain the process is dead, delete %s and try again)", pidpath);
                return 0;
            }
        }
    }

    // Read parameters
    int forceroot = 1;
    for(int i = 1; i < argc; i++){
        char* argument = argv[i];
        unsigned newgid;
        char hwload[8];
        ushort vid, pid;
        if(sscanf(argument, "--gid=%u", &newgid) == 1){
            // Set dev node GID
            gid = newgid;
            ckb_info_nofile("Setting /dev node gid: %u", newgid);
        } else if(!strcmp(argument, "--nobind")){
            // Disable key notifications and rebinding
            features_mask &= ~FEAT_BIND & ~FEAT_NOTIFY;
            ckb_info_nofile("Key binding and key notifications are disabled");
        } else if(!strcmp(argument, "--nonotify")){
            // Disable key notifications
            features_mask &= ~FEAT_NOTIFY;
            ckb_info_nofile("Key notifications are disabled");
        } else if(sscanf(argument, "--hwload=%7s", hwload) == 1){
            if(!strcmp(hwload, "always") || !strcmp(hwload, "yes") || !strcmp(hwload, "y") || !strcmp(hwload, "a")){
                hwload_mode = 2;
                ckb_info_nofile("Setting hardware load: always");
            } else if(!strcmp(hwload, "tryonce") || !strcmp(hwload, "try") || !strcmp(hwload, "once") || !strcmp(hwload, "t") || !strcmp(hwload, "o")){
                hwload_mode = 1;
                ckb_info_nofile("Setting hardware load: tryonce");
            } else if(!strcmp(hwload, "never") || !strcmp(hwload, "none") || !strcmp(hwload, "no") || !strcmp(hwload, "n")){
                hwload_mode = 0;
                ckb_info_nofile("Setting hardware load: never");
            }
        } else if(!strcmp(argument, "--nonroot")){
            // Allow running as a non-root user
            forceroot = 0;
        } else if(sscanf(argument, "--ignore=%hx:%hx", &vid, &pid) == 2){
            // Add the vid/pid to the list
            for(int j = 0; j < DEV_MAX; j++){
                if(ignored_devices[j].idVendor || ignored_devices[j].idProduct)
                    continue;
                ignored_devices[j].idVendor = vid;
                ignored_devices[j].idProduct = pid;
                break;
            }
        } else if(!strncmp(argument, "--search=", 9)) {
            char* searchstr = argument + 9;
            usbdevice dev = {0};

            int count;
            if(sscanf(searchstr, "%hx:%hx%n", &dev.vendor, &dev.product, &count) == 2) {
                searchstr += count;
                if(*searchstr != '\0')
                    searchstr++;
            }

            patchkeys(&dev);

            // Search through the patched keymap
            for (int j = 0; j < N_KEYS_EXTENDED; j++) {
                if (!dev.keymap[j].name)
                    continue;

                if (!strcasecmp(searchstr, dev.keymap[j].name)) {
                    printf("Key %s has id %d\n", dev.keymap[j].name, j);
                    free(dev.keymap);
                    return 0;
                }
            }
            printf("Key %s was not found\n", searchstr);
            free(dev.keymap);
            return 1;
        } else if(!strcmp(argument, "--enable-experimental")) {
            enable_experimental = 1;
        }
#ifdef OS_MAC_LEGACY
        else if(!strcmp(argument, "--nomouseaccel")){
            // On OSX, provide an option to disable mouse acceleration
            features_mask &= ~FEAT_MOUSEACCEL;
            ckb_info_nofile("Mouse acceleration disabled");
        }
#endif
    }

    // Check UID
    if(getuid() != 0){
        if(forceroot){
            ckb_fatal_nofile("ckb-next-daemon must be run as root. Try `sudo %s`", argv[0]);
            exit(0);
        } else
            ckb_warn_nofile("Warning: not running as root, allowing anyway per command-line parameter...");
    }

    // Make root keyboard
    umask(0);
    memset(keyboard, 0, sizeof(keyboard));
    if(!mkdevpath(keyboard))
        ckb_info("Root controller ready at %s0", devpath);

    // Attempt to setup signal-safe signal handlers using socketpair(2)
    if (socketpair(AF_LOCAL, SOCK_STREAM, 0, sighandler_pipe) != -1){
        signal(SIGTERM, sighandler);
        signal(SIGINT, sighandler);
        signal(SIGQUIT, sighandler);
    } else
        ckb_warn_nofile("Unable to setup signal handlers");

    // Set up do-nothing handler for SIGUSR2
    // sa_flags must be 0 so that we can interrupt blocking calls in threads by calling pthread_kill()
    struct sigaction new_action = {
        .sa_handler = nullhandler,
        .sa_flags = 0,
    };
    sigemptyset (&new_action.sa_mask);
    sigaction(SIGUSR2, &new_action, NULL);

    srand(time(NULL));

    if(init_cond_monotonic())  {
        ckb_fatal("Failed to initialize monotonic clock.");
        exit(1);
    }

    // Start the USB system
    int result = usbmain();
    quit();
    return result;
}

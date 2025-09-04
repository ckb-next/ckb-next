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

#define TERM_REV_VID "\033[7m"
#define TERM_NORM    "\033[0m"

// usb.c
extern _Atomic int reset_stop;
extern int features_mask;
extern int enable_experimental;
extern const char pidpath[];
int sighandler_pipe[2] = { 0, 0 };

// Timespec utility function
void timespec_add(struct timespec* timespec, int64_t nanoseconds){
    nanoseconds += timespec->tv_nsec;
    timespec->tv_sec += nanoseconds / 1000000000;
    timespec->tv_nsec = nanoseconds % 1000000000;
}

// Daemon run states--- used to determine whether special actions (e.g. USB reload) have been commanded
enum { DAEMON_PAUSED, DAEMON_RUNNING, RELOAD_DAEMON } runstate = DAEMON_PAUSED;

#define SIGDESCR(S) { SIG ## S, "SIG" # S }
struct signal_descriptor { int value; const char *name; };
static struct signal_descriptor signals_trapped[] = {
    SIGDESCR(TERM), SIGDESCR(INT), SIGDESCR(QUIT), /* These are the traditional shutdown signals */
    SIGDESCR(HUP), SIGDESCR(USR1), /* These will trigger a reload of the USB subsystem */
    SIGDESCR(USR2)                 /* This signal is used only for synchronization between heavyweight threads */
    /* TODO: could add additional signals here to try to reset keyboard to HID mode
     * in the event of an abnormal daemon process exit (e.g. SEGV, ILL, ABRT).
     * If not trapped, those signals will likely leave the device unusable until it is reset,
     * either via onboard firmware or disconnecting from and reconnecting to the USB bus.
     */
};

///
/// \brief initialize_keyboard
/// Establish the keyboard controller device.
/// Must be called before any non-trivial sighandler() is set.
static void initialize_keyboard(void) {
    memset(keyboard, 0, sizeof(keyboard));
    if(!mkdevpath(keyboard))
        ckb_info("Root controller ready at %s0", devpath);
}

///
/// \brief usb_shutdown
/// Disconnect daemon from the USB subsystem.
/// Called if we received a termination (TERM/INT/QUIT) or reload (HUP/USR1) signal.
/// In this case, locking the device-mutex is ok.
/// For reload signals, will be followed by re-establishing the entire USB device tree.
/// For termination signals, will be followed by daemon exit.
static void usb_shutdown() {
    // Abort any USB resets in progress
    reset_stop = 1;

    // Before closing, set all keyboards back to HID input mode so that the stock driver can still talk to them
    for(int i = 1; i < DEV_MAX; i++){
        queued_mutex_lock(devmutex + i);
        if((keyboard + i)->status == DEV_STATUS_CONNECTING || (keyboard + i)->status == DEV_STATUS_CONNECTED)
            revertusb(keyboard + i);
        queued_mutex_unlock(devmutex + i);
    }

    // We do this in a separate loop so that devices with children won't be removed before the children have been set to "idle"
    for(int i = 1; i < DEV_MAX; i++){
        queued_mutex_lock(devmutex + i);
        if((keyboard + i)->status == DEV_STATUS_CONNECTING || (keyboard + i)->status == DEV_STATUS_CONNECTED)
            closeusb(keyboard + i);
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
/// \brief get_signal_name
/// Return a read-only string representation (in static heap memory) of the signal name.
/// The caller does not need to free() anything afterward, but unhandled signals will be reported
/// simply as "UNKNOWN" with no indication of the actual number
static const char *get_signal_name(int type) {
    for (size_t i = 0; i < sizeof(signals_trapped) / sizeof(signals_trapped[0]); i++)
        if (signals_trapped[i].value == type)
            return signals_trapped[i].name;
    return "UNKNOWN";
}

///
/// \brief get_temp_signal_name
/// Return a writable string representation of the signal name.
/// The caller is responsible for free()-ing the memory afterward, but unhandled signals will have
/// their number included in the returned string to support more detailed troubleshooting.
static char *get_temp_signal_name(int type) {
    char *tmp;
    for (size_t i = 0; i < sizeof(signals_trapped) / sizeof(signals_trapped[0]); i++)
        if (signals_trapped[i].value == type)
            return strdup(signals_trapped[i].name);
    tmp = malloc(32);
    if (tmp)
        snprintf(tmp, 32, "UNKNOWN (%d)", type);
    return tmp;
}


///
/// \brief ignore_signal
/// Nested signal handler for previously received signals.
/// \param type received signal type
void ignore_signal(int type){
    // Use signal-safe(7) write(3) call to print warnings

    static int interrupt_count = 0;
    if (type == SIGINT) {
        // Failsafe abort on double-^C
        if (interrupt_count++) {
            safe_write("\n[W] Received double SIGINT; aborting daemon even though signal processing nominally suspended\n");
            usb_shutdown();
            exit(2);
        } else
            safe_write("\n[W] Received SIGINT while signal processing suspended; send again to force daemon exit\n");
    } else {
        interrupt_count = 0;
        safe_write("\n[W] Ignoring signal ");
        safe_write(get_signal_name(type));
        safe_write(" (signal processing suspended during daemon shutdown or reload)\n");
    }
}

///
/// \brief suspend_all_signals
/// Temporarily set null handlers for any further incoming signals
/// Useful to ensure (unless signals arrive in very close succession) that we aren't
/// interrupted again while handling the first signal.
/// Could more efficiently be handled with a global in_signal_handler semaphore,
/// but this approach more closely mirrors the original daemon code.
static void suspend_all_signals()
{
    for (size_t i = 0; i < sizeof(signals_trapped) / sizeof(signals_trapped[0]); i++)
        if (signals_trapped[i].value != SIGUSR2)
            signal(signals_trapped[i].value, ignore_signal);
}


///
/// \brief restore_signal_handlers
/// Set up signal handlers for each supported signal.
/// \param which ONLY_NULLHANDLER -> only set up SIGUSR2 (useful if we were unable to
/// establish the communications pipe for non-trivial signals)
/// RESTORE_ALL -> set up all supported handlers
/// RESTORE_LAST -> restore the previous behavior
enum { ONLY_NULLHANDLER, RESTORE_ALL, RESTORE_LAST };
static void restore_signal_handlers(int which)
{
    void sighandler(int), nullhandler(int);
    static int last_signal_set = -1;

    if (which == RESTORE_ALL || (which == RESTORE_LAST && last_signal_set == RESTORE_ALL))
        for (size_t i = 0; i < sizeof(signals_trapped) / sizeof(signals_trapped[0]); i++)
            if (signals_trapped[i].value != SIGUSR2)
                signal(signals_trapped[i].value, sighandler);

    // Set up do-nothing handler for SIGUSR2
    // sa_flags must be 0 so that we can interrupt blocking calls in threads by calling pthread_kill()
    struct sigaction new_action = {
        .sa_handler = nullhandler,
        .sa_flags = 0,
    };
    sigemptyset (&new_action.sa_mask);
    sigaction(SIGUSR2, &new_action, NULL);

    if (which != RESTORE_LAST)
        last_signal_set = which;
}

///
/// \brief exithandler
/// Main signal handler to catch further signals and call reload or shutdown
/// sequence of daemon, as appropriate. This function is allowed to call unsafe
/// (signal-safe(7)) function calls, as it is itself executed in another
/// process via the socket handler.
/// If exithandler() returns to caller, usbmain() should eventually terminate and return to main()
/// \param type received signal type
void exithandler(int type){
    printf("\n[I] Caught signal %d\n", type);
    suspend_all_signals();

    if (type == SIGTERM || type == SIGINT || type == SIGQUIT) {
        usb_shutdown();
        exit(0);
    } else if (type == SIGHUP || type == SIGUSR1) {
        runstate = RELOAD_DAEMON;
        restore_signal_handlers(RESTORE_LAST);
    } else {
        // Never expected to be triggered; we somehow set sighandler() on a signal that was not added to this function's logic.
        char *name = get_temp_signal_name(type);
        safe_write("[E] exithandler() passed unsupported signal ");
        safe_write(name ? name : "(null)");
        if (name)
            free(name);
        safe_write("\nAborting daemon.\n");
        usb_shutdown();
        exit(1);
    }
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
    char *name = get_temp_signal_name(s);
    safe_write("[I] Caught internal signal ");
    safe_write(name ? name : "(null)");
    if (name)
        free(name);
    safe_write("\n");
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

// For --search
extern void fill_usbdevice_protocol(usbdevice* kb);

int main(int argc, char** argv){
    // Set output pipes to buffer on newlines, if they weren't set that way already
    setlinebuf(stdout);
    setlinebuf(stderr);

    // If --help occurs anywhere in the command-line, don't launch the program but instead print usage
    for(int i = 1; i < argc; i++){
        if(!strcmp(argv[i], "--help")){
            printf(
#ifdef OS_MAC_LEGACY
                        "Usage: ckb-next-daemon [--version] [--gid=<gid>] [--nonotify] [--nobind] [--nomouseaccel] [--nonroot]\n"
#else
                        "Usage: ckb-next-daemon [--version] [--gid=<gid>] [--nonotify] [--nobind] [--nonroot]\n"
#endif
                        "%s\n\n"
                        "Options:\n"
                        "    --version\n"
                        "        Print version string to stdout and quit.\n"
                        "    --gid=<gid>\n"
                        "        Restrict access to %s* nodes to users in group <gid>.\n"
                        "        (Ordinarily they are accessible to anyone)\n"
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
                        "        This will almost certainly not work. Use only if you know what you're doing.\n",
                        CKB_NEXT_DESCRIPTION, devpath);
            return 0;
        } else if (!strcmp(argv[i], "--version")){
            printf("ckb-next-daemon %s\n", CKB_NEXT_VERSION_STR);
            return 0;
        }
    }

    printf("ckb-next-daemon %s\n", CKB_NEXT_VERSION_STR);

#ifdef OS_MAC
    if(argc == 2 && getuid() != 0 && !(strcmp(argv[1], "--request-hid-permission-because-it-doesnt-work-as-root-thanks-apple")))
        return request_hid_access_mac();
#endif

    // Check PID, quit if already running
    pid_t dpid;
    if((dpid = is_pid_running())){
        ckb_fatal_nofile("ckb-next-daemon is already running (PID %ld).", (long)dpid);
        ckb_fatal_nofile("Try `systemctl stop ckb-next-daemon` or `killall ckb-next-daemon`.");
        ckb_fatal_nofile("(If you're certain the process is dead, delete %s and try again.)", pidpath);
        return 1;
    }

    // Read parameters
    int forceroot = 1;
    for(int i = 1; i < argc; i++){
        char* argument = argv[i];
        unsigned newgid;
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
            // TODO: repeat this search process on a daemon reload signal?
            char* searchstr = argument + 9;
            usbdevice dev = {0};

            int count;
            if(sscanf(searchstr, "%hx:%hx%n", &dev.vendor, &dev.product, &count) == 2) {
                searchstr += count;
                if(*searchstr != '\0')
                    searchstr++;
            }

            // We need to call this before patchkeys, otherwise the bragi keymap will never be applied
            fill_usbdevice_protocol(&dev);

            patchkeys(&dev);

            int found = -1;

            // Search through the patched keymap
            for (int j = 0; j < N_KEYS_EXTENDED; j++) {
                // Special case, where we're searching for the first NULL entry
                if (!dev.keymap[j].name) {
                    if (*searchstr != '\0')
                        continue;

                    found = j;
                    break;
                }

                if (!strcasecmp(searchstr, dev.keymap[j].name)) {
                    found = j;
                    break;
                }
            }

            for (int j = 0; j < N_KEYS_EXTENDED; j++) {
                if(j == found)
                    fputs(TERM_REV_VID, stdout);

                printf("{ %10s, %3hd, %4hd },", (dev.keymap[j].name ? dev.keymap[j].name : "NULL"),
                    dev.keymap[j].led, dev.keymap[j].scan);

                if(j == found)
                    fputs(TERM_NORM, stdout);
                putchar('\n');
            }

            putchar('\n');

            if(found == -1) {
                printf("Key %s was not found\n", searchstr);
                return 1;
            } else if (*searchstr == '\0') {
                printf("First NULL key has id %d\n", found);
            } else {
                printf("Key %s has id %d\n", dev.keymap[found].name, found);
            }

            return 0;
        } else if(!strcmp(argument, "--enable-experimental")) {
            enable_experimental = 1;
#ifdef ckb_next_VERSION_IS_RELEASE
            ckb_info("You have enabled support for experimental devices in a ckb-next stable release. It is recommended to use the latest code from git.");
#endif
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
    // Do this before setting signal handlers just in case we get an early signal
    // (usb_shutdown() expects to find a valid keyboard structure)
    umask(0);
    initialize_keyboard();

    // Attempt to setup signal-safe signal handlers using socketpair(2)
    if (socketpair(AF_LOCAL, SOCK_STREAM, 0, sighandler_pipe) != -1) {
        restore_signal_handlers(RESTORE_ALL);
    } else {
        ckb_warn_nofile("Unable to setup pipe for signal handling");
        restore_signal_handlers(ONLY_NULLHANDLER);
    }

    srand(time(NULL));

    if(init_cond_monotonic())  {
        ckb_fatal("Failed to initialize monotonic clock.");
        exit(1);
    }

    // Start the USB system
    while (1) {
        int result;

        runstate = DAEMON_RUNNING;
        result = usbmain();
        usb_shutdown();
        if (runstate != RELOAD_DAEMON)
            return result;

        // Reloading, so re-initialize USB subsystem and loop back to running state
        initialize_keyboard();
    }
}

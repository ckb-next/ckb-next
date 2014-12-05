#include "device.h"
#include "devnode.h"
#include "input.h"
#include "led.h"
#include "notify.h"

// Not supported on OSX...
#ifdef OS_MAC
#define pthread_mutex_timedlock(mutex, timespec) pthread_mutex_lock(mutex)
#endif

void quit(){
    // Wait at most 1s for mutex locks. Better to crash than to freeze shutting down.
    struct timespec timeout = { 1, 0 };
    pthread_mutex_timedlock(&kblistmutex, &timeout);
    for(int i = 1; i < DEV_MAX; i++){
        // Before closing, set all keyboards back to HID input mode so that the stock driver can still talk to them
        if(IS_ACTIVE(keyboard + i)){
            pthread_mutex_timedlock(&keyboard[i].mutex, &timeout);
            // Stop the uinput device now to ensure no keys get stuck
            inputclose(keyboard + i);
            // Empty the USB queue first
            while(keyboard[i].queuecount > 0){
                DELAY_SHORT;
                if(usbdequeue(keyboard + i) <= 0)
                    break;
            }
            DELAY_MEDIUM;
            setinput(keyboard + i, IN_HID);
            while(keyboard[i].queuecount > 0){
                DELAY_SHORT;
                if(usbdequeue(keyboard + i) <= 0)
                    break;
            }
            // Set the M-keys back into hardware mode and restore the RGB profile. It has to be sent twice for some reason.
            uchar msg[MSG_SIZE] = { 0x07, 0x04, 0x01, 0 };
            usbqueue(keyboard + i, msg, 1);
            usbqueue(keyboard + i, msg, 1);
            // Flush the USB queue and close the device
            while(keyboard[i].queuecount > 0){
                DELAY_MEDIUM;
                if(usbdequeue(keyboard + i) <= 0)
                    break;
            }
            closeusb(keyboard + i);
        }
    }
    pthread_mutex_timedlock(&keyboard[0].mutex, &timeout);
    closeusb(keyboard);
    usbdeinit();
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

pthread_t sigthread;

void* sigmain(void* context){
    // Allow signals in this thread
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
    while(1){
        sleep(-1);
    }
    return 0;
}

int main(int argc, char** argv){
    printf("ckb Corsair Keyboard RGB driver v0.0.11\n");

    // Read parameters
    for(int i = 1; i < argc; i++){
        char* argument = argv[i];
        char layout[10];
        unsigned newfps, newgid;
        if(sscanf(argument, "--fps=%u", &newfps) == 1){
            // Set FPS
            setfps(newfps);
        } else if(sscanf(argument, "--layout=%9s", layout) == 1){
            // Set keyboard layout
            keymap_system = getkeymap(layout);
            if(keymap_system)
                printf("Setting default layout: %s\n", layout);
        } else if(sscanf(argument, "--gid=%u", &newgid) == 1){
            // Set dev node GID
            gid = newgid;
            printf("Setting /dev node gid: %u\n", newgid);
        }
    }

    // Set FPS if not done already
    if(!fps)
        setfps(60);

    // If the keymap wasn't set via command-line, get it from the system locale
    if(!keymap_system){
        setlocale(LC_ALL, "");
        const char* locale = setlocale(LC_CTYPE, 0);
        // Check for UK layout
        if(strstr(locale, "en_GB") || strstr(locale, "en-GB")
                || strstr(locale, "en_gb") || strstr(locale, "en-gb")){
            keymap_system = keymap_uk;
            printf("Setting default layout: uk\n");
        } else {
            // Default to US
            keymap_system = keymap_us;
            printf("Setting default layout: us\n");
        }
    }

    // Make root keyboard
    umask(0);
    memset(keyboard, 0, sizeof(keyboard));
    pthread_mutex_init(&keyboard[0].mutex, 0);
    keyboard[0].model = -1;
    if(!makedevpath(keyboard))
        printf("Root controller ready at %s0\n", devpath);

    // Don't let any spawned threads handle signals
    sigset_t signals;
    sigfillset(&signals);
    pthread_sigmask(SIG_SETMASK, &signals, 0);

    // Start the USB system
    if(usbinit()){
        quit();
        return -1;
    }

    // Start the signal handling thread
    pthread_create(&sigthread, 0, sigmain, 0);

    int frame = 0;
    struct timespec time, nexttime;
    while(1){
        clock_gettime(CLOCK_MONOTONIC, &time);
        // No need to run most of these functions on every single frame
        if(!frame){
            // Process FIFOs
            pthread_mutex_lock(&kblistmutex);
            for(int i = 0; i < DEV_MAX; i++){
                if(keyboard[i].infifo){
                    const char* line;
                    if(readlines(keyboard[i].infifo, &line))
                        readcmd(keyboard + i, line);
                }
            }
        } else
            pthread_mutex_lock(&kblistmutex);
        // Run the USB queue. Messages must be queued because sending multiple messages at the same time can cause the interface to freeze
        for(int i = 1; i < DEV_MAX; i++){
            if(IS_ACTIVE(keyboard + i)){
                pthread_mutex_lock(&keyboard[i].mutex);
                if(usbdequeue(keyboard + i) == 0
                        && usb_tryreset(keyboard + i)){
                    // If it failed and couldn't be reset, close the keyboard
                    closeusb(keyboard + i);
                } else {
                    // Update indicator LEDs for this keyboard. These are polled rather than processed during events because they don't update
                    // immediately and may be changed externally by the OS.
                    if(!frame)
                        updateindicators(keyboard + i, 0);
                    pthread_mutex_unlock(&keyboard[i].mutex);
                }
            }
        }
        pthread_mutex_unlock(&kblistmutex);
        // Sleep for long enough to achieve the desired frame rate (5 packets per frame).
        long newnsec = time.tv_nsec + 1000000000 / fps / 5;
        nexttime.tv_sec = time.tv_sec + newnsec / 1000000000;
        nexttime.tv_nsec = newnsec % 1000000000;
        // Don't ever sleep for less than 1.5ms. It can lock the keyboard.
        clock_gettime(CLOCK_MONOTONIC, &time);
        newnsec = time.tv_nsec + 1500000;
        time.tv_sec += newnsec / 1000000000;
        time.tv_nsec = newnsec % 1000000000;
        // Sleep for whichever time period is greater. Restart the sleep if it gets interrupted.
        if(nexttime.tv_sec > time.tv_sec || (nexttime.tv_sec == time.tv_sec && nexttime.tv_nsec > time.tv_nsec))
            while(clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &nexttime, 0) == EINTR);
        else
            while(clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &time, 0) == EINTR);
        frame = (frame + 1) % 5;
    }
    quit();
    return 0;
}

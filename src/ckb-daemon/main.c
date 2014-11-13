#include "device.h"
#include "devnode.h"
#include "input.h"

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
            setinput(keyboard + i, IN_HID);
            // Flush the USB queue and close the device
            while(keyboard[i].queuecount > 0){
                usleep(3333);
                if(usbdequeue(keyboard + i) <= 0)
                    break;
            }
            closeusb(keyboard + i);
        }
    }
    pthread_mutex_unlock(&kblistmutex);
    pthread_mutex_timedlock(&keyboard[0].mutex, &timeout);
    closeusb(keyboard);
    usbdeinit();
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
    printf("ckb Corsair Keyboard RGB driver v0.1\n");

    // Read parameters
    int fps = 60;
    for(int i = 1; i < argc; i++){
        char* argument = argv[i];
        char layout[10];
        if(sscanf(argument, "--fps=%d", &fps) == 1){
            if(fps > 60 || fps <= 0){
                // There's no point running higher than 60FPS.
                // The LED controller is locked to 60Hz so it will only cause tearing and/or device freezes.
                printf("Warning: Requested %d FPS but capping at 60\n", fps);
                fps = 60;
            }
        } else if(sscanf(argument, "--layout=%9s", layout) == 1){
            // Set keyboard layout from command-line parameter
            keymap_system = getkeymap(layout);
            printf("Setting default layout: %s\n", layout);
        }
    }

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
    while(1){
        // No need to run most of these functions on every single frame
        if(!frame){
            // Process FIFOs
            pthread_mutex_lock(&kblistmutex);
            for(int i = 0; i < DEV_MAX; i++){
                if(keyboard[i].infifo){
                    const char** lines;
                    int nlines = readlines(keyboard[i].infifo, &lines);
                    for(int j = 0; j < nlines; j++){
                        if(lines[j][0] != 0 && lines[j][1] != 0)
                            readcmd(keyboard + i, lines[j]);
                    }
                }
            }
        } else
            pthread_mutex_lock(&kblistmutex);
        // Run the USB queue. Messages must be queued because sending multiple messages at the same time can cause the interface to freeze
        for(int i = 1; i < DEV_MAX; i++){
            if(IS_ACTIVE(keyboard + i)){
                pthread_mutex_lock(&keyboard[i].mutex);
                if(usbdequeue(keyboard + i) == 0){
                    printf("Device ckb%d not responding, trying to reset...\n", i);
                    if(resetusb(keyboard + i))
                        closeusb(keyboard + i);
                    else
                        updateindicators(keyboard + i, 1);
                } else {
                    // Update indicator LEDs for this keyboard. These are polled rather than processed during events because they don't update
                    // immediately and may be changed externally by the OS.
                    if(!frame)
                        updateindicators(keyboard + i, 0);
                }
                pthread_mutex_unlock(&keyboard[i].mutex);
            }
        }
        pthread_mutex_unlock(&kblistmutex);
        // Sleep for long enough to achieve the desired frame rate (5 packets per frame).
        usleep(1000000 / fps / 5);
        frame = (frame + 1) % 5;
    }
    quit();
    return 0;
}

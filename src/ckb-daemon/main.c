#include "usb.h"
#include "device.h"
#include "devnode.h"
#include "led.h"
#include "input.h"

static int kblocked = 0;

void quit(){
    if(!kblocked)
        pthread_mutex_lock(&kblistmutex);
    for(int i = 1; i < DEV_MAX; i++){
        // Before closing, set all keyboards back to HID input mode so that the stock driver can still talk to them
        if(keyboard[i].fifo && keyboard[i].handle){
            pthread_mutex_lock(&keyboard[i].mutex);
            setinput(keyboard + i, IN_HID);
            // Stop the uinput device now to ensure no keys get stuck
            inputclose(i);
            // Flush the USB queue and close the device
            while(keyboard[i].queuecount > 0){
                usleep(3333);
                if(usbdequeue(keyboard + i) <= 0)
                    break;
            }
            closeusb(i);
            pthread_mutex_unlock(&keyboard[i].mutex);
        }
    }
    if(!kblocked)
        pthread_mutex_unlock(&kblistmutex);
    closeusb(0);
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

int main(int argc, char** argv){
    printf("ckb Corsair Keyboard RGB driver v0.1\n");

    // Read parameters
    int fps = 60;
    for(int i = 1; i < argc; i++){
        char* argument = argv[i];
        if(sscanf(argument, "--fps=%d", &fps) == 1){
            if(fps > 60 || fps <= 0){
                // There's no point running higher than 60FPS.
                // The LED controller is locked to 60Hz so it will only cause tearing and/or device freezes.
                printf("Warning: Requested %d FPS but capping at 60\n", fps);
                fps = 60;
            }
        }
    }

    // Make root keyboard
    umask(0);
    memset(keyboard, 0, sizeof(keyboard));
    keyboard[0].model = -1;
    if(!makedevpath(0))
        printf("Root controller ready at %s0\n", devpath);

    // Don't let any spawned threads handle signals
    sigset_t signals, oldsignals;
    sigfillset(&signals);
    pthread_sigmask(SIG_SETMASK, &signals, &oldsignals);

    // Start the USB system
    if(usbinit()){
        quit();
        return -1;
    }

    // Set up signal handlers for quitting the service.
    pthread_sigmask(SIG_SETMASK, &oldsignals, 0);
    signal(SIGTERM, sighandler);
    signal(SIGINT, sighandler);
    signal(SIGQUIT, sighandler);

    int frame = 0;
    while(1){
        // No need to run most of these functions on every single frame
        if(!frame){
            usbmainloop();
            // Process FIFOs
            kblocked = 1;
            pthread_mutex_lock(&kblistmutex);
            for(int i = 0; i < DEV_MAX; i++){
                if(keyboard[i].fifo){
                    const char** lines;
                    int nlines = readlines(keyboard[i].fifo, &lines);
                    for(int j = 0; j < nlines; j++){
                        if(lines[j][0] != 0 && lines[j][1] != 0)
                            readcmd(keyboard + i, lines[j]);
                    }
                }
            }
        } else {
            kblocked = 1;
            pthread_mutex_lock(&kblistmutex);
        }
        // Run the USB queue. Messages must be queued because sending multiple messages at the same time can cause the interface to freeze
        for(int i = 1; i < DEV_MAX; i++){
            if(keyboard[i].fifo && keyboard[i].handle){
                pthread_mutex_lock(&keyboard[i].mutex);
                usbdequeue(keyboard + i);
                // Update indicator LEDs for this keyboard. These are polled rather than processed during events because they don't update
                // immediately and may be changed externally by the OS.
                if(!frame)
                    updateindicators(keyboard + i, 0);
                pthread_mutex_unlock(&keyboard[i].mutex);
            }
        }
        pthread_mutex_unlock(&kblistmutex);
        kblocked = 0;
        // Sleep for long enough to achieve the desired frame rate (5 packets per frame).
        usleep(1000000 / fps / 5);
        frame = (frame + 1) % 5;
    }
    quit();
    return 0;
}

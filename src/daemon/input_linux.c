#include "command.h"
#include "device.h"
#include "input.h"
#include "usb.h"

#ifdef OS_LINUX

// Xorg has buggy handling of combined keyboard + mouse devices, so instead we should create two separate devices:
// One for keyboard events, one for mouse.
int uinputopen(struct uinput_user_dev* indev, int mouse){
    int fd = open("/dev/uinput", O_RDWR);
    if(fd < 0){
        // If that didn't work, try /dev/input/uinput instead
        fd = open("/dev/input/uinput", O_RDWR);
        if(fd < 0){
            ckb_err("Failed to open uinput: %s", strerror(errno));
            return 0;
        }
    }

    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    if(mouse){
        // Enable only the mouse buttons as a workaround for libgdx crashing
        // https://github.com/libgdx/libgdx/issues/5857
        for(int i = BTN_LEFT; i <= BTN_TASK; i++)
            ioctl(fd, UI_SET_KEYBIT, i);
        // Enable mouse axes
        ioctl(fd, UI_SET_EVBIT, EV_REL);
        ioctl(fd, UI_SET_RELBIT, REL_X);
        ioctl(fd, UI_SET_RELBIT, REL_Y);
        ioctl(fd, UI_SET_RELBIT, REL_WHEEL);
        ioctl(fd, UI_SET_RELBIT, REL_HWHEEL);
    } else {
        // Enable common keyboard keys
        for(int i = KEY_ESC; i <= KEY_MEDIA; i++)
            ioctl(fd, UI_SET_KEYBIT, i);
        // Enable KEY_FN as that's also used
        ioctl(fd, UI_SET_KEYBIT, KEY_FN);
        // Enable LEDs
        ioctl(fd, UI_SET_EVBIT, EV_LED);
        //for(int i = 0; i < LED_CNT; i++)
        // Enable only the first 3 LEDs to work around a kernel bug
        for(int i = 0; i < 3; i++)
            ioctl(fd, UI_SET_LEDBIT, i);
        // Enable autorepeat
        ioctl(fd, UI_SET_EVBIT, EV_REP);
    }
    // Enable sychronization
    ioctl(fd, UI_SET_EVBIT, EV_SYN);
    // Create the device
    if(write(fd, indev, sizeof(*indev)) <= 0)
        ckb_warn("uinput write failed: %s", strerror(errno));
    if(ioctl(fd, UI_DEV_CREATE)){
        ckb_err("Failed to create uinput device: %s", strerror(errno));
        close(fd);
        return 0;
    }
    return fd + 1;
}

///
/// \brief os_inputopen
/// \param kb
/// \return
///
/// Some tips on using [uinput_user_dev in](http://thiemonge.org/getting-started-with-uinput)
int os_inputopen(usbdevice* kb){
    /// Let's see if uinput is already available
    int fd = open("/dev/uinput", O_RDWR);
    if(fd < 0){
        fd = open("/dev/input/uinput", O_RDWR);
    }

    // If not available, load the module
    if(fd < 0){
        if(system("modprobe uinput") != 0) {
            ckb_fatal("Failed to load uinput module");
            return 1;
        }
    }
    close(fd);

    if(IS_SINGLE_EP(kb)) {
        kb->uinput_kb = 0;
        kb->uinput_mouse = 0;
        return 0;
    }
    // Create the new input device
    int index = INDEX_OF(kb, keyboard);
    struct uinput_user_dev indev;
    memset(&indev, 0, sizeof(indev));
    snprintf(indev.name, UINPUT_MAX_NAME_SIZE - 5, "ckb%d: %s", index, kb->name);
    strcat(indev.name, " vKB");
    indev.id.bustype = BUS_USB;
    indev.id.vendor = kb->vendor;
    indev.id.product = kb->product;
    indev.id.version = kb->fwversion;
    // Open keyboard
    fd = uinputopen(&indev, 0);
    kb->uinput_kb = fd;
    if(fd <= 0)
        return 0;
    // Open mouse
    snprintf(indev.name, UINPUT_MAX_NAME_SIZE - 5, "ckb%d: %s", index, kb->name);
    strcat(indev.name, " vM");
    fd = uinputopen(&indev, 1);
    kb->uinput_mouse = fd;
    return fd <= 0;
}

void os_inputclose(usbdevice* kb){
    if(kb->uinput_kb <= 0 || kb->uinput_mouse <= 0)
        return;

    // Tell the led thread to stop and then join it
    if(kb->ledthread){
        kb->shutdown_ledthread = 1;
        queued_mutex_unlock(imutex(kb));
        queued_mutex_unlock(dmutex(kb));
        pthread_kill(*kb->ledthread, SIGUSR2);
        pthread_join(*kb->ledthread, NULL);
        queued_mutex_lock(dmutex(kb));
        queued_mutex_lock(imutex(kb));
    }
    free(kb->ledthread);
    kb->ledthread = NULL;

    // Set all keys released
    struct input_event event;
    memset(&event, 0, sizeof(event));
    event.type = EV_KEY;
    for(int k = 0; k < KEY_CNT; k++){
        event.code = k;
        if(write(kb->uinput_kb - 1, &event, sizeof(event)) <= 0)
            ckb_warn("uinput write failed: %s", strerror(errno));
        if(write(kb->uinput_mouse - 1, &event, sizeof(event)) <= 0)
            ckb_warn("uinput write failed: %s", strerror(errno));
    }
    event.type = EV_SYN;
    event.code = SYN_REPORT;
    if(write(kb->uinput_kb - 1, &event, sizeof(event)) <= 0)
        ckb_warn("uinput write failed: %s", strerror(errno));
    if(write(kb->uinput_mouse - 1, &event, sizeof(event)) <= 0)
        ckb_warn("uinput write failed: %s", strerror(errno));
    // Close the keyboard
    ioctl(kb->uinput_kb - 1, UI_DEV_DESTROY);
    close(kb->uinput_kb - 1);
    kb->uinput_kb = 0;
    // Close the mouse
    ioctl(kb->uinput_mouse - 1, UI_DEV_DESTROY);
    close(kb->uinput_mouse - 1);
    kb->uinput_mouse = 0;
}

// Generate SYN reports to synchronize device
static void isync(int fd){
    struct input_event event;
    memset(&event, 0, sizeof(event));
    event.type = EV_SYN;
    event.code = SYN_REPORT;
    if(write(fd, &event, sizeof(event)) <= 0)
        ckb_warn("uinput write failed: %s", strerror(errno));
}

void os_keypress(usbdevice* kb, int scancode, int down){
    struct input_event event;
    memset(&event, 0, sizeof(event));
    // Mouse buttons and key events are both EV_KEY. The scancodes are already correct, just remove the ckb bit
    event.type = EV_KEY;
    event.code = scancode & ~SCAN_MOUSE;
    event.value = down;
    int is_mouse = scancode & SCAN_MOUSE;
    int fd = (is_mouse ? kb->uinput_mouse : kb->uinput_kb) - 1;
    if(write(fd, &event, sizeof(event)) <= 0)
        ckb_warn("uinput write failed: %s", strerror(errno));
}

void os_mousemove(usbdevice* kb, int x, int y){
    struct input_event event;
    memset(&event, 0, sizeof(event));
    event.type = EV_REL;
    int fd = kb->uinput_mouse - 1;
    //send X
    if(x){
        event.code = REL_X;
        event.value = x;
        if(write(fd, &event, sizeof(event)) <= 0)
            ckb_warn("uinput write failed: %s", strerror(errno));
    }
    //send Y
    if(y){
        event.code = REL_Y;
        event.value = y;
        if(write(fd, &event, sizeof(event)) <= 0)
            ckb_warn("uinput write failed: %s", strerror(errno));
    }
}

void os_inputsync(usbdevice* kb, int s_kb, int m){
    if(s_kb){
        isync(kb->uinput_kb - 1);
#ifdef DEBUG_INPUT_SYNC
        ckb_info("Uinput keyboard sync");
#endif
    }

    if(m){
        isync(kb->uinput_mouse - 1);
#ifdef DEBUG_INPUT_SYNC
        ckb_info("Uinput mouse sync");
#endif
    }
}

void os_mousescroll(usbdevice* kb, int x, int y){
    struct input_event event = {0};
    event.type = EV_REL;
    int fd = kb->uinput_mouse - 1;

    if(x){
        event.code = REL_HWHEEL;
        event.value = x;
        if(write(fd, &event, sizeof(event)) <= 0)
            ckb_warn("uinput write failed: %s", strerror(errno));
    }

    if(y){
        event.code = REL_WHEEL;
        event.value = y;
        if(write(fd, &event, sizeof(event)) <= 0)
            ckb_warn("uinput write failed: %s", strerror(errno));
    }
}

void* _ledthread(void* ctx){
    usbdevice* kb = ctx;
    uchar ileds = 0;
    // Read LED events from the uinput device
    struct input_event event;
    while (read(kb->uinput_kb - 1, &event, sizeof(event)) > 0) {
        if (event.type == EV_LED && event.code < 8){
            char which = 1 << event.code;
            if(event.value)
                ileds |= which;
            else
                ileds &= ~which;
        }
        // Update them if needed
        wait_until_suspend_processed();
        queued_mutex_lock(dmutex(kb));

        if(kb->hw_ileds != ileds){
            kb->hw_ileds = ileds;
            kb->vtable.updateindicators(kb, 0);
        }

        // This check needs to be after updateindicators.
        // Some devices have quirks that delay URB Controls, so it's possible for the SIGUSR2 signal
        // to be sent before we have reached the read() in this loop as we're waiting for the kernel
        // to finish the control transfer.
        //
        // Perhaps this should be converted to a select() with a pipe instead of a signal
        if(kb->shutdown_ledthread){
            queued_mutex_unlock(dmutex(kb));
            break;
        }
        queued_mutex_unlock(dmutex(kb));
    }
    ckb_info("Stopping indicator thread for ckb%d (%d)", INDEX_OF(kb, keyboard), errno);
    return 0;
}

int os_setupindicators(usbdevice* kb){
    // Initialize LEDs to all off
    kb->hw_ileds = kb->hw_ileds_old = kb->ileds = 0;
    // Create and detach thread to read LED events
    kb->ledthread = malloc(sizeof(pthread_t));
    if(!kb->ledthread){
        ckb_fatal("Failed to allocate memory for the indicator led thread");
        return 1;
    }

    int err = pthread_create(kb->ledthread, 0, _ledthread, kb);
    if(err != 0)
        return err;

    // Give the thread a reasonable name
    char ledthread_name[THREAD_NAME_MAX] = "ckbX led";

    ledthread_name[3] = INDEX_OF(kb, keyboard) + '0';
    pthread_setname_np(*kb->ledthread, ledthread_name);
    return 0;
}

#endif

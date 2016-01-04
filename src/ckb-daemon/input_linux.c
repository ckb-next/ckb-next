#include "command.h"
#include "device.h"
#include "input.h"

#ifdef OS_LINUX

// Xorg has buggy handling of combined keyboard + mouse devices, so instead we should create two separate devices:
// One for keyboard events, one for mouse.
int uinputopen(struct uinput_user_dev* indev, int mouse){
    int fd = open("/dev/uinput", O_RDWR);
    if(fd < 0){
        // If that didn't work, try /dev/input/uinput instead
        fd = open("/dev/input/uinput", O_RDWR);
        if(fd < 0){
            ckb_err("Failed to open uinput: %s\n", strerror(errno));
            return 0;
        }
    }
    // Enable all keys and mouse buttons
    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    for(int i = 0; i < KEY_CNT; i++)
        ioctl(fd, UI_SET_KEYBIT, i);
    if(mouse){
        // Enable mouse axes
        ioctl(fd, UI_SET_EVBIT, EV_REL);
        for(int i = 0; i < REL_CNT; i++)
            ioctl(fd, UI_SET_RELBIT, i);
    } else {
        // Enable LEDs
        ioctl(fd, UI_SET_EVBIT, EV_LED);
        for(int i = 0; i < LED_CNT; i++)
            ioctl(fd, UI_SET_LEDBIT, i);
        // Eanble autorepeat
        ioctl(fd, UI_SET_EVBIT, EV_REP);
    }
    // Enable sychronization
    ioctl(fd, UI_SET_EVBIT, EV_SYN);
    // Create the device
    if(write(fd, indev, sizeof(*indev)) <= 0)
        ckb_warn("uinput write failed: %s\n", strerror(errno));
    if(ioctl(fd, UI_DEV_CREATE)){
        ckb_err("Failed to create uinput device: %s\n", strerror(errno));
        close(fd);
        return 0;
    }
    return fd + 1;
}

int os_inputopen(usbdevice* kb){
    // Create the new input device
    int index = INDEX_OF(kb, keyboard);
    struct uinput_user_dev indev;
    memset(&indev, 0, sizeof(indev));
    snprintf(indev.name, UINPUT_MAX_NAME_SIZE, "ckb%d: %s", index, kb->name);
    indev.id.bustype = BUS_USB;
    indev.id.vendor = kb->vendor;
    indev.id.product = kb->product;
    indev.id.version = kb->fwversion;
    // Open keyboard
    int fd = uinputopen(&indev, 0);
    kb->uinput_kb = fd;
    if(fd <= 0)
        return 0;
    // Open mouse
    fd = uinputopen(&indev, 1);
    kb->uinput_mouse = fd;
    return fd <= 0;
}

void os_inputclose(usbdevice* kb){
    if(kb->uinput_kb <= 0 || kb->uinput_mouse <= 0)
        return;
    // Set all keys released
    struct input_event event;
    memset(&event, 0, sizeof(event));
    event.type = EV_KEY;
    for(int key = 0; key < KEY_CNT; key++){
        event.code = key;
        if(write(kb->uinput_kb - 1, &event, sizeof(event)) <= 0)
            ckb_warn("uinput write failed: %s\n", strerror(errno));
        if(write(kb->uinput_mouse - 1, &event, sizeof(event)) <= 0)
            ckb_warn("uinput write failed: %s\n", strerror(errno));
    }
    event.type = EV_SYN;
    event.code = SYN_REPORT;
    if(write(kb->uinput_kb - 1, &event, sizeof(event)) <= 0)
        ckb_warn("uinput write failed: %s\n", strerror(errno));
    if(write(kb->uinput_mouse - 1, &event, sizeof(event)) <= 0)
        ckb_warn("uinput write failed: %s\n", strerror(errno));
    // Close the keyboard
    ioctl(kb->uinput_kb - 1, UI_DEV_DESTROY);
    close(kb->uinput_kb - 1);
    kb->uinput_kb = 0;
    // Close the mouse
    ioctl(kb->uinput_mouse - 1, UI_DEV_DESTROY);
    close(kb->uinput_mouse - 1);
    kb->uinput_mouse = 0;
}

void os_keypress(usbdevice* kb, int scancode, int down){
    struct input_event event;
    memset(&event, 0, sizeof(event));
    int is_mouse = 0;
    if(scancode == BTN_WHEELUP || scancode == BTN_WHEELDOWN){
        // The mouse wheel is a relative axis
        if(!down)
            return;
        event.type = EV_REL;
        event.code = REL_WHEEL;
        event.value = (scancode == BTN_WHEELUP ? 1 : -1);
        is_mouse = 1;
    } else {
        // Mouse buttons and key events are both EV_KEY. The scancodes are already correct, just remove the ckb bit
        event.type = EV_KEY;
        event.code = scancode & ~SCAN_MOUSE;
        event.value = down;
        is_mouse = !!(scancode & SCAN_MOUSE);
    }
    if(write((is_mouse ? kb->uinput_mouse : kb->uinput_kb) - 1, &event, sizeof(event)) <= 0)
        ckb_warn("uinput write failed: %s\n", strerror(errno));
}

void os_mousemove(usbdevice* kb, int x, int y){
    struct input_event event;
    memset(&event, 0, sizeof(event));
    event.type = EV_REL;
    if(x != 0){
        event.code = REL_X;
        event.value = x;
        if(write(kb->uinput_mouse - 1, &event, sizeof(event)) <= 0)
            ckb_warn("uinput write failed: %s\n", strerror(errno));
    }
    if(y != 0){
        event.code = REL_Y;
        event.value = y;
        if(write(kb->uinput_mouse - 1, &event, sizeof(event)) <= 0)
            ckb_warn("uinput write failed: %s\n", strerror(errno));
    }
}

void os_isync(usbdevice* kb){
    struct input_event event;
    memset(&event, 0, sizeof(event));
    event.type = EV_SYN;
    event.code = SYN_REPORT;
    if(write(kb->uinput_kb - 1, &event, sizeof(event)) <= 0)
        ckb_warn("uinput write failed: %s\n", strerror(errno));
    if(write(kb->uinput_mouse - 1, &event, sizeof(event)) <= 0)
        ckb_warn("uinput write failed: %s\n", strerror(errno));
}

void* _ledthread(void* ctx){
    usbdevice* kb = ctx;
    uchar ileds = 0;
    // Read LED events from the uinput device
    struct input_event event;
    while(read(kb->uinput_kb - 1, &event, sizeof(event)) > 0){
        if(event.type == EV_LED && event.code < 8){
            char which = 1 << event.code;
            if(event.value)
                ileds |= which;
            else
                ileds &= ~which;
        }
        // Update them if needed
        pthread_mutex_lock(dmutex(kb));
        if(kb->hw_ileds != ileds){
            kb->hw_ileds = ileds;
            kb->vtable->updateindicators(kb, 0);
        }
        pthread_mutex_unlock(dmutex(kb));
    }
    return 0;
}

int os_setupindicators(usbdevice* kb){
    // Initialize LEDs to all off
    kb->hw_ileds = kb->hw_ileds_old = kb->ileds = 0;
    // Create and detach thread to read LED events
    pthread_t thread;
    int err = pthread_create(&thread, 0, _ledthread, kb);
    if(err != 0)
        return err;
    pthread_detach(thread);
    return 0;
}

#endif

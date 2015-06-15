#include "device.h"
#include "input.h"

#ifdef OS_LINUX

int uinputopen(struct uinput_user_dev* indev){
    int fd = open("/dev/uinput", O_RDWR | O_NONBLOCK);
    if(fd <= 0){
        // If that didn't work, try /dev/input/uinput instead
        fd = open("/dev/input/uinput", O_RDWR | O_NONBLOCK);
        if(fd <= 0){
            ckb_err("Failed to open uinput: %s\n", strerror(errno));
            return 0;
        }
    }
    // Enable all keys and mouse buttons
    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    for(int i = 0; i < KEY_CNT; i++)
        ioctl(fd, UI_SET_KEYBIT, i);
    // Enable mouse axes
    ioctl(fd, UI_SET_EVBIT, EV_REL);
    for(int i = 0; i < REL_CNT; i++)
        ioctl(fd, UI_SET_RELBIT, i);
    // Enable LEDs
    ioctl(fd, UI_SET_EVBIT, EV_LED);
    for(int i = 0; i < LED_CNT; i++)
        ioctl(fd, UI_SET_LEDBIT, i);
    // Eanble autorepeat
    ioctl(fd, UI_SET_EVBIT, EV_REP);
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
    return fd;
}

int os_inputopen(usbdevice* kb){
    // Create the new input device
    int index = INDEX_OF(kb, keyboard);
    struct uinput_user_dev indev;
    memset(&indev, 0, sizeof(indev));
    snprintf(indev.name, UINPUT_MAX_NAME_SIZE, "ckb%d", index);
    indev.id.bustype = BUS_USB;
    indev.id.vendor = kb->vendor;
    indev.id.product = kb->product;
    indev.id.version = kb->fwversion;
    int fd = uinputopen(&indev);
    kb->uinput = fd;
    return fd <= 0;
}

void os_inputclose(usbdevice* kb){
    if(kb->uinput <= 0)
        return;
    // Set all keys released
    struct input_event event;
    memset(&event, 0, sizeof(event));
    event.type = EV_KEY;
    for(int key = 0; key < KEY_CNT; key++){
        event.code = key;
        if(write(kb->uinput, &event, sizeof(event)) <= 0)
            ckb_warn("uinput write failed: %s\n", strerror(errno));
    }
    event.type = EV_SYN;
    event.code = SYN_REPORT;
    if(write(kb->uinput, &event, sizeof(event)) <= 0)
        ckb_warn("uinput write failed: %s\n", strerror(errno));
    // Close the device
    ioctl(kb->uinput, UI_DEV_DESTROY);
    close(kb->uinput);
    kb->uinput = 0;
}

void os_keypress(usbdevice* kb, int scancode, int down){
    struct input_event event;
    memset(&event, 0, sizeof(event));
    if(scancode == BTN_WHEELUP || scancode == BTN_WHEELDOWN){
        // The mouse wheel is a relative axis
        if(!down)
            return;
        event.type = EV_REL;
        event.code = REL_WHEEL;
        event.value = (scancode == BTN_WHEELUP ? 1 : -1);
    } else {
        // Mouse buttons and key events are both EV_KEY. The scancodes are already correct, just remove the ckb bit
        event.type = EV_KEY;
        event.code = scancode & ~SCAN_MOUSE;
        event.value = down;
    }
    if(write(kb->uinput, &event, sizeof(event)) <= 0)
        ckb_warn("uinput write failed: %s\n", strerror(errno));
}

void os_mousemove(usbdevice* kb, int x, int y){
    struct input_event event;
    memset(&event, 0, sizeof(event));
    event.type = EV_REL;
    if(x != 0){
        event.code = REL_X;
        event.value = x;
        if(write(kb->uinput, &event, sizeof(event)) <= 0)
            ckb_warn("uinput write failed: %s\n", strerror(errno));
    }
    if(y != 0){
        event.code = REL_Y;
        event.value = y;
        if(write(kb->uinput, &event, sizeof(event)) <= 0)
            ckb_warn("uinput write failed: %s\n", strerror(errno));
    }
}

void os_isync(usbdevice* kb){
    struct input_event event;
    memset(&event, 0, sizeof(event));
    event.type = EV_SYN;
    event.code = SYN_REPORT;
    if(write(kb->uinput, &event, sizeof(event)) <= 0)
        ckb_warn("uinput write failed: %s\n", strerror(errno));
}

void os_updateindicators(usbdevice* kb, int force){
    // Read LED events from the device
    uchar ileds = kb->ev_ileds;
    struct input_event event;
    while(read(kb->uinput, &event, sizeof(event)) > 0){
        if(event.type == EV_LED && event.code < 8){
            char which = 1 << event.code;
            if(event.value)
                ileds |= which;
            else
                ileds &= ~which;
        }
    }
    kb->ev_ileds = ileds;
    // Update them if needed
    if(kb->active){
        usbmode* mode = kb->profile->currentmode;
        ileds = (ileds & ~mode->ioff) | mode->ion;
    }
    if(force || ileds != kb->ileds){
        kb->ileds = ileds;
        DELAY_SHORT(kb);
        struct usbdevfs_ctrltransfer transfer = { 0x21, 0x09, 0x0200, 0x00, 1, 5000, &kb->ileds };
        ioctl(kb->handle, USBDEVFS_CONTROL, &transfer);
    }
}

#endif

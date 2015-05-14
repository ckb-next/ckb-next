#include "device.h"
#include "input.h"

#ifdef OS_LINUX

// Get event device. Needed to listen to indicator LEDs (caps lock, etc)
static int evopen(int uinput, const char* indevname){
    char uiname[256] = { 0 }, uipath[FILENAME_MAX] = { 0 };
    const char* uidevbase = "/sys/devices/virtual/input";
#if UINPUT_VERSION >= 4
    if(ioctl(uinput, UI_GET_SYSNAME(256), uiname) >= 0)
        snprintf(uipath, FILENAME_MAX, "%s/%s", uidevbase, uiname);
    else {
#endif
        // If the system's version of uinput doesn't support getting the device name, or if it failed, scan the directory for this device
        DIR* uidir = opendir(uidevbase);
        if(!uidir){
            ckb_warn("Couldn't open uinput base path: %s\n", strerror(errno));
            return 0;
        }
        struct dirent* uifile;
        while((uifile = readdir(uidir))){
            int uilength;
            snprintf(uipath, FILENAME_MAX, "%s/%s%n/name", uidevbase, uifile->d_name, &uilength);
            int namefd = open(uipath, O_RDONLY);
            if(namefd > 0){
                char name[10] = { 0 }, trimmedname[10] = { 0 };
                ssize_t len = read(namefd, name, 9);
                sscanf(name, "%9s", trimmedname);
                if(len >= 0 && !strcmp(trimmedname, indevname)){
                    uipath[uilength] = 0;
                    strcpy(uiname, uifile->d_name);
                    break;
                }
                close(namefd);
            }
            uipath[0] = 0;
        }
        closedir(uidir);
#if UINPUT_VERSION >= 4
    }
#endif
    if(strlen(uipath) == 0)
        return 0;
    // Look in the uinput directory for a file named event*
    DIR* dir = opendir(uipath);
    if(!dir){
        ckb_warn("Couldn't open uinput path: %s\n", strerror(errno));
        return 0;
    }
    int eid = -1;
    struct dirent* file;
    while((file = readdir(dir))){
        if(sscanf(file->d_name, "event%d", &eid) == 1)
            break;
    }
    closedir(dir);
    if(eid < 0){
        ckb_warn("Couldn't find event at uinput path\n");
        return 0;
    }
    // Open the corresponding device in /dev/input
    snprintf(uipath, FILENAME_MAX, "/dev/input/event%d", eid);
    int fd = open(uipath, O_RDONLY | O_NONBLOCK);
    if(fd <= 0){
        ckb_warn("Couldn't open event device: %s\n", strerror(errno));
        return 0;
    }
    return fd;
}

int uinputopen(struct uinput_user_dev* indev, int* event){
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if(fd <= 0){
        // If that didn't work, try /dev/input/uinput instead
        fd = open("/dev/input/uinput", O_WRONLY | O_NONBLOCK);
        if(fd <= 0){
            ckb_err("Failed to open uinput: %s\n", strerror(errno));
            *event = 0;
            return 0;
        }
    }
    // Enable all supported keys, LEDs, and mouse axes
    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    for(int i = 0; i < KEY_CNT; i++)
        ioctl(fd, UI_SET_KEYBIT, i);
    ioctl(fd, UI_SET_EVBIT, EV_LED);
    for(int i = 0; i < LED_CNT; i++)
        ioctl(fd, UI_SET_LEDBIT, i);
    ioctl(fd, UI_SET_EVBIT, EV_REL);
    for(int i = 0; i < REL_CNT; i++)
        ioctl(fd, UI_SET_RELBIT, i);
    // Enable sychronization events
    ioctl(fd, UI_SET_EVBIT, EV_SYN);
    if(write(fd, indev, sizeof(*indev)) <= 0)
        ckb_warn("uinput write failed: %s\n", strerror(errno));
    if(ioctl(fd, UI_DEV_CREATE)){
        ckb_err("Failed to create uinput device: %s\n", strerror(errno));
        close(fd);
        *event = 0;
        return 0;
    }
    *event = evopen(fd, indev->name);
    return fd;
}

int os_inputopen(usbdevice* kb){
    // Create the new input device
    int index = INDEX_OF(kb, keyboard);
    struct uinput_user_dev indev;
    memset(&indev, 0, sizeof(indev));
    snprintf(indev.name, UINPUT_MAX_NAME_SIZE, "ckb%d", index);
    indev.id.bustype = BUS_USB;
    indev.id.vendor = keyboard[index].vendor;
    indev.id.product = keyboard[index].product;
    indev.id.version = (UINPUT_VERSION > 4 ? 4 : UINPUT_VERSION);
    int event;
    int fd = uinputopen(&indev, &event);
    if(fd && !event)
        ckb_warn("No event device found. Indicator lights will be disabled\n");
    keyboard[index].uinput = fd;
    keyboard[index].event = event;
    return fd <= 0;
}

void os_inputclose(usbdevice* kb){
    if(kb->uinput <= 0)
        return;
    close(kb->event);
    kb->event = 0;
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
    // Read the indicator LEDs for this device and update them if necessary.
    char leds[LED_CNT / 8] = { 0 };
    ioctl(kb->event, EVIOCGLED(sizeof(leds)), &leds);
    char ileds = leds[0];
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

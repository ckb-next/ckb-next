#include "device.h"
#include "input.h"

#ifdef OS_LINUX

int uinputopen(struct uinput_user_dev* indev, int* event){
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if(fd <= 0){
        // If that didn't work, try /dev/input/uinput instead
        fd = open("/dev/input/uinput", O_WRONLY | O_NONBLOCK);
        if(fd <= 0){
            printf("Error: Failed to open uinput: %s\n", strerror(errno));
            return fd;
        }
    }
    // Set up as a keyboard device and enable all keys as well as all LEDs
    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    ioctl(fd, UI_SET_EVBIT, EV_LED);
    ioctl(fd, UI_SET_EVBIT, EV_SYN);
    for(int i = 0; i < 256; i++)
        ioctl(fd, UI_SET_KEYBIT, i);
    for(int i = 0; i < LED_CNT; i++)
        ioctl(fd, UI_SET_LEDBIT, i);
    if(write(fd, indev, sizeof(*indev)) <= 0)
        printf("Write error: %s\n", strerror(errno));
    if(ioctl(fd, UI_DEV_CREATE)){
        printf("Error: Failed to create uinput device: %s\n", strerror(errno));
        close(fd);
        return -1;
    }
    // Get event device. Needed to listen to indicator LEDs (caps lock, etc)
    char uiname[256] = { 0 }, uipath[FILENAME_MAX] = { 0 };
    const char* uidevbase = "/sys/devices/virtual/input";
#if UINPUT_VERSION >= 4
    if(ioctl(fd, UI_GET_SYSNAME(256), uiname) >= 0)
        snprintf(uipath, FILENAME_MAX, "%s/%s", uidevbase, uiname);
    else {
#endif
        // If the system's version of uinput doesn't support getting the device name, or if it failed, scan the directory for this device
        DIR* uidir = opendir(uidevbase);
        if(!uidir){
            printf("Warning: Couldn't open virtual device path: %s\n", strerror(errno));
            return fd;
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
                if(len >= 0 && !strcmp(trimmedname, indev->name)){
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
    if(strlen(uipath) > 0){
        // Look in the uinput directory for a file named event*
        DIR* dir = opendir(uipath);
        if(!dir){
            printf("Warning: Couldn't open uinput path: %s\n", strerror(errno));
            return fd;
        }
        int eid = -1;
        struct dirent* file;
        while((file = readdir(dir))){
            if(sscanf(file->d_name, "event%d", &eid) == 1)
                break;
        }
        closedir(dir);
        if(eid < 0){
            printf("Warning: Couldn't find event at uinput path\n");
            return fd;
        }
        // Open the corresponding device in /dev/input
        snprintf(uipath, FILENAME_MAX, "/dev/input/event%d", eid);
        int fd2 = open(uipath, O_RDONLY | O_NONBLOCK);
        if(fd2 <= 0){
            printf("Warning: Couldn't open event device: %s\n", strerror(errno));
            return fd;
        }
        *event = fd2;
    }
    return fd;
}

int inputopen(usbdevice* kb){
    // Create the new input device
    int index = INDEX_OF(kb, keyboard);
    struct uinput_user_dev indev;
    memset(&indev, 0, sizeof(indev));
    snprintf(indev.name, UINPUT_MAX_NAME_SIZE, "ckb%d", index);
    indev.id.bustype = BUS_USB;
    indev.id.vendor = V_CORSAIR;
    indev.id.product = (keyboard[index].model == 95 ? P_K95 : P_K70);
    indev.id.version = (UINPUT_VERSION > 4 ? 4 : UINPUT_VERSION);
    int event;
    int fd = uinputopen(&indev, &event);
    if(fd <= 0){
        keyboard[index].uinput = keyboard[index].event = 0;
        return 0;
    }
    keyboard[index].uinput = fd;
    if(event <= 0){
        printf("No event device found. Indicator lights will be disabled\n");
        keyboard[index].event = 0;
    } else
        keyboard[index].event = event;
    return 1;
}

void inputclose(usbdevice* kb){
    if(kb->uinput <= 0)
        return;
    close(kb->event);
    kb->event = 0;
    // Set all keys released
    struct input_event event;
    memset(&event, 0, sizeof(event));
    event.type = EV_KEY;
    for(int key = 0; key < 256; key++){
        event.code = key;
        if(write(kb->uinput, &event, sizeof(event)) <= 0)
            printf("Write error: %s\n", strerror(errno));
    }
    event.type = EV_SYN;
    event.code = SYN_REPORT;
    if(write(kb->uinput, &event, sizeof(event)) <= 0)
        printf("Write error: %s\n", strerror(errno));
    // Close the device
    ioctl(kb->uinput, UI_DEV_DESTROY);
    close(kb->uinput);
    kb->uinput = 0;
}

void os_keypress(usbdevice* kb, int scancode, int down){
    struct input_event event;
    memset(&event, 0, sizeof(event));
    event.type = EV_KEY;
    event.code = scancode;
    event.value = down;
    if(write(kb->uinput, &event, sizeof(event)) <= 0)
        printf("Write error: %s\n", strerror(errno));
}

void os_kpsync(usbdevice* kb){
    struct input_event event;
    memset(&event, 0, sizeof(event));
    event.type = EV_SYN;
    event.code = SYN_REPORT;
    if(write(kb->uinput, &event, sizeof(event)) <= 0)
        printf("Write error: %s\n", strerror(errno));
}

void os_updateindicators(usbdevice* kb, int force){
    if(!IS_CONNECTED(kb) || NEEDS_FW_UPDATE(kb))
        return;
    // Read the indicator LEDs for this device and update them if necessary.
    char leds[LED_CNT / 8] = { 0 };
    ioctl(kb->event, EVIOCGLED(sizeof(leds)), &leds);
    char ileds = leds[0];
    usbmode* mode = kb->profile.currentmode;
    if(mode && kb->active)
        ileds = (ileds & ~mode->ioff) | mode->ion;
    if(force || ileds != kb->ileds){
        kb->ileds = ileds;
        struct usbdevfs_ctrltransfer transfer = { 0x21, 0x09, 0x0200, 0x00, 1, 500, &kb->ileds };
        ioctl(kb->handle, USBDEVFS_CONTROL, &transfer);
    }
}

#endif

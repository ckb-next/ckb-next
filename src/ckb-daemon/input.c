#include "usb.h"
#include "input.h"

int uinputopen(int index, const struct libusb_device_descriptor* descriptor){
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if(fd <= 0){
        // If that didn't work, try /dev/input/uinput instead
        fd = open("/dev/input/uinput", O_WRONLY | O_NONBLOCK);
        if(fd <= 0){
            printf("Error: Failed to open uinput: %s\n", strerror(errno));
            return fd;
        }
    }
    printf("Setting up uinput device ckb%d\n", index);
    // Set up as a keyboard device and enable all keys as well as all LEDs
    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    ioctl(fd, UI_SET_EVBIT, EV_LED);
    ioctl(fd, UI_SET_EVBIT, EV_SYN);
    for(int i = 0; i < 256; i++)
        ioctl(fd, UI_SET_KEYBIT, i);
    for(int i = 0; i < LED_CNT; i++)
        ioctl(fd, UI_SET_LEDBIT, i);
    // Create the new input device
    struct uinput_user_dev indev;
    memset(&indev, 0, sizeof(indev));
    snprintf(indev.name, UINPUT_MAX_NAME_SIZE, "ckb%d", index);
    indev.id.bustype = BUS_USB;
    indev.id.vendor = descriptor->idVendor;
    indev.id.product = descriptor->idProduct;
    indev.id.version = (UINPUT_VERSION > 4 ? 4 : UINPUT_VERSION);
    if(write(fd, &indev, sizeof(indev)) <= 0)
        printf("Write error: %s\n", strerror(errno));
    if(ioctl(fd, UI_DEV_CREATE)){
        printf("Error: Failed to create uinput device: %s\n", strerror(errno));
        close(fd);
        return -1;
    }
    // Get event device. Needed to listen to indicator LEDs (caps lock, etc)
    char uiname[256], uipath[FILENAME_MAX];
    const char* uidevbase = "/sys/devices/virtual/input";
#if UINPUT_VERSION >= 4
    if(ioctl(fd, UI_GET_SYSNAME(256), uiname) >= 0)
        snprintf(uipath, FILENAME_MAX, "%s/%s", uidevbase, uiname);
    else {
        printf("Warning: Couldn't get uinput path (trying workaround): %s\n", strerror(errno));
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
                if(len >= 0 && !strcmp(trimmedname, indev.name)){
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
        printf("%s/%s set up\n", uidevbase, uiname);
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
        keyboard[index].event = fd2;
        printf("Opened /dev/input/event%d\n", eid);
    }
    return fd;
}

void uinputclose(int index){
    usbdevice* kb = keyboard + index;
    if(kb->uinput <= 0)
        return;
    printf("Closing uinput device %d\n", index);
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

void uinputupdate(usbdevice* kb){
    for(int byte = 0; byte < N_KEYS / 8; byte++){
        char oldb = kb->previntinput[byte], newb = kb->intinput[byte];
        if(oldb == newb)
            continue;
        for(int bit = 0; bit < 8; bit++){
            key* map = keymap + byte * 8 + bit;
            char mask = 1 << bit;
            char old = oldb & mask, new = newb & mask;
            // If the key state changed, send it to the uinput device
            if(old != new && map->scan != 0){
                struct input_event event;
                memset(&event, 0, sizeof(event));
                event.type = EV_KEY;
                event.code = map->scan;
                event.value = !!new;
                if(write(kb->uinput, &event, sizeof(event)) <= 0)
                    printf("Write error: %s\n", strerror(errno));
                // The volume wheel doesn't generate keyups, so create them automatically
                if(map->scan == KEY_VOLUMEUP || map->scan == KEY_VOLUMEDOWN){
                    kb->intinput[byte] &= ~mask;
                    event.type = EV_KEY;
                    event.code = map->scan;
                    event.value = 0;
                    if(write(kb->uinput, &event, sizeof(event)) <= 0)
                        printf("Write error: %s\n", strerror(errno));
                }
                event.type = EV_SYN;
                event.code = SYN_REPORT;
                event.value = 0;
                if(write(kb->uinput, &event, sizeof(event)) <= 0)
                    printf("Write error: %s\n", strerror(errno));
            }
        }
    }
    memcpy(kb->previntinput, kb->intinput, N_KEYS / 8);
}

void updateindicators(usbdevice* kb, int force){
    // Read the indicator LEDs for this device and update them if necessary.
    if(!kb->handle)
        return;
    if(kb->event <= 0){
        if(force){
            kb->ileds = 0;
            libusb_control_transfer(kb->handle, 0x21, 0x09, 0x0200, 0, &kb->ileds, 1, 0);
        }
        return;
    }
    char leds[LED_CNT / 8] = { 0 };
    if(ioctl(kb->event, EVIOCGLED(sizeof(leds)), &leds)){
        char ileds = leds[0];
        if(ileds != kb->ileds || force){
            kb->ileds = ileds;
            libusb_control_transfer(kb->handle, 0x21, 0x09, 0x0200, 0, &ileds, 1, 0);
        }
    }
}

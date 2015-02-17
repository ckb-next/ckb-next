#include "device.h"
#include "devnode.h"
#include "input.h"
#include "notify.h"
#include "usb.h"

#ifdef OS_MAC

#define INCOMPLETE (IOHIDDeviceRef)-1l

int _usbdequeue(usbdevice* kb, const char* file, int line){
    if(kb->queuecount == 0 || !kb->handle)
        return -1;
    IOReturn res = IOHIDDeviceSetReport(kb->handle, kIOHIDReportTypeFeature, 0, kb->queue[0], MSG_SIZE);
    // Rotate queue
    uchar* first = kb->queue[0];
    for(int i = 1; i < QUEUE_LEN; i++)
        kb->queue[i - 1] = kb->queue[i];
    kb->queue[QUEUE_LEN - 1] = first;
    kb->queuecount--;
    kb->lastError = res;
    if(res != kIOReturnSuccess){
        printf("Error: usbdequeue (%s:%d): Got return value 0x%x\n", file, line, res);
        return 0;
    }
    return MSG_SIZE;
}

int _usbinput(usbdevice* kb, uchar* message, const char* file, int line){
    if(!IS_ACTIVE(kb))
        return -1;
    CFIndex length = MSG_SIZE;
    IOReturn res = IOHIDDeviceGetReport(kb->handle, kIOHIDReportTypeFeature, 0, message, &length);
    kb->lastError = res;
    if(res != kIOReturnSuccess){
        printf("Error: usbinput (%s:%d): Got return value 0x%x\n", file, line, res);
        return 0;
    }
    if(length != MSG_SIZE)
        printf("Warning: usbinput (%s:%d): Read %d bytes (expected %d)\n", file, line, (int)length, MSG_SIZE);
    return length;
}

void closehandle(usbdevice* kb){
    kb->handle = 0;
    int count = (IS_RGB(kb->vendor, kb->product)) ? 4 : 3;
    for(int i = 0; i < count; i++)
        kb->handles[i] = 0;
}

int os_resetusb(usbdevice* kb, const char* file, int line){
    // Don't try if the keyboard was disconnected
    if(kb->lastError == kIOReturnBadArgument)
        return -2;
    // USB reset via IOHIDDevice doesn't seem to be possible.
    // Just wait a little and then return success anyway...
    DELAY_LONG;
    return 0;
}

void usbremove(void* context, IOReturn result, void* sender){
    pthread_mutex_lock(&kblistmutex);
    usbdevice* kb = context;
    for(int i = 0; i < DEV_MAX; i++){
        if(keyboard + i == kb){
            pthread_mutex_lock(&keyboard[i].mutex);
            closeusb(keyboard + i);
        }
    }
    pthread_mutex_unlock(&kblistmutex);
}

void reportcallback(void* context, IOReturn result, void* sender, IOHIDReportType reporttype, uint32_t reportid, uint8_t* data, CFIndex length){
    usbdevice* kb = context;
    if(!HAS_FEATURES(kb, FEAT_RGB)){
        // For non RGB keyboards, translate HID input
        switch(length){
        case 8:
            hid_translate(kb->kbinput, 1, 8, kb->urbinput);
            break;
        case 4:
            hid_translate(kb->kbinput, 2, 4, kb->urbinput + 8);
            break;
        case 15:
            hid_translate(kb->kbinput, 3, 15, kb->urbinput + 8 + 4);
            break;
        }
        inputupdate(kb);
    } else {
        // For RGB keyboards, process Corsair input or BIOS input
        if(length == 8){
            hid_translate(kb->kbinput, 1, 8, kb->urbinput);
            inputupdate(kb);
        } else if(length == MSG_SIZE)
            inputupdate(kb);
    }
}

void openusb(usbdevice* kb, short vendor, short product){
    // The driver sometimes isn't completely ready yet, so give it a short delay
    sleep(1);

    kb->lastkeypress = -1;
    if(IS_RGB(vendor, product))
        // Handle 3 is the control handle
        kb->handle = kb->handles[3];
    else
        // Non RGB keyboards don't have one, so just use 0
        kb->handle = kb->handles[0];

    // Set up the device
    int setup = setupusb(kb, vendor, product);
    if(setup == -1 || (setup && usb_tryreset(kb))){
        closehandle(kb);
        pthread_mutex_unlock(&kb->mutex);
        pthread_mutex_destroy(&kb->mutex);
        pthread_mutex_destroy(&kb->keymutex);
        return;
    }

    // Start handling HID reports for the input
    IOHIDDeviceRegisterInputReportCallback(kb->handles[0], kb->urbinput, 8, reportcallback, kb);
    if(IS_RGB(vendor, product))
        IOHIDDeviceRegisterInputReportCallback(kb->handles[1], kb->urbinput + 8, 21, reportcallback, kb);
    else
        IOHIDDeviceRegisterInputReportCallback(kb->handles[1], kb->urbinput + 8, 4, reportcallback, kb);
    if(IS_RGB(vendor, product))
        IOHIDDeviceRegisterInputReportCallback(kb->handles[2], kb->kbinput, MSG_SIZE, reportcallback, kb);
    else
        IOHIDDeviceRegisterInputReportCallback(kb->handles[2], kb->urbinput + 8 + 4, 15, reportcallback, kb);

    // Register for close notification
    IOHIDDeviceRegisterRemovalCallback(kb->handle, usbremove, kb);

    // Update connected
    updateconnected();
    notifyconnect(kb, 1);
    int index = INDEX_OF(kb, keyboard);
    printf("Device ready at %s%d\n", devpath, index);
    pthread_mutex_unlock(&kb->mutex);
}

long usbgetvalue(IOHIDDeviceRef device, CFStringRef key){
    long res = 0;
    CFTypeRef output = IOHIDDeviceGetProperty(device, key);
    if(output && CFGetTypeID(output) == CFNumberGetTypeID() && CFNumberGetValue(output, kCFNumberLongType, &res))
        return res;
    return 0;
}

void usbadd(void* context, IOReturn result, void* sender, IOHIDDeviceRef device){
    if(CFGetTypeID(device) != IOHIDDeviceGetTypeID())
        return;
    // Get the model and serial number
    long idproduct = usbgetvalue(device, CFSTR(kIOHIDProductIDKey));
    char serial[SERIAL_LEN];
    CFTypeRef cfserial = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDSerialNumberKey));
    if(!cfserial || CFGetTypeID(cfserial) != CFStringGetTypeID() || !CFStringGetCString(cfserial, serial, SERIAL_LEN, kCFStringEncodingASCII))
        return;
    pthread_mutex_lock(&kblistmutex);
    // A single keyboard will generate 4 match events, so each handle has to be added to the board separately.
    // Look for any partially-set up boards matching this serial number
    int index = -1;
    for(int i = 1; i < DEV_MAX; i++){
        if(!strcmp(keyboard[i].profile.serial, serial) && keyboard[i].handle == INCOMPLETE){
            index = i;
            break;
        }
    }
    // If none was found, grab the first free device
    if(index == -1){
        for(int i = 1; i < DEV_MAX; i++){
            if(!keyboard[i].handle){
                // Mark the device as in use and print out a message
                index = i;
                keyboard[i].handle = INCOMPLETE;
                strcpy(keyboard[i].profile.serial, serial);
                CFTypeRef cfname = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductKey));
                if(cfname && CFGetTypeID(cfname) == CFStringGetTypeID())
                    CFStringGetCString(cfname, keyboard[i].name, NAME_LEN, kCFStringEncodingASCII);
                printf("Connecting %s (S/N: %s)\n", keyboard[i].name, keyboard[i].profile.serial);
                break;
            }
        }
    }
    if(index == -1){
        printf("Error: No free devices\n");
        pthread_mutex_unlock(&kblistmutex);
        return;
    }
    usbdevice* kb = keyboard + index;
    // There's no direct way to tell which of the four handles this is, but there's a workaround
    // Each handle has a unique maximum packet size combination, so use that to place them
    long input = usbgetvalue(device, CFSTR(kIOHIDMaxInputReportSizeKey));
    long output = usbgetvalue(device, CFSTR(kIOHIDMaxOutputReportSizeKey));
    long feature = usbgetvalue(device, CFSTR(kIOHIDMaxFeatureReportSizeKey));

    // Handle 0 is for BIOS mode/non-RGB key input
    if(input == 8 && output == 1 && feature == 0)
        kb->handles[0] = device;
    // Handle 1 is for standard HID key input
    else if((input == 21 || input == 4) && output == 1 && feature == 1)
        kb->handles[1] = device;
    // Handle 2 is for Corsair inputs
    else if((input == 64 || input == 15) && output == 0 && feature == 0)
        kb->handles[2] = device;
    // Handle 3 is for controlling the device
    else if(input == 0 && output == 0 && feature == 64)
        kb->handles[3] = device;
    else
        printf("Warning: Got unknown handle (I: %d, O: %d, F: %d)\n", (int)input, (int)output, (int)feature);

    // If all handles have been set up, finish initializing the keyboard
    if(kb->handles[0] && kb->handles[1] && kb->handles[2] && (kb->handles[3] || !IS_RGB(V_CORSAIR, (short)idproduct)))
        openusb(kb, V_CORSAIR, (short)idproduct);
    pthread_mutex_unlock(&kblistmutex);
}

static IOHIDManagerRef usbmanager;
static pthread_t usbthread;
static pthread_t keyrepeatthread;
static CFMachPortRef eventTap;

CGEventRef tapcallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void* refcon){
    if(type == kCGEventTapDisabledByUserInput || type == kCGEventTapDisabledByTimeout){
        // Re-enable the tap if it gets disabled
        CGEventTapEnable(eventTap, true);
        return 0;
    }
    if((type == kCGEventKeyDown || type == kCGEventKeyUp)){
        CGEventFlags flags = CGEventGetFlags(event);
        // This flag gets inserted into all of our keyboard events automatically. It can't be removed when the event is broadcast.
        // It must be removed or else Cmd+Option+Esc, but ONLY Cmd+Option+Esc, fails to work.
        //  LOGIC! 
        flags &= ~0x20000000;
        CGEventSetFlags(event, flags);
    } else {
        CGEventFlags flags = CGEventGetFlags(event);
        for(int i = 1; i < DEV_MAX; i++)
            flags |= keyboard[i].eventflags;
        CGEventSetFlags(event, flags);
    }
    return event;
}

extern long keyrepeatinterval();
extern void keyretrigger(usbdevice*, int);

void* krthread(void* context){
    while(1){
        // Re-scan every 1ms
        usleep(1000);
        long interval = keyrepeatinterval();
        struct timespec time;
        clock_gettime(CLOCK_MONOTONIC, &time);
        for(int i = 1; i < DEV_MAX; i++){
            if(IS_ACTIVE(keyboard + i)){
                pthread_mutex_lock(&keyboard[i].keymutex);
                if(keyboard[i].lastkeypress != KEY_NONE){
                    // Repeat the key as many times as needed to catch up
                    while(timespec_ge(time, keyboard[i].keyrepeat)){
                        keyretrigger(keyboard + i, keyboard[i].lastkeypress);
                        timespec_add(&keyboard[i].keyrepeat, interval);
                    }
                }
                pthread_mutex_unlock(&keyboard[i].keymutex);
            }
        }
    }
    return 0;
}

void* threadrun(void* context){
    // Tell it which devices we want to look for
    CFMutableArrayRef devices = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
    if(devices){
        int vendor = V_CORSAIR;
        int products[] = { P_K65, P_K70, P_K70_NRGB, P_K95 };
        for(int i = 0; i < sizeof(products) / sizeof(int); i++){
            int product = products[i];
            CFMutableDictionaryRef device = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
            if(device){
                CFDictionarySetValue(device, CFSTR(kIOHIDVendorIDKey), CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &vendor));
                CFDictionarySetValue(device, CFSTR(kIOHIDProductIDKey), CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &product));
                CFArrayAppendValue(devices, device);
                CFRelease(device);
            }
        }
        IOHIDManagerSetDeviceMatchingMultiple(usbmanager, devices);
        CFRelease(devices);
    }

    // Set up device add callback
    IOHIDManagerRegisterDeviceMatchingCallback(usbmanager, usbadd, 0);
    IOHIDManagerScheduleWithRunLoop(usbmanager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    IOHIDManagerOpen(usbmanager, kIOHIDOptionsTypeSeizeDevice);

    // Run an event tap to modify the state of mouse events. The OS won't take care of this for us so this is needed for Shift and other modifiers to work
    while(!eventTap){
        // When the daemon is run at boot the event tap might not work immediately, so wait until it does.
        usleep(0);
        eventTap = CGEventTapCreate(kCGHIDEventTap, kCGHeadInsertEventTap, kCGEventTapOptionDefault, CGEventMaskBit(kCGEventKeyDown) | CGEventMaskBit(kCGEventKeyUp) | CGEventMaskBit(kCGEventLeftMouseDown) | CGEventMaskBit(kCGEventLeftMouseDragged) | CGEventMaskBit(kCGEventLeftMouseUp) | CGEventMaskBit(kCGEventRightMouseDown) | CGEventMaskBit(kCGEventRightMouseDragged) | CGEventMaskBit(kCGEventRightMouseUp), tapcallback, 0);
    }
    CFRunLoopAddSource(CFRunLoopGetCurrent(), CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0), kCFRunLoopDefaultMode);

    // Another thing the OS won't do on its own: key repeats. Make a new thread for that
    pthread_create(&keyrepeatthread, 0, krthread, 0);

    // Run the event loop. Existing devices will be detected automatically.
    while(1){
        CFRunLoopRun();
    }
    return 0;
}

int usbinit(){
    // Create the device manager
    if(!(usbmanager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeSeizeDevice))){
        printf("Fatal: Failed to create device manager\n");
        return -1;
    }
    // Start the device thread
    if(pthread_create(&usbthread, 0, threadrun, 0)){
        printf("Fatal: Failed to start USB thread\n");
        return -1;
    }
    return 0;
}

void usbdeinit(){
    IOHIDManagerClose(usbmanager, kIOHIDOptionsTypeNone);
}

#endif

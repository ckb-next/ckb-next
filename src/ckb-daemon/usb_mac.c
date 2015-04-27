#include "device.h"
#include "devnode.h"
#include "input.h"
#include "notify.h"
#include "usb.h"

#ifdef OS_MAC

#define INCOMPLETE (IOHIDDeviceRef)-1l

int _usbdequeue(usbdevice* kb, const char* file, int line){
    if(kb->queuecount == 0 || !kb->handle || !HAS_FEATURES(kb, FEAT_RGB))
        return -1;
    IOReturn res = IOHIDDeviceSetReport(kb->handle, kIOHIDReportTypeFeature, 0, kb->queue[0], MSG_SIZE);
    // Rotate queue
    uchar* first = kb->queue[0];
    for(int i = 1; i < QUEUE_LEN; i++)
        kb->queue[i - 1] = kb->queue[i];
    kb->queue[QUEUE_LEN - 1] = first;
    kb->queuecount--;
    kb->lastError = res;
    if(res != kIOReturnSuccess && res != 0xe0004051){   // Can't find e0004051 documented, but it seems to be a harmless error, so ignore it.
        printf("usbdequeue (%s:%d): Got return value 0x%x\n", file, line, res);
        return 0;
    }
    return MSG_SIZE;
}

int _usbinput(usbdevice* kb, uchar* message, const char* file, int line){
    if(!IS_CONNECTED(kb) || !HAS_FEATURES(kb, FEAT_RGB))
        return -1;
    CFIndex length = MSG_SIZE;
    IOReturn res = IOHIDDeviceGetReport(kb->handle, kIOHIDReportTypeFeature, 0, message, &length);
    kb->lastError = res;
    if(res != kIOReturnSuccess && res != 0xe0004051){
        printf("usbinput (%s:%d): Got return value 0x%x\n", file, line, res);
        return 0;
    }
    if(length != MSG_SIZE)
        printf("usbinput (%s:%d): Read %d bytes (expected %d)\n", file, line, (int)length, MSG_SIZE);
    return length;
}

int _nk95cmd(usbdevice* kb, uchar bRequest, ushort wValue, const char* file, int line){
    // TODO: stub
    return 0;
}

void closehandle(usbdevice* kb){
    kb->handle = 0;
    int count = (IS_RGB(kb->vendor, kb->product)) ? 4 : 3;
    for(int i = 0; i < count; i++){
        //IOHIDDeviceClose(kb->handles[i], kIOHIDOptionsTypeSeizeDevice);
        // ^ This is the right thing to do only when shutting down the daemon. If done on device disconnect, it causes a crash.
        // Technically a memory leak, but one that only happens on shutdown, so no harm done.
        kb->handles[i] = 0;
    }
}

int os_resetusb(usbdevice* kb, const char* file, int line){
    // Don't try if the keyboard was disconnected
    if(kb->lastError == kIOReturnBadArgument || kb->lastError == kIOReturnNotOpen)
        return -2;
    // This does more harm than good...
    //if(!IOHIDDeviceSetProperty(kb->handle, CFSTR(kIOHIDResetKey), kCFBooleanTrue))
    //    return -1;
    sleep(1);
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
    if(HAS_FEATURES(kb, FEAT_RGB)){
        switch(length){
        case 8:
            // RGB EP 1: 6KRO (BIOS mode) input
            hid_translate(kb->kbinput, -1, 8, kb->urbinput);
            inputupdate(kb);
            break;
        case 21:
        case 5:
            // RGB EP 2: NKRO (non-BIOS) input. Accept only if keyboard is inactive
            if(!kb->active){
                hid_translate(kb->kbinput, -2, 21, kb->urbinput + 8);
                inputupdate(kb);
            }
            break;
        case MSG_SIZE:
            // RGB EP 3: Corsair input
            inputupdate(kb);
            break;
        }
    } else {
        switch(length){
        case 8:
            // Non-RGB EP 1: 6KRO input
            hid_translate(kb->kbinput, 1, 8, kb->urbinput);
            inputupdate(kb);
            break;
        case 4:
            // Non-RGB EP 2: media keys
            hid_translate(kb->kbinput, 2, 4, kb->urbinput + 8);
            inputupdate(kb);
            break;
        case 15:
            // Non-RGB EP 3: NKRO input
            hid_translate(kb->kbinput, 3, 15, kb->urbinput + 8 + 4);
            inputupdate(kb);
            break;
        }
    }
}

void openusb(usbdevice* kb, short vendor, short product){
    kb->lastkeypress = KEY_NONE;
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
    long idvendor = V_CORSAIR, idproduct = usbgetvalue(device, CFSTR(kIOHIDProductIDKey));
    char serial[SERIAL_LEN];
    CFTypeRef cfserial = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDSerialNumberKey));
    if(!cfserial || CFGetTypeID(cfserial) != CFStringGetTypeID() || !CFStringGetCString(cfserial, serial, SERIAL_LEN, kCFStringEncodingASCII))
        // If the serial can't be read, make one up
        snprintf(serial, SERIAL_LEN, "%04x:%x04-NoID", (uint)idvendor, (uint)idproduct);

    // For non-RGB models, get the firmware version here as well
    long fwversion = 0;
    if(!IS_RGB(idvendor, idproduct))
        fwversion = usbgetvalue(device, CFSTR(kIOHIDVersionNumberKey));

    pthread_mutex_lock(&kblistmutex);
    // A single keyboard will generate multiple match events, so each handle has to be added to the board separately.
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
                keyboard[i].fwversion = fwversion;
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

    // Handle 0 is for BIOS mode input (RGB) or non-RGB key input
    if(input == 8 && output == 1 && feature == 0)
        kb->handles[0] = device;
    // Handle 1 is for standard HID input (RGB) or media keys (non-RGB)
    else if((input == 21 && output == 1 && feature == 1)
            || (input == 4 && output == 0 && feature == 0))
        kb->handles[1] = device;
    // Handle 2 is for Corsair inputs, unused on non-RGB
    else if(((input == 64 || input == 15) && output == 0 && feature == 0)
            || (input == 64 && output == 64 && feature == 0))           // FW >= 1.20
        kb->handles[2] = device;
    // Handle 3 is for controlling the device (only exists for RGB)
    else if((input == 0 && output == 0 && feature == 64)
            || (input == 64 && output == 64 && feature == 64))          // FW >= 1.20
        kb->handles[3] = device;
    else
        printf("Warning: Got unknown handle (I: %d, O: %d, F: %d)\n", (int)input, (int)output, (int)feature);

    // If all handles have been set up, finish initializing the keyboard
    if(kb->handles[0] && kb->handles[1] && kb->handles[2]
            && (kb->handles[3] || !IS_RGB(idvendor, idproduct)))
        openusb(kb, (short)idvendor, (short)idproduct);
    pthread_mutex_unlock(&kblistmutex);
}

static IOHIDManagerRef usbmanager;
static pthread_t usbthread;
static pthread_t keyrepeatthread;
// input_mac.c
extern void* krthread(void* context);

void* threadrun(void* context){
    // Tell the device manager which devices we want to look for
    CFMutableArrayRef devices = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
    if(devices){
        int vendor = V_CORSAIR;
        int products[] = { P_K65, P_K70, P_K70_NRGB, P_K95, P_K95_NRGB };
        for(uint i = 0; i < sizeof(products) / sizeof(int); i++){
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

    // Make a new thread to handle key repeats. The OS won't do it for us.
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
    IOHIDManagerClose(usbmanager, kIOHIDOptionsTypeSeizeDevice);
}

#endif

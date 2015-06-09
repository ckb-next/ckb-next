#include "device.h"
#include "devnode.h"
#include "input.h"
#include "notify.h"
#include "usb.h"

#ifdef OS_MAC

#define INCOMPLETE (hid_dev_t)-1l

int os_usbsend(usbdevice* kb, uchar* messages, int count, const char* file, int line){
    for(int i = 0; i < count; i++){
        DELAY_SHORT(kb);
        // Firmware versions above 1.20 use Output instead of Feature reports for improved performance
        // However, it doesn't work well when retrieving the hardware profile, so use Feature reports until it's finished
        // TODO: figure out why? It doesn't have this problem on Linux
        IOHIDReportType type = (kb->fwversion >= 0x120 && kb->hw ? kIOHIDReportTypeOutput : kIOHIDReportTypeFeature);
        kern_return_t res = (*kb->handle)->setReport(kb->handle, type, 0, messages + i * MSG_SIZE, MSG_SIZE, 5000, 0, 0, 0);
        kb->lastresult = res;
        if(res != kIOReturnSuccess){
            ckb_err_fn("Got return value 0x%x\n", file, line, res);
            return 0;
        }
    }
    return MSG_SIZE * count;
}

int os_usbrecv(usbdevice* kb, uchar* message, const char* file, int line){
    DELAY_MEDIUM(kb);
    CFIndex length = MSG_SIZE;
    kern_return_t res = (*kb->handle)->getReport(kb->handle, kIOHIDReportTypeFeature, 0, message, &length, 5000, 0, 0, 0);
    kb->lastresult = res;
    if(res != kIOReturnSuccess){
        ckb_err_fn("Got return value 0x%x\n", file, line, res);
        return 0;
    }
    if(length != MSG_SIZE)
        ckb_err_fn("Read %d bytes (expected %d)\n", file, line, (int)length, MSG_SIZE);
    return length;
}

int _nk95cmd(usbdevice* kb, uchar bRequest, ushort wValue, const char* file, int line){
    // TODO: stub
    return 0;
}

int os_resetusb(usbdevice* kb, const char* file, int line){
    kern_return_t res = kb->lastresult;
    if(res == kIOReturnBadArgument || res == kIOReturnNotOpen || res == kIOReturnNotAttached)
        // Don't try if the keyboard was disconnected
        return -2;
    // Reset all handles
    int count = IS_RGB(kb->vendor, kb->product) ? 4 : 3;
    for(int i = 0; i < count; i++){
        if((*kb->handles[i])->setProperty(kb->handles[i], CFSTR(kIOHIDResetKey), kCFBooleanTrue) != kIOReturnSuccess)
            return -1;
    }
    return 0;
}

static void intreport(void* context, IOReturn result, void* sender, IOHIDReportType reporttype, uint32_t reportid, uint8_t* data, CFIndex length){
    usbdevice* kb = context;
    pthread_mutex_lock(imutex(kb));
    if(IS_MOUSE(kb->vendor, kb->product)){
        if(length == 10)
            hid_mouse_translate(kb->input.keys, &kb->input.rel_x, &kb->input.rel_y, -2, length, data);
    } else if(HAS_FEATURES(kb, FEAT_RGB)){
        switch(length){
        case 8:
            // RGB EP 1: 6KRO (BIOS mode) input
            hid_kb_translate(kb->input.keys, -1, length, data);
            break;
        case 21:
        case 5:
            // RGB EP 2: NKRO (non-BIOS) input. Accept only if keyboard is inactive
            if(!kb->active)
                hid_kb_translate(kb->input.keys, -2, length, data);
            break;
        case MSG_SIZE:
            // RGB EP 3: Corsair input
            memcpy(kb->input.keys, data, N_KEYBYTES_KB);
            break;
        }
    } else {
        switch(length){
        case 8:
            // Non-RGB EP 1: 6KRO input
            hid_kb_translate(kb->input.keys, 1, length, data);
            break;
        case 4:
            // Non-RGB EP 2: media keys
            hid_kb_translate(kb->input.keys, 2, length, data);
            break;
        case 15:
            // Non-RGB EP 3: NKRO input
            hid_kb_translate(kb->input.keys, 3, length, data);
            break;
        }
    }
    inputupdate(kb);
    pthread_mutex_unlock(imutex(kb));
}

// input_mac.c
extern void keyretrigger(usbdevice* kb);

void* os_inputmain(void* context){
    usbdevice* kb = context;
    int index = INDEX_OF(kb, keyboard);
    // Schedule async events for the device on this thread
    CFRunLoopRef runloop = CFRunLoopGetCurrent();
    int count = (IS_RGB(kb->vendor, kb->product)) ? 4 : 3;
    for(int i = 0; i < count; i++){
        CFTypeRef eventsource;
        kern_return_t res = (*kb->handles[i])->getAsyncEventSource(kb->handles[i], &eventsource);
        if(res != kIOReturnSuccess){
            ckb_err("Failed to start input thread for %s%d: %x\n", devpath, index, res);
            return 0;
        }
        if(CFGetTypeID(eventsource) == CFRunLoopSourceGetTypeID())
            CFRunLoopAddSource(runloop, (CFRunLoopSourceRef)eventsource, kCFRunLoopDefaultMode);
        else if(CFGetTypeID(eventsource) == CFRunLoopTimerGetTypeID())
            CFRunLoopAddTimer(runloop, (CFRunLoopTimerRef)eventsource, kCFRunLoopDefaultMode);
    }
    ckb_info("Starting input thread for %s%d\n", devpath, index);

    // Start getting reports
    uint8_t* urbinput[] = { malloc(8), malloc(32), malloc(MSG_SIZE) };
    (*kb->handles[0])->setInputReportCallback(kb->handles[0], urbinput[0], 8, intreport, kb, 0);
    if(IS_RGB(kb->vendor, kb->product)){
        (*kb->handles[1])->setInputReportCallback(kb->handles[1], urbinput[1], 21, intreport, kb, 0);
        (*kb->handles[2])->setInputReportCallback(kb->handles[2], urbinput[2], MSG_SIZE, intreport, kb, 0);
    } else {
        (*kb->handles[1])->setInputReportCallback(kb->handles[1], urbinput[1], 4, intreport, kb, 0);
        (*kb->handles[2])->setInputReportCallback(kb->handles[2], urbinput[2], 15, intreport, kb, 0);
    }

    // Run the run loop for up to 2ms at a time, then check for key repeats
    while(1){
        CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.002, false);
        pthread_mutex_lock(imutex(kb));
        if(!IS_CONNECTED(kb)){
            // Make sure the device hasn't disconnected
            pthread_mutex_unlock(imutex(kb));
            break;
        }
        keyretrigger(kb);
        pthread_mutex_unlock(imutex(kb));
    }

    // Clean up
    ckb_info("Stopping input thread for %s%d\n", devpath, index);
    free(urbinput[0]);
    free(urbinput[1]);
    free(urbinput[2]);
    return 0;
}

static long usbgetlong(hid_dev_t handle, CFStringRef key){
    long raw = 0;
    CFTypeRef cf;
    if((*handle)->getProperty(handle, key, &cf) != kIOReturnSuccess)
        return 0;
    if(!cf || CFGetTypeID(cf) != CFNumberGetTypeID() || !CFNumberGetValue(cf, kCFNumberLongType, &raw))
        return 0;
    return raw;
}

static void usbgetstr(hid_dev_t handle, CFStringRef key, char* output, int out_len){
    CFTypeRef cf;
    if((*handle)->getProperty(handle, key, &cf) != kIOReturnSuccess){
        output[0] = 0;
        return;
    }
    if(!cf || CFGetTypeID(cf) != CFStringGetTypeID() || !CFStringGetCString(cf, output, out_len, kCFStringEncodingASCII))
        output[0] = 0;
}

int os_setupusb(usbdevice* kb){
    kb->lastkeypress = KEY_NONE;
    // Handle 3 is the control handle
    if(IS_RGB(kb->vendor, kb->product))
        kb->handle = kb->handles[3];
    else
        // Non RGB keyboards don't have one, so just use 0
        kb->handle = kb->handles[0];

    // Get the device firmware version
    long fwversion = usbgetlong(kb->handle, CFSTR(kIOHIDVersionNumberKey));
    kb->fwversion = fwversion;
    return 0;
}

void os_closeusb(usbdevice* kb){
    kb->handle = 0;
    int count = (IS_RGB(kb->vendor, kb->product)) ? 4 : 3;
    for(int i = 0; i < count; i++){
        (*kb->handles[i])->Release(kb->handles[i]);
        kb->handles[i] = 0;
    }
}

usbdevice* usbadd(hid_dev_t handle, io_object_t** rm_notify){
    // Get the model and serial number
    long idvendor = V_CORSAIR, idproduct = usbgetlong(handle, CFSTR(kIOHIDProductIDKey));
    // Each keyboard generates multiple match events (one for each endpoint)
    // Use the location ID key to group the handles together
    long location = usbgetlong(handle, CFSTR(kIOHIDLocationIDKey));

    // Look for any partially-set up boards matching this device
    int index = -1;
    for(int i = 1; i < DEV_MAX; i++){
        if(pthread_mutex_trylock(devmutex + i))
            // If the mutex is locked then the device is obviously set up already, keep going
            continue;
        if(keyboard[i].handle == INCOMPLETE
                && keyboard[i].vendor == idvendor && keyboard[i].product == idproduct
                && keyboard[i].location_id == location){
            // Matched; continue setting up this device
            index = i;
            // Device mutex remains locked
            break;
        }
        pthread_mutex_unlock(devmutex + i);
    }
    // If none was found, grab the first free device
    if(index == -1){
        for(int i = 1; i < DEV_MAX; i++){
            if(pthread_mutex_trylock(devmutex + i))
                continue;
            if(!keyboard[i].handle){
                // Mark the device as in use and print out a message
                index = i;
                keyboard[i].handle = INCOMPLETE;
                keyboard[i].location_id = location;
                keyboard[i].vendor = idvendor;
                keyboard[i].product = idproduct;
                // Read the serial number and name
                usbgetstr(handle, CFSTR(kIOHIDSerialNumberKey), keyboard[i].serial, SERIAL_LEN);
                usbgetstr(handle, CFSTR(kIOHIDProductKey), keyboard[i].name, KB_NAME_LEN);
                ckb_info("Connecting %s (S/N: %s)\n", keyboard[i].name, keyboard[i].serial);
                // Device mutex remains locked
                break;
            }
            pthread_mutex_unlock(devmutex + i);
        }
    }
    if(index == -1){
        ckb_err("No free devices\n");
        return 0;
    }
    usbdevice* kb = keyboard + index;
    // There's no direct way to tell which of the endpoints this is, but there's a workaround
    // Each handle has a unique maximum packet size combination, so use that to place them
    long input = usbgetlong(handle, CFSTR(kIOHIDMaxInputReportSizeKey));
    long output = usbgetlong(handle, CFSTR(kIOHIDMaxOutputReportSizeKey));
    long feature = usbgetlong(handle, CFSTR(kIOHIDMaxFeatureReportSizeKey));

    int handle_idx;
    // Handle 0 is for BIOS mode input (RGB) or non-RGB key input
    if(((input == 8 && output == 1)             // Keyboards
            || (input == 7 && output == 0))     // Mice
            && feature == 0)
        handle_idx = 0;
    // Handle 1 is for standard HID input (RGB) or media keys (non-RGB)
    else if(((input == 21 || input == 10) && output == 1 && feature == 1)
            || (input == 4 && output == 0 && feature == 0))
        handle_idx = 1;
    // Handle 2 is for Corsair inputs, unused on non-RGB
    else if(((input == 64 || input == 15) && output == 0 && feature == 0)
            || (input == 64 && output == 64 && feature == 0))           // FW >= 1.20
        handle_idx = 2;
    // Handle 3 is for controlling the device (only exists for RGB)
    else if((input == 0 && output == 0 && feature == 64)
            || (input == 64 && output == 64 && feature == 64))          // FW >= 1.20
        handle_idx = 3;
    else {
        ckb_warn("Got unknown handle (I: %d, O: %d, F: %d)\n", (int)input, (int)output, (int)feature);
        return 0;
    }

    // If all handles have been set up, finish initializing the keyboard
    kb->handles[handle_idx] = handle;
    if(kb->handles[0] && kb->handles[1] && kb->handles[2]
            && (kb->handles[3] || !IS_RGB(idvendor, idproduct)))
        setupusb(kb);
    else
        pthread_mutex_unlock(devmutex + index);
    *rm_notify = kb->rm_notify + handle_idx;
    return kb;
}

static CFRunLoopRef mainloop = 0;
static IONotificationPortRef notify = 0;
// input_mac.c
extern void* krthread(void* context);

static void remove_device(void* context, io_service_t device, uint32_t message_type, void* message_argument){
    if(message_type != kIOMessageServiceIsTerminated)
        return;
    usbdevice* kb = context;
    if(kb){
        // If the handle is connected to a device, close it
        pthread_mutex_lock(dmutex(kb));
        closeusb(kb);
        pthread_mutex_unlock(dmutex(kb));
    }
    IOObjectRelease(device);
}

static void iterate_devices(void* context, io_iterator_t iterator){
    io_service_t device;
    while((device = IOIteratorNext(iterator)) != 0){
        // Get the plugin interface for the device
        IOCFPlugInInterface** plugin;
        SInt32 score;
        kern_return_t err = IOCreatePlugInInterfaceForService(device, kIOHIDDeviceTypeID, kIOCFPlugInInterfaceID, &plugin, &score);
        if(err != kIOReturnSuccess){
            ckb_err("Failed to create device plugin: %x\n", err);
            continue;
        }
        // Get the device interface
        hid_dev_t handle;
        err = (*plugin)->QueryInterface(plugin, CFUUIDGetUUIDBytes(kIOHIDDeviceDeviceInterfaceID), (LPVOID*)&handle);
        if(err != kIOReturnSuccess){
            ckb_err("QueryInterface failed: %x\n", err);
            continue;
        }
        // Plugin is no longer needed
        IODestroyPlugInInterface(plugin);
        // Seize the device handle
        euid_guard_start;
        err = (*handle)->open(handle, kIOHIDOptionsTypeSeizeDevice);
        euid_guard_stop;
        if(err != kIOReturnSuccess){
            ckb_err("Failed to seize device: %x\n", err);
            continue;
        }
        // Connect it
        io_object_t* rm_notify = 0;
        usbdevice* kb = usbadd(handle, &rm_notify);
        if(kb)
            // If successful, register for removal notification
            IOServiceAddInterestNotification(notify, device, kIOGeneralInterest, remove_device, kb, rm_notify);
        else
            // Otherwise, release it now
            remove_device(0, device, kIOMessageServiceIsTerminated, 0);
    }
}

int usbmain(){
    int vendor = V_CORSAIR;
    int products[] = { P_K65, P_K70, P_K70_NRGB, P_K95, P_K95_NRGB, P_M65 };
    // Tell IOService which type of devices we want (IOHIDDevices matching the supported vendor/products)
    CFMutableDictionaryRef match = IOServiceMatching(kIOHIDDeviceKey);
    CFNumberRef cfvendor = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &vendor);
    CFDictionarySetValue(match, CFSTR(kIOHIDVendorIDKey), cfvendor);
    CFRelease(cfvendor);
    CFMutableArrayRef cfproducts = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
    for(uint i = 0; i < sizeof(products) / sizeof(int); i++){
        int product = products[i];
        CFNumberRef cfproduct = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &product);
        CFArrayAppendValue(cfproducts, cfproduct);
        CFRelease(cfproduct);
    }
    CFDictionarySetValue(match, CFSTR(kIOHIDProductIDArrayKey), cfproducts);
    CFRelease(cfproducts);

    notify = IONotificationPortCreate(kIOMasterPortDefault);
    CFRunLoopAddSource(mainloop = CFRunLoopGetCurrent(), IONotificationPortGetRunLoopSource(notify), kCFRunLoopDefaultMode);
    io_iterator_t iterator;
    IOReturn res = IOServiceAddMatchingNotification(notify, kIOMatchedNotification, match, iterate_devices, 0, &iterator);
    if(res != kIOReturnSuccess){
        ckb_fatal("Failed to list devices: %x\n", res);
        return -1;
    }
    // Iterate existing devices
    iterate_devices(0, iterator);
    // Enter loop to scan/connect new devices
    CFRunLoopRun();
    return 0;
}

void usbkill(){
    CFRunLoopStop(mainloop);
}

#endif

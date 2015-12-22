#include "device.h"
#include "devnode.h"
#include "input.h"
#include "notify.h"
#include "usb.h"

#ifdef OS_MAC

#define INCOMPLETE (hid_dev_t)-1l

#define IS_TEMP_FAILURE(res)        ((res) == kIOUSBTransactionTimeout || (res) == kIOUSBTransactionReturned || (res) == kIOUSBPipeStalled)
#define IS_DISCONNECT_FAILURE(res)  ((res) == kIOReturnBadArgument || (res) == kIOReturnNoDevice || (res) == kIOReturnNotOpen || (res) == kIOReturnNotAttached || (res) == kIOReturnExclusiveAccess)

int os_usbsend(usbdevice* kb, const uchar* out_msg, int is_recv, const char* file, int line){
    // Firmware versions above 1.20 use Output instead of Feature reports for improved performance
    // It doesn't work well when receiving data, however
    IOHIDReportType type = (kb->fwversion >= 0x120 && !is_recv ? kIOHIDReportTypeOutput : kIOHIDReportTypeFeature);
    kern_return_t res = (*kb->handle)->setReport(kb->handle, type, 0, out_msg, MSG_SIZE, 5000, 0, 0, 0);
    kb->lastresult = res;
    if(res != kIOReturnSuccess){
        ckb_err_fn("Got return value 0x%x\n", file, line, res);
        if(IS_TEMP_FAILURE(res))
            return -1;
        else
            return 0;
    }
    return MSG_SIZE;
}

int os_usbrecv(usbdevice* kb, uchar* in_msg, const char* file, int line){
    CFIndex length = MSG_SIZE;
    kern_return_t res = (*kb->handle)->getReport(kb->handle, kIOHIDReportTypeFeature, 0, in_msg, &length, 5000, 0, 0, 0);
    kb->lastresult = res;
    if(res != kIOReturnSuccess){
        ckb_err_fn("Got return value 0x%x\n", file, line, res);
        if(IS_TEMP_FAILURE(res))
            return -1;
        else
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
    if(IS_DISCONNECT_FAILURE(res))
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
        else if(length == MSG_SIZE)
            corsair_mousecopy(kb->input.keys, data);
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
            corsair_kbcopy(kb->input.keys, data);
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
extern void keyretrigger(CFRunLoopTimerRef timer, void* info);

void* os_inputmain(void* context){
    usbdevice* kb = context;
    int index = INDEX_OF(kb, keyboard);
    // Schedule async events for the device on this thread
    CFRunLoopRef runloop = kb->input_loop = CFRunLoopGetCurrent();
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

    // Start a timer for key repeat broadcasts
    CFRunLoopTimerContext krctx = { 0, kb, NULL, NULL, NULL };
    CFRunLoopTimerRef krtimer = kb->krtimer = CFRunLoopTimerCreate(kCFAllocatorDefault,
                                                                   CFAbsoluteTimeGetCurrent() + 0.001, 0.001,   // Set it to run every 1ms
                                                                   0, 0,
                                                                   keyretrigger, &krctx);
    CFRunLoopTimerSetTolerance(krtimer, 0.015);         // Set a maximum tolerance of 15ms
    // We don't actually add the timer to the run loop yet. There's no need to run the function until a key is actually pressed,
    // so the timer is added and removed dynamically.

    // Start the run loop
    while(1){
        CFRunLoopRun();
        // If we get here, the device should be disconnected
        pthread_mutex_lock(imutex(kb));
        if(!IS_CONNECTED(kb)){
            pthread_mutex_unlock(imutex(kb));
            break;
        }
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
        (*kb->handles[i])->close(kb->handles[i], kIOHIDOptionsTypeNone);
        (*kb->handles[i])->Release(kb->handles[i]);
        kb->handles[i] = 0;
    }
    if(kb->input_loop){
        CFRunLoopStop(kb->input_loop);
        kb->input_loop = 0;
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
                ckb_info("Connecting %s at %s%d\n", keyboard[i].name, devpath, i);
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

static int seize_wait(hid_dev_t handle){
    // HACK: We shouldn't seize the HID device until it's successfully added to the service registry.
    // Otherwise, OSX might think there's no keyboard/mouse connected.
    long location = usbgetlong(handle, CFSTR(kIOHIDLocationIDKey));
    char location_var[18], location_fixed[18];
    snprintf(location_var, sizeof(location_var), "@%lx", location);
    snprintf(location_fixed, sizeof(location_fixed), "@%08x", (int)location);
    // Open master port (if not done yet)
    static mach_port_t master = 0;
    kern_return_t res;
    if(!master && (res = IOMasterPort(bootstrap_port, &master)) != kIOReturnSuccess){
        master = 0;
        ckb_warn("Unable to open master port: 0x%08x\n", res);
        return -1;
    }
    const int max_tries = 50;     // give up after ~5s
    for(int try = 0; try < max_tries; try++){
        usleep(100);
        // Iterate the whole IOService registry
        io_iterator_t child_iter;
        if((res = IORegistryCreateIterator(master, kIOServicePlane, kIORegistryIterateRecursively, &child_iter)) != kIOReturnSuccess)
            return -2;

        io_registry_entry_t child_service;
        while((child_service = IOIteratorNext(child_iter)) != 0){
            io_string_t path;
            IORegistryEntryGetPath(child_service, kIOServicePlane, path);
            IOObjectRelease(child_service);
            // Look for an entry that matches the location of the device and says "HID". If found, we can proceed with adding the device
            if((strstr(path, location_var) || strstr(path, location_fixed)) && strstr(path, "HID")){
                IOObjectRelease(child_iter);
                return 0;
            }
        }
        IOObjectRelease(child_iter);
    }
    // Timed out
    return -3;
}

// Hacky way of trying something over and over again until it works. 500ms intervals, max 5s
#define wait_loop(error, operation)  do {               \
    int trial = 0;                                      \
    while(((error) = (operation)) != kIOReturnSuccess){ \
        if(++trial == 10)                               \
            break;                                      \
        usleep(500000);                                 \
    } } while(0)

static void iterate_devices(void* context, io_iterator_t iterator){
    io_service_t device;
    euid_guard_start;
    while((device = IOIteratorNext(iterator)) != 0){
        // Get the plugin interface for the device
        IOCFPlugInInterface** plugin = 0;
        SInt32 score = 0;
        kern_return_t err;
        wait_loop(err, IOCreatePlugInInterfaceForService(device, kIOHIDDeviceTypeID, kIOCFPlugInInterfaceID, &plugin, &score));
        if(err != kIOReturnSuccess){
            ckb_err("Failed to create device plugin: %x\n", err);
            goto release;
        }
        // Get the device interface
        hid_dev_t handle;
        wait_loop(err, (*plugin)->QueryInterface(plugin, CFUUIDGetUUIDBytes(kIOHIDDeviceDeviceInterfaceID), (LPVOID*)&handle));
        if(err != kIOReturnSuccess){
            ckb_err("QueryInterface failed: %x\n", err);
            goto release;
        }
        // Plugin is no longer needed
        IODestroyPlugInInterface(plugin);
        // Seize the device handle
        if(seize_wait(handle))
            ckb_warn("seize_wait failed, connecting anyway...\n");
        err = (*handle)->open(handle, kIOHIDOptionsTypeSeizeDevice);
        if(err != kIOReturnSuccess){
            ckb_err("Failed to seize device: %x\n", err);
            goto release;
        }
        // Connect it
        io_object_t* rm_notify = 0;
        usbdevice* kb = usbadd(handle, &rm_notify);
        if(kb)
            // If successful, register for removal notification
            IOServiceAddInterestNotification(notify, device, kIOGeneralInterest, remove_device, kb, rm_notify);
        else
            // Otherwise, release it now
            (*handle)->close(handle, kIOHIDOptionsTypeSeizeDevice);
        release:
        IOObjectRelease(device);
    }
    euid_guard_stop;
}

int usbmain(){
    int vendor = V_CORSAIR;
    int products[] = {
        // Keyboards
        P_K65, P_K70, P_K70_NRGB, P_K95, P_K95_NRGB, P_STRAFE, P_STRAFE_NRGB,
        // Mice
        P_M65, P_SABRE_O, P_SABRE_L, P_SCIMITAR
    };
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
    io_iterator_t iterator = 0;
    IOReturn res = IOServiceAddMatchingNotification(notify, kIOMatchedNotification, match, iterate_devices, 0, &iterator);
    if(res != kIOReturnSuccess){
        ckb_fatal("Failed to list devices: %x\n", res);
        return -1;
    }
    // Iterate existing devices
    if(iterator)
        iterate_devices(0, iterator);
    // Enter loop to scan/connect new devices
    CFRunLoopRun();
    return 0;
}

void usbkill(){
    CFRunLoopStop(mainloop);
}

#endif

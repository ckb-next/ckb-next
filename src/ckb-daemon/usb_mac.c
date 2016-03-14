#include "device.h"
#include "devnode.h"
#include "input.h"
#include "notify.h"
#include "usb.h"

#ifdef OS_MAC

static CFRunLoopRef mainloop = 0;
static IONotificationPortRef notify = 0;

static long hidgetlong(hid_dev_t handle, CFStringRef key){
    long raw = 0;
    CFTypeRef cf;
    if((*handle)->getProperty(handle, key, &cf) != kIOReturnSuccess)
        return 0;
    if(!cf || CFGetTypeID(cf) != CFNumberGetTypeID() || !CFNumberGetValue(cf, kCFNumberLongType, &raw))
        return 0;
    return raw;
}

static void hidgetstr(hid_dev_t handle, CFStringRef key, char* output, int out_len){
    CFTypeRef cf;
    if((*handle)->getProperty(handle, key, &cf) != kIOReturnSuccess){
        output[0] = 0;
        return;
    }
    if(!cf || CFGetTypeID(cf) != CFStringGetTypeID() || !CFStringGetCString(cf, output, out_len, kCFStringEncodingASCII))
        output[0] = 0;
}

#define INCOMPLETE (usb_dev_t)-1l
#define HAS_ALL_HANDLES(kb) ((kb)->epcount > 0 && (kb)->epcount_hid + (kb)->epcount_usb >= (kb)->epcount)

// Hacky way of trying something over and over again until it works. 100ms intervals, max 1s
#define wait_loop(error, operation)  do {               \
    int trial = 0;                                      \
    while(((error) = (operation)) != kIOReturnSuccess){ \
    if(++trial == 10)                               \
    break;                                      \
    usleep(100000);                                 \
    } } while(0)

#define IS_TEMP_FAILURE(res)        ((res) == kIOUSBTransactionTimeout || (res) == kIOUSBTransactionReturned || (res) == kIOUSBPipeStalled)
#define IS_DISCONNECT_FAILURE(res)  ((res) == kIOReturnBadArgument || (res) == kIOReturnNoDevice || (res) == kIOReturnNotOpen || (res) == kIOReturnNotAttached || (res) == kIOReturnExclusiveAccess)

int os_usbsend(usbdevice* kb, const uchar* out_msg, int is_recv, const char* file, int line){
    kern_return_t res;
    // FIXME: unhack this
    if(kb->ifusb[2] && !is_recv){
        usb_iface_t handle = kb->ifusb[2];
        res = (*handle)->WritePipe(handle, 1, (void*)out_msg, MSG_SIZE);
    } else {
        hid_dev_t handle = kb->ifhid[3];
        if(!handle)
            return 0;
        // Firmware versions above 1.20 use Output instead of Feature reports for improved performance
        // It doesn't work well when receiving data, however
        IOHIDReportType type = (kb->fwversion >= 0x120 && !is_recv ? kIOHIDReportTypeInput : kIOHIDReportTypeFeature);
        res = (*handle)->setReport(handle, type, 0, out_msg, MSG_SIZE, 5000, 0, 0, 0);
    }
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
    hid_dev_t handle = kb->ifhid[3];
    if(!handle)
        return 0;
    kern_return_t res = (*handle)->getReport(handle, kIOHIDReportTypeFeature, 0, in_msg, &length, 5000, 0, 0, 0);
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

void os_sendindicators(usbdevice* kb){
    uchar ileds = kb->ileds;
    // Get a list of LED elements from handle 0
    long ledpage = kHIDPage_LEDs;
    const void* keys[] = { CFSTR(kIOHIDElementUsagePageKey) };
    const void* values[] = { CFNumberCreate(kCFAllocatorDefault, kCFNumberLongType, &ledpage) };
    CFDictionaryRef matching = CFDictionaryCreate(kCFAllocatorDefault, keys, values, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFRelease(values[0]);
    CFArrayRef leds;
    kern_return_t res = (*kb->ifhid[0])->copyMatchingElements(kb->ifhid[0], matching, &leds, 0);
    CFRelease(matching);
    if(res != kIOReturnSuccess)
        return;
    // Iterate through them and update the LEDs which have changed
    CFIndex count = CFArrayGetCount(leds);
    for(CFIndex i = 0; i < count; i++){
        IOHIDElementRef led = (void*)CFArrayGetValueAtIndex(leds, i);
        uint32_t usage = IOHIDElementGetUsage(led);
        IOHIDValueRef value = IOHIDValueCreateWithIntegerValue(kCFAllocatorDefault, led, 0, !!(ileds & (1 << (usage - 1))));
        (*kb->ifhid[0])->setValue(kb->ifhid[0], led, value, 5000, 0, 0, 0);
        CFRelease(value);
    }
    CFRelease(leds);
}

int os_resetusb(usbdevice* kb, const char* file, int line){
    kern_return_t res = kb->lastresult;
    if(IS_DISCONNECT_FAILURE(res))
        // Don't try if the keyboard was disconnected
        return -2;
    // Reset all handles
    int count = IS_RGB(kb->vendor, kb->product) ? 4 : 3;
    for(int i = 0; i < count; i++){
        if((*kb->ifhid[i])->setProperty(kb->ifhid[i], CFSTR(kIOHIDResetKey), kCFBooleanTrue) != kIOReturnSuccess)
            return -1;
    }
    return 0;
}

static void intreport(void* context, IOReturn result, void* sender, IOHIDReportType reporttype, uint32_t reportid, uint8_t* data, CFIndex length){
    usbdevice* kb = context;
    pthread_mutex_lock(imutex(kb));
    if(IS_MOUSE(kb->vendor, kb->product)){
        switch(length){
        case 7:
        case 8:
        case 10:
        case 11:
            hid_mouse_translate(kb->input.keys, &kb->input.rel_x, &kb->input.rel_y, -2, length, data);
            break;
        case MSG_SIZE:
            corsair_mousecopy(kb->input.keys, kb->ifhid[3] ? 3 : 2, data);
            break;
        }
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
    int count = kb->epcount_hid;
    if(count > 3)
        count = 3;
    for(int i = 0; i < count; i++){
        CFTypeRef eventsource;
        if(!kb->ifhid[i])
            continue;
        kern_return_t res = (*kb->ifhid[i])->getAsyncEventSource(kb->ifhid[i], &eventsource);
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
    uint8_t* urbinput[IFACE_MAX] = { 0 };
    for(int i = 0; i < count; i++){
        hid_dev_t handle = kb->ifhid[i];
        if(!handle)
            continue;
        long maxsize = hidgetlong(handle, CFSTR(kIOHIDMaxInputReportSizeKey));
        uint8_t* buffer = malloc(maxsize);
        urbinput[i] = buffer;
        (*handle)->setInputReportCallback(handle, buffer, maxsize, intreport, kb, 0);
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

int os_setupusb(usbdevice* kb){
    kb->lastkeypress = KEY_NONE;
    // Get the device firmware version
    (*kb->handle)->GetDeviceReleaseNumber(kb->handle, &kb->fwversion);
    return 0;
}

void os_closeusb(usbdevice* kb){
    // Close HID handles
    int count = kb->epcount_hid;
    for(int i = 0; i < count; i++){
        hid_dev_t iface = kb->ifhid[i];
        if(iface){
            (*iface)->close(iface, kIOHIDOptionsTypeNone);
            (*iface)->Release(iface);
            kb->ifhid[i] = 0;
        }
    }
    kb->epcount_hid = 0;
    // Close USB handles
    count = kb->epcount_usb;
    for(int i = 0; i < count; i++){
        usb_iface_t iface = kb->ifusb[i];
        if(iface){
            (*iface)->USBInterfaceClose(iface);
            (*iface)->Release(iface);
            kb->ifusb[i] = 0;
        }
    }
    kb->epcount_usb = 0;
    usb_dev_t iface = kb->handle;
    if(iface){
        (*iface)->USBDeviceClose(iface);
        (*iface)->Release(iface);
        kb->handle = 0;
    }
    // Close input thread
    if(kb->input_loop){
        CFRunLoopStop(kb->input_loop);
        kb->input_loop = 0;
    }
}

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

// Finds a USB device by location ID. Returns a new device if none was found or -1 if no devices available.
// If successful, devmutex[index] will be locked when the function returns. Unlock it when finished.
static int find_device(long idvendor, long idproduct, long location){
    // Look for any partially-set up boards matching this device
    for(int i = 1; i < DEV_MAX; i++){
        if(pthread_mutex_trylock(devmutex + i))
            // If the mutex is locked then the device is obviously set up already, keep going
            continue;
        if(keyboard[i].vendor == idvendor && keyboard[i].product == idproduct
                && keyboard[i].location_id == location){
            // Matched; continue setting up this device
            // Device mutex remains locked
            return i;
        }
        pthread_mutex_unlock(devmutex + i);
    }
    // If none was found, grab the first free device
    for(int i = 1; i < DEV_MAX; i++){
        if(pthread_mutex_trylock(devmutex + i))
            continue;
        if(!keyboard[i].handle){
            // Mark the device as in use and print out a message
            keyboard[i].handle = INCOMPLETE;
            keyboard[i].location_id = location;
            keyboard[i].vendor = idvendor;
            keyboard[i].product = idproduct;
            // Device mutex remains locked
            return i;
        }
        pthread_mutex_unlock(devmutex + i);
    }
    return -1;
}

static usbdevice* add_usb(usb_dev_t handle, io_object_t** rm_notify){
    int iface_count = 0, iface_success = 0;
    io_iterator_t iterator = 0;
    io_service_t iface = 0;
    // Get device properties
    UInt16 idvendor, idproduct;
    UInt32 location;
    (*handle)->GetDeviceVendor(handle, &idvendor);
    (*handle)->GetDeviceProduct(handle, &idproduct);
    (*handle)->GetLocationID(handle, &location);
    // Use the location ID key to group the USB handle with the HID handles
    int index = find_device(idvendor, idproduct, location);
    if(index == -1){
        ckb_err("No free devices\n");
        return 0;
    }
    usbdevice* kb = keyboard + index;

    // Set the handle for the keyboard
    if(kb->handle && kb->handle != INCOMPLETE){
        // This should never happen
        ckb_warn("Tried to set up handle for device ckb%d, but it was already set up. Skipping...\n", index);
        goto error;
    }
    kb->handle = handle;

    // Iterate through the USB interfaces. Most of these will fail to open because they're already grabbed by the HID system.
    IOUSBFindInterfaceRequest interfaceRequest;
    interfaceRequest.bInterfaceClass = kIOUSBFindInterfaceDontCare;
    interfaceRequest.bInterfaceSubClass = kIOUSBFindInterfaceDontCare;
    interfaceRequest.bInterfaceProtocol = kIOUSBFindInterfaceDontCare;
    interfaceRequest.bAlternateSetting = kIOUSBFindInterfaceDontCare;
    (*handle)->CreateInterfaceIterator(handle, &interfaceRequest, &iterator);
    // Count the total number of interfaces as well as the number successfully opened.
    while((iface = IOIteratorNext(iterator)) != 0){
        if(iface_count >= IFACE_MAX){
            ckb_warn("Too many interfaces. Dropping the rest.\n");
            break;
        }
        // Get device interface
        IOCFPlugInInterface** plugin = 0;
        SInt32 score = 0;
        kern_return_t err;
        wait_loop(err, IOCreatePlugInInterfaceForService(iface, kIOUSBInterfaceUserClientTypeID, kIOCFPlugInInterfaceID, &plugin, &score));
        if(err != kIOReturnSuccess){
            ckb_err("Failed to create interface plugin: %x\n", err);
            goto release;
        }
        usb_iface_t if_handle;
        wait_loop(err, (*plugin)->QueryInterface(plugin, CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID), (LPVOID)&if_handle));
        if(err != kIOReturnSuccess){
            ckb_err("QueryInterface failed: %x\n", err);
            continue;
        }
        // Plugin is no longer needed
        IODestroyPlugInInterface(plugin);

        // Try to open the interface. If it succeeds, add it to the device's interface list.
        err = (*if_handle)->USBInterfaceOpen(if_handle);    // no wait_loop here because this is expected to fail
        if(err == kIOReturnSuccess){
            kb->ifusb[iface_count] = if_handle;
            iface_success++;
            // Register for removal notification
            IOServiceAddInterestNotification(notify, iface, kIOGeneralInterest, remove_device, kb, kb->rm_notify + 1 + iface_count);
        } else {
            kb->ifusb[iface_count] = 0;
            (*if_handle)->Release(if_handle);
        }

release:
        iface_count++;
        IOObjectRelease(iface);
    }
    if(iface_count == 0){
        // This shouldn't happen, but if it does, assume EP count based on what the device is supposed to have
        iface_count = (HAS_FEATURES(kb, FEAT_RGB) ? 4 : 3);
        ckb_warn("Unable to count endpoints, assuming %d...\n", iface_count);
    }
    kb->epcount = iface_count;
    kb->epcount_usb = iface_success;

    // If the HID handles are already opened, set up the device
    if(HAS_ALL_HANDLES(kb))
        setupusb(kb);
    else
        pthread_mutex_unlock(devmutex + index);
    *rm_notify = kb->rm_notify;
    return kb;

error:
    pthread_mutex_unlock(devmutex + index);
    return 0;
}

static void iterate_devices_usb(void* context, io_iterator_t iterator){
    io_service_t device;
    euid_guard_start;
    while((device = IOIteratorNext(iterator)) != 0){
        IOCFPlugInInterface** plugin = 0;
        SInt32 score = 0;
        kern_return_t err;
        wait_loop(err, IOCreatePlugInInterfaceForService(device, kIOUSBDeviceUserClientTypeID, kIOCFPlugInInterfaceID, &plugin, &score));
        if(err != kIOReturnSuccess){
            ckb_err("Failed to create device plugin: %x\n", err);
            goto release;
        }
        // Get the device interface
        usb_dev_t handle;
        wait_loop(err, (*plugin)->QueryInterface(plugin, CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID), (LPVOID*)&handle));
        if(err != kIOReturnSuccess){
            ckb_err("QueryInterface failed: %x\n", err);
            goto release;
        }
        // Plugin is no longer needed
        IODestroyPlugInInterface(plugin);

        err = (*handle)->USBDeviceOpen(handle);
        if(err != kIOReturnSuccess){
            ckb_err("USBDeviceOpen failed: %x\n", err);
            continue;
        }
        // Connect it
        io_object_t* rm_notify = 0;
        usbdevice* kb = add_usb(handle, &rm_notify);
        if(kb){
            // If successful, register for removal notification
            IOServiceAddInterestNotification(notify, device, kIOGeneralInterest, remove_device, kb, rm_notify);
        } else
            // Otherwise, release it now
            (*handle)->USBDeviceClose(handle);
release:
        IOObjectRelease(device);
    }
    euid_guard_stop;
}

static usbdevice* add_hid(hid_dev_t handle, io_object_t** rm_notify){
    // Get the model and serial number
    long idvendor = hidgetlong(handle, CFSTR(kIOHIDVendorIDKey)), idproduct = hidgetlong(handle, CFSTR(kIOHIDProductIDKey));
    // Each keyboard generates multiple match events (one for each endpoint)
    // Use the location ID key to group the handles together
    long location = hidgetlong(handle, CFSTR(kIOHIDLocationIDKey));
    int index = find_device(idvendor, idproduct, location);
    if(index == -1){
        ckb_err("No free devices\n");
        return 0;
    }
    usbdevice* kb = keyboard + index;

    // Read the serial number and name (if not done yet)
    if(!keyboard[index].serial[0] && !keyboard[index].name[0]){
        hidgetstr(handle, CFSTR(kIOHIDSerialNumberKey), keyboard[index].serial, SERIAL_LEN);
        hidgetstr(handle, CFSTR(kIOHIDProductKey), keyboard[index].name, KB_NAME_LEN);
        ckb_info("Connecting %s at %s%d\n", keyboard[index].name, devpath, index);
    }

    // There's no direct way to tell which of the endpoints this is, but there's a workaround
    // Each handle has a unique maximum packet size combination, so use that to place them
    long input = hidgetlong(handle, CFSTR(kIOHIDMaxInputReportSizeKey));
    long output = hidgetlong(handle, CFSTR(kIOHIDMaxOutputReportSizeKey));
    long feature = hidgetlong(handle, CFSTR(kIOHIDMaxFeatureReportSizeKey));
    int handle_idx;
    // Handle 0 is for BIOS mode input (RGB) or non-RGB key input
    if(((input == 8 && output == 1)             // Keyboards
            || (input == 7 && output == 0))     // Mice
            && feature == 0)
        handle_idx = 0;
    // Handle 1 is for standard HID input (RGB) or media keys (non-RGB)
    else if(((input == 21 || input == 10) && output == 1 && feature == 1)
            || (input == 4 && output == 0 && feature == 0)
            || (input == 64 && output == 1 && feature == 1))            // FW >= 2.00 (Scimitar)
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
        goto error;
    }

    // Set the handle
    if(kb->ifhid[handle_idx]){
        // This should never happen
        ckb_warn("Tried to set up HID %d for device ckb%d, but it was already set up. Skipping...\n", handle_idx, index);
        goto error;
    }
    kb->ifhid[handle_idx] = handle;
    kb->epcount_hid++;
    if(HAS_ALL_HANDLES(kb))
        // If all handles have been opened, we're ready to set up the device
        setupusb(kb);
    else
        // Otherwise, return and keep going
        pthread_mutex_unlock(devmutex + index);
    *rm_notify = kb->rm_notify + IFACE_MAX + 1 + handle_idx;
    return kb;

error:
    pthread_mutex_unlock(devmutex + index);
    return 0;
}

static int seize_wait(hid_dev_t handle){
    // HACK: We shouldn't seize the HID device until it's successfully added to the service registry.
    // Otherwise, OSX might think there's no keyboard/mouse connected.
    long location = hidgetlong(handle, CFSTR(kIOHIDLocationIDKey));
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

#define HANDLE_MAX  (DEV_MAX * 4)
#define RESET_MAX   5

static void iterate_devices_hid(void* context, io_iterator_t iterator){
    io_service_t device;
    hid_dev_t done_handles[HANDLE_MAX];
    int done_handle_count = 0;
    int reset_when_done, reset_count = 0;
    euid_guard_start;

on_reset:
    reset_when_done = 0;
    while((device = IOIteratorNext(iterator)) != 0){
        // Get the plugin interface for the device
        IOCFPlugInInterface** plugin = 0;
        SInt32 score = 0;
        kern_return_t err;
        wait_loop(err, IOCreatePlugInInterfaceForService(device, kIOHIDDeviceTypeID, kIOCFPlugInInterfaceID, &plugin, &score));
        if(err != kIOReturnSuccess){
            ckb_err("Failed to create device plugin: %x\n", err);
            reset_when_done = 1;
            goto release;
        }
        // Get the device interface
        hid_dev_t handle;
        wait_loop(err, (*plugin)->QueryInterface(plugin, CFUUIDGetUUIDBytes(kIOHIDDeviceDeviceInterfaceID), (LPVOID*)&handle));
        if(err != kIOReturnSuccess){
            ckb_err("QueryInterface failed: %x\n", err);
            goto release;
        }
        // Make sure not to add the same handle more than once (could happen due to the iterator getting reset)
        for(int i = 0; i < done_handle_count; i++){
            if(done_handles[i] == handle)
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
        usbdevice* kb = add_hid(handle, &rm_notify);
        if(kb){
            // If successful, register for removal notification
            IOServiceAddInterestNotification(notify, device, kIOGeneralInterest, remove_device, kb, rm_notify);
            if(done_handle_count < HANDLE_MAX)
                done_handles[done_handle_count++] = handle;
        } else
            // Otherwise, release it now
            (*handle)->close(handle, kIOHIDOptionsTypeSeizeDevice);
release:
        IOObjectRelease(device);
    }
    // If there was an error prepping the devices, reset the iterator and try again
    if(reset_when_done){
        if(reset_count++ >= RESET_MAX){
            ckb_warn("Iterator reset too many times; giving up.\n");
        } else {
            usleep(100000);
            IOIteratorReset(iterator);
            goto on_reset;
        }
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

    // Setup global variables
    notify = IONotificationPortCreate(kIOMasterPortDefault);
    mainloop = CFRunLoopGetCurrent();
    CFRunLoopAddSource(mainloop, IONotificationPortGetRunLoopSource(notify), kCFRunLoopDefaultMode);

    // This gets really complicated.
    // The USB interfaces are *usually* grabbed by the HID system, which makes them inaccessible to ckb.
    // In this case, we have to get the HID handles and communicate through them.
    // But sometimes, the interfaces are not grabbed. In this case, we have to talk to the USB device directly.
    // So what do we do? Grab both HID and USB handles for all devices, try to set them up, and determine which ones to use
    // based on which interfaces were grabbed.

    // Let's start by searching for USB devices...
    CFMutableDictionaryRef match = IOServiceMatching(kIOUSBDeviceClassName);
    CFNumberRef cfvendor = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &vendor);
    CFDictionarySetValue(match, CFSTR(kUSBVendorName), cfvendor);
    CFRelease(cfvendor);
    CFMutableArrayRef cfproducts = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
    for(uint i = 0; i < sizeof(products) / sizeof(int); i++){
        int product = products[i];
        CFNumberRef cfproduct = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &product);
        CFArrayAppendValue(cfproducts, cfproduct);
        CFRelease(cfproduct);
    }
    CFDictionarySetValue(match, CFSTR(kUSBProductIdsArrayName), cfproducts);
    CFRelease(cfproducts);

    io_iterator_t iterator_usb = 0;
    IOReturn res = IOServiceAddMatchingNotification(notify, kIOMatchedNotification, match, iterate_devices_usb, 0, &iterator_usb);
    if(res != kIOReturnSuccess){
        ckb_fatal("Failed to list USB devices: %x\n", res);
        return -1;
    }
    // Iterate existing devices
    if(iterator_usb)
        iterate_devices_usb(0, iterator_usb);

    // Now move on to HID devices
    // It is in fact necessary to recreate the matching dictionary, as the keys are different
    match = IOServiceMatching(kIOHIDDeviceKey);
    cfvendor = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &vendor);
    CFDictionarySetValue(match, CFSTR(kIOHIDVendorIDKey), cfvendor);
    CFRelease(cfvendor);
    cfproducts = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
    for(uint i = 0; i < sizeof(products) / sizeof(int); i++){
        int product = products[i];
        CFNumberRef cfproduct = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &product);
        CFArrayAppendValue(cfproducts, cfproduct);
        CFRelease(cfproduct);
    }
    CFDictionarySetValue(match, CFSTR(kIOHIDProductIDArrayKey), cfproducts);
    CFRelease(cfproducts);

    io_iterator_t iterator_hid = 0;
    res = IOServiceAddMatchingNotification(notify, kIOMatchedNotification, match, iterate_devices_hid, 0, &iterator_hid);
    if(res != kIOReturnSuccess){
        ckb_fatal("Failed to list HID devices: %x\n", res);
        return -1;
    }
    // Iterate existing devices
    if(iterator_hid)
        iterate_devices_hid(0, iterator_hid);

    // Enter loop to scan/connect new devices
    CFRunLoopRun();
    return 0;
}

void usbkill(){
    CFRunLoopStop(mainloop);
}

#endif

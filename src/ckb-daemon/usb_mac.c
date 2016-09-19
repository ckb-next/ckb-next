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

// profile.c
void u16dec(ushort* in, char* out, size_t* srclen, size_t* dstlen);

static void usbgetstr(usb_dev_t handle, uint8 string_index, char* output, int out_len){
    // Make a temporary buffer so the request won't fail if too large
    char buffer[256] = { 0 };
    IOUSBDevRequest rq = { 0x80, 0x06, 0x0300 | string_index, 0x0409, sizeof(buffer) - 2, buffer, 0};
    kern_return_t res = (*handle)->DeviceRequest(handle, &rq);
    if(res != kIOReturnSuccess){
        output[0] = 0;
        return;
    }
    size_t inl = sizeof(buffer) / 2, outl = out_len;
    u16dec((ushort*)buffer + 1, output, &inl, &outl);
    output[out_len - 1] = 0;
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

// When reading/writing USB handles we have to ensure we select the correct pipe, else it will fail
static int get_pipe_index(usb_iface_t handle, int desired_direction){
    uchar count;
    if((*handle)->GetNumEndpoints(handle, &count) != kIOReturnSuccess)
        return 1;
    for(int i = 0; i < count; i++){
        uchar direction, number, transfertype, interval;
        ushort maxpacketsize;
        if((*handle)->GetPipeProperties(handle, i + 1, &direction, &number, &transfertype, &maxpacketsize, &interval) == kIOReturnSuccess){
            if(direction == desired_direction)
                return i + 1;
        }
    }
    // Not found - default to 1
    return 1;
}

int os_usbsend(usbdevice* kb, const uchar* out_msg, int is_recv, const char* file, int line){
    kern_return_t res = kIOReturnSuccess;
    if(kb->fwversion < 0x120 || is_recv){
        int ep = kb->epcount;
        // For old devices, or for receiving data, use control transfers
        IOUSBDevRequestTO rq = { 0x21, 0x09, 0x0200, ep - 1, MSG_SIZE, (void*)out_msg, 0, 5000, 5000 };
        res = (*kb->handle)->DeviceRequestTO(kb->handle, &rq);
        if(res == kIOReturnNotOpen){
            // Device handle not open - try to send directly to the endpoint instead
            usb_iface_t h_usb = kb->ifusb[ep - 1];
            hid_dev_t h_hid = kb->ifhid[ep - 1];
            if(h_usb)
                res = (*h_usb)->ControlRequestTO(h_usb, 0, &rq);
            else if(h_hid)
                res = (*h_hid)->setReport(h_hid, kIOHIDReportTypeFeature, 0, out_msg, MSG_SIZE, 5000, 0, 0, 0);
        }
    } else {
        // For newer devices, use interrupt transfers
        int ep = (kb->fwversion >= 0x130 && kb->fwversion < 0x200) ? 4 : 3;
        usb_iface_t h_usb = kb->ifusb[ep - 1];
        hid_dev_t h_hid = kb->ifhid[ep - 1];
        if(h_usb)
            res = (*h_usb)->WritePipe(h_usb, get_pipe_index(h_usb, kUSBOut), (void*)out_msg, MSG_SIZE);
        else if(h_hid)
            res = (*h_hid)->setReport(h_hid, kIOHIDReportTypeOutput, 0, out_msg, MSG_SIZE, 5000, 0, 0, 0);
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
    int ep = kb->epcount;
    IOUSBDevRequestTO rq = { 0xa1, 0x01, 0x0200, ep - 1, MSG_SIZE, in_msg, 0, 5000, 5000 };
    kern_return_t res = (*kb->handle)->DeviceRequestTO(kb->handle, &rq);
    CFIndex length = rq.wLenDone;
    if(res == kIOReturnNotOpen){
        // Device handle not open - try to send directly to the endpoint instead
        usb_iface_t h_usb = kb->ifusb[ep - 1];
        hid_dev_t h_hid = kb->ifhid[ep - 1];
        if(h_usb)
            res = (*h_usb)->ControlRequestTO(h_usb, 0, &rq);
        else if(h_hid)
            res = (*h_hid)->getReport(h_hid, kIOHIDReportTypeFeature, 0, in_msg, &length, 5000, 0, 0, 0);
    }
    kb->lastresult = res;
    if(res != kIOReturnSuccess){
        ckb_err_fn("Got return value 0x%x\n", file, line, res);
        if(IS_TEMP_FAILURE(res))
            return -1;
        else
            return 0;
    }
    if(length != MSG_SIZE)
        ckb_err_fn("Read %ld bytes (expected %d)\n", file, line, length, MSG_SIZE);
    return length;
}

int _nk95cmd(usbdevice* kb, uchar bRequest, ushort wValue, const char* file, int line){
    IOUSBDevRequestTO rq = { 0x40, bRequest, wValue, 0, 0, 0, 0, 5000, 5000 };
    kern_return_t res = (*kb->handle)->DeviceRequestTO(kb->handle, &rq);
    if(res != kIOReturnSuccess){
        ckb_err_fn("Got return value 0x%x\n", file, line, res);
        return 1;
    }
    return 0;
}

void os_sendindicators(usbdevice* kb){
    IOUSBDevRequestTO rq = { 0x21, 0x09, 0x0200, 0x00, 1, &kb->ileds, 0, 500, 500 };
    kern_return_t res = (*kb->handle)->DeviceRequestTO(kb->handle, &rq);
    if(res == kIOReturnNotOpen){
        // Handle not open - try to go through the HID system instead
        hid_dev_t handle = kb->ifhid[0];
        if(handle){
            uchar ileds = kb->ileds;
            // Get a list of LED elements from handle 0
            long ledpage = kHIDPage_LEDs;
            const void* keys[] = { CFSTR(kIOHIDElementUsagePageKey) };
            const void* values[] = { CFNumberCreate(kCFAllocatorDefault, kCFNumberLongType, &ledpage) };
            CFDictionaryRef matching = CFDictionaryCreate(kCFAllocatorDefault, keys, values, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
            CFRelease(values[0]);
            CFArrayRef leds;
            kern_return_t res = (*handle)->copyMatchingElements(handle, matching, &leds, 0);
            CFRelease(matching);
            if(res != kIOReturnSuccess)
                return;
            // Iterate through them and update the LEDs which have changed
            CFIndex count = CFArrayGetCount(leds);
            for(CFIndex i = 0; i < count; i++){
                IOHIDElementRef led = (void*)CFArrayGetValueAtIndex(leds, i);
                uint32_t usage = IOHIDElementGetUsage(led);
                IOHIDValueRef value = IOHIDValueCreateWithIntegerValue(kCFAllocatorDefault, led, 0, !!(ileds & (1 << (usage - 1))));
                (*handle)->setValue(handle, led, value, 5000, 0, 0, 0);
                CFRelease(value);
            }
            CFRelease(leds);
        }
    }
    if(res != kIOReturnSuccess)
        ckb_err("Got return value 0x%x\n", res);
}

int os_resetusb(usbdevice* kb, const char* file, int line){
    kern_return_t res = kb->lastresult;
    if(IS_DISCONNECT_FAILURE(res))
        // Don't try if the keyboard was disconnected
        return -2;
    // Otherwise, just wait and try again
    usleep(100000);
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
            corsair_mousecopy(kb->input.keys, kb->epcount >= 4 ? -3 : -2, data);
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
            corsair_kbcopy(kb->input.keys, kb->epcount >= 4 ? -3 : -2, data);
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

typedef struct {
    usbdevice* kb;
    uchar* buffer;
    int index;
    int pipe;
    int maxsize;
} urbctx;

static void pipecomplete(void* refcon, IOReturn result, void* arg0){
    if(result != kIOReturnSuccess)
        return;
    intptr_t length = (intptr_t)arg0;
    urbctx* ctx = refcon;
    // Wrap the URB to the interrupt report handler
    usbdevice* kb = ctx->kb;
    uchar* buffer = ctx->buffer;
    intreport(kb, result, 0, 0, 0, buffer, length);
    // Re-submit the URB to read the next packet
    usb_iface_t handle = kb->ifusb[ctx->index];
    (*handle)->ReadPipeAsync(handle, ctx->pipe, buffer, ctx->maxsize, pipecomplete, ctx);
}

// input_mac.c
extern void keyretrigger(CFRunLoopTimerRef timer, void* info);

void* os_inputmain(void* context){
    usbdevice* kb = context;
    int index = INDEX_OF(kb, keyboard);
    // Monitor input transfers on all endpoints for non-RGB devices
    // For RGB, monitor all but the last, as it's used for input/output
    int count = IS_RGB_DEV(kb) ? (kb->epcount - 1) : kb->epcount;
    // Schedule async events for the device on this thread
    CFRunLoopRef runloop = kb->input_loop = CFRunLoopGetCurrent();
    for(int i = 0; i < count; i++){
        CFTypeRef eventsource;
        kern_return_t res;
        if(kb->ifusb[i])
            res = (*kb->ifusb[i])->CreateInterfaceAsyncEventSource(kb->ifusb[i], (CFRunLoopSourceRef*)&eventsource);
        else if(kb->ifhid[i])
            res = (*kb->ifhid[i])->getAsyncEventSource(kb->ifhid[i], &eventsource);
        else
            continue;
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
    urbctx input[IFACE_MAX];
    memset(input, 0, sizeof(input));
    for(int i = 0; i < count; i++){
        if(kb->ifusb[i]){
            usb_iface_t handle = kb->ifusb[i];
            int pipe = get_pipe_index(handle, kUSBIn);
            uchar direction, number, transfertype, interval;
            ushort maxpacketsize;
            (*handle)->GetPipeProperties(handle, pipe, &direction, &number, &transfertype, &maxpacketsize, &interval);
            if(direction != kUSBIn && direction != kUSBAnyDirn)
                continue;
            uchar* buffer = malloc(maxpacketsize);
            input[i].kb = kb;
            input[i].buffer = buffer;
            input[i].index = i;
            input[i].pipe = pipe;
            input[i].maxsize = maxpacketsize;
            (*handle)->ReadPipeAsync(handle, 1, buffer, maxpacketsize, pipecomplete, input + i);
        } else if(kb->ifhid[i]){
            hid_dev_t handle = kb->ifhid[i];
            long maxsize = hidgetlong(handle, CFSTR(kIOHIDMaxInputReportSizeKey));
            uchar* buffer = malloc(maxsize);
            input[i].buffer = buffer;
            (*handle)->setInputReportCallback(handle, buffer, maxsize, intreport, kb, 0);
        }
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
    for(int i = 0; i < count; i++)
        free(input[i].buffer);
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
static int find_device(uint16_t idvendor, uint16_t idproduct, uint32_t location, int handle_idx){
    // Look for any partially-set up boards matching this device
    for(int i = 1; i < DEV_MAX; i++){
        if(pthread_mutex_trylock(devmutex + i))
            // If the mutex is locked then the device is obviously set up already, keep going
            continue;
        if(keyboard[i].vendor == idvendor && keyboard[i].product == idproduct){
            for(int iface = 0; iface <= IFACE_MAX; iface++){
                if(keyboard[i].location_id[iface] == location){
                    // Matched; continue setting up this device
                    keyboard[i].location_id[handle_idx] = location;
                    // Device mutex remains locked
                    return i;
                }
            }
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
            keyboard[i].location_id[handle_idx] = location;
            keyboard[i].vendor = idvendor;
            keyboard[i].product = idproduct;
            // Device mutex remains locked
            return i;
        }
        pthread_mutex_unlock(devmutex + i);
    }
    return -1;
}

static int seize_wait(long location){
    // HACK: We shouldn't seize the device until it's successfully added to the service registry.
    // Otherwise, OSX might think there's no keyboard/mouse connected.
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
    const int max_tries = 20;     // give up after ~2s
    for(int try = 0; try < max_tries; try++){
        usleep(100000);
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
    int index = find_device(idvendor, idproduct, location, 0);
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

    // Read the serial number and name (if not done yet)
    if(!kb->serial[0] && !kb->name[0]){
        UInt8 serial_idx, product_idx;
        if((*handle)->USBGetSerialNumberStringIndex(handle, &serial_idx) == kIOReturnSuccess)
            usbgetstr(handle, serial_idx, kb->serial, SERIAL_LEN);
        if((*handle)->USBGetProductStringIndex(handle, &product_idx) == kIOReturnSuccess)
            usbgetstr(handle, product_idx, kb->name, KB_NAME_LEN);
        ckb_info("Connecting %s at %s%d\n", keyboard[index].name, devpath, index);
    }

    // Iterate through the USB interfaces. Most of these will fail to open because they're already grabbed by the HID system.
    if(seize_wait(location))
        ckb_warn("seize_wait failed, connecting anyway...\n");
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
            IOObjectRelease(iface);
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
        wait_loop(err, (*plugin)->QueryInterface(plugin, CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID183), (LPVOID)&if_handle));
        if(err != kIOReturnSuccess){
            ckb_err("QueryInterface failed: %x\n", err);
            goto release;
        }
        // Plugin is no longer needed
        IODestroyPlugInInterface(plugin);

        // Get location ID in case it's different from the main USB
        (*if_handle)->GetLocationID(if_handle, kb->location_id + iface_count + 1);
        // Try to open the interface. If it succeeds, add it to the device's interface list.
        err = (*if_handle)->USBInterfaceOpenSeize(if_handle);   // no wait_loop here because this is expected to fail
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
        wait_loop(err, (*plugin)->QueryInterface(plugin, CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID182), (LPVOID*)&handle));
        if(err != kIOReturnSuccess){
            ckb_err("QueryInterface failed: %x\n", err);
            goto release;
        }
        // Plugin is no longer needed
        IODestroyPlugInInterface(plugin);

        err = (*handle)->USBDeviceOpenSeize(handle);
        if(err == kIOReturnExclusiveAccess){
            // We can't send control transfers but most of the other functions should work
            ckb_warn("Unable to seize USB handle, continuing anyway...\n");
        } else if(err != kIOReturnSuccess){
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
    // Each keyboard generates multiple match events (one for each endpoint)
    // There's no direct way to tell which of the endpoints this is, but there's a workaround
    // Each handle has a unique maximum packet size combination, so use that to place them
    long input = hidgetlong(handle, CFSTR(kIOHIDMaxInputReportSizeKey));
    long output = hidgetlong(handle, CFSTR(kIOHIDMaxOutputReportSizeKey));
    long feature = hidgetlong(handle, CFSTR(kIOHIDMaxFeatureReportSizeKey));
    int handle_idx;
    // Handle 3 is for controlling the device (only exists for RGB)
    if(feature == 64)
        handle_idx = 3;
    // Handle 2 is for Corsair inputs, unused on non-RGB
    else if(feature == 0 &&
            (((input == 64 ||
               input == 15) && output == 0) ||
            (input == 64 && output == 64) ||    // FW >= 1.20
            (input <= 1 && output == 64)))      // FW >= 2.00 (Scimitar)
        handle_idx = 2;
    // Handle 0 is for BIOS mode input (RGB) or non-RGB key input
    else if(output <= 1 && feature <= 1 &&
            (input == 8 ||                      // Keyboards
             input == 7))                       // Mice
        handle_idx = 0;
    // Handle 1 is for standard HID input (RGB) or media keys (non-RGB)
    else if(output <= 1 && feature <= 1 &&
            (input == 21 || input == 10 ||
             input == 4 ||
             input == 64))                      // FW >= 2.00 (Scimitar)
        handle_idx = 1;
    else {
        ckb_warn("Got unknown handle (I: %d, O: %d, F: %d)\n", (int)input, (int)output, (int)feature);
        return 0;
    }

    // Get the model and serial number
    uint16_t idvendor = hidgetlong(handle, CFSTR(kIOHIDVendorIDKey)), idproduct = hidgetlong(handle, CFSTR(kIOHIDProductIDKey));
    // Use the location ID key to group the handles together
    uint32_t location = hidgetlong(handle, CFSTR(kIOHIDLocationIDKey));
    int index = find_device(idvendor, idproduct, location, handle_idx + 1);
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


    // Set the handle
    if(kb->ifhid[handle_idx]){
        // This should never happen
        ckb_warn("Tried to set up ifhid[%d] for device ckb%d, but it was already set up. Skipping...\n", handle_idx, index);
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

static void iterate_devices_hid(void* context, io_iterator_t iterator){
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
        if(seize_wait(hidgetlong(handle, CFSTR(kIOHIDLocationIDKey))))
            ckb_warn("seize_wait failed, connecting anyway...\n");
        wait_loop(err, (*handle)->open(handle, kIOHIDOptionsTypeSeizeDevice));
        if(err != kIOReturnSuccess){
            ckb_warn("Failed to open device: %x\n", err);
            goto release;
        }
        // Connect it
        io_object_t* rm_notify = 0;
        usbdevice* kb = add_hid(handle, &rm_notify);
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

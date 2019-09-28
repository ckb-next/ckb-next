#include "request_hid_mac.h"

extern CFMutableDictionaryRef create_hid_device_dict();

hid_req_ret request_hid_access_mac(){
    CFMutableDictionaryRef match_dict = create_hid_device_dict();
    
    io_service_t device = IOServiceGetMatchingService(kIOMasterPortDefault, match_dict);
    if(!device){
        // Free?
        return REQUEST_ERROR;
    }
    
    IOHIDDeviceRef ref = IOHIDDeviceCreate(kCFAllocatorDefault, device);
    
    // Try to open the device
    IOReturn ret = IOHIDDeviceOpen(ref, kIOHIDOptionsTypeNone);
    if(ret == kIOReturnNotPermitted){
        // Is there a way to check if our request even succeeded?
        return REQUEST_SUCCEEDED;
    } else if(ret == kIOReturnSuccess) {
        IOHIDDeviceClose(ref, kIOHIDOptionsTypeNone);
        return REQUEST_ALREADY_ALLOWED;
    }

    // If we get here, an unknown error occurred. It may or may not be fatal, we can't really know.
    fprintf(stderr, "[E] IOHIDDeviceOpen returned %x\n", ret);
    return REQUEST_ERROR;
}

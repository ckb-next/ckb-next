#ifndef REQUEST_HID_MAC_H
#define REQUEST_HID_MAC_H
#include "os.h"

#ifdef OS_MAC
typedef enum {
    REQUEST_SUCCEEDED = 1,
    REQUEST_ALREADY_ALLOWED,
    REQUEST_ERROR
} hid_req_ret;
    
hid_req_ret request_hid_access_mac();
#endif // OS_MAC
#endif // REQUEST_HID_MAC_H

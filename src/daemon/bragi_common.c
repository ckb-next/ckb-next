#include "bragi_common.h"
#include "bragi_proto.h"

// Gets a property using the bragi protocol
// Error when return value < 0
int64_t bragi_get_property(usbdevice* kb, const uchar prop) {
    uchar pkt[BRAGI_JUMBO_SIZE] = {BRAGI_MAGIC, BRAGI_GET, prop, 0};
    uchar response[BRAGI_JUMBO_SIZE] = {0};
    if(!usbrecv(kb, pkt, sizeof(pkt), response))
        return -1;
    if(response[2]) {
        if(response[2] == BRAGI_ERROR_NOTSUPPORTED)
            ckb_warn("Failed to get property 0x%hhx. Device said it's not supported.", prop);
        else
            ckb_err("Failed to get property 0x%hhx. Error was 0x%hhx", prop, response[2]);
        return -2;
    }
    return ((uint32_t)response[5] << 16) | ((uint32_t)response[4] << 8) | response[3];
}

// Sets a property using the bragi protocol
// Error when return value < 0. Success == 0
int bragi_set_property(usbdevice* kb, const uchar prop, const ushort val) {
    uchar pkt[BRAGI_JUMBO_SIZE] = {BRAGI_MAGIC, BRAGI_SET, prop, 0, val&255, val>>8};
    uchar response[BRAGI_JUMBO_SIZE] = {0};
    if(!usbrecv(kb, pkt, sizeof(pkt), response))
        return -1;
    if(response[2]) {
        if(response[2] == BRAGI_ERROR_NOTSUPPORTED)
            ckb_warn("Failed to set property 0x%hhx. Device said it's not supported.", prop);
        else
            ckb_err("Failed to set property 0x%hhx. Error was 0x%hhx", prop, response[2]);
        return -2;
    }
    return 0;
}

static inline size_t bragi_calculate_buffer_size_common(usbdevice* kb, uint32_t data_len, int offset){
    // Calculate how many bytes are required for the buffer
    // The first packet is going to be len - ep_out_packet_size + header (offset bytes)
    int64_t size_without_first = (int64_t)data_len - kb->out_ep_packet_size + offset;
    if(size_without_first >= 0){
        // Divide to get the number of packets (out_ep_packet_size - continue write header (3 bytes))
        // 1 + because we have to account for the first packet as well
        size_t req_pkt_count = 1 + size_without_first / (kb->out_ep_packet_size - 3);
        // Get the number of bytes in the last packet. If non-zero, add another packet to the count
        if(size_without_first % (kb->out_ep_packet_size - 3))
            req_pkt_count++;
        return req_pkt_count * kb->out_ep_packet_size;
    }
    return kb->out_ep_packet_size;
}

size_t bragi_calculate_buffer_size(usbdevice* kb, uint32_t data_len){
    return bragi_calculate_buffer_size_common(kb, data_len, 7);
}

// First offset bytes must be zeroed and will be overwritten by these functions
// This is done to avoid having to allocate more memory and copy it on every write
static inline int bragi_write_to_handle_common(usbdevice* kb, uchar* pkt, uchar handle, size_t buf_len, uint32_t data_len, int offset){
#ifndef NDEBUG
    size_t bytes_req = bragi_calculate_buffer_size_common(kb, data_len, offset);
    if(bytes_req > buf_len){
        ckb_fatal("Buffer not large enough. Needs to be at least %zu bytes.", bytes_req);
        return 0;
    }
#endif
    // Add the header
    pkt[0] = BRAGI_MAGIC;
    pkt[1] = BRAGI_WRITE_DATA;
    pkt[2] = handle;
    // Add the length to the header
    // If we ever want to support big endian, len needs to be shifted
    memcpy(pkt + 3, &data_len, sizeof(uint32_t));
    // zero out the next four bytes
    //memset(pkt + 7, 0, 4);

    // Send the first packet as-is
    uchar response[BRAGI_JUMBO_SIZE] = {0};
    if(!usbrecv(kb, pkt, BRAGI_JUMBO_SIZE, response))
        return 1;

    // After sending the first chunk, if buf_len < data_len
    // start sending the additional chunks
    uchar* pkt_out = pkt;
    // Get to the end of the last packet, go back 3 bytes and insert the header for the continue write
    while((pkt_out += kb->out_ep_packet_size) < pkt + data_len + offset) {
        //printf("buf sent %ld, remaining %ld\n", pkt_out - pkt, (int64_t)data_len + offset - (pkt_out - pkt));
        pkt_out -= 3;
        pkt_out[0] = BRAGI_MAGIC;
        pkt_out[1] = BRAGI_CONTINUE_WRITE;
        pkt_out[2] = BRAGI_LIGHTING_HANDLE;
        // Send the new packet
        if(!usbrecv(kb, pkt_out, BRAGI_JUMBO_SIZE, response))
            return 1;
        // Don't return if the packet failed, as it might be something recoverable.
        // We don't really know what the error codes mean yet.
        bragi_check_success(pkt_out, response);
    }

    return 0;
}

int bragi_write_to_handle(usbdevice* kb, uchar* pkt, uchar handle, size_t buf_len, uint32_t data_len){
    return bragi_write_to_handle_common(kb, pkt, handle, buf_len, data_len, 7);
}

// Returns the size of the opened handle
static inline uint32_t bragi_probe_handle(usbdevice* kb, uchar handle){
    uchar pkt[BRAGI_JUMBO_SIZE] = {BRAGI_MAGIC, BRAGI_PROBE_HANDLE, handle, 0};
    uchar response[BRAGI_JUMBO_SIZE] = {0};
    if(!usbrecv(kb, pkt, sizeof(pkt), response))
        return 0;
    if(response[2]) {
        ckb_err("Failed to probe handle 0x%hhx. Error was 0x%hhx", handle, response[2]);
        return 0;
    }
    uint32_t len;
    // If we ever want to support big endian, len needs to be shifted
    memcpy(&len, response + 5, sizeof(uint32_t));
    return len;
}

uint32_t bragi_read_from_handle(usbdevice* kb, uchar handle, uchar** data){
    *data = NULL;
    uint32_t len = bragi_probe_handle(kb, handle);
    if(len <= 0)
        return 0;

    // Let's assume that len won't be more than 100MiB for safety
    if(len > 100*1024*1024){
        ckb_fatal("Attempted to read file larger than 100MiB (%" PRIu32 " bytes) from handle 0x%hhx", len, handle);
        return 0;
    }

    // Don't forget to free!
    *data = malloc(len);

    // Keep asking for data as long as there's more
    // signed int here because the subtraction at the end will result in negative numbers
    int64_t bytes_remaining = len;
    while(bytes_remaining > 0){
        uchar pkt[BRAGI_JUMBO_SIZE] = {BRAGI_MAGIC, BRAGI_READ_DATA, handle, 0};
        uchar response[BRAGI_JUMBO_SIZE] = {0};
        if(!usbrecv(kb, pkt, sizeof(pkt), response)){
            free(*data);
            *data = NULL;
            return 0;
        }
        if(response[2]) {
            ckb_err("Failed to read from handle 0x%hhx. Error was 0x%hhx", handle, response[2]);
            free(*data);
            *data = NULL;
            return 0;
        }

        // Subtract the header
        int64_t actual_payload = kb->out_ep_packet_size - 3;
        if(actual_payload > bytes_remaining)
            actual_payload = bytes_remaining;

        memcpy(*data + (len - bytes_remaining), response + 3, actual_payload);
        bytes_remaining -= actual_payload;
    }

    return len;
}

int bragi_close_handle(usbdevice* kb, uchar handle){
    uchar handleclose[BRAGI_JUMBO_SIZE] = {BRAGI_MAGIC, BRAGI_CLOSE_HANDLE, 1, handle, 0};
    uchar response[BRAGI_JUMBO_SIZE] = {0};
    if(!usbrecv(kb, handleclose, sizeof(handleclose), response))
        return -1;
    if(response[2])
        ckb_warn("Failed to close handle 0x%hhx. Error was 0x%hhx", handle, response[2]);
    return response[2];
}

int bragi_open_handle(usbdevice* kb, uchar handle, ushort resource){
    // FIXME: Endianness
    uchar handleopen[BRAGI_JUMBO_SIZE] = {BRAGI_MAGIC, BRAGI_OPEN_HANDLE, handle, resource & 0xFF, resource >> 8, 0};
    uchar response[BRAGI_JUMBO_SIZE] = {0};
    if(!usbrecv(kb, handleopen, sizeof(handleopen), response))
        return -1;
    if(response[2] == 0x03){
        bragi_close_handle(kb, handle);
        if(!usbrecv(kb, handleopen, sizeof(handleopen), response))
            return -1;
        if(response[2])
            ckb_err("Failed to open handle 0x%hhx. Error was 0x%hhx", handle, response[2]);
    }
    else if(response[2])
        ckb_err("Failed to open handle 0x%hhx. Error was 0x%hhx", handle, response[2]);
    return response[2];
}

// We use this to get the name of the device as it's not reported wirelessly
// Only wireless devices (and not dongles) should be added here
static inline char* bragi_wl_string(usbdevice* kb){
    if(kb->vendor != V_CORSAIR){
        ckb_err("Vendor is not V_CORSAIR");
        return NULL;
    }
    switch(kb->product){
    case P_K57_U:
        return "CORSAIR K57 RGB Wireless Gaming Keyboard";
    case P_IRONCLAW_W_U:
        return "CORSAIR IRONCLAW RGB WIRELESS Gaming Mouse";
    case P_HARPOON_WL_U:
        return "CORSAIR HARPOON RGB WIRELESS Gaming Mouse";
    case P_DARK_CORE_RGB_PRO:
        return "CORSAIR DARK CORE RGB PRO Gaming Mouse";
    case P_DARK_CORE_RGB_PRO_SE:
        return "CORSAIR DARK CORE RGB PRO SE Gaming Mouse";
    default:
        ckb_err("Unknown product 0x%hx", kb->product);
        return NULL;
    }
}

// Returns the first available usbdevice in `keyboard`
// If it can't find one, it returns NULL
// It locks dmutex and it remains locked after the function returns
static inline usbdevice* find_available_usbdevice(){
    // Find a free device slot
    for(int i = 1; i < DEV_MAX; i++){
        usbdevice* kb = keyboard + i;
        if(queued_mutex_trylock(dmutex(kb))){
            // If the mutex is locked then the device is obviously in use, so keep going
            continue;
        }

        // Ignore it if it has already been initialised
        if(kb->status > DEV_STATUS_DISCONNECTED){
            queued_mutex_unlock(dmutex(kb));
            continue;
        }

        return kb;
    }
    return NULL;
}

void bragi_update_dongle_subdevs(usbdevice* kb, int prop){
    // We don't want any other threads messing with this while we're probing
    // Do note that this also blocks USB IO
    pthread_mutex_lock(cmutex(kb));

    // First, check if any devices have been disconnected
    for(int i = 1; i < 8; i++){
        if((prop >> i) & 1)
            continue;

        if(!kb->children[i-1])
            continue;

        // Disconnect the device
        usbdevice* subkb = kb->children[i-1];
        queued_mutex_lock(dmutex(subkb));
        ckb_info("ckb%d: Bragi subdevice ckb%d disappeared", INDEX_OF(kb, keyboard), INDEX_OF(subkb, keyboard));
        closeusb(subkb);
        kb->children[i-1] = NULL;
        queued_mutex_unlock(dmutex(subkb));
    }

    // Then, check if any new devices have been connected
    for(int i = 1; i < 8; i++){
        if(!((prop >> i) & 1))
            continue;

        // Skip this device if it's already been added
        if(kb->children[i-1])
            continue;

        ckb_info("Found new bragi subdevice %d", i);

        usbdevice* subkb = find_available_usbdevice();
        if(!subkb){
            ckb_err("No more free devices");
            break;
        }
        subkb->status = DEV_STATUS_CONNECTING;
        subkb->fwversion = 1234; // invalid
        subkb->parent = kb;

        subkb->out_ep_packet_size = kb->out_ep_packet_size;

        // Assign a mouse vtable for now, can be changed later after we get vid/pid
        memcpy(&subkb->vtable, &vtable_bragi_mouse, sizeof(devcmd));

        subkb->bragi_child_id = i;

        // Add the device to our children array
        kb->children[i-1] = subkb;
        // Must be unlocked as soon as possible, before we try to get any properties
        pthread_mutex_unlock(cmutex(kb));

        // Fill dev information
        ushort vid = bragi_get_property(subkb, BRAGI_VID);
        ushort pid = bragi_get_property(subkb, BRAGI_PID);

        ckb_info("ckb%d: Subkb vendor: 0x%hx, product: 0x%hx", INDEX_OF(subkb, keyboard), vid, pid);
        subkb->vendor = vid;
        subkb->product = pid;

        if(vid == 0xffff || pid == 0xffff){
            closeusb(subkb);
            queued_mutex_unlock(dmutex(subkb));
            pthread_mutex_lock(cmutex(kb));
            kb->children[i-1] = NULL;
            pthread_mutex_unlock(cmutex(kb));
            continue;
        }

        const char* const name = bragi_wl_string(subkb);
        if(name)
            snprintf(subkb->name, KB_NAME_LEN, "%s", name);

        setupusb(subkb);
        pthread_mutex_lock(cmutex(kb));
    }
    pthread_mutex_unlock(cmutex(kb));
}

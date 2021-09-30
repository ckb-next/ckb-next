#include "bragi_common.h"
#include "bragi_proto.h"

// Gets a property using the bragi protocol
// Error when return value < 0
long int bragi_get_property(usbdevice* kb, const uchar prop) {
    uchar pkt[BRAGI_JUMBO_SIZE] = {BRAGI_MAGIC, BRAGI_GET, prop, 0};
    uchar response[BRAGI_JUMBO_SIZE] = {0};
    if(!usbrecv(kb, pkt, sizeof(pkt), response))
        return -1;
    if(response[2]) {
        ckb_err("Failed to get property 0x%hhx. Error was 0x%hhx", prop, response[2]);
        return -2;
    }
    return ((ushort)response[4] << 8) | response[3];
}

// Sets a property using the bragi protocol
// Error when return value < 0. Success == 0
int bragi_set_property(usbdevice* kb, const uchar prop, const uchar val) {
    uchar pkt[BRAGI_JUMBO_SIZE] = {BRAGI_MAGIC, BRAGI_SET, prop, 0, val, 0};
    uchar response[BRAGI_JUMBO_SIZE] = {0};
    if(!usbrecv(kb, pkt, sizeof(pkt), response))
        return -1;
    if(response[2]) {
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
#warning "Check if the device responded with success"
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

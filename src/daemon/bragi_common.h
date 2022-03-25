#ifndef BRAGI_COMMON_H
#define BRAGI_COMMON_H

#include "includes.h"
#include "structures.h"
#include "usb.h"
#include "device.h" // for usbdevice keyboard[]
#include <stdint.h>

long int bragi_get_property(usbdevice* kb, const uchar prop);
int bragi_set_property(usbdevice* kb, const uchar prop, const ushort val);

#define bragi_check_success(a, b) _bragi_check_success((a), (b), INDEX_OF(kb, keyboard), __func__, __FILE__, __LINE__)

static inline int _bragi_check_success(const uchar* out, const uchar* in, int dev, const char* fn, const char* file, int line){
    int res = (out[1] != in[1] || in[2]);
    if(res)
        ckb_err("ckb%d: %s (via %s:%d) Bragi device returned failure (0x%hhx - 0x%hhx, 0x%hhx)", dev, fn, file, line, out[1], in[1], in[2]);
    return res;
};


#define BRAGI_PKT_SIZE_CHECK(len, f, l)      if(len != BRAGI_JUMBO_SIZE) \
                                    { \
                                        ckb_fatal_fn("ckb%d: Invalid len (%lld) in Bragi backend. Expected %d", f, l, INDEX_OF(kb, keyboard), (long long int)len, BRAGI_JUMBO_SIZE); \
                                        return -1; \
                                    }

size_t bragi_calculate_buffer_size(usbdevice* kb, uint32_t data_len);
int bragi_write_to_handle(usbdevice* kb, uchar* pkt, uchar handle, size_t buf_len, uint32_t data_len);
uint32_t bragi_read_from_handle(usbdevice* kb, uchar handle, uchar** data);
int bragi_open_handle(usbdevice* kb, uchar handle, ushort resource);
int bragi_close_handle(usbdevice* kb, uchar handle);
void bragi_update_dongle_subdevs(usbdevice* kb, int prop);
#endif

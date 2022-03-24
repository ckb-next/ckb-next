#include "bragi_notification.h"
#include "bragi_common.h"

typedef struct {
    usbdevice* kb;
    uchar prop;
} bragithreadctx;

// This runs on a new thread so that the input thread isn't blocked
static void* _add_remove_new_bragi_device(void* context){
    bragithreadctx* ctx = context;
    usbdevice* kb = ctx->kb;
    ckb_info("ckb%d: bragi dongle hotplug thread started", INDEX_OF(kb, keyboard));

    bragi_update_dongle_subdevs(kb, ctx->prop);

    free(context);
    ckb_info("ckb%d: bragi dongle hotplug thread finished", INDEX_OF(kb, keyboard));
    return NULL;
}

void bragi_process_notification(usbdevice* kb, usbdevice* subkb, const uchar* const buffer){
    // Do NOT lock dmutex here unless it's required in very specific notifications
    // Doing so will result in devices failing to initialise

    // We'll just assume that no device other than a dongle ever sends BRAGI_NOTIFICATION_CONNECTIONSTATUS
    if(buffer[2] == BRAGI_NOTIFICATION_CONNECTIONSTATUS){
        bragithreadctx* ctx = malloc(sizeof(bragithreadctx));
        ctx->kb = kb;
        ctx->prop = buffer[4];

        pthread_t t;
        pthread_create(&t, NULL, _add_remove_new_bragi_device, ctx);
        pthread_detach(t);
    } else {
        ckb_warn("ckb%d: Unknown bragi notification for dev %d", INDEX_OF(kb, keyboard), INDEX_OF(subkb, keyboard));
    }
}

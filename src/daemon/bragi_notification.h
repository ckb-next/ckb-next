#ifndef BRAGI_NOTIFICATION_H
#define BRAGI_NOTIFICATION_H

#include "structures.h"
#include "bragi_proto.h"

void bragi_process_notification(usbdevice* kb, usbdevice* subkb, const uchar* const buffer);

#endif // BRAGI_NOTIFICATION_H

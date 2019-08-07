#include "command.h"
#include "device.h"
#include "devnode.h"
#include "firmware.h"
#include "input.h"
#include "profile.h"
#include "usb.h"

int cmd_active_lightning_node(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4){
    (void)dummy1;
    (void)dummy2;
    (void)dummy3;
    (void)dummy4;
    kb->active = !!1;
    kb->profile->lastlight.forceupdate = 1;
    return 0;
}

int cmd_idle_lightning_node(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4){
    (void)dummy1;
    (void)dummy2;
    (void)dummy3;
    (void)dummy4;
    kb->active = !!0;
    kb->profile->lastlight.forceupdate = 1;
    return 0;
}
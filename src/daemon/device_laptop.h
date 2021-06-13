#ifndef DEVICE_LAPTOP_H
#define DEVICE_LAPTOP_H

int cmd_active_laptop(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4);
int cmd_idle_laptop(usbdevice* kb, usbmode* dummy1, int dummy2, int dummy3, const char* dummy4);
int updatergb_laptop(usbdevice* kb, int force);
int start_laptop_kb(usbdevice* kb, int makeactive);

#endif

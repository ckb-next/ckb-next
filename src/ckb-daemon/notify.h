#ifndef NOTIFY_H
#define NOTIFY_H

#include "includes.h"

// Prints output to a keyboard's notification node. Use nodenumber = -1 to print to all nodes.
// Specify a USB mode to print "mode <n>" before the notification. A null mode will not print a number.
void nprintf(usbdevice* kb, int nodenumber, usbmode* mode, const char* format, ...);

// Prints output to a root notification node. Use nodenumber = -1 to print to all nodes.
void nrprintf(int nodenumber, const char* format, ...);

// Notifies of a device connection or disconnection.
void notifyconnect(usbdevice* kb, int connecting);

// Prints a key's current state to a notification node
void nprintkey(usbdevice* kb, int nnumber, const key* keymap, int keyindex, int down);
// Prints the current indicator state to a notification node (led should be an I_ constant)
void nprintind(usbdevice* kb, int nnumber, int led, int on);

// Enables or disables notification for a key
void cmd_notify(usbdevice* kb, usbmode* mode, const key* keymap, int nnumber, int keyindex, const char* toggle);
// Gets info about a particular setting.
void getinfo(usbdevice* kb, usbmode* mode, int nnumber, const char* setting);

#endif

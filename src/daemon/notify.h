#ifndef NOTIFY_H
#define NOTIFY_H

#include "includes.h"
#include "device.h"

// Note: Lock imutex (see device.h) before accessing notification settings/nodes
// The cmd_ functions do this automatically, all others need to be done before

// Prints output to a keyboard's notification node. Use nodenumber = -1 to print to all nodes.
// Specify a USB mode to print "mode <n>" before the notification. A null mode will not print a number
void nprintf(usbdevice* kb, int nodenumber, usbmode* mode, const char* format, ...);

// Prints a key's current state to a notification node
void nprintkey(usbdevice* kb, int nnumber, int keyindex, int down);
// Prints the current indicator state to a notification node (led should be an I_ constant)
// MUTEXES: Lock imutex before calling
void nprintind(usbdevice* kb, int nnumber, int led, int on);

// Enables or disables notification for a key
// MUTEXES: Locks imutex during operation. Unlocks on close.
void cmd_notify(usbdevice* kb, usbmode* mode, int nnumber, int keyindex, const char* toggle);
// Gets info about a particular setting.
// MUTEXES: Locks imutex during operation. Unlocks on close.
void cmd_get(usbdevice* kb, usbmode* mode, int nnumber, int dummy, const char* setting);

#endif  // NOTIFY_H

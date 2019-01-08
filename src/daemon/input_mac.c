#include "command.h"
#include "device.h"
#include "input.h"

#ifdef OS_MAC
#ifndef OS_MAC_LEGACY
#include <errno.h>
#include <sys/sysctl.h>
#include "input_mac_vhid.h"

// Functions to add/remove keys from report
void add_to_keys(int scan, vhid_keys* input){
    for(int i = 0; i < VHID_KEYS_LEN; i++){
        if(input->keys_[i] == 0){
            input->keys_[i] = scan;
            return;
        }
    }
    ckb_warn("Dropping excess keypress\n");
}

void remove_from_keys(int scan, vhid_keys* input){
    for(int i = 0; i < VHID_KEYS_LEN; i++){
        if(input->keys_[i] == scan){
            input->keys_[i] = 0;
            return;
        }
    }
    ckb_warn("Couldn't find key to release\n");
}

// Functions to add/remove buttons from report
void add_to_buttons(int scan, vhid_mouseinput* input){
    input->buttons.buttons_ |= (0x1 << (scan - 1));
}

void remove_from_buttons(int scan, vhid_mouseinput* input){
    input->buttons.buttons_ &= ~(0x1 << (scan - 1));
}

void (*add_remove_keys)(int scan, vhid_keys* input);

int os_inputopen(usbdevice* kb){
    memset(&kb->kbinput_key, 0, sizeof(kb->kbinput_key));
    kb->kbinput_key.report_id_ = 1;

    memset(&kb->kbinput_consumer, 0, sizeof(kb->kbinput_consumer));
    kb->kbinput_consumer.report_id_ = 2;

    memset(&kb->kbinput_avtopcase, 0, sizeof(kb->kbinput_avtopcase));
    kb->kbinput_avtopcase.report_id_ = 3;

    memset(&kb->kbinput_vendor, 0, sizeof(kb->kbinput_vendor));
    kb->kbinput_vendor.report_id_ = 4;

    memset(&kb->mouseinput, 0, sizeof(kb->mouseinput));

    // Check if Karabiner VirtualHIDDevice is loaded
    io_service_t service = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceNameMatching(VIRTUAL_HID_ROOT_NAME));

    if(!service){
        ckb_info("Attempting to load kext\n");
        system("kextutil " VIRTUAL_HID_KEXT_PATH );
        ckb_info("Waiting for kext...\n");

        // Check every half a second if the kext is initialised, with a max of 10 seconds.
        for(int i = 0; i < 20; i++){
            clock_nanosleep(CLOCK_MONOTONIC, 0, &(struct timespec) {.tv_nsec = 500000000}, NULL);
            service = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceNameMatching(VIRTUAL_HID_ROOT_NAME));
            // Break out of the loop if we matched a service
            if(service)
                break;
        }

        // If the loop finished and the VHIDDevice is still not available, abort
        if(!service){
            ckb_fatal("Unable to open VirtualHIDDevice\n");
            return 1;
        }
    }

    kern_return_t kr;
    kr = IOServiceOpen(service, mach_task_self(), kIOHIDServerConnectType, &(kb->event));
    if(kr != KERN_SUCCESS)
        ckb_fatal("IOServiceOpen for VirtualHIDKeyboard error\n");

    kr = IOServiceOpen(service, mach_task_self(), kIOHIDServerConnectType, &(kb->event_mouse));
    if(kr != KERN_SUCCESS)
        ckb_fatal("IOServiceOpen for VirtualHIDPointing error\n");

    vhid_properties properties = {0};

    kr = IOConnectCallStructMethod(kb->event,
                                    initialize_virtual_hid_keyboard,
                                    &properties, sizeof(vhid_properties),
                                    NULL, 0);
    if(kr != KERN_SUCCESS){
        ckb_fatal("VirtualHIDKeyboard init error\n");
    }

    ckb_info("Waiting for VirtualHIDKeyboard...\n");

    while(1){
        bool ready = false;
        size_t readysize = sizeof(ready);
        kr = IOConnectCallStructMethod(kb->event,
                                    is_virtual_hid_keyboard_ready,
                                    NULL, 0,
                                    &ready, &readysize);
        if (kr == KERN_SUCCESS && ready){
            ckb_info("VirtualHIDKeyboard ready\n");
            break;
        }

        clock_nanosleep(CLOCK_MONOTONIC, 0, &(struct timespec) {.tv_sec = 1}, NULL);
    }

    /* MOUSE */

    kr = IOConnectCallStructMethod(kb->event_mouse,
                                    initialize_virtual_hid_pointing,
                                    NULL, 0,
                                    NULL, 0);
    if(kr != KERN_SUCCESS)
        ckb_fatal("VirtualHIDPointing init error %x\n", kr);

    clock_nanosleep(CLOCK_MONOTONIC, 0, &(struct timespec) {.tv_nsec = 10000000}, NULL);

    return 0;
}

void os_inputclose(usbdevice* kb){
    if(kb->event){
        ckb_info("Terminating VirtualHIDKeyboard\n");
        IOConnectCallStructMethod(kb->event, reset_virtual_hid_keyboard, NULL, 0, NULL, 0);
        clock_nanosleep(CLOCK_MONOTONIC, 0, &(struct timespec) {.tv_nsec = 10000000}, NULL);
        IOConnectCallStructMethod(kb->event, terminate_virtual_hid_keyboard, NULL, 0, NULL, 0);
        IOServiceClose(kb->event);
        kb->event = 0;
    }
    if(kb->event_mouse){
        ckb_info("Terminating VirtualHIDPointing\n");
        IOConnectCallStructMethod(kb->event_mouse, reset_virtual_hid_pointing, NULL, 0, NULL, 0);
        clock_nanosleep(CLOCK_MONOTONIC, 0, &(struct timespec) {.tv_nsec = 10000000}, NULL);
        IOConnectCallStructMethod(kb->event_mouse, terminate_virtual_hid_pointing, NULL, 0, NULL, 0);
        IOServiceClose(kb->event_mouse);
        kb->event_mouse = 0;
    }
}

// Unlike Linux, OSX keyboards have independent caps lock states. This means they're set by the driver itself so we don't poll for external events.
// However, updating indicator state requires locking dmutex and we never want to do that in the input thread.
// Instead, we launch a single-shot thread to update the state.
static void* indicator_update(void* context){
    char indicthread_name[THREAD_NAME_MAX] = "ckbX indicator";
    usbdevice* kb = context;

    indicthread_name[3] = INDEX_OF(kb, keyboard) + '0';
    pthread_setname_np(indicthread_name);

    pthread_mutex_lock(dmutex(kb));
    {
        pthread_mutex_lock(imutex(kb));
        IOOptionBits modifiers = kb->modifiers;
        // Allow the thread to be spawned again
        kb->indicthread = 0;
        pthread_mutex_unlock(imutex(kb));
        // Num lock on, Caps dependent on modifier state
        uchar ileds = 1 | !!(modifiers & NX_ALPHASHIFTMASK) << 1;
        kb->hw_ileds = ileds;
        kb->vtable->updateindicators(kb, 0);
    }
    pthread_mutex_unlock(dmutex(kb));
    return 0;
}

void os_keypress(usbdevice* kb, int scancode, int down){
    if(scancode & SCAN_MOUSE){
        if(scancode == BTN_WHEELUP || scancode == BTN_WHEELDOWN) {
            kb->mouseinput.vertical_wheel = (down ? ((scancode == BTN_WHEELUP) ? 1 : -1) : 0);
        } else {
            int scan = (scancode & ~SCAN_MOUSE);
            if(down)
                add_to_buttons(scan, &kb->mouseinput);
            else
                remove_from_buttons(scan, &kb->mouseinput);
        }
        IOConnectCallStructMethod(kb->event_mouse, post_pointing_input_report, &kb->mouseinput, sizeof(kb->mouseinput), NULL, 0);
        return;
    }
    // Pick the appropriate add or remove function
    add_remove_keys = down ? &add_to_keys : &remove_from_keys;

    if(scancode == KEY_FN) {
        (*add_remove_keys)(scancode, &(kb->kbinput_avtopcase.keys));
        IOConnectCallStructMethod(kb->event, post_apple_vendor_top_case_input_report, &kb->kbinput_avtopcase, sizeof(kb->kbinput_avtopcase), NULL, 0);
    }/* else if(IS_VENDOR(scancode)) {
        scancode = scancode - KEY_CONSUMER;
        ckb_info("Vendor %x\n", scancode);
        (*add_remove_keys)(scancode, &(kb->kbinput_vendor.keys));
        kb->kbinput_vendor.keys.keys_[0] = 0x10;
        IOConnectCallStructMethod(kb->event, post_apple_vendor_keyboard_input_report, &kb->kbinput_vendor, sizeof(kb->kbinput_vendor), NULL, 0);
    }*/ else if(IS_CONSUMER(scancode)){
        scancode = scancode - KEY_CONSUMER;
        (*add_remove_keys)(scancode, &(kb->kbinput_consumer.keys));
        IOConnectCallStructMethod(kb->event, post_consumer_input_report, &kb->kbinput_consumer, sizeof(kb->kbinput_consumer), NULL, 0);
    } else {
        if(scancode == KEY_CAPSLOCK){
            if(down) {
                kb->modifiers ^= NX_ALPHASHIFTMASK;

                // Detach a thread to update the indicator state
                if(!kb->indicthread){
                    // The thread is only spawned if kb->indicthread is null.
                    // Due to the logic inside the thread, this means that it could theoretically be spawned twice, but never a third time.
                    // Moreover, if it is spawned more than once, the indicator state will remain correct due to dmutex staying locked.
                    if(!pthread_create(&kb->indicthread, 0, indicator_update, kb))
                        pthread_detach(kb->indicthread);
                }
            }
        }
        (*add_remove_keys)(scancode, &(kb->kbinput_key.keys));
        IOConnectCallStructMethod(kb->event, post_keyboard_input_report, &kb->kbinput_key, sizeof(kb->kbinput_key), NULL, 0);
    }
}

void os_mousemove(usbdevice* kb, int x, int y){
    kb->mouseinput.x = x;
    kb->mouseinput.y = y;
    IOConnectCallStructMethod(kb->event_mouse, post_pointing_input_report, &kb->mouseinput, sizeof(kb->mouseinput), NULL, 0);
    // If these aren't cleared, button events will also move the cursor
    kb->mouseinput.x = 0;
    kb->mouseinput.y = 0;

}

int os_setupindicators(usbdevice* kb){
    // Set NumLock on permanently
    kb->hw_ileds = kb->hw_ileds_old = kb->ileds = 1;
    return 0;
}

#endif
#endif

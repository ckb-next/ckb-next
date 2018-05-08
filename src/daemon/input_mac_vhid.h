#ifndef OS_MAC_LEGACY

// Based on https://github.com/tekezo/Karabiner-Elements/blob/master/src/vendor/Karabiner-VirtualHIDDevice/dist/include/karabiner_virtual_hid_device.hpp

#define VIRTUAL_HID_ROOT_NAME "org_pqrs_driver_Karabiner_VirtualHIDDevice_VirtualHIDRoot_v060800"
#define VIRTUAL_HID_KEXT_PATH "/Library/Extensions/org.pqrs.driver.Karabiner.VirtualHIDDevice.v060800.kext"
// Kext constant. Don't modify unless you're absolutely sure you know what you're doing.
#define VHID_KEYS_LEN 32

enum user_client_method {
    // VirtualHIDKeyboard
    initialize_virtual_hid_keyboard,
    terminate_virtual_hid_keyboard,
    is_virtual_hid_keyboard_ready,
    post_keyboard_input_report,
    post_consumer_input_report,
    post_apple_vendor_top_case_input_report,
    post_apple_vendor_keyboard_input_report,
    reset_virtual_hid_keyboard,

    // VirtualHIDPointing
    initialize_virtual_hid_pointing,
    terminate_virtual_hid_pointing,
    post_pointing_input_report,
    reset_virtual_hid_pointing,

    end_,
};

enum modifier {
    left_control = 0x1 << 0,
    left_shift = 0x1 << 1,
    left_option = 0x1 << 2,
    left_command = 0x1 << 3,
    right_control = 0x1 << 4,
    right_shift = 0x1 << 5,
    right_option = 0x1 << 6,
    right_command = 0x1 << 7,
};

// VirtualHIDKeyboard
typedef struct keyboard_initialization {
    uint8_t country_code;
} vhid_properties;

typedef struct keys {
    uint8_t keys_[VHID_KEYS_LEN];
} vhid_keys;

typedef struct modifiers {
    uint8_t modifiers_;
} vhid_modifiers;

typedef struct keyboard_input {
    uint8_t report_id_;
    vhid_modifiers modifiers;
    uint8_t reserved;
    vhid_keys keys;
} vhid_kbinput;

typedef struct consumer_input {
    uint8_t report_id_;
    vhid_keys keys;
} vhid_kbconsumerinput;

typedef struct vendor_input {
    uint8_t report_id_;
    vhid_keys keys;
} vhid_kbvendorinput;

typedef struct avtopcase_input {
    uint8_t report_id_;
    vhid_keys keys;
} vhid_kbavtopcaseinput;


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

// VirtualHIDPointing
typedef struct buttons {
    uint32_t buttons_;
} vhid_buttons;

typedef struct pointing_input {
    vhid_buttons buttons;
    uint8_t x;
    uint8_t y;
    uint8_t vertical_wheel;
    uint8_t horizontal_wheel;
} vhid_mouseinput;

// Functions to add/remove buttons from report
void add_to_buttons(int scan, vhid_mouseinput* input){
    input->buttons.buttons_ |= (0x1 << (scan - 1));
}

void remove_from_buttons(int scan, vhid_mouseinput* input){
    input->buttons.buttons_ &= ~(0x1 << (scan - 1));
}

void (*add_remove_keys)(int scan, vhid_keys* input);
#endif

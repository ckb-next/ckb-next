#include <jansson.h>
#include <ckbnextconfig.h>
#include "includes.h"
#include "structures.h"
#include <sodium.h>
#include "keymap.h"
#include "json.h"

#ifndef JANSSON_THREAD_SAFE_REFCOUNT
#error "Jansson is built without thread safe refcount"
#endif

static const json_t* hid_json_root = NULL;

extern const char* proto_str_map[_PROTO_MAX];

static const char* json_paths[] = {
    // FIXME: Add ckb-next-daemon path hid.json, not PWD
    "./hid.json",
    CKB_NEXT_DAEMON_ETC_PATH "hid.json",
    CKB_NEXT_DAEMON_SHARE_PATH "hid.json",
};

void json_deinit() {
    if(!hid_json_root)
        return;
    json_decref((json_t*)hid_json_root);
}

static inline bool ensure_protocols_exist(const json_t* json) {
    for(int i = 0; i < _PROTO_MAX; i++) {
        json_t* proto;
        if(!(proto = json_object_get(json, proto_str_map[i]))) {
            return false;
        }

        json_t* descriptors;
        if(!(descriptors = json_object_get(proto, "descriptors"))) {
            return false;
        }

        // FIXME descriptors must be array

    }
    return true;
}

bool json_init() {
    json_error_t error;
    for(size_t i = 0; i < sizeof(json_paths)/sizeof(*json_paths); i++) {
        hid_json_root = json_load_file(json_paths[i], 0, &error);
        if(hid_json_root) {
            ckb_info("Loaded %s", json_paths[i]);
            break;
        }
    }
    if(!hid_json_root) {
        if(error.line < 0)
            ckb_fatal("Error loading json: %s", error.text);
        else
            ckb_fatal("Error parsing json at line %d: %s", error.line, error.text);
        return false;
    }

    if(!json_is_object(hid_json_root)) {
        ckb_fatal("No root json object");
        json_deinit();
        return false;
    }

    json_t* protocols = json_object_get(hid_json_root, "protocols");
    if(!protocols) {
        ckb_fatal("Could not find protocols object");
        json_deinit();
        return false;
    }

    ensure_protocols_exist(protocols);
    if(!ensure_protocols_exist(protocols)) {
        ckb_fatal("Could not find all protocols in hid.json");
        json_deinit();
        return false;
    }

    return true;
}

void json_print_descriptor_comment(json_t* json) {
    json_t* comment_json = json_object_get(json, "comment");
    if(!comment_json) {
        ckb_fatal("Could not find report descriptor hash for %s", "FIXME");
        return;
    }

    if(!json_is_string(comment_json)) {
        ckb_fatal("Descriptor comment is not a string");
        return;
    }

    const char* comment = json_string_value(comment_json);
    ckb_info("Descriptor matched %s", comment);
}

bool json_parse_descriptor_handler(json_t* json, endpoint_t* descriptor) {
    json_t* handler_json = json_object_get(json, "handler");
    if(!handler_json) {
        ckb_fatal("Could not find report descriptor handler for %s", "FIXME");
        return false;
    }

    if(json_is_null(handler_json)) {
        ckb_info("Handler is NULL");
        descriptor->handler = NULL;
        return true;
    } else if(!json_is_string(handler_json)) {
        ckb_fatal("Descriptor handler is not a string");
        return false;
    }

    const char* handler = json_string_value(handler_json);
    if(!strcmp(handler, "proces_input_urb")) {
        descriptor->handler = process_input_urb;
    } else {
        ckb_fatal("Unknown handler %s", handler);
        return false;
    }

    return true;
}

bool json_parse_descriptor_flags(json_t* json, endpoint_type_t* ep_type) {
    json_t* flags_array = json_object_get(json, "flags");
    if(!flags_array) {
        ckb_fatal("Could not find flags array");
        return false;
    }
    if(!json_is_array(flags_array)) {
        ckb_fatal("Flags are not an array");
        return false;
    }

    for(size_t i = 0; i < json_array_size(flags_array); i++) {
        json_t* flag_json = json_array_get(flags_array, i);
        if(!flag_json) {
            ckb_fatal("This should never happen in a flag array");
            return false;
        }

        if(!json_is_string(flag_json)) {
            ckb_fatal("Flag is not a string");
            return false;
        }

        const char* flag = json_string_value(flag_json);
        if(!strcmp(flag, "hid_in")) {
            ep_type->hid_in = true;
        } else if(!strcmp(flag, "hid_extra_in")) {
            ep_type->hid_extra_in = true;
        } else if(!strcmp(flag, "vendor_in")) {
            ep_type->vendor_in = true;
        } else if(!strcmp(flag, "cmd_in")) {
            ep_type->cmd_in = true;
        } else if(!strcmp(flag, "cmd_out")) {
            ep_type->cmd_out = true;
        } else if(!strcmp(flag, "bios_mode")) {
            ep_type->bios_mode = true;
        } else if(!strcmp(flag, "dont_claim")) {
            ep_type->dont_claim = true;
        } else {
            ckb_err("Unknown endpoint flag %s", flag);
        }

    }

    return true;
}

bool json_parse_descriptor_hash(json_t* json, uchar bytes[crypto_hash_sha256_BYTES]) {
    //static_assert(sizeof(bytes) == sizeof(((usbdevice*)0)->hid_interfaces[0].report_descriptor_hash));
    json_t* hash_json_str = json_object_get(json, "hash");
    if(!hash_json_str) {
        ckb_fatal("Could not find report descriptor hash for %s", "FIXME");
        return false;
    }

    const char* hash_hex = json_string_value(hash_json_str);
    if(!hash_hex) {
        ckb_fatal("Could not read report descriptor hash");
        return false;
    }

    if(strlen(hash_hex) != 64) {
        ckb_fatal("Hash is not 64 characters");
        return false;
    }

    for(int s = 0, i = 0; s < 64; s+=2, i++) {
        if(sscanf(hash_hex + s, "%2hhx", bytes + i) != 1) {
            ckb_fatal("Invalid character in json");
            return false;
        }
    }

    return true;
}

json_t* json_get_descriptors_for_protocol(usbdevice* kb) {
    // json_init() ensures these exist
    json_t* protocols = json_object_get(hid_json_root, "protocols");
    json_t* bragi = json_object_get(protocols, proto_str_map[kb->protocol]);
    json_t* descriptors = json_object_get(bragi, "descriptors");
    return descriptors;
}

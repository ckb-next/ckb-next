#include "structures.h"
#include <jansson.h>
#include <sodium.h>

//extern const json_t* hid_json_root;

void json_print_descriptor_comment(json_t* json);
bool json_parse_descriptor_handler(json_t* json, endpoint_t* descriptor);
bool json_parse_descriptor_flags(json_t* json, endpoint_type_t* ep_type);
bool json_parse_descriptor_hash(json_t* json, uchar bytes[crypto_hash_sha256_BYTES]);
json_t* json_get_descriptors_for_protocol(usbdevice* kb);

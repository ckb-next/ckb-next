#include "../ckb/ckb-anim.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>

void ckb_info(){
    // Plugin info
    CKB_NAME("Random");
    CKB_VERSION("0.3");
    CKB_COPYRIGHT("2014", "MSC");
    CKB_LICENSE("GPLv2");
    CKB_GUID("{22418DA4-A181-4B93-A4D3-03682BA283D2}");
    CKB_DESCRIPTION("An effect that changes key colors randomly.");

    // Effect parameters
    CKB_PARAM_BOOL("fade", "Fade in", 0);
    CKB_PARAM_BOOL("useopacity", "Randomize opacity", 0);

    // Timing/input parameters
    CKB_PARAM_TRIGGER(1);
    CKB_PARAM_DURATION(1.);
}

int fadein = 0, useopacity = 0;

void ckb_parameter(ckb_runctx* context, const char* name, const char* value){
    CKB_PARSE_BOOL("fade", &fadein) {}
    CKB_PARSE_BOOL("useopacity", &useopacity) {}
}

void ckb_keypress(ckb_runctx* context, ckb_key* key, int state){

}

typedef struct {
    unsigned char a, r, g, b;
} rgb;

rgb* current = 0;
rgb* target = 0;

void newtarget(rgb* data, unsigned count){
    for(unsigned i = 0; i < count; i++){
        rgb* key = data + i;
        key->a = useopacity ? rand() % 256 : 255;
        key->r = rand() % 256;
        key->g = rand() % 256;
        key->b = rand() % 256;
    }
}

void ckb_start(ckb_runctx* context){
    ckb_key* keys = context->keys;
    unsigned count = context->keycount;
    if(!current){
        current = malloc(count * sizeof(rgb));
        target = malloc(count * sizeof(rgb));
        srand(time(NULL));
    }
    newtarget(current, count);
    newtarget(target, count);
    // Set all keys to current
    for(unsigned i = 0; i < count; i++){
        ckb_key* key = keys + i;
        if(fadein)
            current[i].a = 0;
        key->a = current[i].a;
        key->r = current[i].r;
        key->g = current[i].g;
        key->b = current[i].b;
    }
}

int ckb_frame(ckb_runctx* context, double delta){
    ckb_key* keys = context->keys;
    unsigned count = context->keycount;
    static double phase = 0.;
    phase += delta;
    if(phase >= 1.){
        phase -= 1.;
        rgb* temp = target;
        target = current;
        current = temp;
        newtarget(target, count);
    }
    for(unsigned i = 0; i < count; i++){
        ckb_key* key = keys + i;
        key->a = round(current[i].a * (1. - phase) + target[i].a * phase);
        key->r = round(current[i].r * (1. - phase) + target[i].r * phase);
        key->g = round(current[i].g * (1. - phase) + target[i].g * phase);
        key->b = round(current[i].b * (1. - phase) + target[i].b * phase);
    }
    return 0;
}

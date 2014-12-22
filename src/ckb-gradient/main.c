#include "../ckb/ckb-anim.h"
#include <math.h>

void ckb_info(){
    // Plugin info
    CKB_NAME("Gradient");
    CKB_VERSION("0.2");
    CKB_COPYRIGHT("2014", "MSC");
    CKB_LICENSE("GPLv2");
    CKB_GUID("{54DD2975-E192-457D-BCFC-D912A24E33B4}");
    CKB_DESCRIPTION("A transition from one color to another.");

    // Effect parameters
    CKB_PARAM_ARGB("color", "Color:", "", 255, 255, 255, 255);

    // Timing/input parameters
    CKB_PARAM_DURATION(1.);
    CKB_PARAM_TRIGGER(1);
    CKB_PARAM_TRIGGER_KP(0);
    CKB_PARAM_REPEAT(-1.);
    CKB_PARAM_REPEAT_KP(-1.);
}

unsigned char aa = 0, ar = 0, ag = 0, ab = 0;
float* target = 0;

void ckb_parameter(ckb_runctx* context, const char* name, const char* value){
    CKB_PARSE_ARGB("color", &aa, &ar, &ag, &ab){}
}

void ckb_keypress(ckb_runctx* context, ckb_key* key, int state){
    if(!target){
        unsigned count = context->keycount;
        target = malloc(count * sizeof(float));
        for(unsigned i = 0; i < count; i++)
            target[i] = 1.f;
    }
    if(state)
        target[key - context->keys] = 0.f;
}

void ckb_start(ckb_runctx* context){
    unsigned count = context->keycount;
    if(!target)
        target = calloc(count, sizeof(float));
    else
        memset(target, 0, count * sizeof(float));
}

int ckb_frame(ckb_runctx* context, double delta){
    if(!target)
        return 0;
    unsigned count = context->keycount;
    for(unsigned i = 0; i < count; i++){
        float phase = (target[i] += delta);
        if(phase >= 1.f)
            context->keys[i].a = 0;
        else {
            ckb_key* key = context->keys + i;
            key->a = aa * (1.f - phase);
            key->r = ar;
            key->g = ag;
            key->b = ab;
        }
    }
    return 0;
}

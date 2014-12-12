#include "../ckb/ckb-anim.h"
#include "math.h"

void ckb_info(){
    // Plugin info
    CKB_NAME("Gradient");
    CKB_VERSION("0.1");
    CKB_COPYRIGHT("2014 MSC");
    CKB_LICENSE("GPLv2");
    CKB_GUID("{54DD2975-E192-457D-BCFC-D912A24E33B4}");
}

void ckb_parameter(ckb_runctx* context, const char* name, const char* value){

}

void ckb_keypress(ckb_runctx* context, const char* keyname, int state){

}

float* target = 0;

void ckb_start(ckb_runctx* context){
    unsigned count = context->keycount;
    if(!target)
        target = calloc(count, sizeof(float));
    else
        memset(target, 0, count * sizeof(float));
}

int ckb_frame(ckb_runctx* context, double delta){
    unsigned count = context->keycount;
    for(unsigned i = 0; i < count; i++){
        float phase = (target[i] += delta);
        if(phase >= 1.f)
            context->keys[i].a = 0;
        else {
            ckb_key* key = context->keys + i;
            key->r = 255;
            key->g = 255;
            key->b = 255;
            key->a = 255 * (1.f - phase);
        }
    }
    return 0;
}

#include "../ckb/ckb-anim.h"
#include <math.h>

void ckb_info(){
    // Plugin info
    CKB_NAME("Wave");
    CKB_VERSION("0.1");
    CKB_COPYRIGHT("2014 MSC");
    CKB_LICENSE("GPLv2");
    CKB_GUID("{E0BBA19E-C328-4C0E-8E3C-A06D5722B4FC}");
}

void ckb_parameter(ckb_runctx* context, const char* name, const char* value){

}

#define ANIM_MAX 1
struct {
    int active;
    float x;
} anim[ANIM_MAX] = { };

void ckb_keypress(ckb_runctx* context, const char* keyname, int state){

}

void ckb_start(ckb_runctx* context){
    anim[0].active = 1;
    anim[0].x = -36.f;
}

int ckb_frame(ckb_runctx* context, double delta){
    CKB_KEYCLEAR(context);
    for(unsigned i = 0; i < ANIM_MAX; i++){
        if(anim[i].active){
            anim[i].x += (context->width + 36.f) * delta;
            if(anim[i].x >= context->width + 36.f)
                anim[i].x = -36.f;
            unsigned count = context->keycount;
            ckb_key* keys = context->keys;
            for(ckb_key* key = keys; key < keys + count; key++){
                float distance = fabs(key->x - anim[i].x);
                if(distance < 36.f){
                    float a = (36.f - distance) / 36.f;
                    if(key->a == 0){
                        key->r = 255.f;
                        key->g = 255.f;
                        key->b = 255.f;
                    } else {
                        key->r = key->r * (1.f - a) + 255.f * a;
                        key->g = key->g * (1.f - a) + 255.f * a;
                        key->b = key->b * (1.f - a) + 255.f * a;
                    }
                    key->a = key->a * (1.f - a) + 255.f * a;
                }
            }
        }
    }
    return 0;
}

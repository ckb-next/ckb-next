#include "../ckb/ckb-anim.h"
#include <math.h>

void ckb_info(){
    // Plugin info
    CKB_NAME("Ripple");
    CKB_VERSION("0.1");
    CKB_COPYRIGHT("2014 MSC");
    CKB_LICENSE("GPLv2");
    CKB_GUID("{097D69F0-70B2-48B8-AFE2-25A1CDB02C9D}");
}

void ckb_parameter(ckb_runctx* context, const char* name, const char* value){

}

#define ANIM_MAX 1
struct {
    int active;
    float x, y;
    float maxsize;
    float cursize;
} anim[ANIM_MAX] = { };

void ckb_keypress(ckb_runctx* context, const char* keyname, int state){

}

void ckb_start(ckb_runctx* context){
    anim[0].active = 1;
    anim[0].x = context->width / 2.f;
    anim[0].y = context->height / 2.f;
    anim[0].maxsize = sqrt(context->width * context->width / 4.f + context->height * context->height / 4.f) + 36.f;
    anim[0].cursize = -36.f;
}

int ckb_frame(ckb_runctx* context, double delta){
    CKB_KEYCLEAR(context);
    float kbsize = sqrt(context->width * context->width / 4.f + context->height * context->height / 4.f) + 36.f + 36.f;
    for(unsigned i = 0; i < ANIM_MAX; i++){
        if(anim[i].active){
            anim[i].cursize += kbsize * delta;
            if(anim[i].cursize >= anim[i].maxsize)
                anim[i].cursize = -36.f;
            unsigned count = context->keycount;
            ckb_key* keys = context->keys;
            for(ckb_key* key = keys; key < keys + count; key++){
                float distance = fabs(sqrt(pow(key->x - anim[i].x, 2.f) + pow(key->y - anim[i].y, 2.f)) - anim[i].cursize);
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

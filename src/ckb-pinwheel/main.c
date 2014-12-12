#include "../ckb/ckb-anim.h"
#include "math.h"

void ckb_info(){
    // Plugin info
    CKB_NAME("Pinwheel");
    CKB_VERSION("0.1");
    CKB_COPYRIGHT("2014 MSC");
    CKB_LICENSE("GPLv2");
    CKB_GUID("{07551A90-D97A-4DD0-A770-E9E280A90891}");
}

void ckb_parameter(ckb_runctx* context, const char* name, const char* value){

}

#define ANIM_MAX 1
struct {
    int active;
    float x, y;
    float position;
} anim[ANIM_MAX] = { };

#define ANGLE(theta) fmod((theta) + M_PI * 2., M_PI * 2.)

void ckb_keypress(ckb_runctx* context, const char* keyname, int state){

}

void ckb_start(ckb_runctx* context){
    anim[0].active = 1;
    anim[0].x = context->width / 2.f;
    anim[0].y = context->height / 2.f;
    anim[0].position = 0.f;
}

int ckb_frame(ckb_runctx* context, double delta){
    CKB_KEYCLEAR(context);
    for(unsigned i = 0; i < ANIM_MAX; i++){
        if(anim[i].active){
            anim[i].position = ANGLE(anim[i].position - delta * M_PI * 2.);
            unsigned count = context->keycount;
            ckb_key* keys = context->keys;
            for(ckb_key* key = keys; key < keys + count; key++){
                float theta;
                if(key->x == anim[i].x && key->y == anim[i].y)
                    theta = 0.f;
                else
                    theta = ANGLE(ANGLE(atan2(anim[i].x - key->x, anim[i].y - key->y)) - anim[i].position);
                if(theta > M_PI)
                    theta = M_PI * 2. - theta;
                if(theta < 0.8f){
                    float a = (0.8f - theta) / 0.8f;
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

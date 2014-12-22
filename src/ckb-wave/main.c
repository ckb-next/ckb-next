#include "../ckb/ckb-anim.h"
#include <math.h>

void ckb_info(){
    // Plugin info
    CKB_NAME("Wave");
    CKB_VERSION("0.2");
    CKB_COPYRIGHT("2014", "MSC");
    CKB_LICENSE("GPLv2");
    CKB_GUID("{E0BBA19E-C328-4C0E-8E3C-A06D5722B4FC}");
    CKB_DESCRIPTION("A moving wave effect.");

    // Effect parameters
    CKB_PARAM_ARGB("color", "Wave color:", "", 255, 255, 255, 255);
    CKB_PARAM_DOUBLE("length", "Wave length:", "%", 25, 1., 100.);

    // Timing/input parameters
    CKB_PARAM_DURATION(2.);
    CKB_PARAM_TRIGGER(1);
    CKB_PARAM_TRIGGER_KP(0);
    CKB_PARAM_REPEAT(2.);
    CKB_PARAM_REPEAT_KP(-1.);
}

unsigned char aa = 0, ar = 0, ag = 0, ab = 0;
double animlength = 0.;

void ckb_parameter(ckb_runctx* context, const char* name, const char* value){
    CKB_PARSE_ARGB("color", &aa, &ar, &ag, &ab){}
    double len;
    CKB_PARSE_DOUBLE("length", &len){
        animlength = len / 100. * context->width;
    }
}

#define ANIM_MAX (144 * 2)
struct {
    int active;
    float x;
    float maxx;
} anim[ANIM_MAX] = { };

void anim_add(float x, float width){
    for(int i = 0; i < ANIM_MAX; i++){
        if(anim[i].active)
            continue;
        anim[i].active = 1;
        anim[i].x = x;
        anim[i].maxx = width + animlength;
        return;
    }
}

void ckb_keypress(ckb_runctx* context, ckb_key* key, int state){
    if(state)
        anim_add(key->x, context->width);
}

void ckb_start(ckb_runctx* context){
    anim_add(-animlength, context->width);
}

int ckb_frame(ckb_runctx* context, double delta){
    CKB_KEYCLEAR(context);
    for(unsigned i = 0; i < ANIM_MAX; i++){
        if(anim[i].active){
            anim[i].x += (context->width + animlength) * delta;
            if(anim[i].x >= anim[i].maxx){
                anim[i].active = 0;
                continue;
            }
            unsigned count = context->keycount;
            ckb_key* keys = context->keys;
            for(ckb_key* key = keys; key < keys + count; key++){
                float distance = fabs(key->x - anim[i].x);
                if(distance < animlength){
                    float a = (animlength - distance) / animlength;
                    if(key->a == 0){
                        key->a = aa * a;
                        key->r = ar;
                        key->g = ag;
                        key->b = ab;
                    } else {
                        key->a = key->a * (1.f - a) + aa * a;
                        key->r = key->r * (1.f - a) + ar * a;
                        key->g = key->g * (1.f - a) + ag * a;
                        key->b = key->b * (1.f - a) + ab * a;
                    }
                }
            }
        }
    }
    return 0;
}

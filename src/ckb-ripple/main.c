#include "../ckb/ckb-anim.h"
#include <math.h>

void ckb_info(){
    // Plugin info
    CKB_NAME("Ripple");
    CKB_VERSION("0.3");
    CKB_COPYRIGHT("2014", "MSC");
    CKB_LICENSE("GPLv2");
    CKB_GUID("{097D69F0-70B2-48B8-AFE2-25A1CDB02C9D}");
    CKB_DESCRIPTION("An expanding ring effect.");

    // Effect parameters
    CKB_PARAM_ARGB("color", "Ripple color:", "", 255, 255, 255, 255);
    CKB_PARAM_DOUBLE("length", "Ring length:", "%", 25, 1, 100);

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
        double sizex = context->width / 2.;
        double sizey = context->height / 2.;
        animlength = len / 100. * sqrt(sizex * sizex + sizey * sizey);
    }
}

#define ANIM_MAX (144 * 2)
struct {
    int active;
    float x, y;
    float maxsize;
    float cursize;
} anim[ANIM_MAX] = { };

void anim_add(float x, float y, float width, float height){
    for(int i = 0; i < ANIM_MAX; i++){
        if(anim[i].active)
            continue;
        anim[i].active = 1;
        anim[i].x = x;
        anim[i].y = y;
        float sizex = fmax(x, width - x);
        float sizey = fmax(y, height - y);
        anim[i].maxsize = sqrt(sizex * sizex + sizey * sizey) + animlength;
        anim[i].cursize = -animlength;
        return;
    }
}

void ckb_keypress(ckb_runctx* context, ckb_key* key, int state){
    if(state)
        anim_add(key->x, key->y, context->width, context->height);
}

void ckb_start(ckb_runctx* context){
    anim_add(context->width / 2.f, context->height / 2.f, context->width, context->height);
}

int ckb_frame(ckb_runctx* context, double delta){
    CKB_KEYCLEAR(context);
    float kbsize = sqrt(context->width * context->width / 4.f + context->height * context->height / 4.f) + animlength * 2.f;
    for(unsigned i = 0; i < ANIM_MAX; i++){
        if(anim[i].active){
            anim[i].cursize += kbsize * delta;
            if(anim[i].cursize > anim[i].maxsize){
                anim[i].active = 0;
                continue;
            }
            unsigned count = context->keycount;
            ckb_key* keys = context->keys;
            for(ckb_key* key = keys; key < keys + count; key++){
                float distance = fabs(sqrt(pow(key->x - anim[i].x, 2.f) + pow(key->y - anim[i].y, 2.f)) - anim[i].cursize);
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

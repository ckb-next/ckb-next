#include "../ckb/ckb-anim.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void ckb_info(){
    // Plugin info
    CKB_NAME("Pinwheel");
    CKB_VERSION("0.2");
    CKB_COPYRIGHT("2014", "MSC");
    CKB_LICENSE("GPLv2");
    CKB_GUID("{07551A90-D97A-4DD0-A770-E9E280A90891}");
    CKB_DESCRIPTION("A spinning pinwheel effect.");

    // Effect parameters
    CKB_PARAM_ARGB("color", "Wheel color:", "", 255, 255, 255, 255);
    CKB_PARAM_DOUBLE("length", "Wheel size:", "%", 10, 1., 100.);

    // Timing/input parameters
    CKB_PARAM_DURATION(2.);
    CKB_PARAM_TRIGGER(1);
    CKB_PARAM_REPEAT(2.);
}
unsigned char aa = 0, ar = 0, ag = 0, ab = 0;
double animlength = 0.;

void ckb_parameter(ckb_runctx* context, const char* name, const char* value){
    CKB_PARSE_ARGB("color", &aa, &ar, &ag, &ab){}
    double len;
    CKB_PARSE_DOUBLE("length", &len){
        animlength = len / 100. * M_PI * 2.;
    }
}

#define ANIM_MAX 1
struct {
    double frame;
    int active;
    float x, y;
    float position;
} anim[ANIM_MAX] = { };

#define ANGLE(theta) fmod((theta) + M_PI * 2., M_PI * 2.)

void ckb_keypress(ckb_runctx* context, ckb_key* key, int state){

}

void ckb_start(ckb_runctx* context){
    anim[0].frame = 0.f;
    anim[0].active = 1;
    anim[0].x = context->width / 2.f;
    anim[0].y = context->height / 2.f;
    anim[0].position = 0.f;
}

int ckb_frame(ckb_runctx* context, double delta){
    CKB_KEYCLEAR(context);
    for(unsigned i = 0; i < ANIM_MAX; i++){
        if(anim[i].active){
            anim[i].frame += delta;
            if(anim[i].frame >= 1.){
                anim[i].active = 0;
                continue;
            }
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
                if(theta < animlength){
                    float a = (animlength - theta) / animlength;
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

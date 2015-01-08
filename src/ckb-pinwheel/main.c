#include "../ckb/ckb-anim.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void ckb_info(){
    // Plugin info
    CKB_NAME("Pinwheel");
    CKB_VERSION("0.4");
    CKB_COPYRIGHT("2014", "MSC");
    CKB_LICENSE("GPLv2");
    CKB_GUID("{07551A90-D97A-4DD0-A770-E9E280A90891}");
    CKB_DESCRIPTION("A spinning pinwheel effect.");

    // Effect parameters
    CKB_PARAM_AGRADIENT("color", "Wheel color:", "", "ffffffff");
    CKB_PARAM_DOUBLE("length", "Wheel size:", "%", 100, 1., 100.);
    CKB_PARAM_BOOL("symmetric", "Symmetric", 0);

    // Timing/input parameters
    CKB_KPMODE(CKB_KP_NONE);
    CKB_TIMEMODE(CKB_TIME_DURATION);
    CKB_REPEAT(FALSE);
    CKB_DEFAULT_DURATION(2.);
    CKB_DEFAULT_TRIGGER(TRUE);
    CKB_DEFAULT_TRIGGER_KP(FALSE);
}

ckb_gradient animcolor = { 0 };
double animlength = 0.;
int symmetric = 0;

void ckb_parameter(ckb_runctx* context, const char* name, const char* value){
    CKB_PARSE_AGRADIENT("color", &animcolor){}
    double len;
    CKB_PARSE_DOUBLE("length", &len){
        animlength = len / 100. * M_PI * 2.;
    }
    CKB_PARSE_BOOL("symmetric", &symmetric){}
}

void ckb_init(ckb_runctx* context){

}

void ckb_keypress(ckb_runctx* context, ckb_key* key, int x, int y, int state){

}

double frame = -1.;
float x, y;

#define ANGLE(theta) fmod((theta) + M_PI * 2., M_PI * 2.)

void ckb_start(ckb_runctx* context){
    frame = 0.;
    x = context->width / 2.f;
    y = context->height / 2.f;
}

int ckb_frame(ckb_runctx* context, double delta){
    if(frame < 0.)
        return 0;
    CKB_KEYCLEAR(context);
    frame += delta;
    if(frame > 1.)
        frame -= 1.;

    float position = ANGLE(-frame * M_PI * 2.);
    unsigned count = context->keycount;
    ckb_key* keys = context->keys;
    for(ckb_key* key = keys; key < keys + count; key++){
        float theta;
        if(key->x == x && key->y == y)
            theta = 0.f;
        else
            theta = ANGLE(ANGLE(atan2(x - key->x, y - key->y)) - position);
        if(symmetric && theta > M_PI)
            theta = M_PI * 2. - theta;
        if(theta < animlength){
            float distance = theta / animlength;
            float a, r, g, b;
            ckb_grad_color(&a, &r, &g, &b, &animcolor, distance * 100.);
            ckb_alpha_blend(key, a, r, g, b);
        }
    }
    return 0;
}

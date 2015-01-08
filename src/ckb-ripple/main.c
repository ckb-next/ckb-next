#include "../ckb/ckb-anim.h"
#include <math.h>

void ckb_info(){
    // Plugin info
    CKB_NAME("Ripple");
    CKB_VERSION("0.4");
    CKB_COPYRIGHT("2014", "MSC");
    CKB_LICENSE("GPLv2");
    CKB_GUID("{097D69F0-70B2-48B8-AFE2-25A1CDB02C9D}");
    CKB_DESCRIPTION("An expanding ring effect.");

    // Effect parameters
    CKB_PARAM_AGRADIENT("color", "Ripple color:", "", "ffffffff");
    CKB_PARAM_DOUBLE("length", "Ring length:", "%", 100, 1, 100);
    CKB_PARAM_BOOL("symmetric", "Symmetric", 0);

    // Timing/input parameters
    CKB_KPMODE(CKB_KP_POSITION);
    CKB_TIMEMODE(CKB_TIME_DURATION);
    CKB_PREEMPT(TRUE);
    CKB_DEFAULT_DURATION(2.);
    CKB_DEFAULT_TRIGGER(TRUE);
    CKB_DEFAULT_TRIGGER_KP(FALSE);
}

void ckb_init(ckb_runctx* context){
    // Nothing to do
}

ckb_gradient animcolor = { 0 };
int symmetric = 0, kprelease = 0;
double animlength = 0.;

void ckb_parameter(ckb_runctx* context, const char* name, const char* value){
    CKB_PARSE_AGRADIENT("color", &animcolor){}
    double len;
    CKB_PARSE_DOUBLE("length", &len){
        double sizex = context->width / 2.;
        double sizey = context->height / 2.;
        animlength = len / 100. * sqrt(sizex * sizex + sizey * sizey);
    }
    CKB_PARSE_BOOL("symmetric", &symmetric){}
    CKB_PARSE_BOOL("kprelease", &kprelease){}
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
        anim[i].cursize = (symmetric) ? -animlength : 0;
        return;
    }
}

void ckb_keypress(ckb_runctx* context, ckb_key* key, int x, int y, int state){
    if(state)
        anim_add(x, y, context->width, context->height);
    else if(kprelease){
        // Cancel existing animation if stop on keyup is enabled
        for(int i = 0; i < ANIM_MAX; i++){
            if(anim[i].active && anim[i].x == x && anim[i].y == y)
                anim[i].active = 0;
        }
    }
}

void ckb_start(ckb_runctx* context){
    anim_add(context->width / 2.f, context->height / 2.f, context->width, context->height);
}

int ckb_frame(ckb_runctx* context, double delta){
    CKB_KEYCLEAR(context);
    float kbsize = sqrt(context->width * context->width / 4.f + context->height * context->height / 4.f);
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
                float distance = anim[i].cursize - sqrt(pow(key->x - anim[i].x, 2.f) + pow(key->y - anim[i].y, 2.f));
                if(symmetric)
                    distance = fabs(distance);
                if(distance >= 0. && distance < animlength){
                    float a, r, g, b;
                    ckb_grad_color(&a, &r, &g, &b, &animcolor, distance / animlength * 100.);
                    ckb_alpha_blend(key, a, r, g, b);
                }
            }
        }
    }
    return 0;
}

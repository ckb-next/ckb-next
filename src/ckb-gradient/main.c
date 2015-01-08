#include "../ckb/ckb-anim.h"
#include <math.h>

void ckb_info(){
    // Plugin info
    CKB_NAME("Gradient");
    CKB_VERSION("0.4");
    CKB_COPYRIGHT("2014", "MSC");
    CKB_LICENSE("GPLv2");
    CKB_GUID("{54DD2975-E192-457D-BCFC-D912A24E33B4}");
    CKB_DESCRIPTION("A transition from one color to another.");

    // Effect parameters
    CKB_PARAM_AGRADIENT("color", "Color:", "", "ffffffff");

    // Timing/input parameters
    CKB_KPMODE(CKB_KP_NAME);
    CKB_TIMEMODE(CKB_TIME_DURATION);
    CKB_DEFAULT_DURATION(1.);
    CKB_DEFAULT_TRIGGER(FALSE);
    CKB_DEFAULT_TRIGGER_KP(TRUE);
}

ckb_gradient animcolor = { 0 };
int kprelease = 0;
float* target = 0;

void ckb_init(ckb_runctx* context){
    // Initialize all keys to 100% (animation over)
    unsigned count = context->keycount;
    target = malloc(count * sizeof(float));
    for(unsigned i = 0; i < count; i++)
        target[i] = 1.f;
}

void ckb_parameter(ckb_runctx* context, const char* name, const char* value){
    CKB_PARSE_AGRADIENT("color", &animcolor){}
    CKB_PARSE_BOOL("kprelease", &kprelease){}
}

void ckb_keypress(ckb_runctx* context, ckb_key* key, int x, int y, int state){
    // Start or stop animation on key
    if(state)
        target[key - context->keys] = 0.f;
    else if(kprelease)
        target[key - context->keys] = 1.f;
}

void ckb_start(ckb_runctx* context){
    // Start all keys
    unsigned count = context->keycount;
    memset(target, 0, count * sizeof(float));
}

int ckb_frame(ckb_runctx* context, double delta){
    unsigned count = context->keycount;
    for(unsigned i = 0; i < count; i++){
        float phase = (target[i] += delta);
        if(phase > 1.f)
            phase = 1.f;
        ckb_key* key = context->keys + i;
        float a, r, g, b;
        ckb_grad_color(&a, &r, &g, &b, &animcolor, phase * 100.);
        key->a = a;
        key->r = r;
        key->g = g;
        key->b = b;
    }
    return 0;
}

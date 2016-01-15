#include "../ckb/ckb-anim.h"
#include <math.h>

void ckb_info(){
    // Plugin info
    CKB_NAME("Gradient");
    CKB_VERSION("0.9");
    CKB_COPYRIGHT("2014-2016", "MSC");
    CKB_LICENSE("GPLv2");
    CKB_GUID("{54DD2975-E192-457D-BCFC-D912A24E33B4}");
    CKB_DESCRIPTION("A transition from one color to another.");

    // Effect parameters
    CKB_PARAM_AGRADIENT("color", "Color:", "", "ffffffff");
    CKB_PARAM_BOOL("kphold", "Freeze until key is released", TRUE);

    // Timing/input parameters
    CKB_KPMODE(CKB_KP_NAME);
    CKB_TIMEMODE(CKB_TIME_DURATION);
    CKB_LIVEPARAMS(TRUE);

    // Presets
    CKB_PRESET_START("Fade in");
    CKB_PRESET_PARAM("color", "ff000000");
    CKB_PRESET_PARAM("duration", "1.0");
    CKB_PRESET_PARAM("stop", "0");
    CKB_PRESET_PARAM("kpstop", "0");
    CKB_PRESET_END;

    CKB_PRESET_START("Rainbow");
    CKB_PRESET_PARAM("color", "0:ffff0000 17:ffffff00 33:ff00ff00 50:ff00ffff 67:ff0000ff 83:ffff00ff 100:ffff0000");
    CKB_PRESET_PARAM("duration", "2.0");
    CKB_PRESET_END;

    CKB_PRESET_START("Type lighting");
    CKB_PRESET_PARAM("duration", "1.0");
    CKB_PRESET_PARAM("trigger", "0");
    CKB_PRESET_PARAM("kptrigger", "1");
    CKB_PRESET_END;
}

#define NONE -1.f
#define HOLD -2.f

ckb_gradient animcolor = { 0 };
int kphold = 0, kprelease = 0;
float* target = 0;

void ckb_init(ckb_runctx* context){
    // Initialize all keys to 100% (animation over)
    unsigned count = context->keycount;
    target = malloc(count * sizeof(float));
    for(unsigned i = 0; i < count; i++)
        target[i] = NONE;
}

void ckb_parameter(ckb_runctx* context, const char* name, const char* value){
    CKB_PARSE_AGRADIENT("color", &animcolor){}
    CKB_PARSE_BOOL("kphold", &kphold){}
    CKB_PARSE_BOOL("kprelease", &kprelease){}
}

void ckb_keypress(ckb_runctx* context, ckb_key* key, int x, int y, int state){
    // Start or stop animation on key
    if(state){
        if(kphold)
            target[key - context->keys] = HOLD;
        else
            target[key - context->keys] = 0.f;
    } else {
        if(kprelease)
            target[key - context->keys] = NONE;
        else if(kphold)
            target[key - context->keys] = 0.f;
    }
}

void ckb_start(ckb_runctx* context, int state){
    // Start/stop all keys
    unsigned count = context->keycount;
    if(state)
        memset(target, 0, count * sizeof(float));
    else {
        for(unsigned i = 0; i < count; i++)
            target[i] = NONE;
    }
}

void ckb_time(ckb_runctx* context, double delta){
    // Advance animation on each key
    unsigned count = context->keycount;
    for(unsigned i = 0; i < count; i++){
        float phase = target[i];
        if(phase > 1.f || phase < 0.f)
            continue;
        target[i] = phase + delta;
    }
}

int ckb_frame(ckb_runctx* context){
    // Draw key colors
    unsigned count = context->keycount;
    for(unsigned i = 0; i < count; i++){
        float phase = target[i];
        if(phase == HOLD)
            phase = 0.f;
        else if(phase < 0.f)
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

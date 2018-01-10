#include <ckb-next/animation.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
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
    CKB_PARAM_BOOL("randomize", "Randomly select from gradient", 0);

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

struct keyAnim {
    float target;
    ckb_gradient gradient;
};

ckb_gradient animcolor = { 0 };
int kphold = 0, kprelease = 0, randomize = 0;

struct keyAnim* anim;

void ckb_init(ckb_runctx* context){
    // Initialize all keys to 100% (animation over)

    srand((unsigned)time(NULL));
    unsigned count = context->keycount;
    anim =  malloc(count * sizeof *anim);
    for(unsigned i = 0; i < count; i++) {
        anim[i].target = NONE;
    }
}

void ckb_parameter(ckb_runctx* context, const char* name, const char* value){
    CKB_PARSE_AGRADIENT("color", &animcolor){}
    CKB_PARSE_BOOL("kphold", &kphold){}
    CKB_PARSE_BOOL("kprelease", &kprelease){}
    CKB_PARSE_BOOL("randomize", &randomize){}
}

void ckb_keypress(ckb_runctx* context, ckb_key* key, int x, int y, int state){
    // Start or stop animation on key
    int i = key - context->keys;
    if(state){
        if (randomize) {
            float a, r, g, b;
            float colorChoice = (float)rand()/(float)(RAND_MAX);
            ckb_grad_color(&a, &r, &g, &b, &animcolor, colorChoice * 100.);
            ckb_gradient newGradient = { 0 };
            newGradient.ptcount = 2;
            newGradient.pts[0] = 0;
            newGradient.pts[1] = 100;
            newGradient.a[0] = 255;
            newGradient.b[0] = b;
            newGradient.g[0] = g;
            newGradient.r[0] = r;
            newGradient.a[1] = 0;
            newGradient.b[1] = b;
            newGradient.g[1] = g;
            newGradient.r[1] = r;

            anim[i].gradient = newGradient;
        }

        if(kphold) {
            anim[i].target = HOLD;
        } else {
            anim[i].target = 0.f;
        }
    } else {
        if(kprelease) {
            anim[i].target = NONE;
        }
        else if(kphold) {
            anim[i].target = 0.f;
        }
    }
}

void ckb_start(ckb_runctx* context, int state){
    // Start/stop all keys
    unsigned count = context->keycount;
    if(state) {
        memset(anim, 0, count * sizeof *anim);
    } else {
        for(unsigned i = 0; i < count; i++) {
            anim[i].target = NONE;
        }
    }
}

void ckb_time(ckb_runctx* context, double delta){
    // Advance animation on each key
    unsigned count = context->keycount;
    for(unsigned i = 0; i < count; i++){
        float phase = anim[i].target;
        if(phase > 1.f || phase < 0.f)
            continue;
        anim[i].target = phase + delta;
    }
}

int ckb_frame(ckb_runctx* context){
    // Draw key colors
    unsigned count = context->keycount;
    for(unsigned i = 0; i < count; i++){
        float phase = anim[i].target;

        if(phase == HOLD)
            phase = 0.f;
        else if(phase < 0.f)
            phase = 1.f;
        ckb_key* key = context->keys + i;
        float a, r, g, b;
        ckb_gradient thisGradient;
        if (randomize) {
            thisGradient = anim[i].gradient;
        } else {
            thisGradient = animcolor;
        }
        ckb_grad_color(&a, &r, &g, &b, &thisGradient, phase * 100.);
        key->a = a;
        key->r = r;
        key->g = g;
        key->b = b;
    }
    return 0;
}

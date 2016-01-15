#include "../ckb/ckb-anim.h"
#include <math.h>

void ckb_info(){
    // Plugin info
    CKB_NAME("Ripple");
    CKB_VERSION("0.10");
    CKB_COPYRIGHT("2014-2016", "MSC");
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
    CKB_LIVEPARAMS(TRUE);
    CKB_PREEMPT(TRUE);

    // Presets
    CKB_PRESET_START("Shimmer");
    CKB_PRESET_PARAM("duration", "2.0");
    CKB_PRESET_PARAM("length", "50.0");
    CKB_PRESET_PARAM("symmetric", "1");
    CKB_PRESET_END;

    CKB_PRESET_START("Rainbow");
    CKB_PRESET_PARAM("color", "0:ffff0000 17:ffffff00 33:ff00ff00 50:ff00ffff 67:ff0000ff 83:ffff00ff 100:ffff0000");
    CKB_PRESET_PARAM("duration", "2.0");
    CKB_PRESET_END;

    CKB_PRESET_START("Type lighting");
    CKB_PRESET_PARAM("duration", "1.0");
    CKB_PRESET_PARAM("length", "25.0");
    CKB_PRESET_PARAM("symmetric", "1");
    CKB_PRESET_PARAM("trigger", "0");
    CKB_PRESET_PARAM("kptrigger", "1");
    CKB_PRESET_END;
}

float kbsize = 0.f;
ckb_gradient animcolor = { 0 };
int symmetric = 0, kprelease = 0;
double animlength = 0.;

void ckb_init(ckb_runctx* context){
    kbsize = sqrt(context->width * context->width / 4.f + context->height * context->height / 4.f);
}

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

void anim_remove(float x, float y){
    for(int i = 0; i < ANIM_MAX; i++){
        if(anim[i].active && anim[i].x == x && anim[i].y == y)
            anim[i].active = 0;
    }
}

void ckb_keypress(ckb_runctx* context, ckb_key* key, int x, int y, int state){
    // Add or remove a ring on this key
    if(state)
        anim_add(x, y, context->width, context->height);
    else if(kprelease)
        anim_remove(x, y);
}

void ckb_start(ckb_runctx* context, int state){
    // Add or remove a ring in the center of the keyboard
    if(state)
        anim_add(context->width / 2.f, context->height / 2.f, context->width, context->height);
    else
        anim_remove(context->width / 2.f, context->height / 2.f);
}

void ckb_time(ckb_runctx* context, double delta){
    // Advance animation on all rings
    for(unsigned i = 0; i < ANIM_MAX; i++){
        if(anim[i].active){
            anim[i].cursize += kbsize * delta;
            if(anim[i].cursize > anim[i].maxsize)
                anim[i].active = 0;
        }
    }
}

int ckb_frame(ckb_runctx* context){
    CKB_KEYCLEAR(context);
    // Draw rings
    unsigned count = context->keycount;
    ckb_key* keys = context->keys;
    for(unsigned i = 0; i < ANIM_MAX; i++){
        if(anim[i].active){
            for(ckb_key* key = keys; key < keys + count; key++){
                // Calculate distance between this key and the ring
                float distance = anim[i].cursize - sqrt(pow(key->x - anim[i].x, 2.f) + pow(key->y - anim[i].y, 2.f));
                // Divide distance by ring size (use absolute distance if symmetric)
                distance /= animlength;
                if(symmetric)
                    distance = fabs(distance);
                else if(distance >= -0.005f && distance < 0.f)
                    // If not symmetric, round down values very close to zero
                    distance = 0.f;
                if(distance > 1.f && distance <= 1.005f)
                    // Round values close to 1
                    distance = 1.f;
                // Blend color gradient according to position
                if(distance >= 0. && distance <= 1.f){
                    float a, r, g, b;
                    ckb_grad_color(&a, &r, &g, &b, &animcolor, distance * 100.);
                    ckb_alpha_blend(key, a, r, g, b);
                }
            }
        }
    }
    return 0;
}

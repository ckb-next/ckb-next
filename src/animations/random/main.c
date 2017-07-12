#include "../ckb/ckb-anim.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>

void ckb_info(){
    // Plugin info
    CKB_NAME("Random");
    CKB_VERSION("0.9");
    CKB_COPYRIGHT("2014-2016", "MSC");
    CKB_LICENSE("GPLv2");
    CKB_GUID("{22418DA4-A181-4B93-A4D3-03682BA283D2}");
    CKB_DESCRIPTION("An effect that changes key colors randomly.");

    // Effect parameters
    CKB_PARAM_BOOL("fade", "Fade in", 0);
    CKB_PARAM_BOOL("useopacity", "Randomize opacity", 0);

    // Timing/input parameters
    CKB_KPMODE(CKB_KP_NONE);
    CKB_TIMEMODE(CKB_TIME_DURATION);
    CKB_LIVEPARAMS(TRUE);
    CKB_REPEAT(FALSE);

    // Presets
    CKB_PRESET_START("Default");
    CKB_PRESET_PARAM("duration", "1.");
    CKB_PRESET_END;
}

int fadein = 0, useopacity = 0;

void ckb_parameter(ckb_runctx* context, const char* name, const char* value){
    CKB_PARSE_BOOL("fade", &fadein) {}
    CKB_PARSE_BOOL("useopacity", &useopacity) {}
}

void ckb_keypress(ckb_runctx* context, ckb_key* key, int x, int y, int state){
    // Unused
}

typedef struct {
    unsigned char a, r, g, b;
} rgb;

rgb* current = 0;
rgb* target = 0;
double phase = -1.;

// Make a new target color for each key
void newtarget(rgb* data, unsigned count){
    for(unsigned i = 0; i < count; i++){
        rgb* key = data + i;
        key->a = useopacity ? rand() % 256 : 255;
        key->r = rand() % 256;
        key->g = rand() % 256;
        key->b = rand() % 256;
    }
}

void ckb_init(ckb_runctx* context){
    unsigned count = context->keycount;
    current = malloc(count * sizeof(rgb));
    target = malloc(count * sizeof(rgb));
    srand(time(NULL));
}

void ckb_start(ckb_runctx* context, int state){
    if(state == 0){
        // Stop animation
        phase = -1.;
        return;
    }
    // Start animation
    phase = 0.;
    ckb_key* keys = context->keys;
    unsigned count = context->keycount;
    // Randomize starting colors and pick a random target
    newtarget(current, count);
    // Over the course of the animation, the keys fade between the current pattern and the target pattern
    newtarget(target, count);
    // Set all keys to current
    for(unsigned i = 0; i < count; i++){
        ckb_key* key = keys + i;
        if(fadein)
            current[i].a = 0;
        key->a = current[i].a;
        key->r = current[i].r;
        key->g = current[i].g;
        key->b = current[i].b;
    }
}

void ckb_time(ckb_runctx* context, double delta){
    if(phase < 0.)
        return;
    // Advance animation
    phase += delta;
    if(phase > 1.){
        // If the animation is complete, pick a new target pattern and start again
        phase -= 1.;
        rgb* temp = target;
        target = current;
        current = temp;
        newtarget(target, context->keycount);
    }
}

int ckb_frame(ckb_runctx* context){
    if(phase < 0.){
        CKB_KEYCLEAR(context);
        return 0;
    }
    ckb_key* keys = context->keys;
    unsigned count = context->keycount;
    for(unsigned i = 0; i < count; i++){
        // Color each key according to the position between the last color set and the new color set
        ckb_key* key = keys + i;
        key->a = round(current[i].a * (1. - phase) + target[i].a * phase);
        key->r = round(current[i].r * (1. - phase) + target[i].r * phase);
        key->g = round(current[i].g * (1. - phase) + target[i].g * phase);
        key->b = round(current[i].b * (1. - phase) + target[i].b * phase);
    }
    return 0;
}

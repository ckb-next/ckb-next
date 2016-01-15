#include "../ckb/ckb-anim.h"

void ckb_info(){
    // Plugin info
    CKB_NAME("Pinwheel");
    CKB_VERSION("0.9");
    CKB_COPYRIGHT("2014-2016", "MSC");
    CKB_LICENSE("GPLv2");
    CKB_GUID("{07551A90-D97A-4DD0-A770-E9E280A90891}");
    CKB_DESCRIPTION("A spinning pinwheel effect.");

    // Effect parameters
    CKB_PARAM_AGRADIENT("color", "Wheel color:", "", "ffffffff");
    CKB_PARAM_DOUBLE("length", "Wheel size:", "%", 100, 1., 100.);
    CKB_PARAM_BOOL("counter_clock", "Counter Clockwise", 0);
    CKB_PARAM_BOOL("symmetric", "Symmetric", 0);

    // Timing/input parameters
    CKB_KPMODE(CKB_KP_NONE);
    CKB_TIMEMODE(CKB_TIME_DURATION);
    CKB_LIVEPARAMS(TRUE);
    CKB_REPEAT(FALSE);

    // Presets
    CKB_PRESET_START("Search light");
    CKB_PRESET_PARAM("duration", "2.0");
    CKB_PRESET_PARAM("length", "50.0");
    CKB_PRESET_PARAM("symmetric", "1");
    CKB_PRESET_END;

    CKB_PRESET_START("Rainbow");
    CKB_PRESET_PARAM("color", "0:ffff0000 17:ffffff00 33:ff00ff00 50:ff00ffff 67:ff0000ff 83:ffff00ff 100:ffff0000");
    CKB_PRESET_PARAM("duration", "2.0");
    CKB_PRESET_END;
}

ckb_gradient animcolor = { 0 };
double animlength = 0.;
int symmetric = 0;
int counter_clock = 0;

void ckb_parameter(ckb_runctx* context, const char* name, const char* value){
    CKB_PARSE_AGRADIENT("color", &animcolor){}
    double len;
    CKB_PARSE_DOUBLE("length", &len){
        animlength = len / 100. * M_PI * 2.;
    }
    CKB_PARSE_BOOL("symmetric", &symmetric){}
    CKB_PARSE_BOOL("counter_clock", &counter_clock){}
}

void ckb_init(ckb_runctx* context){
    // Unused
}

void ckb_keypress(ckb_runctx* context, ckb_key* key, int x, int y, int state){
    // Unused
}

double frame = -1.;
float x, y;

#define ANGLE(theta) fmod((theta) + M_PI * 2., M_PI * 2.)

void ckb_start(ckb_runctx* context, int state){
    // Begin or end animation
    frame = state ? 0. : -1.;
    x = context->width / 2.f;
    y = context->height / 2.f;
}

void ckb_time(ckb_runctx* context, double delta){
    if(frame < 0.)
        return;
    // Spin the wheel
    frame += delta;
    if(frame > 1.)
        frame -= 1.;
}

int ckb_frame(ckb_runctx* context){
    CKB_KEYCLEAR(context);
    if(frame < 0.)
        return 0;
    // Color each key according to its angle from the center
        float position;
        if(counter_clock)
        position = ANGLE(frame * M_PI * 2.);
        else
            position = ANGLE(-frame * M_PI * 2.);
    unsigned count = context->keycount;
    ckb_key* keys = context->keys;
    for(ckb_key* key = keys; key < keys + count; key++){
        float theta;
        if(key->x == x && key->y == y)
            // Dead center = 0°
            theta = 0.f;
        else
            theta = ANGLE(ANGLE(atan2(x - key->x, y - key->y)) - position);
        // If the animation is symmetric, mirror the second half
        if(symmetric && theta > M_PI)
            theta = M_PI * 2. - theta;
        // Draw the gradient position that corresponds to this angle
        if(theta < animlength){
            float distance = theta / animlength;
            float a, r, g, b;
            ckb_grad_color(&a, &r, &g, &b, &animcolor, distance * 100.);
            ckb_alpha_blend(key, a, r, g, b);
        }
    }
    return 0;
}

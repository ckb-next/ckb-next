#include "../ckb/ckb-anim.h"
#include <math.h>

void ckb_info(){
    // Plugin info
    CKB_NAME("Wave");
    CKB_VERSION("0.10");
    CKB_COPYRIGHT("2014-2016", "MSC");
    CKB_LICENSE("GPLv2");
    CKB_GUID("{E0BBA19E-C328-4C0E-8E3C-A06D5722B4FC}");
    CKB_DESCRIPTION("A moving wave effect.");

    // Effect parameters
    CKB_PARAM_ANGLE("angle", "Angle:", "", 90);
    CKB_PARAM_AGRADIENT("color", "Wave color:", "", "ffffffff");
    CKB_PARAM_DOUBLE("length", "Wave length:", "%", 100, 1., 100.);
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

    CKB_PRESET_START("Vertical rainbow");
    CKB_PRESET_PARAM("color", "0:ffff0000 17:ffffff00 33:ff00ff00 50:ff00ffff 67:ff0000ff 83:ffff00ff 100:ffff0000");
    CKB_PRESET_PARAM("duration", "2.0");
    CKB_PRESET_PARAM("angle", "180");
    CKB_PRESET_END;
}

void ckb_init(ckb_runctx* context){
    // Nothing to do
}

ckb_gradient animcolor = { 0 };
int symmetric = 0, kprelease = 0;
double angle = 0.;
double left = 0., top = 0.;
double animlength = 0., width = 0.;

void ckb_parameter(ckb_runctx* context, const char* name, const char* value){
    CKB_PARSE_AGRADIENT("color", &animcolor){}
    double len;
    CKB_PARSE_DOUBLE("length", &len){
        animlength = len / 100.;
    }
    CKB_PARSE_BOOL("symmetric", &symmetric){}
    CKB_PARSE_BOOL("kprelease", &kprelease){}
    long _angle;
    CKB_PARSE_ANGLE("angle", &_angle){
        angle = CKB_REAL_ANGLE(_angle);
        // Get each of the four corners of the keyboard, relative to the center
        double wOver2 = context->width / 2., hOver2 = context->height / 2.;
        double x[4] = {
            -wOver2, wOver2,
            -wOver2, wOver2
        };
        double y[4] = {
            -hOver2, -hOver2,
            hOver2, hOver2
        };
        // Rotate them in the direction that the animation will travel
        for(int i = 0; i < 4; i++){
            double x2 = x[i] * cos(angle) - y[i] * sin(angle);
            y[i] = x[i] * sin(angle) + y[i] * cos(angle);
            x[i] = x2;
        }
        // Determine the leftmost and rightmost points
        double min_x = INFINITY, max_x = -INFINITY;
        for(int i = 0; i < 4; i++){
            if(x[i] < min_x)
                min_x = x[i];
            if(x[i] > max_x)
                max_x = x[i];
        }
        // The leftmost point is where the animation will begin when started with the mode,
        // and the difference between the two is the animation width.
        // Animations always travel left to right in their own coordinate system.
        left = min_x * cos(-angle) + wOver2;
        top = min_x * sin(-angle) + hOver2;
        width = max_x - min_x;
    }
}

#define ANIM_MAX (144 * 2)
struct {
    int active;
    float x, y;
    float curx;
} anim[ANIM_MAX] = { };

void anim_add(float x, float y){
    for(int i = 0; i < ANIM_MAX; i++){
        if(anim[i].active)
            continue;
        anim[i].active = 1;
        anim[i].x = x;
        anim[i].y = y;
        anim[i].curx = symmetric ? -animlength * width : 0.f;
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
    // Start or stop animation on a key
    if(state)
        anim_add(x, y);
    else
        anim_remove(x, y);
}

void ckb_start(ckb_runctx* context, int state){
    // Start or stop animation on the edge of the keyboard
    if(state)
        anim_add(left, top);
    else if(kprelease)
        anim_remove(left, top);
}

void ckb_time(ckb_runctx* context, double delta){
    // Advance all waves, remove them if they pass the edge
    double length = animlength * width;
    for(unsigned i = 0; i < ANIM_MAX; i++){
        if(anim[i].active){
            anim[i].curx += width * delta;
            if(anim[i].curx > width + length)
                anim[i].active = 0;
        }
    }
}

int ckb_frame(ckb_runctx* context){
    CKB_KEYCLEAR(context);
    // Draw keys
    double length = animlength * width;
    unsigned count = context->keycount;
    ckb_key* keys = context->keys;
    for(unsigned i = 0; i < ANIM_MAX; i++){
        if(anim[i].active){
            for(ckb_key* key = keys; key < keys + count; key++){
                // Translate and rotate the key position into the animation's coordinate system
                float x = key->x - anim[i].x, y = key->y - anim[i].y;
                // Distance is the current X minus the key's X
                float distance = anim[i].curx - (x * cos(angle) - y * sin(angle));
                distance /= length;
                // If symmetric, use absolute distance
                if(symmetric)
                    distance = fabs(distance);
                else if(distance >= -0.005f && distance < 0.f)
                    // If not symmetric, round values close to 0
                    distance = 0.f;
                if(distance > 1.f && distance <= 1.005f)
                    // Round values close to 1
                    distance = 1.f;
                // Pick gradient position based on distance
                if(distance <= 1.f && distance >= 0.){
                    float a, r, g, b;
                    ckb_grad_color(&a, &r, &g, &b, &animcolor, distance * 100.);
                    ckb_alpha_blend(key, a, r, g, b);
                }
            }
        }
    }
    return 0;
}

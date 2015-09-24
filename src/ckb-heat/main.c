#include "../ckb/ckb-anim.h"
#include <math.h>

void ckb_info(){
    // Plugin info
    CKB_NAME("HeatMap");
    CKB_VERSION("0.5");
    CKB_COPYRIGHT("2015", "RULER501");
    CKB_LICENSE("LGPLv3");
    CKB_GUID("{097D69F0-70B2-48B8-AFE2-25A1CDB0D92C}");
    CKB_DESCRIPTION("A spot effect on pressed keys that shows usage");

    // Effect parameters
    CKB_PARAM_AGRADIENT("color", "Fade color:", "", "ffffffff");
    CKB_PARAM_BOOL("random", "Random Coloring", 0);
    //CKB_PARAM_LONG("frequency", "Spot Radius:", "", "1", "0", "150");

    // Timing/input parameters
    CKB_KPMODE(CKB_KP_POSITION);
    CKB_TIMEMODE(CKB_TIME_DURATION);
    CKB_REPEAT(FALSE);
    CKB_LIVEPARAMS(TRUE);
    
    // Presets
    CKB_PRESET_START("Single Spot");
    CKB_PRESET_PARAM("duration", "1.0");
    //CKB_PRESET_PARAM("radius2", "0");
    CKB_PRESET_PARAM("trigger", "0");
    CKB_PRESET_PARAM("kptrigger", "1");
    CKB_PRESET_END;
}

ckb_gradient animcolor = { 0 };
long radius = 1;
int* keyUsages = NULL;
int* keyPressed = NULL;
double* keyTiming = NULL;
double duration = 1.f;
int randomBright = 0;

void ckb_init(ckb_runctx* context){
    keyUsages = calloc(context->keycount, sizeof(int));
    keyPressed = calloc(context->keycount, sizeof(int));
    keyTiming = calloc(context->keycount, sizeof(double));
}

void ckb_parameter(ckb_runctx* context, const char* name, const char* value){
    CKB_PARSE_AGRADIENT("color", &animcolor){}
    //CKB_PARSE_LONG("frequency", &radius){}
    CKB_PARSE_DOUBLE("duration", &duration){}
    CKB_PARSE_BOOL("random", &randomBright){}
}

#define ANIM_MAX (144 * 2)
struct {
    int active;
    float x, y;
    ckb_key* press;
} anim[ANIM_MAX] = { };

void anim_add(ckb_key* press, float x, float y){
    for(int i = 0; i < ANIM_MAX; i++){
		if(anim[i].press == press && anim[i].active)
			return;
	}
    for(int i = 0; i < ANIM_MAX; i++){
        if(anim[i].active)
            continue;
        anim[i].active = 1;
        anim[i].x = x;
        anim[i].y = y;
        anim[i].press = press;
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
    // Add or remove a spot on this key
    if(state){
        anim_add(key, x, y);
		keyUsages[key - context->keys] += 30*duration;
		keyPressed[key - context->keys] = 1;
		keyTiming[key - context->keys] = 0;
	}
    else{
        //anim_remove(x, y);
		keyPressed[key - context->keys] = 0;
	}
}

void ckb_start(ckb_runctx* context, int state){
	return;
}

void ckb_time(ckb_runctx* context, double delta){
/*	int i = context->keycount;
	for(;i --> 0;){
		if(keyUsages[i] && !keyPressed[i]){
			keyTiming[i] -= delta;
			while(keyTiming[i] < 0){
				keyUsages[i]--;
				keyTiming[i] += 0.03333333333;
			}
		}
	}*/
}

inline int max(int a, int b){
	return a > b ? a : b;
}

inline int min(int a, int b){
	return a < b ? a : b;
}

inline int clamp(int a, int b, int t){
	return max(min(a,b), t);
}

int gcounter = 0;

int ckb_frame(ckb_runctx* context){
    CKB_KEYCLEAR(context);
    // Draw spots
    unsigned count = context->keycount;
    ckb_key* keys = context->keys;
    for(unsigned i = 0; i < ANIM_MAX; i++){
        //if(anim[i].active){
            for(ckb_key* key = keys; key < keys + count; key++){
				if(!(anim[i].press && keyUsages[anim[i].press - keys]))
					continue;
                if(max(abs(anim[i].x - key->x), abs(anim[i].y - key->y)) <= radius){
					float a, r, g, b;
                   if(randomBright)
					   ckb_grad_color(&a, &r, &g, &b, &animcolor, ((float)(rand() % 1000))/10.f);
				   else
				       ckb_grad_color(&a, &r, &g, &b, &animcolor, ((float)min(keyUsages[anim[i].press-keys],300))/3.f);
		   ckb_alpha_blend(key, a, r, g, b);
                }	 
            }
        //}
    }
    gcounter++;
    if(!(gcounter %= 3))
    	for(;count --> 0;)
	    if(keyUsages[count] && !keyPressed[count])
	    	keyUsages[count]--;
    return 0;
}

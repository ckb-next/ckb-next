/*
 * 
 * Copyright (C) 2017  Devon Richards
 * ckb-heat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * ckb-heat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with ckb-heat.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "../ckb/ckb-anim.h"
#include <math.h>

void ckb_info(){
    // Plugin info
    CKB_NAME("Heat Map");
    CKB_VERSION("0.2");
    CKB_COPYRIGHT("2017", "RULER501");
    CKB_LICENSE("GPLv2");
    CKB_GUID("{097D69F0-70B2-48B8-AFE2-25A1CDB0D92C}");
    CKB_DESCRIPTION("A spot effect on pressed keys that shows usage");

    // Effect parameters
    CKB_PARAM_AGRADIENT("color", "Fade color:", "", "ffffffff");
    CKB_PARAM_BOOL("random", "Random Brightness", 0);
	CKB_PARAM_LONG("ffade", "Frames to fade", "frames", 30, 10, 1000);
	CKB_PARAM_DOUBLE("pressestofull", "Presses to full", "keypresses", 10.f, 1.f, 100.f);

    // Timing/input parameters
    CKB_KPMODE(CKB_KP_POSITION);
    CKB_TIMEMODE(CKB_TIME_ABSOLUTE);
    CKB_REPEAT(FALSE);
    CKB_LIVEPARAMS(TRUE);
    
    // Presets
    CKB_PRESET_START("Single Spot");
	CKB_PRESET_PARAM("random", "0");
	CKB_PRESET_PARAM("ffade", "30");
	CKB_PRESET_PARAM("pressestofull", "10");
	CKB_PRESET_PARAM("trigger", "0");
    CKB_PRESET_PARAM("kptrigger", "1");
    CKB_PRESET_END;
}

ckb_gradient animcolor = { 0 };
int randomBright = 0;
long ffade = 30;
double pressestofull = 10.f;

typedef struct{
	unsigned int usages;
	int pressed;
	double timing;
	int x;
	int y;
} keyanim;

keyanim* anims = NULL;

void ckb_init(ckb_runctx* context){
	anims = calloc(context->keycount, sizeof(keyanim));
	for(int i=0; i < context->keycount; i++){
		anims[i].x = context->keys[i].x;
		anims[i].y = context->keys[i].y;
	}
}

void ckb_parameter(ckb_runctx* context, const char* name, const char* value){
    CKB_PARSE_AGRADIENT("color", &animcolor){}
	CKB_PARSE_BOOL("random", &randomBright){}
    CKB_PARSE_LONG("ffade", &ffade){}
	CKB_PARSE_DOUBLE("pressestofull", &pressestofull){}
}

void anim_add(int index){
	anims[index].usages += ffade;
	anims[index].pressed = 1;
	anims[index].timing = 0.f;
}

void anim_remove(int index){
	anims[index].pressed = 0;
}

void ckb_keypress(ckb_runctx* context, ckb_key* key, int x, int y, int state){
    // Add or remove a spot on this key
    if(state)
        anim_add(key - context->keys);
    else
        anim_remove(key - context->keys);
}

void ckb_start(ckb_runctx* context, int state){
	return;
}

void ckb_time(ckb_runctx* context, double delta){
	unsigned int count = context->keycount;
	for(unsigned int i=0; i < count; i++){
		if(anims[i].usages && !anims[i].pressed){
			anims[i].timing -= delta;
			while(anims[i].timing < 0){
				anims[i].timing += 1.f/30.f;
				if(anims[i].usages > 0)
					anims[i].usages--;
			}	
		}
	}
}

int ckb_frame(ckb_runctx* context){
    CKB_KEYCLEAR(context);
    // Draw spots
    unsigned count = context->keycount;
    ckb_key* keys = context->keys;
    unsigned int full = pressestofull*ffade;
	for(int i =0; i < count; i++){
		if(!anims[i].usages)
			continue;
		float a, r, g, b;
		if(randomBright)
			ckb_grad_color(&a, &r, &g, &b, &animcolor, ((float)(rand() % 1024))/10.f);
		else
			ckb_grad_color(&a, &r, &g, &b, &animcolor, (float)fmin(anims[i].usages,full)*100.f/full);
		ckb_alpha_blend(keys+i, a, r, g, b);
    }
    return 0;
}

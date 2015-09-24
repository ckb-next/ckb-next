#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include <pulse/simple.h>

#include "../ckb/ckb-anim.h"

//#define FIXED_POINT 16
#include "kiss_fftr.h"

void ckb_info(){
    // Plugin info
    CKB_NAME("Music Visualization");
    CKB_VERSION("0.5");
    CKB_COPYRIGHT("2015", "RULER501");
    CKB_LICENSE("LGPLv3");
    CKB_GUID("{097D69F0-70B2-48B8-AFE2-25CA1DB0D92C}");
    CKB_DESCRIPTION("A collection of music visualization effects");

    // Effect parameters
    CKB_PARAM_AGRADIENT("color", "Fade color:", "", "ffffffff");
    //CKB_PARAM_BOOL("random", "Random Coloring", 0);
    //CKB_PARAM_LONG("frequency", "Spot Radius:", "", "1", "0", "150");

    // Timing/input parameters
    CKB_KPMODE(CKB_KP_NONE);
    CKB_TIMEMODE(CKB_TIME_ABSOLUTE);
    CKB_REPEAT(FALSE);
    CKB_LIVEPARAMS(TRUE);
    
    // Presets
    CKB_PRESET_START("Default");
    //CKB_PRESET_PARAM("duration", "1.0");
    //CKB_PRESET_PARAM("radius2", "0");
    CKB_PRESET_PARAM("trigger", "0");
    CKB_PRESET_PARAM("kptrigger", "1");
    CKB_PRESET_END;
}

ckb_gradient animcolor = { 0 };
pa_simple *pas = NULL;
FILE* t;

typedef struct{
	int freq;
    unsigned int power;
} freqdec;

freqdec buf[2049];
kiss_fft_cpx* inbuf;
kiss_fft_cpx* outbuf;

void ckb_init(ckb_runctx* context){
	static const pa_sample_spec ss ={
		.format = PA_SAMPLE_S16LE,
		.rate = 44100,
		.channels = 1
	};
	int error;
	t = fopen("/run/media/devon/Share/Projects/ckb/stderr", "w");
	pas = pa_simple_new(NULL, "CKB Music Viz", PA_STREAM_RECORD, NULL, "CKB Music Viz", &ss, NULL, NULL, &error);
	fprintf(t, "Initialized Pulse\n");
	fflush(t);
	if(!pas){
		fprintf(t, "Very serious problem with initialization of pulse\n"); fflush(t);
	}
	for(int i=0; i<2048; i++)
		buf[i].freq = 22050*i/2048;
	inbuf = malloc(2048*sizeof(kiss_fft_cpx));
	outbuf = malloc(2048*sizeof(kiss_fft_cpx));
}

void ckb_parameter(ckb_runctx* context, const char* name, const char* value){
    CKB_PARSE_AGRADIENT("color", &animcolor){}
    //CKB_PARSE_LONG("frequency", &radius){}
    //CKB_PARSE_DOUBLE("duration", &duration){}
    //CKB_PARSE_BOOL("random", &randomBright){}
}

void anim_add(ckb_key* press, float x, float y){
    /*for(int i = 0; i < ANIM_MAX; i++){
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
	*/
}

void anim_remove(float x, float y){
    /*for(int i = 0; i < ANIM_MAX; i++){
        if(anim[i].active && anim[i].x == x && anim[i].y == y)
            anim[i].active = 0;
    }
	*/
}

void ckb_keypress(ckb_runctx* context, ckb_key* key, int x, int y, int state){
    // Add or remove a spot on this key
    /*if(state){
        anim_add(key, x, y);
		keyUsages[key - context->keys] += 30*duration;
		keyPressed[key - context->keys] = 1;
		keyTiming[key - context->keys] = 0;
	}
    else{
        //anim_remove(x, y);
		keyPressed[key - context->keys] = 0;
	}
	*/
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

void getFreqDec(){
	int16_t data[2048];
	int error;
	fprintf(t, "Trying to read from pulse\n"); fflush(t);
	pa_simple_read(pas, data, sizeof(data), &error);
	fprintf(t, "Made it past reading, and kiss_fftr expects %d sized scalars\n", (int)sizeof(kiss_fft_scalar)); fflush(t);
	//kiss_fftr_cfg config = kiss_fftr_alloc(4096, 0, NULL, NULL);
	//kiss_fft_cpx in[2048];
	for(int j=0; j<2048; j++){
		inbuf[j].r = data[j];
		inbuf[j].i = 0;
	}
	kiss_fft_cfg config = kiss_fft_alloc(2048, 0, NULL, NULL);
	//kiss_fft_cpx out[2048];
	kiss_fft(config, inbuf, outbuf);
	//kiss_fftr(config, data, out);
	fprintf(t, "made it past calculating the fft\n"); fflush(t);

	for(unsigned int j=0; j < 2048; j++)
		buf[j].power = sqrt(outbuf[j].r*outbuf[j].r + outbuf[j].i*outbuf[j].i);
	
	kiss_fft_free(config);
	kiss_fft_cleanup();
	//fprintf(t, "Somehow made it through getFreqDec\n");fflush(t);
	return;
}

int ckb_frame(ckb_runctx* context){
    CKB_KEYCLEAR(context);
    ckb_key* keys = context->keys;
	ckb_key* maxkey = keys+context->keycount-1;
	fprintf(t, "Calling getFreqDec\n"); fflush(t);
	getFreqDec();
	fprintf(t, "ckb_frame resumed control\n"); fflush(t);
	unsigned int frames = context->width*context->height - 1;
	int height = context->height;
	if(!frames)
		fprintf(t, "Frames is empty\n"); fflush(t);
	for(ckb_key* key = keys; key < maxkey; key++){
		int posl = height*key->x + key->y - 1;
		posl = posl > 0 ? posl : 0;
		int posr = height*key->x + key->y + 1;
		posr = posr > 0 ? posr : 0;
		int lowfreq = floorf(pow(2,posl*11.f/frames));
		int highfreq = ceilf(pow(2,posr*11.f/frames));
		highfreq = highfreq < sizeof(buf)/sizeof(freqdec) ? highfreq : sizeof(buf)/sizeof(freqdec) - 1;
		lowfreq = lowfreq > 0 ? lowfreq : 0;
		int count = 0;
		long long total = 0;
		unsigned int height = context->height;
		fprintf(t, "too the for loop\n"); fflush(t);
		for(unsigned int i = lowfreq; i <= highfreq; i++){
			//fprintf(t, "Entering the for loop\n"); fflush(t);
			//float lfreq = log(buf[i].freq > 0 ? buf[i].freq : 1);
			//if(lowfreq <= lfreq && lfreq <= highfreq){
			total += buf[i].power;
			count++;
			//}
		}
		if(count)
			total /= count;
		else
			total = 0;
		float a, r, g, b;
		ckb_grad_color(&a, &r, &g, &b, &animcolor, total/4096.f);
		ckb_alpha_blend(key, a, r, g, b);
	}
	fprintf(t, "Made it through the frame\n"); fflush(t);
/*    for(unsigned i = 0; i < ANIM_MAX; i++){
        //if(anim[i].active){
            for(ckb_key* key = keys; key < keys + count; key++){
				if(!(anim[i].press && keyUsages[anim[i].press - keys]))
					continue;
                if(max(abs(anim[i].x - key->x), abs(anim[i].y - key->y)) <= radius){
					float a, r, g, b;
                   if(random)
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
*/
    return 0;
}

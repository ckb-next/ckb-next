#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include <pulse/simple.h>

#include "../ckb/ckb-anim.h"

#include "kiss_fftr.h"

void ckb_info(){
    // Plugin info
    CKB_NAME("Music Visualization");
    CKB_VERSION("0.2");
    CKB_COPYRIGHT("2015", "RULER501");
    CKB_LICENSE("LGPLv3");
    CKB_GUID("{097D69F0-70B2-48B8-AFE2-25CA1DB0D92C}");
    CKB_DESCRIPTION("A collection of music visualization effects");

    // Effect parameters
    CKB_PARAM_AGRADIENT("color", "Fade color:", "", "ffffffff");

    // Timing/input parameters
    CKB_KPMODE(CKB_KP_NONE);
    CKB_TIMEMODE(CKB_TIME_ABSOLUTE);
    CKB_REPEAT(FALSE);
    CKB_LIVEPARAMS(TRUE);
    
    // Presets
    CKB_PRESET_START("Default");
    CKB_PRESET_PARAM("trigger", "0");
    CKB_PRESET_PARAM("kptrigger", "1");
    CKB_PRESET_END;
}

typedef struct{
	int freq;
    unsigned int power;
} freqdec;

freqdec buf[2048];
kiss_fft_cpx* inbuf;
kiss_fft_cpx* outbuf;
ckb_gradient animcolor = { 0 };
pa_simple *pas = NULL;

void ckb_init(ckb_runctx* context){
	static const pa_sample_spec ss ={
		.format = PA_SAMPLE_S16LE,
		.rate = 44100,
		.channels = 1
	};
	pas = pa_simple_new(NULL, "CKB Music Viz", PA_STREAM_RECORD, NULL, "CKB Music Viz", &ss, NULL, NULL, NULL);
	for(int i=0; i<2048; i++)
		buf[i].freq = 22050*i/2048;
	inbuf = malloc(2048*sizeof(kiss_fft_cpx));
	outbuf = malloc(2048*sizeof(kiss_fft_cpx));
}

void ckb_parameter(ckb_runctx* context, const char* name, const char* value){
    CKB_PARSE_AGRADIENT("color", &animcolor){}
}

void anim_add(ckb_key* press, float x, float y){
	return;
}

void anim_remove(float x, float y){
	return;
}

void ckb_keypress(ckb_runctx* context, ckb_key* key, int x, int y, int state){
	return;
}

void ckb_start(ckb_runctx* context, int state){
	return;
}

void ckb_time(ckb_runctx* context, double delta){
	return;
}

int max(int a, int b){
	return a > b ? a : b;
}

int min(int a, int b){
	return a < b ? a : b;
}

int gcounter = 0;

void getFreqDec(){
	int16_t data[2048];
	pa_simple_read(pas, data, sizeof(data), NULL);
	for(int j=0; j<2048; j++){
		inbuf[j].r = data[j];
		inbuf[j].i = 0;
	}
	kiss_fft_cfg config = kiss_fft_alloc(2048, 0, NULL, NULL);
	kiss_fft(config, inbuf, outbuf);

	for(unsigned int j=0; j < 2048; j++)
		buf[j].power = sqrt(outbuf[j].r*outbuf[j].r + outbuf[j].i*outbuf[j].i);
	
	kiss_fft_free(config);
	kiss_fft_cleanup();
}

int ckb_frame(ckb_runctx* context){
    CKB_KEYCLEAR(context);
    ckb_key* keys = context->keys;
	ckb_key* maxkey = keys+context->keycount-1;
	getFreqDec();
	unsigned int frames = context->width*context->height - 1;
	int height = context->height;
	for(ckb_key* key = keys; key < maxkey; key++){
		int posl = height*key->x + key->y - 1;
		posl = max(posl, 0); 
		int posr = height*key->x + key->y + 1;
		posr = max(posr, 0);
		int lowi = floorf(pow(2,posl*11.f/frames));
		int highi = ceilf(pow(2,posr*11.f/frames));
		highi= min(highi, (int)sizeof(buf)/sizeof(freqdec)-1); 
		lowi = max(lowi, 0);
		long long total = 0;
		unsigned int height = context->height;
		for(unsigned int i = lowi; i <= highi; i++)
			total += buf[i].power;
		total /= highi - lowi + 1;
		float a, r, g, b;
		ckb_grad_color(&a, &r, &g, &b, &animcolor, total/12288.f);
		ckb_alpha_blend(key, a, r, g, b);
	}
    return 0;
}

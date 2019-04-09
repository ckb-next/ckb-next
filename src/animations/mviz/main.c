/*
 * Copyright (C) 2017  Devon Richards
 * Copyright (C) 2019  Tasos Sahanidis <code@tasossah.com>
 * mviz is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * mviz is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with mviz.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef USE_PORTAUDIO
#include <portaudio.h>
PaStream *stream = NULL;
long int devid = 0;
#else
#include <pulse/simple.h>
pa_simple *stream = NULL;
#endif

#include <kissfft/kiss_fftr.h>
#include <ckb-next/animation.h>

#define MAX_RANGE 1024
int translated_frame[MAX_RANGE] = { 0 };
//#define range 768 // ~15KHz
long int range = 512;
double div_fact = 2000000.0;
// Normalisation factor * 100
double norm_f = 0.06;
long int v_offset = 0;

// int linear = 0;

int single_colour = 0;

void ckb_info(){
    CKB_NAME("Music Visualization");
    CKB_VERSION("1.0");
    CKB_COPYRIGHT("2017-2019", "RULER501 & TasosSah");
    CKB_LICENSE("GPLv2+");
    CKB_GUID("{097D69F0-70B2-48B8-AFE2-25CA1DB0D92C}");
    CKB_DESCRIPTION("A collection of music visualization effects.<br><a href=\"https://github.com/ckb-next/ckb-next/wiki/Animations#mviz-music-visualizer\">Set-up instructions</a>");

    // Effect parameters
    CKB_PARAM_AGRADIENT("color", "Colour:", "", "0:ff00ff00 30:ffffff00 60:ffffa500 77:ffff0000 100:ffff0000");
    CKB_PARAM_ARGB("bgcolour", "Background:", "", 0xFF, 0x00, 0x00, 0x00);
    CKB_PARAM_DOUBLE("norm_f", "Normalisation:", "", 7.5, 0.1, 20.0);
    CKB_PARAM_DOUBLE("div_fact", "Sensitivity:", "", 70.0, 1.0, 100.0);
#ifdef USE_PORTAUDIO
    CKB_PARAM_LONG("devid", "Device:\n(See About)", "", 0, 0, 100);
#endif
    CKB_PARAM_LONG("v_offset", "Vertical Offset:", "", 0, 0, 100);
    CKB_PARAM_LONG("range", "Range:", "", 512, 1, MAX_RANGE);

//    CKB_PARAM_BOOL("linear", "Use linear scale", "0");

    CKB_PARAM_BOOL("singlecol", "Single Colour", 0);

    // Timing/input parameters
    CKB_KPMODE(CKB_KP_NONE);
    CKB_TIMEMODE(CKB_TIME_ABSOLUTE);
    CKB_REPEAT(FALSE);
    CKB_LIVEPARAMS(TRUE);

    // Presets
    CKB_PRESET_START("Default");
    CKB_PRESET_PARAM("trigger", "0");
    CKB_PRESET_PARAM("kptrigger", "1");
    CKB_PRESET_PARAM("norm_f", "7.5");
    CKB_PRESET_PARAM("v_offset", "0");
    CKB_PRESET_PARAM("singlecol", "0");

//    CKB_PRESET_PARAM("linear", "0");

    CKB_PRESET_END;


    CKB_PRESET_START("Single Colour");
    CKB_PRESET_PARAM("trigger", "0");
    CKB_PRESET_PARAM("kptrigger", "1");
    CKB_PRESET_PARAM("norm_f", "7.5");
    CKB_PRESET_PARAM("v_offset", "0");
    CKB_PRESET_PARAM("singlecol", "1");
    CKB_PRESET_END;

}

double powers[1024] = { 0.0 };
double hann_res[2048] = { 0.0 };
kiss_fft_cfg config;
kiss_fft_cpx inbuf[2048] = { 0 };
kiss_fft_cpx outbuf[2048] = { 0 };
ckb_gradient color = { 0 };
unsigned char bg_a, bg_r, bg_g, bg_b;

void destroyCapture(){
    if(stream == NULL)
        return;
#ifdef USE_PORTAUDIO
    Pa_CloseStream(stream);
#else
    pa_simple_free(stream);
#endif
    stream = NULL;
}

void initCapture(){
    if(stream != NULL)
        return;
#ifdef USE_PORTAUDIO
    PaStreamParameters inputParams;
    PaError err;

    if(devid > -1 && devid < Pa_GetDeviceCount())
        inputParams.device = devid;
    else
        inputParams.device = Pa_GetDefaultInputDevice();

    const PaDeviceInfo* inputInfo = Pa_GetDeviceInfo(inputParams.device);
    fprintf(stderr, "Recording from %s", inputInfo->name);

    inputParams.channelCount = 1;
    inputParams.sampleFormat = paInt16;
    inputParams.suggestedLatency = inputInfo->defaultLowInputLatency;
    inputParams.hostApiSpecificStreamInfo = NULL;
    
    err = Pa_OpenStream(&stream, &inputParams, NULL, 44100, 2048, paNoFlag, NULL, NULL);
    if(err != paNoError)
        fprintf(stderr, "Error opening PortAudio stream %d\n", err);
    
    err = Pa_StartStream(stream);
    if(err != paNoError)
        fprintf(stderr, "Error starting PortAudio stream %d\n", err);    
#else
    static const pa_sample_spec ss ={
        .format = PA_SAMPLE_S16LE,
        .rate = 44100,
        .channels = 1
    };
    stream = pa_simple_new(NULL, "CKB Music Viz", PA_STREAM_RECORD, NULL, "CKB Music Viz", &ss, NULL, NULL, NULL);
#endif
}

void ckb_init(ckb_runctx* context){
    config = kiss_fft_alloc(2048, 0, NULL, NULL);

#ifdef USE_PORTAUDIO
#ifdef __linux__
    putenv("PULSE_LATENCY_MSEC=90");
#endif
    PaError err = Pa_Initialize();
    if(err != paNoError){
        fprintf(stderr, "Error initialising PortAudio %d\n", err);
    }
#else
    // We do not need to wait for the device id on pulseaudio
    initCapture();
#endif

    for (int i = 0; i < 2048; i++)
        hann_res[i] = (1 - cos(2 * M_PI * i / 2048)) * 0.5;
}

void ckb_parameter(ckb_runctx* context, const char* name, const char* value){
    CKB_PARSE_AGRADIENT("color", &color){};
    CKB_PARSE_LONG("v_offset", &v_offset){};
    CKB_PARSE_DOUBLE("norm_f", &norm_f){
        norm_f /= 100;
    };
    CKB_PARSE_DOUBLE("div_fact", &div_fact){
        div_fact = 101 - div_fact;
        div_fact *= 100000.0;
    };
    CKB_PARSE_AGRADIENT("color", &color){};
    CKB_PARSE_ARGB("bgcolour", &bg_a, &bg_r, &bg_g, &bg_b){};
    CKB_PARSE_LONG("range", &range){};

    CKB_PARSE_BOOL("singlecol", &single_colour){};

//    CKB_PARSE_BOOL("linear", &linear){};

#ifdef USE_PORTAUDIO
    CKB_PARSE_LONG("devid", &devid){
        // 0 is considered a valid value for portaudio, we use 0 for default
        devid -= 1;
        // If we were already running, close the previous stream
        destroyCapture();
        // Create a new stream
        initCapture();
    };
#endif
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

void getFreqDec(){
    int16_t data[2048] = { 0 };

    if(stream == NULL)
        return;

#ifdef USE_PORTAUDIO
    Pa_ReadStream(stream, data, 2048);
#else
    pa_simple_read(stream, data, sizeof(data), NULL);
#endif

    for(int i = 0; i < 2048; i++){
        inbuf[i].r = data[i] * hann_res[i];
        inbuf[i].i = 0;
    }

    kiss_fft(config, inbuf, outbuf);

    for(int j = 0; j < 1024; j++)
        powers[j] = sqrt(outbuf[j].r*outbuf[j].r + outbuf[j].i*outbuf[j].i);
}

void translate_frame_vertically(int height){
    for(int i = 0; i < range; i++)
        translated_frame[i] = round(height * (powers[i]/(div_fact * 3)) * (norm_f * (i + 1) + 2));
}

int get_key_value(int width, int x){
    int cur_pos = range * x / width;
    cur_pos = (cur_pos > range - 1 ? range - 1 : cur_pos);
    int newval = translated_frame[cur_pos];
    // Each key takes 18 nearby values from the array
    // Pick the largest one for this key.
    if(cur_pos > 17){
        for(int i = 0; i < 18; i++)
            if(translated_frame[cur_pos - i] > newval)
                newval = translated_frame[cur_pos - i];
    }
    return newval;
}

void grad_to_key(ckb_runctx* context, ckb_key* key, int key_y){
    // Get colour for this key
    // Map the LEDs to the gradient, bottom (0) is at 0%, top (height) is at 100%
    float key_colour = (100.f / context->height) * key_y;
    float a, r, g, b;
    ckb_grad_color(&a, &r, &g, &b, &color, key_colour);
    key->a = roundf(a);
    key->r = roundf(r);
    key->g = roundf(g);
    key->b = roundf(b);
}

int ckb_frame(ckb_runctx* context){
    CKB_KEYCLEAR(context);
    ckb_key* keys = context->keys;
    ckb_key* maxkey = keys + context->keycount;
    getFreqDec();

    if(single_colour){
        translate_frame_vertically(100);
        // Find the max value
        int max_light = translated_frame[0];
        for(int i = 0; i < range; i++)
            if(translated_frame[i] > max_light)
                max_light = translated_frame[i];

        float a, r, g, b;
        ckb_grad_color(&a, &r, &g, &b, &color, max_light);
        for(ckb_key* key = keys; key < maxkey; key++){
            // TODO DEDUP
            key->a = roundf(a);
            key->r = roundf(r);
            key->g = roundf(g);
            key->b = roundf(b);
        }

        return 0;
    }

    for(ckb_key* key = keys; key < maxkey; key++){
        translate_frame_vertically(context->height);
        int key_y = context->height - key->y;
        // Blank out the key if it is not supposed to be on
        if(key_y + v_offset <= get_key_value(context->width, key->x)){
            grad_to_key(context, key, key_y);
        } else {
            key->a = bg_a;
            key->r = bg_r;
            key->g = bg_g;
            key->b = bg_b;
        }
    }

    return 0;
}

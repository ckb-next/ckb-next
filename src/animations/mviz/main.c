/*
 * Copyright (C) 2017  Devon Richards
 * Copyright (C) 2019 - 2022  Tasos Sahanidis <code@tasossah.com>
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
#include <stdbool.h>
#include <time.h>
#include <assert.h>

#include <pulse/pulseaudio.h>
static pa_mainloop* pa_ml;
static pa_context* pa_ctx;
static pa_stream* pa_str;
static pa_context_state_t pa_state = PA_CONTEXT_UNCONNECTED;
typedef struct {
    char* dev;
    bool ok;
} server_info_cb_data;

#include <kissfft/kiss_fftr.h>
#include <ckb-next/animation.h>

#define APPEND(a, b) a ## b
#define F_CONST(num) APPEND(num, f)
#define M_PI_F F_CONST(M_PI)

#define FFT_SIZE 1024
static float translated_frame[FFT_SIZE/4] = { 0.f };

#define MAX_LATENCY 16666
static float powers[FFT_SIZE/2] = { 0.f };
static float hann_res[FFT_SIZE] = { 0.f };
static kiss_fft_cfg config;
static ckb_gradient color = { 0 };
static unsigned char bg_a, bg_r, bg_g, bg_b;

static double div_fact = 100.0;
// Normalisation factor * 100
static double norm_f = 0.06;
static long int v_offset = 0;
static size_t actual_fft_size = 0;
static unsigned int* key_lit = NULL;

static int single_colour = 0;

#define ENUMVALSTR_(x) #x
#define ENUMVALSTR(x) ENUMVALSTR_(x)
typedef enum {
    CAPTURE_DEV_SERVER,
    CAPTURE_DEV_MONITOR,
    CAPTURE_DEV_SOURCE,
    CAPTURE_DEV_MAX,
} default_device_t;
// We have to use long int here as this is the only int the functions can understand
// Default value is CAPTURE_DEV_SERVER to not break older profiles
static long int default_device = CAPTURE_DEV_SERVER, old_default_device = CAPTURE_DEV_SERVER;

void ckb_info(){
    CKB_NAME("Music Visualization");
    CKB_VERSION("1.0");
    CKB_COPYRIGHT("2017-2022", "RULER501 & TasosSah");
    CKB_LICENSE("GPLv2+");
    CKB_GUID("{097D69F0-70B2-48B8-AFE2-25CA1DB0D92C}");
    CKB_DESCRIPTION("A collection of music visualization effects.<br><a href=\"https://github.com/ckb-next/ckb-next/wiki/Animations#mviz-music-visualizer\">Set-up instructions</a>");

    // Effect parameters
    CKB_PARAM_AGRADIENT("color", "Colour:", "", "0:ff00ff00 30:ffffff00 60:ffffa500 77:ffff0000 100:ffff0000");
    CKB_PARAM_ARGB("bgcolour", "Background:", "", 0xFF, 0x00, 0x00, 0x00);
    CKB_PARAM_DOUBLE("norm_f", "Normalisation:", "", 7.5, 0.1, 20.0);
    CKB_PARAM_DOUBLE("div_fact", "Sensitivity:", "", 70.0, 1.0, 100.0);
    CKB_PARAM_LONG("v_offset", "Vertical Offset:", "", 0, 0, 100);

    CKB_PARAM_BOOL("singlecol", "Single Colour", 0);
    CKB_PARAM_LONG("default_device", "Audio device type:", "", CAPTURE_DEV_MONITOR, CAPTURE_DEV_SERVER, CAPTURE_DEV_MAX-1);

    // Timing/input parameters
    CKB_KPMODE(CKB_KP_NONE);
    CKB_TIMEMODE(CKB_TIME_DURATION);
    CKB_REPEAT(FALSE);
    CKB_LIVEPARAMS(TRUE);

    // Presets
#define MON_STR " (Monitor)"
#define SRC_STR " (Source)"
#define CST_SRC " (Custom)"
    char enumstr[2];
    // Spectrum
    CKB_PRESET_START("Spectrum" MON_STR);
    CKB_PRESET_PARAM("trigger", "1");
    CKB_PRESET_PARAM("kptrigger", "0");
    CKB_PRESET_PARAM("norm_f", "7.5");
    CKB_PRESET_PARAM("v_offset", "0");
    CKB_PRESET_PARAM("singlecol", "0");
    snprintf(enumstr, sizeof(enumstr), "%1d", (int)CAPTURE_DEV_MONITOR);
    CKB_PRESET_PARAM("default_device", enumstr);
    CKB_PRESET_END;

    CKB_PRESET_START("Spectrum" SRC_STR);
    CKB_PRESET_PARAM("trigger", "1");
    CKB_PRESET_PARAM("kptrigger", "0");
    CKB_PRESET_PARAM("norm_f", "7.5");
    CKB_PRESET_PARAM("v_offset", "0");
    CKB_PRESET_PARAM("singlecol", "0");
    snprintf(enumstr, sizeof(enumstr), "%1d", (int)CAPTURE_DEV_SOURCE);
    CKB_PRESET_PARAM("default_device", enumstr);
    CKB_PRESET_END;

    CKB_PRESET_START("Spectrum" CST_SRC);
    CKB_PRESET_PARAM("trigger", "1");
    CKB_PRESET_PARAM("kptrigger", "0");
    CKB_PRESET_PARAM("norm_f", "7.5");
    CKB_PRESET_PARAM("v_offset", "0");
    CKB_PRESET_PARAM("singlecol", "0");
    snprintf(enumstr, sizeof(enumstr), "%1d", (int)CAPTURE_DEV_SERVER);
    CKB_PRESET_PARAM("default_device", enumstr);
    CKB_PRESET_END;

    // Single Colour
    CKB_PRESET_START("Single Colour" MON_STR);
    CKB_PRESET_PARAM("trigger", "1");
    CKB_PRESET_PARAM("kptrigger", "0");
    CKB_PRESET_PARAM("norm_f", "7.5");
    CKB_PRESET_PARAM("v_offset", "0");
    CKB_PRESET_PARAM("singlecol", "1");
    snprintf(enumstr, sizeof(enumstr), "%1d", (int)CAPTURE_DEV_MONITOR);
    CKB_PRESET_PARAM("default_device", enumstr);
    CKB_PRESET_END;

    CKB_PRESET_START("Single Colour" SRC_STR);
    CKB_PRESET_PARAM("trigger", "1");
    CKB_PRESET_PARAM("kptrigger", "0");
    CKB_PRESET_PARAM("norm_f", "7.5");
    CKB_PRESET_PARAM("v_offset", "0");
    CKB_PRESET_PARAM("singlecol", "1");
    snprintf(enumstr, sizeof(enumstr), "%1d", (int)CAPTURE_DEV_SOURCE);
    CKB_PRESET_PARAM("default_device", enumstr);
    CKB_PRESET_END;

    CKB_PRESET_START("Single Colour" CST_SRC);
    CKB_PRESET_PARAM("trigger", "1");
    CKB_PRESET_PARAM("kptrigger", "0");
    CKB_PRESET_PARAM("norm_f", "7.5");
    CKB_PRESET_PARAM("v_offset", "0");
    CKB_PRESET_PARAM("singlecol", "1");
    snprintf(enumstr, sizeof(enumstr), "%1d", (int)CAPTURE_DEV_SERVER);
    CKB_PRESET_PARAM("default_device", enumstr);
    CKB_PRESET_END;
}

#if 0
static void mviz_destroy_capture(){
    if(pa_str){
        pa_stream_disconnect(pa_str);
        pa_stream_unref(pa_str);
    }
    if(pa_ctx){
        pa_context_disconnect(pa_ctx);
        pa_context_unref(pa_ctx);
    }
    if(pa_ml)
        pa_mainloop_free(pa_ml);
    pa_str = pa_ctx = pa_ml = NULL;
}
#endif

static void pa_state_cb(pa_context* ctx, void* userdata) {
    pa_context_state_t* pa_state_p = userdata;
    *pa_state_p = pa_context_get_state(ctx);
}

static void pa_read_cb(pa_stream* p, size_t nbytes, void* userdata){
    size_t datasz = 0;
    size_t* effective_data = userdata;
    const float* dataptr;

    pa_stream_peek(p, (const void**)&dataptr, &datasz);
    if(dataptr && datasz){
        kiss_fft_cpx inbuf[FFT_SIZE] = { 0 };
        kiss_fft_cpx outbuf[FFT_SIZE];

        // If we get too many samples, ignore the extra ones
        const size_t samples = datasz / sizeof(*dataptr) > FFT_SIZE ? FFT_SIZE : datasz / sizeof(*dataptr);

        for(size_t i = 0; i < samples; i++){
            inbuf[i].r = dataptr[i] * hann_res[i];
            //inbuf[i].i = 0;
        }

        kiss_fft(config, inbuf, outbuf);

        // FFT is symmetric because there are no imaginary elements
        *effective_data = samples / 2;
        for(size_t i = 0; i < *effective_data; i++)
            powers[i] = sqrtf(outbuf[i].r * outbuf[i].r + outbuf[i].i * outbuf[i].i);

    } else {
        *effective_data = 0;
    }
    pa_stream_drop(p);
}

static void server_info_cb(pa_context* c, const pa_server_info* i, void* userdata){
    server_info_cb_data* cb_data = userdata;
    int major, minor, patch;
    // We have to check for PipeWire since it currently doesn't know what to do with "@DEFAULT_MONITOR@"
    // When these old PW versions become irrelevant, this entire callback can be removed, and the switch in the else block
    // can be moved to the init capture function.
    // FIXME: Add the version of PW the bug was fixed in
#if 0
#define PIPEWIRE_BUG_FIX_VER (major == 0 && minor <= 3 && patch < 57)
#else
#define PIPEWIRE_BUG_FIX_VER 1
#endif
    if(sscanf(i->server_name, "PulseAudio (on PipeWire %d.%d.%d)", &major, &minor, &patch) == 3 && PIPEWIRE_BUG_FIX_VER){
        DBG("Applying workaround for PipeWire %d.%d.%d", major, minor, patch);
        switch(default_device){
        case CAPTURE_DEV_MONITOR:
            cb_data->dev = malloc(strlen(i->default_sink_name) + strlen(".monitor") + 1);
            sprintf(cb_data->dev, "%s.monitor", i->default_sink_name);
            break;
        case CAPTURE_DEV_SOURCE:
            cb_data->dev = strdup(i->default_source_name);
            break;
        case CAPTURE_DEV_SERVER:
            break;
        default:
            abort();
        }
    } else {
        const char* dev;
        switch(default_device){
        case CAPTURE_DEV_MONITOR:
            dev = "@DEFAULT_MONITOR@";
            break;
        case CAPTURE_DEV_SOURCE:
            dev = "@DEFAULT_SOURCE@";
            break;
        case CAPTURE_DEV_SERVER:
            dev = NULL;
            break;
        default:
            abort();
        }
        if(dev)
            cb_data->dev = strdup(dev);
    }
    cb_data->ok = true;
}

static bool mviz_init_capture(){
    if(!pa_ml)
        pa_ml = pa_mainloop_new();

    if(!pa_ctx){
        pa_ctx = pa_context_new(pa_mainloop_get_api(pa_ml), "ckb-next music visualizer");
        pa_context_connect(pa_ctx, NULL, 0, NULL);

        pa_context_set_state_callback(pa_ctx, pa_state_cb, &pa_state);
        while(pa_state != PA_CONTEXT_FAILED && pa_state != PA_CONTEXT_TERMINATED && pa_state != PA_CONTEXT_READY)
            pa_mainloop_iterate(pa_ml, 1, NULL);

        if (pa_state != PA_CONTEXT_READY){
            pa_context_disconnect(pa_ctx);
            pa_context_unref(pa_ctx);
            pa_ctx = NULL;
            pa_state = PA_CONTEXT_UNCONNECTED;
            return true;
        }
    }

    if(!pa_str){
        static_assert(sizeof(float) == 4, "Expected 32 bit floats");
        static const pa_sample_spec ss = {
            .format = PA_SAMPLE_FLOAT32LE,
            .rate = 48000,
            .channels = 1
        };

        pa_proplist* pl = pa_proplist_new();

        pa_proplist_sets(pl, PA_PROP_APPLICATION_ICON_NAME, "ckb-next");
        pa_proplist_sets(pl, PA_PROP_APPLICATION_NAME, "ckb-next");

        DBG("New PA capture stream");
        pa_str = pa_stream_new_with_proplist(pa_ctx, "capture", &ss, NULL, pl);

        pa_proplist_free(pl);

        pa_buffer_attr bufattr = {
            .maxlength = (uint32_t)-1,
        };
        bufattr.fragsize = pa_usec_to_bytes(MAX_LATENCY, &ss);

        server_info_cb_data cb_data = {
            .dev = NULL,
            .ok = false
        };
        pa_context_get_server_info(pa_ctx, server_info_cb, &cb_data);
        while(!cb_data.ok)
            pa_mainloop_iterate(pa_ml, 1, NULL);

        DBG("Capture dev: %s", cb_data.dev ? cb_data.dev : "null");
        pa_stream_connect_record(pa_str, cb_data.dev, &bufattr, PA_STREAM_ADJUST_LATENCY);
        free(cb_data.dev);

        pa_stream_set_read_callback(pa_str, pa_read_cb, &actual_fft_size);
    }
    return false;
}

void ckb_init(ckb_runctx* context){
    key_lit = calloc(context->keycount, sizeof(*key_lit));
    config = kiss_fft_alloc(FFT_SIZE, 0, NULL, NULL);

    for (int i = 0; i < FFT_SIZE; i++)
        hann_res[i] = (1.f - cosf(2.f * M_PI_F * i / (float)(FFT_SIZE-1))) * 0.5f;
}

static void stream_state_cb(pa_stream* s, void* userdata) {
    (*(pa_stream_state_t*)userdata) = pa_stream_get_state(s);
}

void ckb_parameter(ckb_runctx* context, const char* name, const char* value){
    CKB_PARSE_AGRADIENT("color", &color){};
    CKB_PARSE_LONG("v_offset", &v_offset){};
    CKB_PARSE_DOUBLE("norm_f", &norm_f){
        norm_f /= 120.0;
    };
    CKB_PARSE_DOUBLE("div_fact", &div_fact){
        div_fact = (101.0 - div_fact) * 12.0;
    };
    CKB_PARSE_AGRADIENT("color", &color){};
    CKB_PARSE_ARGB("bgcolour", &bg_a, &bg_r, &bg_g, &bg_b){};

    CKB_PARSE_BOOL("singlecol", &single_colour){};
    CKB_PARSE_LONG("default_device", &default_device){
        // If we have an existing stream, destroy it and recreate it
        if(pa_str && old_default_device != default_device){
            DBG("Disconnecting PA stream");
            // pa_stream_disconnect sometimes randomly fails with -15, so if that happens, keep retrying...
            // If we don't do this, we'll end up with multiple streams that have not been cleaned up
            pa_stream_state_t state = PA_STREAM_READY;
            pa_stream_set_state_callback(pa_str, stream_state_cb, &state);
            int disconnect = pa_stream_disconnect(pa_str);

            while(state == PA_STREAM_READY){
                DBG("Stream state: %d", state);
                pa_mainloop_iterate(pa_ml, 1, NULL);
                if(disconnect == -PA_ERR_BADSTATE){
                    DBG("pa_stream_disconnect() returned Bad State. Retrying after loop iteration...");
                    disconnect = pa_stream_disconnect(pa_str);
                }
            }

            pa_stream_unref(pa_str);
            pa_str = NULL;

            mviz_init_capture();
            old_default_device = default_device;
        }
    };
}

void ckb_keypress(ckb_runctx* context, ckb_key* key, int x, int y, int state){
    return;
}

void ckb_start(ckb_runctx* context, int state){
    mviz_init_capture();
}

void ckb_time(ckb_runctx* context, double delta){
    ckb_key* keys = context->keys;

    for(unsigned int i = 0; i < context->keycount; i++){
        unsigned int sub = round(delta*50.0*(context->height-keys[i].y+10));
        if(key_lit[i] >= sub)
            key_lit[i] -= sub;
        else
            key_lit[i] = 0;
    }
}

#define BIN_FN (height * ((translated_frame[bin]) / (float)div_fact) * ((float)norm_f * (i + 1) + 2))
// If we want to average
// (height * ((translated_frame[bin] / step) / (float)div_fact) * ((float)norm_f * (i + 1) + 2))
static size_t lin_to_log(unsigned int height)
{
    size_t bin = 0;
    size_t step = 1;
    size_t i;
    for(i = 1; i <= actual_fft_size; i++) {

        if(powers[i - 1] > translated_frame[bin])
            translated_frame[bin] = powers[i - 1];
        // If we want to average
        //translated_frame[bin] += powers[i - 1];

        // Runs every time a multiple of step is hit
        if(!(i % step))
        {
            // Scale it to the height of the device
            translated_frame[bin] = BIN_FN;
            bin++;
        }

        // Check if we have to change the step
        // The loop starts from 1 so that this doesn't trigger on 0
        if (!(i % (step * 10)))
            step *= 10;
    }

    translated_frame[bin] = BIN_FN;

    //DBG("Total number of bins %zu\n", bin);
    return bin + 1;
}

static inline float get_key_value(unsigned int width, int x, size_t bins){
    size_t cur_pos = bins * x / width;
    cur_pos = (cur_pos > bins - 1 ? 1 - 1 : cur_pos);
    float newval = translated_frame[cur_pos];
    // Check this position and one nearby one, and pick the max
    if(cur_pos > 0 && translated_frame[cur_pos - 1] > newval)
        newval = translated_frame[cur_pos - 1];
    return newval;
}

int ckb_frame(ckb_runctx* context){
    // Don't process anything if the main loop hasn't been initialised yet
    if(!pa_ml)
        return 0;

    if(pa_state == PA_CONTEXT_FAILED){
        if(pa_str){
            pa_stream_disconnect(pa_str);
            pa_stream_unref(pa_str);
        }
        if(pa_ctx){
            pa_context_disconnect(pa_ctx);
            pa_context_unref(pa_ctx);
        }
        pa_str = NULL;
        pa_ctx = NULL;
        pa_state = PA_CONTEXT_UNCONNECTED;
        clock_nanosleep(CLOCK_MONOTONIC, 0, &(struct timespec) {.tv_sec = 1}, NULL);
        mviz_init_capture();
    }

    if(pa_state != PA_CONTEXT_READY)
        return 0;

    for(int i = 0; i < 5 && pa_mainloop_iterate(pa_ml, 0, NULL) > 0; i++)
        clock_nanosleep(CLOCK_MONOTONIC, 0, &(struct timespec) {.tv_nsec = 500000}, NULL);

    ckb_key* keys = context->keys;
    ckb_key* maxkey = keys + context->keycount;
    memset(translated_frame, 0, sizeof(translated_frame));

    if(single_colour){
        CKB_KEYCLEAR(context);
        const size_t bins = lin_to_log(100);
        // Find the max value
        float max_light = translated_frame[0];
        for(size_t i = 0; i < bins; i++)
            if(translated_frame[i] > max_light)
                max_light = translated_frame[i];

        float a, r, g, b;
        ckb_grad_color(&a, &r, &g, &b, &color, max_light);
        for(ckb_key* key = keys; key < maxkey; key++){
            key->a = roundf(a);
            key->r = roundf(r);
            key->g = roundf(g);
            key->b = roundf(b);
        }

        return 0;
    }

    const size_t bins = lin_to_log(context->height);
    for(ckb_key* key = keys; key < maxkey; key++){
        int key_y = context->height - key->y;
        // If the key is supposed to be on, light it up and reset its counter
        // If the key's counter is 0, then turn it off
        if(key_y + v_offset <= get_key_value(context->width, key->x, bins)){
            // Get colour for this key
            // Map the LEDs to the gradient, bottom (0) is at 0%, top (height) is at 100%
            float key_colour = (100.f / context->height) * key_y;
            float a, r, g, b;
            ckb_grad_color(&a, &r, &g, &b, &color, key_colour);
            key->a = roundf(a);
            key->r = roundf(r);
            key->g = roundf(g);
            key->b = roundf(b);

            key_lit[key - keys] = 200;
        } else if(key_lit[key - keys] == 0){
            key->a = bg_a;
            key->r = bg_r;
            key->g = bg_g;
            key->b = bg_b;
        }
    }

    return 0;
}

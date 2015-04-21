#ifndef CKB_ANIM_H
#define CKB_ANIM_H

// Standardized header for C/C++ CKB animations.
// If your animation contains multiple source files, #define CKB_NO_MAIN in all but one of them so you don't get duplicate symbols.
// The main function will be defined for you. You must write several specialized functions in order to handle animations:

//   void ckb_info()
//     Prints information about the program and any parameters it wishes to receive. See info helpers section.
//     This function will be caled in a separate invocation of the program. No other functions will be called at this time.

//   void ckb_init(ckb_runctx* context)
//     Called during normal invocation of the program, once, before any other functions. See runtime information section.
//   void ckb_parameter(ckb_runctx* context, const char* name, const char* value)
//     Receives the user-chosen value of a parameter (specified from ckb_info).
//     Do not make assumptions about the order or presence of parameters. Assign sane defaults as fallbacks and use the CKB parameter parsing macros.
//     This function will be called at start up, after ckb_init but before any other functions, once for each parameter.
//     If live params are enabled, it will also be called while the animation is running.

//   void ckb_start(ckb_runctx* context, int state)
//     Starts/restarts (state = 1) or stops (state = 0) an animation in the center of the keyboard (i.e. without a keypress).
//     Do not assume that this will be called at start up, or at all. It may not be the case depending on configuration.
//   void ckb_keypress(ckb_runctx* context, ckb_key* key, int x, int y, int state)
//     Receives a key down / key up event. Called only if KP mode is not NONE.
//     x and y are always filled and valid. key may be null if KP mode is POSITION.
//     Once again, do not assume when (or if) this will be called.
//   void ckb_time(ckb_runctx* context, double delta)
//     Advances the animation frame. Do not assume that this will occur at a regular interval; use delta to see how much time has passed.
//     If timing mode is DURATION, delta is in durations (0 <= delta <= 1). If timing mode is ABSOLUTE, delta is in seconds (0 <= delta).
//     Try to return as quickly as possible, because ckb may stop sending frames if it does not receive a response in time.
//   int ckb_frame(ckb_runctx* context)
//     Requests frame data from the animation. Update the context with appropriate colors.
//     Return 0 to continue running or any other number to exit. On exit, the last-set frame will remain on the keyboard.

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define CKB_DOWN    1
#define CKB_UP      0

// * Info output helpers

#define CKB_CONTAINER(macro) do { macro } while(0)

// Plugin GUID
#define CKB_GUID(guid)                                              CKB_CONTAINER( printf("guid "); printurl(guid); printf("\n"); )
// Plugin name
#define CKB_NAME(name)                                              CKB_CONTAINER( printf("name "); printurl(name); printf("\n"); )
// Plugin version
#define CKB_VERSION(version)                                        CKB_CONTAINER( printf("version "); printurl(version); printf("\n"); )
// Plugin copyright
#define CKB_COPYRIGHT(year, author)                                 CKB_CONTAINER( printf("author "); printurl(author); printf("\nyear %s\n", year); )
// Plugin license
#define CKB_LICENSE(license)                                        CKB_CONTAINER( printf("license "); printurl(license); printf("\n"); )
// Plugin description
#define CKB_DESCRIPTION(description)                                CKB_CONTAINER( printf("description "); printurl(description); printf("\n"); )

// Parameter helpers
#define CKB_PARAM(type, name, prefix, postfix, extra)               CKB_CONTAINER( printf("param %s %s ", type, name); printurl(prefix); printf(" "); printurl(postfix); printf(" "); extra; printf("\n"); )
#define CKB_PARAM_LONG(name, prefix, postfix, default, min, max)    CKB_PARAM("long", name, prefix, postfix, printf("%ld %ld %ld", (long)(default), (long)(min), (long)(max)))
#define CKB_PARAM_DOUBLE(name, prefix, postfix, default, min, max)  CKB_PARAM("double", name, prefix, postfix, printf("%lf %lf %lf", (double)(default), (double)(min), (double)(max)))
#define CKB_PARAM_BOOL(name, text, default)                         CKB_PARAM("bool", name, text, "", printf((default) ? "1" : "0"))
#define CKB_PARAM_RGB(name, prefix, postfix, r, g, b)               CKB_PARAM("rgb", name, prefix, postfix, printf("%02x%02x%02x", (unsigned char)(r), (unsigned char)(g), (unsigned char)(b)))
#define CKB_PARAM_ARGB(name, prefix, postfix, a, r, g, b)           CKB_PARAM("argb", name, prefix, postfix, printf("%02x%02x%02x%02x", (unsigned char)(a), (unsigned char)(r), (unsigned char)(g), (unsigned char)(b)))
#define CKB_PARAM_GRADIENT(name, prefix, postfix, default)          CKB_PARAM("gradient", name, prefix, postfix, printurl(default))
#define CKB_PARAM_AGRADIENT(name, prefix, postfix, default)         CKB_PARAM("agradient", name, prefix, postfix, printurl(default))
#define CKB_PARAM_ANGLE(name, prefix, postfix, default)             CKB_PARAM("angle", name, prefix, postfix, printf("%ld", (long)(default)))
#define CKB_PARAM_STRING(name, prefix, postfix, default)            CKB_PARAM("string", name, prefix, postfix, printurl(default))
#define CKB_PARAM_LABEL(name, text)                                 CKB_PARAM("label", name, text, "", )

#define CKB_PRESET_START(name)                                      CKB_CONTAINER( printf("preset "); printurl(name); )
#define CKB_PRESET_PARAM(name, value)                               CKB_CONTAINER( printf(" %s=", name); printurl(value); )
#define CKB_PRESET_END                                              CKB_CONTAINER( printf("\n"); )

// Keypress interaction (none, by name, or by position). Default: NONE
#define CKB_KP_NONE         "none"
#define CKB_KP_NAME         "name"
#define CKB_KP_POSITION     "position"
#define CKB_KPMODE(mode)                                            CKB_CONTAINER( printf("kpmode %s\n", mode); )
// Timing mode (relative to a duration, or absolute). Default: DURATION
#define CKB_TIME_DURATION   "duration"
#define CKB_TIME_ABSOLUTE   "absolute"
#define CKB_TIMEMODE(mode)                                          CKB_CONTAINER( printf("time %s\n", mode); )
// Repeatability. If enabled, the animation will be restarted after a user-chosen amount of time. Default: TRUE
#define CKB_REPEAT(enable)                                          CKB_CONTAINER( printf("repeat %s\n", (enable) ? "on" : "off"); )
// Startup preemption. Requires duration and repeat enabled. Default: FALSE
// If enabled, ckb will play an extra start command on mode switch, placed 1 duration before the actual starting animation.
#define CKB_PREEMPT(enable)                                         CKB_CONTAINER( printf("preempt %s\n", (enable) ? "on" : "off"); )
// Live parameter updates. Default: FALSE
#define CKB_LIVEPARAMS(enable)                                      CKB_CONTAINER( printf("parammode %s\n", (enable) ? "live" : "static"); )

// * Runtime information

// Parameter input parsers. Usage (within ckb_parameter only):
//  CKB_PARSE_BOOL("mybool", &mybool){
//      <actions to take when bool is parsed>
//  }
// or:
//  CKB_PARSE_BOOL("mybool", &mybool){}

#define CKB_PARSE_LONG(param_name, value_ptr)                       if(!strcmp(name, param_name) && sscanf(value, "%ld", value_ptr) == 1)
#define CKB_PARSE_DOUBLE(param_name, value_ptr)                     if(!strcmp(name, param_name) && sscanf(value, "%lf", value_ptr) == 1)
#define CKB_PARSE_BOOL(param_name, value_ptr)                       if(!strcmp(name, param_name) && sscanf(value, "%u", value_ptr) == 1)
#define CKB_PARSE_RGB(param_name, r_ptr, g_ptr, b_ptr)              if(!strcmp(name, param_name) && sscanf(value, "%2hhx%2hhx%2hhx", r_ptr, g_ptr, b_ptr) == 3)
#define CKB_PARSE_ARGB(param_name, a_ptr, r_ptr, g_ptr, b_ptr)      if(!strcmp(name, param_name) && sscanf(value, "%2hhx%2hhx%2hhx%2hhx", a_ptr, r_ptr, g_ptr, b_ptr) == 4)
#define CKB_PARSE_GRADIENT(param_name, gradient_ptr)                if(!strcmp(name, param_name) && ckb_scan_grad(value, gradient_ptr, 0))
#define CKB_PARSE_AGRADIENT(param_name, gradient_ptr)               if(!strcmp(name, param_name) && ckb_scan_grad(value, gradient_ptr, 1))
#define CKB_PARSE_ANGLE(param_name, value_ptr)                      if(!strcmp(name, param_name) && sscanf(value, "%ld", value_ptr) == 1)
#define CKB_PARSE_STRING(param_name)                                if(!strcmp(name, param_name))

// Converts an angle from ckb output to the correct angle for math functions.
// Input: [0, 359], positive direction CW. Output: [0, 2π), positive direction CCW.
#define CKB_REAL_ANGLE(angle)           fmod((-(angle) + 90.) * M_PI / 180. + M_PI * 2., M_PI * 2.)

// Key definition
#define CKB_KEYNAME_MAX 12
typedef struct {
    char name[CKB_KEYNAME_MAX];
    int x, y;
    unsigned char a, r, g, b;
} ckb_key;

// Run context. This contains all invocation info except for the user-specified parameters.
typedef struct {
    // Keys
    ckb_key* keys;
    unsigned keycount;
    // Keyboard dimensions
    unsigned width, height;
} ckb_runctx;

// Clear all keys in a context (ARGB 00000000).
// Call this at the beginning of ckb_frame to start from a blank slate. If you don't, the colors from the previous frame are left intact.
#define CKB_KEYCLEAR(context)                                       CKB_CONTAINER( ckb_key* key = context->keys; unsigned count = context->keycount; unsigned i = 0; for(; i < count; i++) key[i].a = key[i].r = key[i].g = key[i].b = 0; )

// Gradient structure
#define CKB_GRAD_MAX                100
typedef struct {
    // Number of points (at least two)
    // Gradients MUST have points at 0 and at 100
    int ptcount;
    // Point positions, in order (range: [0, 100])
    char pts[CKB_GRAD_MAX];
    // Point colors
    unsigned char a[CKB_GRAD_MAX];
    unsigned char r[CKB_GRAD_MAX];
    unsigned char g[CKB_GRAD_MAX];
    unsigned char b[CKB_GRAD_MAX];
} ckb_gradient;
int ckb_scan_grad(const char* string, ckb_gradient* gradient, int alpha);

// Color of a point on a gradient, with position given between 0 and 100 inclusive
void ckb_grad_color(float* a, float* r, float* g, float* b, const ckb_gradient* grad, float pos);

// Alpha blend a color into a key
void ckb_alpha_blend(ckb_key* key, float a, float r, float g, float b);


// * Internal functions

#ifndef CKB_NO_MAIN

// URL-encoded string printer
void printurl(const char* src){
    char out[strlen(src) * 3 + 1];
    char* dst = out;
    char s;
    while((s = *src++)){
        if(s <= ',' || s == '/' ||
                (s >= ':' && s <= '@') ||
                s == '[' || s == ']' ||
                s >= 0x7F){
            char a = s >> 4, b = s & 0xF;
            if(a >= 10)
                a += 'A' - 10;
            else
                a += '0';
            if(b >= 10)
                b += 'A' - 10;
            else
                b += '0';
            dst[0] = '%';
            dst[1] = a;
            dst[2] = b;
            dst += 3;
        } else
            *dst++ = s;
    }
    *dst = '\0';
    printf("%s", out);
}

// URL decode
void urldecode(char *dst, const char *src){
    char a, b;
    char s;
    while((s = *src)){
        if((s == '%') &&
                ((a = src[1]) && (b = src[2])) &&
                (isxdigit(a) && isxdigit(b))){
            if(a >= 'a')
                a -= 'a'-'A';
            if(a >= 'A')
                a -= 'A' - 10;
            else
                a -= '0';
            if(b >= 'a')
                b -= 'a'-'A';
            if(b >= 'A')
                b -= 'A' - 10;
            else
                b -= '0';
            *dst++ = 16*a + b;
            src += 3;
        } else {
            *dst++ = s;
            src++;
        }
    }
    *dst = '\0';
}

// Read input line
#define CKB_MAX_WORD    (4 * 1024)
void ckb_getline(char word1[CKB_MAX_WORD], char word2[CKB_MAX_WORD], char word3[CKB_MAX_WORD]){
    char line[CKB_MAX_WORD * 3 + 3];
    fgets(line, sizeof(line), stdin);
    word1[0] = word1[1] = word1[2] = 0;
    int res = sscanf(line, "%s %s %s", word1, word2, word3);
    if(res >= 1)
        urldecode(word1, word1);
    if(res >= 2)
        urldecode(word2, word2);
    if(res >= 3)
        urldecode(word3, word3);
    line[strlen(line) - 1] = 0;
}

// Gradient interpolation
void ckb_grad_color(float* a, float* r, float* g, float* b, const ckb_gradient* grad, float pos){
    // Find the points surrounding this position
    int count = grad->ptcount;
    if(count == 0){
        *a = *r = *g = *b = 0.f;
        return;
    }
    int i = 1;
    for(; i < count; i++){
        if(grad->pts[i] >= pos)
            break;
    }
    // Get color by linear interpolation. Premultiply the alpha value so that it returns the expected color
    // (i.e. stops with zero opacity won't contribute to color)
    float distance = grad->pts[i] - grad->pts[i - 1];
    float dx = (pos - grad->pts[i - 1]) / distance;
    float a1 = grad->a[i] / 255., a2 = grad->a[i - 1] / 255.;
    float a3 = *a = a1 * dx + a2 * (1.f - dx);
    if(a3 == 0.){
        *a = *r = *g = *b = 0.f;
        return;
    }
    *a *= 255.f;
    *r = (grad->r[i] * a1 * dx + grad->r[i - 1] * a2 * (1.f - dx)) / a3;
    *g = (grad->g[i] * a1 * dx + grad->g[i - 1] * a2 * (1.f - dx)) / a3;
    *b = (grad->b[i] * a1 * dx + grad->b[i - 1] * a2 * (1.f - dx)) / a3;
}

// Alpha blend
void ckb_alpha_blend(ckb_key* key, float a, float r, float g, float b){
    a /= 255.f;
    float ka = key->a / 255.f;
    float a2 = a + (1.f - a) * ka;
    if(a2 == 0.f){
        key->a = key->r = key->g = key->b = 0;
        return;
    }
    key->a = round(a2 * 255.f);
    key->r = round((r * a + key->r * ka * (1.f - a)) / a2);
    key->g = round((g * a + key->g * ka * (1.f - a)) / a2);
    key->b = round((b * a + key->b * ka * (1.f - a)) / a2);
}

// Gradient parser
int ckb_scan_grad(const char* string, ckb_gradient* gradient, int alpha){
    char pos = -1;
    unsigned char a = 255, r, g, b;
    int count = 0;
    while(1){
        int scanned = 0;
        char newpos;
        if(sscanf(string, "%hhd:%2hhx%2hhx%2hhx%2hhx%n", &newpos, &a, &r, &g, &b, &scanned) != 5)
            break;
        string += scanned;
        // Don't allow stops out-of-order or past 100
        if(newpos <= pos || newpos > 100)
            return 0;
        pos = newpos;
        if(!alpha)
            a = 255;
        gradient->pts[count] = pos;
        gradient->a[count] = a;
        gradient->r[count] = r;
        gradient->g[count] = g;
        gradient->b[count] = b;
        count++;
    }
    if(count == 0){
        // If nothing was read, try a single ARGB constant.
        if(sscanf(string, "%2hhx%2hhx%2hhx%2hhx", &a, &r, &g, &b) != 4)
            return 0;
        count = 2;
        gradient->pts[0] = 0;
        gradient->pts[1] = 100;
        gradient->a[0] = a;
        gradient->a[1] = 0;
        gradient->r[0] = gradient->r[1] = r;
        gradient->g[0] = gradient->g[1] = g;
        gradient->b[0] = gradient->b[1] = b;
    }
    if(count < 2)
        return 0;
    gradient->ptcount = count;
    return 1;
}

extern void ckb_info();
extern void ckb_init(ckb_runctx* context);
extern void ckb_parameter(ckb_runctx*, const char*, const char*);
extern void ckb_keypress(ckb_runctx*, ckb_key*, int, int, int);
extern void ckb_start(ckb_runctx*, int);
extern void ckb_time(ckb_runctx*, double);
extern int ckb_frame(ckb_runctx*);

// Update parameter values
void ckb_read_params(ckb_runctx* ctx){
    char cmd[CKB_MAX_WORD], param[CKB_MAX_WORD], value[CKB_MAX_WORD];
    do {
        ckb_getline(cmd, param, value);
        if(!*cmd){
            printf("Error [ckb-main]: Reached EOF reading parameters");
            return;
        }
        if(!strcmp(cmd, "end") && !strcmp(param, "params"))
            break;
        if(strcmp(cmd, "param"))
            continue;
        ckb_parameter(ctx, param, value);
    } while(1);
}

int main(int argc, char *argv[]){
    if(argc == 2){
        if(!strcmp(argv[1], "--ckb-info")){
            ckb_info();
            fflush(stdout);
            return 0;
        } else if(!strcmp(argv[1], "--ckb-run")){
            ckb_runctx ctx;
            // Read the keymap lines
            char cmd[CKB_MAX_WORD], param[CKB_MAX_WORD], value[CKB_MAX_WORD];
            // Skip anything up until "begin keymap"
            do {
                ckb_getline(cmd, param, value);
                if(!*cmd){
                    // If end-of-file is reached, abort
                    printf("Error [ckb-main]: Reached EOF looking for \"begin keymap\"");
                    return -2;
                }
            } while(strcmp(cmd, "begin") || strcmp(param, "keymap"));
            ckb_getline(cmd, param, value);
            unsigned keycount;
            if(strcmp(cmd, "keycount") || sscanf(param, "%u", &keycount) != 1 || keycount == 0){
                // If keycount isn't the next line, something is wrong
                printf("Error [ckb-main]: \"begin keymap\" not followed with \"keycount\"");
                return -3;
            }
            ctx.keys = (ckb_key*)calloc(keycount, sizeof(ckb_key));
            ctx.keycount = keycount;
            unsigned max_x = 0, max_y = 0;
            unsigned i = 0;
            for(; i < ctx.keycount; i++){
                ckb_getline(cmd, param, value);
                unsigned x, y;
                if(strcmp(cmd, "key") || !*param || !sscanf(value, "%u,%u", &x, &y)){
                    i--;
                    continue;
                }
                ckb_key* key = ctx.keys + i;
                strncpy(key->name, param, CKB_KEYNAME_MAX);
                key->x = x;
                key->y = y;
                if(x > max_x)
                    max_x = x;
                if(y > max_y)
                    max_y = y;
            }
            ctx.width = max_x + 1;
            ctx.height = max_y + 1;
            // Skip anything else until "end keymap"
            do {
                ckb_getline(cmd, param, value);
                if(!*cmd){
                    printf("Error [ckb-main]: Reached EOF looking for \"end keymap\"");
                    return -2;
                }
            } while(strcmp(cmd, "end") || strcmp(param, "keymap"));
            // Run init function
            ckb_init(&ctx);
            // Skip anything else until "begin params"
            do {
                ckb_getline(cmd, param, value);
                if(!*cmd){
                    printf("Error [ckb-main]: Reached EOF looking for \"begin params\"");
                    return -2;
                }
            } while(strcmp(cmd, "begin") || strcmp(param, "params"));
            // Parse parameters
            ckb_read_params(&ctx);
            // Skip anything else until "begin run"
            do {
                ckb_getline(cmd, param, value);
                if(!*cmd){
                    printf("Error [ckb-main]: Reached EOF looking for \"begin run\"");
                    return -2;
                }
            } while(strcmp(cmd, "begin") || strcmp(param, "run"));
            // Run the main loop
            printf("begin run\n");
            fflush(stdout);
            while(1){
                ckb_getline(cmd, param, value);
                if(!*cmd || (!strcmp(cmd, "end") && !strcmp(param, "run")))
                    break;
                // Parse input
                if(!strcmp(cmd, "start"))
                    ckb_start(&ctx, 1);
                else if(!strcmp(cmd, "stop"))
                    ckb_start(&ctx, 0);
                else if(!strcmp(cmd, "begin") && !strcmp(param, "params"))
                    ckb_read_params(&ctx);
                else if(!strcmp(cmd, "key")){
                    int x, y;
                    if(sscanf(param, "%d,%d", &x, &y) == 2){
                        // Find a key with this position
                        ckb_key* key = 0;
                        for(i = 0; i < keycount; i++){
                            if(ctx.keys[i].x == x && ctx.keys[i].y == y){
                                key = ctx.keys + i;
                                break;
                            }
                        }
                        if(key)
                            ckb_keypress(&ctx, key, key->x, key->y, !strcmp(value, "down"));
                        else
                            ckb_keypress(&ctx, 0, x, y, !strcmp(value, "down"));
                    } else {
                        // Find a key with this name
                        ckb_key* key = 0;
                        for(i = 0; i < keycount; i++){
                            if(!strcmp(ctx.keys[i].name, param)){
                                key = ctx.keys + i;
                                break;
                            }
                        }
                        if(key)
                            ckb_keypress(&ctx, key, key->x, key->y, !strcmp(value, "down"));
                    }
                } else if(!strcmp(cmd, "frame")){
                    int end = ckb_frame(&ctx);
                    // Output the frame
                    printf("begin frame\n");
                    for(i = 0; i < ctx.keycount; i++){
                        ckb_key* key = ctx.keys + i;
                        printf("argb %s %02hhx%02hhx%02hhx%02hhx\n", key->name, key->a, key->r, key->g, key->b);
                    }
                    printf("end frame\n");
                    if(end)
                        break;
                    fflush(stdout);
                } else {
                    double delta = 0.;
                    if(!strcmp(cmd, "time") && sscanf(param, "%lf", &delta) == 1)
                        ckb_time(&ctx, delta);
                }
            }
            printf("end run\n");
            fflush(stdout);
            free(ctx.keys);
            return 0;
        }
    }
    printf("This program must be run from within ckb\n");
    return -1;
}

#endif  // CKB_NO_MAIN

#endif  // CKB_ANIM_H

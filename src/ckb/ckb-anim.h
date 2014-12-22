#ifndef CKB_ANIM_H
#define CKB_ANIM_H

// Standardized header for CKB animations.
// If your animation contains multiple source files, #define CKB_NO_MAIN in all but one of them so you don't get duplicate symbols.
// The main function will be defined for you and will call several specialized functions (which you must write):
//   void ckb_info() :
//     Prints information about the program and any parameters it wishes to receive
//   void ckb_parameter(ckb_runctx* context, const char* name, const char* value) :
//     Receives the value of a parameter specified from ckb_info
//   void ckb_keypress(ckb_runctx* context, ckb_key* key, int state) :
//     Receives a key down / key up event. state = 1 for down, 0 for up.
//   void ckb_start(ckb_runctx* context) :
//     Starts or restarts an animation not based on keypress.
//   int ckb_frame(ckb_runctx* context, double delta) :
//     Advances the animation frame, where a delta of 1.0 represents the entire animation duration. Delta may be zero.
//     Should return 0 to continue running or any other number to exit.

// ckb_info will only be called once and then the program will exit. All other functions may be called multiple times.
// ckb_parameter will only be called at the beginning of the program run; ckb_keypress and ckb_start may be called at any time.
// ckb_frame will be called at least once with a delta of 0 at program start.

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CKB_CONTAINER(macro) do { macro } while(0)

// * Info output helpers

// Plugin GUID
#define CKB_GUID(guid)                  CKB_CONTAINER( printf("guid "); printurl(guid); printf("\n"); )
// Plugin name
#define CKB_NAME(name)                  CKB_CONTAINER( printf("name "); printurl(name); printf("\n"); )
// Plugin version
#define CKB_VERSION(version)            CKB_CONTAINER( printf("version "); printurl(version); printf("\n"); )
// Plugin copyright
#define CKB_COPYRIGHT(year, author)     CKB_CONTAINER( printf("author "); printurl(author); printf("\nyear %s\n", year); )
// Plugin license
#define CKB_LICENSE(license)            CKB_CONTAINER( printf("license "); printurl(license); printf("\n"); )
// Plugin description
#define CKB_DESCRIPTION(description)    CKB_CONTAINER( printf("description "); printurl(description); printf("\n"); )

// Parameter helpers
#define CKB_PARAM(type, name, prefix, postfix, extra)               CKB_CONTAINER( printf("param %s %s ", type, name); printurl(prefix); printf(" "); printurl(postfix); extra; printf("\n"); )
#define CKB_PARAM_LONG(name, prefix, postfix, default, min, max)    CKB_PARAM("long", name, prefix, postfix, printf(" %ld %ld %ld", (long)(default), (long)(min), (long)(max)))
#define CKB_PARAM_DOUBLE(name, prefix, postfix, default, min, max)  CKB_PARAM("double", name, prefix, postfix, printf(" %lf %lf %lf", (double)(default), (double)(min), (double)(max)))
#define CKB_PARAM_BOOL(name, prefix, postfix, default)              CKB_PARAM("bool", name, prefix, postfix, printf((default) ? " 1" : " 0"))
#define CKB_PARAM_RGB(name, prefix, postfix, r, g, b)               CKB_PARAM("rgb", name, prefix, postfix, printf(" %02x%02x%02x", (unsigned char)(r), (unsigned char)(g), (unsigned char)(b)))
#define CKB_PARAM_ARGB(name, prefix, postfix, a, r, g, b)           CKB_PARAM("argb", name, prefix, postfix, printf(" %02x%02x%02x%02x", (unsigned char)(a), (unsigned char)(r), (unsigned char)(g), (unsigned char)(b)))

// Special parameters (most values are ignored)
#define CKB_PARAM_DURATION(default)                                 CKB_PARAM_DOUBLE("duration", "", "", default, 0., 0.)
#define CKB_PARAM_TRIGGER(default)                                  CKB_PARAM_BOOL("trigger", "", "", default)
#define CKB_PARAM_TRIGGER_KP(default)                               CKB_PARAM_BOOL("kptrigger", "", "", default)
#define CKB_PARAM_REPEAT(default)                                   CKB_PARAM_DOUBLE("repeat", "", "", default, 0., 0.)
#define CKB_PARAM_REPEAT_KP(default)                                CKB_PARAM_DOUBLE("kprepeat", "", "", default, 0., 0.)

// * Runtime information

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

// Clear all keys in a context (ARGB 00000000)
#define CKB_KEYCLEAR(context)       CKB_CONTAINER( ckb_key* key = context->keys; unsigned count = context->keycount; unsigned i = 0; for(; i < count; i++) key[i].a = key[i].r = key[i].g = key[i].b = 0; )

// Parameter input parsers. Usage (within ckb_parameter only):
//  CKB_PARSE_BOOL("mybool", &mybool){
//      <actions to take when bool is parsed>
//  }
// or:
//  CKB_PARSE_BOOL("mybool", &mybool){}

#define CKB_PARSE_LONG(param_name, value_ptr)                   if(!strcmp(name, param_name) && sscanf(value, "%ld", value_ptr) == 1)
#define CKB_PARSE_DOUBLE(param_name, value_ptr)                 if(!strcmp(name, param_name) && sscanf(value, "%lf", value_ptr) == 1)
#define CKB_PARSE_BOOL(param_name, value_ptr)                   if(!strcmp(name, param_name) && sscanf(value, "%u", value_ptr) == 1)
#define CKB_PARSE_RGB(param_name, r_ptr, g_ptr, b_ptr)          if(!strcmp(name, param_name) && sscanf(value, "%2hhx%2hhx%2hhx", r_ptr, g_ptr, b_ptr) == 3)
#define CKB_PARSE_ARGB(param_name, a_ptr, r_ptr, g_ptr, b_ptr)  if(!strcmp(name, param_name) && sscanf(value, "%2hhx%2hhx%2hhx%2hhx", a_ptr, r_ptr, g_ptr, b_ptr) == 4)


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

extern void ckb_info();
extern void ckb_parameter(ckb_runctx*, const char*, const char*);
extern void ckb_keypress(ckb_runctx*, ckb_key* key, int);
extern void ckb_start(ckb_runctx*);
extern int ckb_frame(ckb_runctx*, double);
int main(int argc, char *argv[]){
    printf("hi\n");
    fflush(stdout);
    if(argc == 2){
        if(!strcmp(argv[1], "--ckb-info")){
            ckb_info();
            return 0;
        } else if(!strcmp(argv[1], "--ckb-run")){
            ckb_runctx ctx;
            // Read the keymap lines
            char cmd[CKB_MAX_WORD], param[CKB_MAX_WORD], value[CKB_MAX_WORD];
            // Skip anything up until "begin keymap"
            printf("keymap\n");
            fflush(stdout);
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
            printf("params\n");
            fflush(stdout);
            // Skip anything else until "begin params"
            do {
                ckb_getline(cmd, param, value);
                if(!*cmd){
                    printf("Error [ckb-main]: Reached EOF looking for \"begin params\"");
                    return -2;
                }
            } while(strcmp(cmd, "begin") || strcmp(param, "params"));
            // Parse parameters
            do {
                ckb_getline(cmd, param, value);
                if(!*cmd){
                    printf("Error [ckb-main]: Reached EOF reading parameters");
                    return -2;
                }
                if(!strcmp(cmd, "end") && !strcmp(param, "params"))
                    break;
                if(strcmp(cmd, "param"))
                    continue;
                ckb_parameter(&ctx, param, value);
            } while(1);
            printf("run\n");
            fflush(stdout);
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
                    ckb_start(&ctx);
                else if(!strcmp(cmd, "key")){
                    // Find a key with this name
                    ckb_key* key = 0;
                    for(i = 0; i < keycount; i++){
                        if(!strcmp(ctx.keys[i].name, param)){
                            key = ctx.keys + i;
                            break;
                        }
                    }
                    if(key)
                        ckb_keypress(&ctx, key, !strcmp(value, "down"));
                } else {
                    double delta = 0.;
                    if(!strcmp(cmd, "frame") && sscanf(param, "%lf", &delta) == 1){
                        int end = ckb_frame(&ctx, delta);
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
                    }
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

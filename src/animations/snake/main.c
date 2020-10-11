#include <ckb-next/animation.h>
#include <math.h>
#include <time.h>
#include <string.h>

int compare_key_y(const void* a, const void* b){
    ckb_key* key_a = *(ckb_key**)a;
    ckb_key* key_b = *(ckb_key**)b;

    if (key_a->y > key_b->y)
        return +1;
    if (key_a->y < key_b->y)
        return -1;

    if (key_a->x > key_b->x)
        return ((key_a->y / 12) % 2 ? -1 : +1);
    if (key_a->x < key_b->x)
        return ((key_a->y / 12) % 2 ? +1 : -1);

    return 0;
}

void ckb_info(){
    // Plugin info
    CKB_NAME("Snake");
    CKB_VERSION("0.9");
    CKB_COPYRIGHT("2019", "Tasos Sahanidis");
    CKB_LICENSE("GPLv2+");
    CKB_GUID("{40AFB1B0-92E4-4E0F-B368-E1E638F84247}");
    CKB_DESCRIPTION("A snake-like effect.");

    // Effect parameters
    CKB_PARAM_ARGB("firstcol", "1st colour:", "", 0xFF, 164, 200, 10);
    CKB_PARAM_ARGB("secondcol", "2nd colour:", "", 0xFF, 0x00, 0x00, 0xFF);

    // Timing/input parameters
    CKB_KPMODE(CKB_KP_POSITION);
    CKB_TIMEMODE(CKB_TIME_DURATION);
    CKB_LIVEPARAMS(TRUE);

    // Presets
    CKB_PRESET_START("Snake");
    CKB_PRESET_PARAM("duration", "40.0");
    CKB_PRESET_END;
}

ckb_gradient firstcolour = { 0 };
ckb_gradient secondcolour = { 0 };
unsigned char s_a, s_r, s_g, s_b, e_a, e_r, e_g, e_b;
ckb_key** snake_keys = 0;

// TODO: add snake length option

double timer = 0;
double timer_tick = 0.0;
void ckb_init(ckb_runctx* context){
    // Set  the timer to 1.1 (> max timer_tick) initially so that the first frame gets sent immediately
    timer = 1.1;

    // Calculate the duration of each key light
    timer_tick = 1.00 / (context->keycount * 2);

    // Allocate the array and copy the pointers to it. * 2 for the reverse sequence
    snake_keys = malloc(sizeof(ckb_key*) * context->keycount * 2);
    for(unsigned i = 0; i < context->keycount; i++)
        snake_keys[i] = context->keys + i;

    // Sort the first part of the array accordingly
    qsort(snake_keys, context->keycount, sizeof(ckb_key*), compare_key_y);

    // Mirror the first part to the second half
    for(unsigned i = 0; i < context->keycount; i++)
        snake_keys[context->keycount + i] = snake_keys[context->keycount - i - 1];
}

void ckb_parameter(ckb_runctx* context, const char* name, const char* value){
    CKB_PARSE_ARGB("firstcol", &s_a, &s_r, &s_g, &s_b){}
    CKB_PARSE_ARGB("secondcol", &e_a, &e_r, &e_g, &e_b){}
}

void ckb_keypress(ckb_runctx* context, ckb_key* key, int x, int y, int state){
}

void ckb_start(ckb_runctx* context, int state){
}

void ckb_time(ckb_runctx* context, double delta){
    timer += delta;
}

unsigned int pos = 0;
int ckb_frame(ckb_runctx* context){
    // Don't calculate a new frame unless we need to udpate it
    if(timer < timer_tick)
        return 0;
    timer = 0.0;

    CKB_KEYCLEAR(context);

    unsigned count = context->keycount;

    // Pick the appropriate colour
    if(pos > count - 1){
        snake_keys[pos]->a = e_a;
        snake_keys[pos]->r = e_r;
        snake_keys[pos]->g = e_g;
        snake_keys[pos]->b = e_b;
    } else {
        snake_keys[pos]->a = s_a;
        snake_keys[pos]->r = s_r;
        snake_keys[pos]->g = s_g;
        snake_keys[pos]->b = s_b;
    }

    // Increment the key position
    pos++;
    if(pos > count * 2 - 1)
        pos = 0;

    return 0;
}

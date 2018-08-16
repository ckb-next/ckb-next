#include <ckb-next/animation.h>
#include <math.h>
#include <time.h>
#include <string.h>

void ckb_info(){
    // Plugin info
    CKB_NAME("Invaders");
    CKB_VERSION("0.2");
    CKB_COPYRIGHT("2017-8", "Maksim Vladimirovich <edush.maxim@gmail.com>\nTasos Sahanidis <code@tasossah.com>");
    CKB_LICENSE("GPLv3");
    CKB_GUID("{54DD2975-E192-457D-BCFC-D912A24E33B9}");
    CKB_DESCRIPTION("Space invaders - like game.\n\nFire projectiles using the higlighted keys on the left column.\nF1-F12 indicate the level.\nThe \"prtscn\", \"scroll\", and \"pause\" columns indicate continues.\nEsc resets the game.");

    // Timing/input parameters
    CKB_KPMODE(CKB_KP_NAME);
    CKB_TIMEMODE(CKB_TIME_DURATION);
    CKB_LIVEPARAMS(TRUE);

    CKB_PARAM_LONG("continue_count", "Continues: ", "", 3, 0, 14);

    // Presets
    CKB_PRESET_START("Default");
    CKB_PRESET_PARAM("duration", "1.0");
    CKB_PRESET_PARAM("trigger", "0");
    CKB_PRESET_PARAM("kptrigger", "1");
    CKB_PRESET_END;
}

int bullet_row = 1;
float bullet_position = 0.f;
int enemy_row = 1;
float enemy_position = 12.f;
int fire = 0;
int enemies = 0;
int level = 0;
int end_game = 0;
int continues = -1;
long int enemy_pos_int = 12;
long int continue_count = 0;

void restart() {
    enemy_row = 1;
    bullet_row = 1;
    fire = 0;
    enemy_position = 12.f;
    bullet_position = 0.f;
    end_game = 0;
    continues = continue_count;
    level = 0;
}

char* row1[14] = { "grave",  "1", "2", "3", "4", "5", "6", "7", "8",     "9",   "0",     "minus",  "equal",  "back" };
char* row2[14] = { "tab",    "q", "w", "e", "r", "t", "y", "u", "i",     "o",   "p",     "lbrace", "rbrace", "bslash" };
char* row3[13] = { "caps",   "a", "s", "d", "f", "g", "h", "j", "k",     "l",   "colon", "quote",  "enter" };
char* row4[12] = { "lshift", "z", "x", "c", "v", "b", "n", "m", "comma", "dot", "slash", "rshift" };

char* continuesrow[14] = { "prtscn", "scroll", "pause", "ins", "home", "pgup", "del", "del", "end", "pgdn", "up", "left", "down", "right" };

char* levels[12] = {"f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "f10", "f11", "f12" }; // FIXME: Actually use this

void ckb_init(ckb_runctx* context){
    srand(time(NULL));
}

void ckb_parameter(ckb_runctx* context, const char* name, const char* value){
    CKB_PARSE_LONG("continue_count", &continue_count){}
    continues = continue_count;
}

void ckb_keypress(ckb_runctx* context, ckb_key* key, int x, int y, int state){
    // Start or stop animation on key
    char* keyname = key->name;

    if(!strcmp(keyname, "esc"))
        restart();

    if(!fire) {
        if (!strcmp(keyname, row1[0])) {
            fire = 1;
            bullet_row = 1;
        }
        if (!strcmp(keyname, row2[0])) {
            fire = 1;
            bullet_row = 2;
        }
        if (!strcmp(keyname, row3[0])) {
            fire = 1;
            bullet_row = 3;
        }
        if (!strcmp(keyname, row4[0])) {
            fire = 1;
            bullet_row = 4;
        }
    }
}

void ckb_start(ckb_runctx* context, int state) {
}

char* get_key_name(long int ep, int row) {
    switch(row) {
        case 1:
            return row1[ep];
        case 2:
            return row2[ep];
        case 3:
            return row3[ep];
        case 4:
            return row4[ep];
        default:
            return "esc";
    }
}

void explode_enemy(){
    enemy_row = rand() % 4 + 1;
    enemy_position = 12.f;
    enemy_pos_int = 12;
    bullet_position = 0.f;
    fire = 0;
}

void draw_key_pink(ckb_key *key) {
    key->r = 255;
    key->g = 0;
    key->b = 255;
}

// FIXME: Get key pointers once and store them in an array instead of looping every time

int draw_continues(ckb_key *key) {
    int ret = 1;
    char *key_name = key->name;

    if (!strcmp(key_name, continuesrow[0]) && continues > 0)
        draw_key_pink(key);
    else if (!strcmp(key_name, continuesrow[1]) && continues > 1)
        draw_key_pink(key);
    else if (!strcmp(key_name, continuesrow[2]) && continues > 2)
        draw_key_pink(key);
    else if (!strcmp(key_name, continuesrow[3]) && continues > 3)
        draw_key_pink(key);
    else if (!strcmp(key_name, continuesrow[4]) && continues > 4)
        draw_key_pink(key);
    else if (!strcmp(key_name, continuesrow[5]) && continues > 5)
        draw_key_pink(key);
    else if (!strcmp(key_name, continuesrow[6]) && continues > 6)
        draw_key_pink(key);
    else if (!strcmp(key_name, continuesrow[7]) && continues > 7)
        draw_key_pink(key);
    else if (!strcmp(key_name, continuesrow[8]) && continues > 8)
        draw_key_pink(key);
    else if (!strcmp(key_name, continuesrow[9]) && continues > 9)
        draw_key_pink(key);
    else if (!strcmp(key_name, continuesrow[10]) && continues > 10)
        draw_key_pink(key);
    else if (!strcmp(key_name, continuesrow[11]) && continues > 11)
        draw_key_pink(key);
    else if (!strcmp(key_name, continuesrow[12]) && continues > 12)
        draw_key_pink(key);
    else if (!strcmp(key_name, continuesrow[13]) && continues > 13)
        draw_key_pink(key);
    else
        ret = 0;

    return ret;
}

void draw_key_blue(ckb_key *key, unsigned char alpha) {
    key->a = 255;
    key->r = 0;
    key->g = 0;
    key->b = alpha;
}

int draw_level(ckb_key *key, int lvl, unsigned char alpha) {
    int ret = 1;
    char *key_name = key->name;

    if (!strcmp(key_name, "f1") && lvl > 0)
        draw_key_blue(key, alpha);
    else if (!strcmp(key_name, "f2") && lvl > 1)
        draw_key_blue(key, alpha);
    else if (!strcmp(key_name, "f3") && lvl > 2)
        draw_key_blue(key, alpha);
     else if (!strcmp(key_name, "f4") && lvl > 3)
        draw_key_blue(key, alpha);
     else if (!strcmp(key_name, "f5") && lvl > 4)
        draw_key_blue(key, alpha);
     else if (!strcmp(key_name, "f6") && lvl > 5)
        draw_key_blue(key, alpha);
     else if (!strcmp(key_name, "f7") && lvl > 6)
        draw_key_blue(key, alpha);
     else if (!strcmp(key_name, "f8") && lvl > 7)
        draw_key_blue(key, alpha);
     else if (!strcmp(key_name, "f9") && lvl > 8)
        draw_key_blue(key, alpha);
     else if (!strcmp(key_name, "f10") && lvl > 9)
        draw_key_blue(key, alpha);
     else if (!strcmp(key_name, "f11") && lvl > 10)
        draw_key_blue(key, alpha);
     else if (!strcmp(key_name, "f12") && lvl > 11)
        draw_key_blue(key, alpha);
     else 
        ret = 0;

    return ret;
}

unsigned char gameover_ticks = 0;
unsigned char gameover_alpha = 255;
void game_over(ckb_runctx* ctx){
    DBG("GAME OVER! Ticks %u", gameover_ticks);
    // Do something every time gameover_ticks wraps around
    if(!gameover_ticks) {
        for(unsigned i = 0; i < ctx->keycount; i++) {
            ckb_key* key = ctx->keys + i;
            //char *key_name = key->name;
            draw_level(key, 12, gameover_alpha);
        }
        gameover_alpha = ~gameover_alpha;
    }
    gameover_ticks += 8;
}

void ckb_time(ckb_runctx* context, double delta){

    if (end_game) {
        game_over(context);
        return;
    }
    DBG("intpos before %ld", enemy_pos_int);
    if (!enemy_pos_int) {
        continues--;
        DBG("Lost a life! %d continues left.", continues);
        explode_enemy();
        if (continues < 0) {
            end_game = 1;
            return;
        }
    }

    if (fire)
        bullet_position += (0.4f + level * 0.02f);

    if (bullet_position > 12.f) {
        fire = 0;
        bullet_position = 0.f;
    }

    enemy_position -= (0.08f + level * 0.02f);

    enemy_pos_int = lroundf(enemy_position);
    long int bullet_pos_int = lroundf(bullet_position);
    DBG("intpos after %ld", enemy_pos_int);

    if (enemy_pos_int && enemy_pos_int == bullet_pos_int && enemy_row == bullet_row) {
        explode_enemy();
        enemies++;
        if(enemies > 8) {
            level++;
            enemies = 0;
        }
    }

    unsigned count = context->keycount;
    for(unsigned i = 0; i < count; i++){
        ckb_key* key = context->keys + i;
        char* key_name = key->name;
        char* enemy_key_name = get_key_name(enemy_pos_int, enemy_row);
        char* bullet_key_name = get_key_name(bullet_pos_int, bullet_row);
        key->a = 255;
        if (!strcmp(key_name, row1[0])) {
            key->r = 255;
            key->g = 255;
            key->b = 255;
        } else if (!strcmp(key_name, row2[0])) {
            key->r = 255;
            key->g = 255;
            key->b = 255;
        } else if (!strcmp(key_name, row3[0])) {
            key->r = 255;
            key->g = 255;
            key->b = 255;
        } else if (!strcmp(key_name, row4[0])) {
            key->r = 255;
            key->g = 255;
            key->b = 255;
        } else if (!strcmp(key_name, "esc")) {
            key->r = 255;
            key->g = 0;
            key->b = 0;
        } else if (!strcmp(key_name, bullet_key_name) && fire) {
            key->r = 0;
            key->g = 255;
            key->b = 0;
        } else if (!strcmp(key_name, enemy_key_name)) {
            key->r = 255;
            key->g = 0;
            key->b = 0;
        } else if (draw_level(key, level, 255)){
            continue;
        } else if (draw_continues(key)){
            continue;
        } else {
            key->r = 0;
            key->g = 0;
            key->b = 0;
        }
    }
}

int ckb_frame(ckb_runctx* context){
    return 0;
}

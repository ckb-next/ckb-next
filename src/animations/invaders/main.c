#include <ckb-next/animation.h>
#include <math.h>
#include <time.h>
#include <string.h>

void ckb_info(){
    // Plugin info
    CKB_NAME("Invaders");
    CKB_VERSION("0.1");
    CKB_COPYRIGHT("2017", "Maksim Vladimirovich <edush.maxim@gmail.com>");
    CKB_LICENSE("GPLv3");
    CKB_GUID("{54DD2975-E192-457D-BCFC-D912A24E33B9}");
    CKB_DESCRIPTION("Space invaders - like game");

    // Effect parameters
    CKB_PARAM_AGRADIENT("color", "Color:", "", "ffffffff");
    CKB_PARAM_BOOL("kphold", "Freeze until key is released", TRUE);

    // Timing/input parameters
    CKB_KPMODE(CKB_KP_NAME);
    CKB_TIMEMODE(CKB_TIME_DURATION);
    CKB_LIVEPARAMS(TRUE);

    // Presets
    CKB_PRESET_START("Default Settings");
    CKB_PRESET_PARAM("color", "ff000000");
    CKB_PRESET_PARAM("duration", "0.1");
    CKB_PRESET_PARAM("stop", "0");
    CKB_PRESET_PARAM("kpstop", "0");
    CKB_PRESET_END;
}

int kphold = 0, kprelease = 0;

int bullet_row = 1;
float bullet_position = 0;
int enemy_row = 1;
float enemy_position = 12;
int fire = 0;
int enemies = 0;
int level = 0;

void restart() {
    enemy_row = 1;
    bullet_row = 1;
    fire = 0;
    enemy_position = 12;
    bullet_position = 0;
}

char* row1[14] = { "grave",  "1", "2", "3", "4", "5", "6", "7", "8",     "9",   "0",     "minus",  "equal",  "back" };
char* row2[14] = { "tab",    "q", "w", "e", "r", "t", "y", "u", "i",     "o",   "p",     "lbrace", "rbrace", "bslash" };
char* row3[13] = { "caps",   "a", "s", "d", "f", "g", "h", "j", "k",     "l",   "colon", "quote",  "enter" };
char* row4[12] = { "lshift", "z", "x", "c", "v", "b", "n", "m", "comma", "dot", "slash", "rshift" };

char* levels[12] = {"f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "f10", "f11", "f12" };

void ckb_init(ckb_runctx* context){
    srand(time(NULL));
}

void ckb_parameter(ckb_runctx* context, const char* name, const char* value){
    CKB_PARSE_BOOL("kphold", &kphold){}
    CKB_PARSE_BOOL("kprelease", &kprelease){}
}

void ckb_keypress(ckb_runctx* context, ckb_key* key, int x, int y, int state){
    // Start or stop animation on key
    char* keyname = key->name;

    if(strcmp(keyname, "esc") == 0) {
        restart();
    }

    if(fire == 0) {
        if (strcmp(keyname, row1[0]) == 0) {
            fire = 1;
            bullet_row = 1;
        }
        if (strcmp(keyname, row2[0]) == 0) {
            fire = 1;
            bullet_row = 2;
        }
        if (strcmp(keyname, row3[0]) == 0) {
            fire = 1;
            bullet_row = 3;
        }
        if (strcmp(keyname, row4[0]) == 0) {
            fire = 1;
            bullet_row = 4;
        }
    }
}

void ckb_start(ckb_runctx* context, int state) {
}

char* get_bullet_key_name() {
    int bp = round(bullet_position);
    if(bullet_row == 1) {
        return row1[bp];
    } else if (bullet_row == 2) {
        return row2[bp];
    } else if (bullet_row == 3) {
        return row3[bp];
    } else if (bullet_row == 4) {
        return row4[bp];
    }
    return "esc";
}

char* get_enemy_key_name() {
    int ep = round(enemy_position);
    DBG("Pos %i", ep);
    if(enemy_row == 1) {
        return row1[ep];
    } else if (enemy_row == 2) {
        return row2[ep];
    } else if (enemy_row == 3) {
        return row3[ep];
    } else if (enemy_row == 4) {
        return row4[ep];
    }
    return "esc";
}

void explode_enemy(){
    enemy_row = rand() % 4 + 1;
    enemy_position = 12;
    bullet_position = 0;
    fire = 0;
}

void game_over(){
    DBG("GAME OVER!");
}

int draw_level(ckb_key *key, int level) {
    int ret = 1;
    char *key_name = key->name;

    if (strcmp(key_name, "f1") == 0 && level > 0) {
        key->a = 255;
        key->r = 0;
        key->g = 0;
        key->b = 255;
    } else if (strcmp(key_name, "f2") == 0 && level > 1) {
        key->a = 255;
        key->r = 0;
        key->g = 0;
        key->b = 255;
    } else if (strcmp(key_name, "f3") == 0 && level > 2) {
        key->a = 255;
        key->r = 0;
        key->g = 0;
        key->b = 255;
    } else if (strcmp(key_name, "f4") == 0 && level > 3) {
        key->a = 255;
        key->r = 0;
        key->g = 0;
        key->b = 255;
    } else if (strcmp(key_name, "f5") == 0 && level > 4) {
        key->a = 255;
        key->r = 0;
        key->g = 0;
        key->b = 255;
    } else if (strcmp(key_name, "f6") == 0 && level > 5) {
        key->a = 255;
        key->r = 0;
        key->g = 0;
        key->b = 255;
    } else if (strcmp(key_name, "f7") == 0 && level > 6) {
        key->a = 255;
        key->r = 0;
        key->g = 0;
        key->b = 255;
    } else if (strcmp(key_name, "f8") == 0 && level > 7) {
        key->a = 255;
        key->r = 0;
        key->g = 0;
        key->b = 255;
    } else if (strcmp(key_name, "f9") == 0 && level > 8) {
        key->a = 255;
        key->r = 0;
        key->g = 0;
        key->b = 255;
    } else if (strcmp(key_name, "f10") == 0 && level > 9) {
        key->a = 255;
        key->r = 0;
        key->g = 0;
        key->b = 255;
    } else if (strcmp(key_name, "f11") == 0 && level > 10) {
        key->a = 255;
        key->r = 0;
        key->g = 0;
        key->b = 255;
    } else if (strcmp(key_name, "f12") == 0 && level > 11) {
        key->a = 255;
        key->r = 0;
        key->g = 0;
        key->b = 255;
    } else 
        ret = 0;

    return ret;
}

void ckb_time(ckb_runctx* context, double delta){
    if(fire == 1) {
        bullet_position += (0.4 + level * 0.02);
    }
    if(bullet_position > 12) {
        fire = 0;
        bullet_position = 0;
    }
    enemy_position -= (0.08 + level * 0.02);
    if(enemy_position < 0)
        enemy_position = -1;

    if(enemy_position == -1){
        game_over();
        return;
    } else if (round(enemy_position) == round(bullet_position) && bullet_row == enemy_row) {
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
        char* bullet_key_name = get_bullet_key_name();
        char* enemy_key_name = get_enemy_key_name();
        key->a = 255;
        if (strcmp(key_name, row1[0]) == 0) {
            key->r = 255;
            key->g = 255;
            key->b = 255;
        } else if (strcmp(key_name, row2[0]) == 0) {
            key->r = 255;
            key->g = 255;
            key->b = 255;
        } else if (strcmp(key_name, row3[0]) == 0) {
            key->r = 255;
            key->g = 255;
            key->b = 255;
        } else if (strcmp(key_name, row4[0]) == 0) {
            key->r = 255;
            key->g = 255;
            key->b = 255;
        } else if (strcmp(key_name, "scroll") == 0) {
            key->r = 255;
            key->g = 0;
            key->b = 255;
        } else if (strcmp(key_name, bullet_key_name) == 0 && fire == 1) {
            key->r = 0;
            key->g = 255;
            key->b = 0;
        } else if (strcmp(key_name, enemy_key_name) == 0) {
            key->r = 255;
            key->g = 0;
            key->b = 0;
        } else if (draw_level(key, level)){
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

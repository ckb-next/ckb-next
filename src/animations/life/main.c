#include <ckb-next/animation.h>
#include <stdio.h>

void ckb_info() {
    // Plugin info
    CKB_NAME("Life");
    CKB_VERSION("0.1");
    CKB_COPYRIGHT("2019", "blob_");
    CKB_LICENSE("GPLv2");
    CKB_GUID("{512CD200-5DB7-4E18-BB3B-9281811E482B}");
    CKB_DESCRIPTION("Conway's Game of Life on your keyboard.");

    // Effect parameters
    CKB_PARAM_AGRADIENT("color", "Fade color:", "", "ffffffff");
	CKB_PARAM_LONG("delay", "Frames to next generation", "frames", 120, 10, 1000);

    // Timing/input parameters
    CKB_KPMODE(CKB_KP_NAME);
    CKB_TIMEMODE(CKB_TIME_DURATION);
    CKB_REPEAT(FALSE);
    CKB_LIVEPARAMS(TRUE);
    
    // Presets
    CKB_PRESET_START("Default");
    CKB_PRESET_PARAM("delay", "120");
    CKB_PRESET_PARAM("trigger", "0");
    CKB_PRESET_PARAM("kptrigger", "1");
    CKB_PRESET_END;
}

ckb_gradient animcolor = { 0 };
long tng = 120;
long delay = 120;
int keystate[108] = { [0 ... 107] = 1 };
int keypressed[108] = { 0 };
//translate keycontext to adjacencygraph
int keymap[108];
int mapkey[108];


typedef struct {
    char* name;
    char* neighbors[10];
} adjacencynode;

//kissing connectivity
adjacencynode adjacencygraph[108] = {
    {"esc", {"f1", "grave", "1", "", "", "", "", "", "", ""}}, {"f1", {"esc", "f2", "grave", "1", "2", "", "", "", "", ""}}, {"f2", {"f1", "f3", "2", "3", "", "", "", "", "", ""}}, {"f3", {"f2", "f4", "3", "4", "", "", "", "", "", ""}}, {"f4", {"f3", "f5", "4", "5", "6", "", "", "", "", ""}}, {"f5", {"f4", "f6", "5", "6", "7", "", "", "", "", ""}}, {"f6", {"f5", "f7", "7", "8", "", "", "", "", "", ""}}, {"f7", {"f6", "f8", "8", "9", "", "", "", "", "", ""}}, {"f8", {"f7", "f9", "9", "0", "minus", "", "", "", "", ""}}, {"f9", {"f8", "f10", "0", "minus", "equal", "", "", "", "", ""}}, {"f10", {"f9", "f11", "minus", "equal", "bspace", "", "", "", "", ""}}, {"f11", {"f10", "f12", "equal", "bspace", "", "", "", "", "", ""}}, {"f12", {"f11", "prtscn", "bspace", "", "", "", "", "", "", ""}}, {"prtscn", {"f12", "scroll", "bspace", "ins", "home", "", "", "", "", ""}}, {"scroll", {"prtscn", "pause", "ins", "home", "pgup", "", "", "", "", ""}}, {"pause", {"scroll", "stop", "home", "pgup", "numlock", "", "", "", "", ""}}, {"stop", {"pause", "prev", "pgup", "numlock", "numslash", "", "", "", "", ""}}, {"prev", {"stop", "play", "numlock", "numslash", "numstar", "", "", "", "", ""}}, {"play", {"prev", "next", "numslash", "numstar", "numminus", "", "", "", "", ""}}, {"next", {"play", "numstar", "numminus", "", "", "", "", "", "", ""}},
    
    {"grave", {"esc", "f1", "1", "tab", "", "", "", "", "", ""}}, {"1", {"esc", "f1", "grave", "2", "tab", "q", "", "", "", ""}}, {"2", {"f1", "f2", "1", "3", "q", "w", "", "", "", ""}}, {"3", {"f2", "f3", "2", "4", "w", "e", "", "", "", ""}}, {"4", {"f3", "f4", "3", "5", "e", "r", "", "", "", ""}}, {"5", {"f4", "f5", "4", "6", "r", "t", "", "", "", ""}}, {"6", {"f4", "f5", "5", "7", "t", "y", "", "", "", ""}}, {"7", {"f5", "f6", "6", "8", "y", "u", "", "", "", ""}}, {"8", {"f6", "f7", "7", "9", "u", "i", "", "", "", ""}}, {"9", {"f7", "f8", "8", "0", "i", "o", "", "", "", ""}}, {"0", {"f8", "f9", "9", "minus", "o", "p", "", "", "", ""}}, {"minus", {"f8", "f9", "f10", "0", "equal", "p", "lbrace", "", "", ""}}, {"equal", {"f9", "f10", "f11", "minus", "bspace", "lbrace", "rbrace", "", "", ""}}, {"bspace", {"f10", "f11", "f12", "prtscn", "equal", "ins", "rbrace", "bslash", "del", ""}}, {"ins", {"f12", "prtscn", "scroll", "bspace", "home", "bslash", "del", "end", "", ""}}, {"home", {"prtsscn", "scroll", "pause", "ins", "pgup", "del", "end", "pgdn", "", ""}}, {"pgup", {"scroll", "pause", "stop", "home", "pgup", "numlock", "end", "pgdn", "num7", ""}}, {"numlock", {"pause", "stop", "prev", "pgup", "numslash", "pgdn", "num7", "num8", "", ""}}, {"numslash", {"stop", "prev", "play", "numlock", "numstar", "num7", "num8", "num9", "", ""}}, {"numstar", {"prev", "play", "next", "numslash", "numminus", "num8", "num9", "numplus", "", ""}}, {"numminus", {"play", "next", "num9", "numplus", "", "", "", "", "", ""}},

    {"tab", {"grave", "1", "q", "caps", "", "", "", "", "", ""}}, {"q", {"tab", "1", "2", "w", "a", "caps", "", "", "", ""}}, {"w", {"q", "2", "3", "e", "s", "a", "", "", "", ""}}, {"e", {"w", "3", "4", "r", "d", "s", "", "", "", ""}}, {"r", {"e", "4", "5", "t", "f", "d", "", "", "", ""}}, {"t", {"r", "5", "6", "y", "g", "f", "", "", "", ""}}, {"y", {"t", "6", "7", "u", "h", "g", "", "", "", ""}}, {"u", {"y", "7", "8", "i", "j", "h", "", "", "", ""}}, {"i", {"u", "8", "9", "o", "k", "j", "", "", "", ""}}, {"o", {"i", "9", "0", "p", "l", "k", "", "", "", ""}}, {"p", {"o", "0", "minus", "lbrace", "colon", "l", "", "", "", ""}}, {"lbrace", {"p", "minus", "equal", "rbrace", "quote", "colon", "", "", "", ""}}, {"rbrace", {"lbrace", "equal", "bspace", "bslash", "enter", "quote", "", "", "", ""}}, {"bslash", {"rbrace", "bspace", "ins", "del", "enter", "", "", "", "", ""}}, {"del", {"bslash", "bspace", "ins", "home", "end", "enter", "", "", "", ""}}, {"end", {"del", "ins", "home", "pgup", "pgdn", "", "", "", "", ""}}, {"pgdn", {"end", "home", "pgup", "numlock", "num7", "num4", "", "", "", ""}}, {"num7", {"pgdn", "pgup", "numlock", "numslash", "num8", "num5", "num4", "", "", ""}}, {"num8", {"numlock", "numslash", "numstar", "num9", "num6", "num5", "num4", "", "", ""}}, {"num9", {"num8", "numslash", "numstar", "numminus", "numplus", "num6", "num5", "", "", ""}}, {"numplus", {"num9", "numstar", "numminus", "numenter", "num3", "num6", "", "", "", ""}},

    {"caps", {"tab", "q", "a", "lshift", "", "", "", "", "", ""}}, {"a", {"caps", "q", "w", "s", "z", "lshift", "", "", "", ""}}, {"s", {"a", "w", "e", "d", "x", "z", "", "", "", ""}}, {"d", {"s", "e", "r", "f", "c", "x", "", "", "", ""}}, {"f", {"d", "r", "t", "g", "v", "c", "", "", "", ""}}, {"g", {"f", "t", "y", "h", "b", "v", "", "", "", ""}}, {"h", {"g", "y", "u", "j", "n", "b", "", "", "", ""}}, {"j", {"h", "u", "i", "k", "m", "n", "", "", "", ""}}, {"k", {"j", "i", "o", "l", "comma", "m", "", "", "", ""}}, {"l", {"k", "o", "p", "colon", "dot", "comma", "", "", "", ""}}, {"colon", {"l", "p", "lbrace", "quote", "slash", "dot", "", "", "", ""}}, {"quote", {"colon", "lbrace", "rbrace", "enter", "rshift", "slash", "", "", "", ""}}, {"enter", {"quote", "rbrace", "bslash", "del", "rshift", "", "", "", "", ""}}, {"num4", {"pgdn", "num7", "num8", "num5", "num2", "num1", "", "", "", ""}}, {"num5", {"num4", "num7", "num8", "num9", "num6", "num3", "num2", "num1", "", ""}}, {"num6", {"num5", "num8", "num9", "numplus", "numenter", "num3", "num2", "", "", ""}},

    {"lshift", {"caps", "a", "z", "lctrl", "lwin", "", "", "", "", ""}}, {"z", {"lshift", "a", "s", "x", "lalt", "lwin", "", "", "", ""}}, {"x", {"z", "s", "d", "c", "space", "lalt", "", "", "", ""}}, {"c", {"x", "d", "f", "v", "space", "", "", "", "", ""}}, {"v", {"c", "f", "g", "b", "space", "", "", "", "", ""}}, {"b", {"v", "g", "h", "n", "space", "", "", "", "", ""}}, {"n", {"b", "h", "j", "m", "space", "", "", "", "", ""}}, {"m", {"n", "j", "k", "comma", "space", "", "", "", "", ""}}, {"comma", {"m", "k", "l", "dot", "ralt", "space", "", "", "", ""}}, {"dot", {"comma", "l", "colon", "slash", "ralt", "space", "", "", "", ""}}, {"slash", {"dot", "colon", "quote", "rshift", "rwin", "ralt", "", "", "", ""}}, {"rshift", {"slash", "quote", "enter", "left", "rctrl", "rmenu", "rwin", "", "", ""}}, {"up", {"left", "down", "right", "", "", "", "", "", "", ""}}, {"num1", {"num4", "num5", "num2", "num0", "", "", "", "", "", ""}}, {"num2", {"num1", "num4", "num5", "num6", "num3", "numdot", "num0", "", "", ""}}, {"num3", {"num2", "num5", "num6", "numpluss", "numenter", "numdot", "num0", "", "", ""}}, {"numenter", {"num3", "num6", "numplus", "numdot", "", "", "", "", "", ""}}, 
    
    {"lctrl", {"lshift", "lwin", "", "", "", "", "", "", "", ""}}, {"lwin", {"lctrl", "lshift", "z", "lalt", "", "", "", "", "", ""}}, {"lalt", {"lwin", "z", "x", "space", "", "", "", "", "", ""}}, {"space", {"lalt", "x", "c", "v", "b", "n", "m", "comma", "dot", "ralt"}}, {"ralt", {"space", "comma", "dot", "slash", "rwin", "", "", "", "", ""}}, {"rwin", {"ralt", "slash", "rshift", "rmenu", "", "", "", "", "", ""}}, {"rmenu", {"rwin", "rshift", "rctrl", "", "", "", "", "", "", ""}}, {"rctrl", {"rmenu", "rshift", "left", "", "", "", "", "", "", ""}}, {"left", {"rctrl", "rshift", "up", "down", "", "", "", "", "", ""}}, {"down", {"left", "up", "down", "right", "", "", "", "", "", ""}}, {"right", {"down", "up", "num1", "num0", "", "", "", "", "", ""}}, {"num0", {"right", "num1", "num2", "num3", "numdot", "", "", "", "", ""}}, {"numdot", {"num0", "num1", "num2", "num3", "numenter", "", "", "", "", ""}}
};

void ckb_parameter(ckb_runctx* context, const char* name, const char* value) {
    CKB_PARSE_AGRADIENT("color", &animcolor){}
    CKB_PARSE_LONG("delay", &delay) {}
    tng = delay;
}

void draw_key_state(ckb_key *key, int s) {
    float a, r, g, b;
    //tng = delay causes keys to go dark for a second here, I like it.
    ckb_grad_color(&a, &r, &g, &b, &animcolor, tng*100.f/delay);
    a = s>0? 255: 0;
    key->a = a;
    key->r = r;
    key->g = g;
    key->b = b;
}

void ckb_keypress(ckb_runctx* context, ckb_key* key, int x, int y, int state) {
    for (unsigned i = 0; i < 108; i++) {
        if (!strcmp(key->name, adjacencygraph[i].name)) {
            //flip the designated key
            keypressed[i]++;
            if (keypressed[i] > 1) { keypressed[i] = 0; }
            else {
                keystate[i]++;
                if (keystate[i] > 1) { keystate[i] = 0; }
            }
        }
    }
}

void ckb_time(ckb_runctx* context, double delta) {
    //This is the game loop
    unsigned count = context->keycount;
    //Will require a clock speed
    tng -= delta;
    if (tng == 0) {
        //Grow/Die each cell this tick
        tng += delay;
        //record the current living cells
        int livmap[108];
        for (unsigned i = 0; i < 108; i++) { livmap[i] = keystate[i]; }
        //compute changes before ticking
        for (unsigned i = 0; i < 108; i++) {
            int neighborhood = 0;
            //count the active keys near this key
            for (unsigned j = 0; j < 10; j++) {
                //match neighbors
                for (unsigned k = 0; k < count; k++) {
                    if (!strcmp(adjacencygraph[mapkey[k]].name, adjacencygraph[i].neighbors[j])) {
                        if (keystate[mapkey[k]]) {
                            neighborhood++;
                        }
                    }
                }
            }
            if (neighborhood < 2) {
                livmap[i] = 0;
            } else if (neighborhood == 3) {
                livmap[i] = 1;
            } else if (neighborhood > 3) {
                livmap[i] = 0;
            }
        }
        //tick changes
        for (unsigned i = 0; i < 108; i++) {
            keystate[i] = livmap[i];
        }
    }
    //render
    for (unsigned i = 0; i < 108; i++) {
        ckb_key* key = context->keys + keymap[i];
        if (keystate[i] > 0) {
            draw_key_state(key, 1);
        } else {
            draw_key_state(key, 0);
        }
    }
}

void ckb_init(ckb_runctx* context) {
    unsigned count = context->keycount;
    for (unsigned i = 0; i < count; i++) {
        ckb_key* key = context->keys + i;
        for (unsigned j = 0; j < 108; j++) {
            if (!strcmp(key->name, adjacencygraph[j].name)) {
                keymap[j] = (int) i;
                mapkey[i] = (int) j;
            }
        }
    }
}
 
void ckb_start(ckb_runctx* context, int state) {
    
}
 
int ckb_frame(ckb_runctx* context) {
    return 0;
}

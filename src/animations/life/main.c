#include <ckb-next/animation.h>
#include <math.h>
#include <string.h>

void ckb_info() {
    // Plugin info
    CKB_NAME("Life");
    CKB_VERSION("0.1");
    CKB_COPYRIGHT("2019", "blob_");
    CKB_LICENSE("GPLv2");
    CKB_GUID("{512CD200-5DB7-4E18-BB3B-9281811E482B}");
    CKB_DESCRIPTION("Conway's Game of Life on your keyboard.");

    // Effect parameters
    CKB_PARAM_AGRADIENT("lcolor", "Dead Color", "Live Color", "0:ff000000 50:ffffffff 100:ffffffff");
    CKB_PARAM_DOUBLE("growdelay", "Frames to next generation", "frames", 60, 10, 1000);
    CKB_PARAM_BOOL("refresh", "Refresh living cells on keypress", 1);
    CKB_PARAM_BOOL("gridy", "Gridy mapping", 1);

    // Timing/input parameters
    CKB_KPMODE(CKB_KP_POSITION);
    CKB_TIMEMODE(CKB_TIME_ABSOLUTE);
    CKB_REPEAT(FALSE);
    CKB_LIVEPARAMS(TRUE);

    // Presets
    CKB_PRESET_START("Default");
    CKB_PRESET_PARAM("trigger", "0");
    CKB_PRESET_PARAM("kptrigger", "1");
    CKB_PRESET_END;

    CKB_PRESET_START("Red/Green");
    CKB_PRESET_PARAM("trigger", "0");
    CKB_PRESET_PARAM("kptrigger", "1");
    CKB_PRESET_PARAM("lcolor", "0:ffff0000 50:ff00ff00 100:ff00ff00");
    CKB_PRESET_END;

    CKB_PRESET_START("Transparent");
    CKB_PRESET_PARAM("trigger", "0");
    CKB_PRESET_PARAM("kptrigger", "1");
    CKB_PRESET_PARAM("lcolor", "0:00000000 50:ffffffff 100:ffffffff");
    CKB_PRESET_END;
}

// user parameter variables
ckb_gradient lcolor = { 0 };
double tng = 60;
double growdelay = 60;
int refreshing = 0;
int gridymap = 1;

// main memory
int keystate[108] = { 0 };

typedef struct {
    char* name;
    char* neighbors[10];
} adjacencynode;

typedef struct {
    int address[10];
} addressbook;

// translate keycontext to adjacencygraph
int keymap[108] = { 0 };
// translate keynames to adjacencygraph
addressbook neighbors[108] = { 0 };
// editable adjacencygraph using kissing connectivity
adjacencynode adjacencygraph[108] = {
    {"esc", {"f1", "grave", "1", "", "", "", "", "", "", ""}}, {"f1", {"esc", "f2", "grave", "1", "2", "", "", "", "", ""}}, {"f2", {"f1", "f3", "2", "3", "", "", "", "", "", ""}}, {"f3", {"f2", "f4", "3", "4", "", "", "", "", "", ""}}, {"f4", {"f3", "f5", "4", "5", "6", "", "", "", "", ""}}, {"f5", {"f4", "f6", "5", "6", "7", "", "", "", "", ""}}, {"f6", {"f5", "f7", "7", "8", "", "", "", "", "", ""}}, {"f7", {"f6", "f8", "8", "9", "", "", "", "", "", ""}}, {"f8", {"f7", "f9", "9", "0", "minus", "", "", "", "", ""}}, {"f9", {"f8", "f10", "0", "minus", "equal", "", "", "", "", ""}}, {"f10", {"f9", "f11", "minus", "equal", "bspace", "", "", "", "", ""}}, {"f11", {"f10", "f12", "equal", "bspace", "", "", "", "", "", ""}}, {"f12", {"f11", "prtscn", "bspace", "", "", "", "", "", "", ""}}, {"prtscn", {"f12", "scroll", "bspace", "ins", "home", "", "", "", "", ""}}, {"scroll", {"prtscn", "pause", "ins", "home", "pgup", "", "", "", "", ""}}, {"pause", {"scroll", "stop", "home", "pgup", "numlock", "", "", "", "", ""}}, {"stop", {"pause", "prev", "pgup", "numlock", "numslash", "", "", "", "", ""}}, {"prev", {"stop", "play", "numlock", "numslash", "numstar", "", "", "", "", ""}}, {"play", {"prev", "next", "numslash", "numstar", "numminus", "", "", "", "", ""}}, {"next", {"play", "numstar", "numminus", "", "", "", "", "", "", ""}},
    
    {"grave", {"esc", "f1", "1", "tab", "", "", "", "", "", ""}}, {"1", {"esc", "f1", "grave", "2", "tab", "q", "", "", "", ""}}, {"2", {"f1", "f2", "1", "3", "q", "w", "", "", "", ""}}, {"3", {"f2", "f3", "2", "4", "w", "e", "", "", "", ""}}, {"4", {"f3", "f4", "3", "5", "e", "r", "", "", "", ""}}, {"5", {"f4", "f5", "4", "6", "r", "t", "", "", "", ""}}, {"6", {"f4", "f5", "5", "7", "t", "y", "", "", "", ""}}, {"7", {"f5", "f6", "6", "8", "y", "u", "", "", "", ""}}, {"8", {"f6", "f7", "7", "9", "u", "i", "", "", "", ""}}, {"9", {"f7", "f8", "8", "0", "i", "o", "", "", "", ""}}, {"0", {"f8", "f9", "9", "minus", "o", "p", "", "", "", ""}}, {"minus", {"f8", "f9", "f10", "0", "equal", "p", "lbrace", "", "", ""}}, {"equal", {"f9", "f10", "f11", "minus", "bspace", "lbrace", "rbrace", "", "", ""}}, {"bspace", {"f10", "f11", "f12", "prtscn", "equal", "ins", "rbrace", "bslash", "del", ""}}, {"ins", {"f12", "prtscn", "scroll", "bspace", "home", "bslash", "del", "end", "", ""}}, {"home", {"prtscn", "scroll", "pause", "ins", "pgup", "del", "end", "pgdn", "", ""}}, {"pgup", {"scroll", "pause", "stop", "home", "numlock", "end", "pgdn", "num7", "", ""}}, {"numlock", {"pause", "stop", "prev", "pgup", "numslash", "pgdn", "num7", "num8", "", ""}}, {"numslash", {"stop", "prev", "play", "numlock", "numstar", "num7", "num8", "num9", "", ""}}, {"numstar", {"prev", "play", "next", "numslash", "numminus", "num8", "num9", "numplus", "", ""}}, {"numminus", {"play", "next", "num9", "numplus", "", "", "", "", "", ""}},

    {"tab", {"grave", "1", "q", "caps", "", "", "", "", "", ""}}, {"q", {"tab", "1", "2", "w", "a", "caps", "", "", "", ""}}, {"w", {"q", "2", "3", "e", "s", "a", "", "", "", ""}}, {"e", {"w", "3", "4", "r", "d", "s", "", "", "", ""}}, {"r", {"e", "4", "5", "t", "f", "d", "", "", "", ""}}, {"t", {"r", "5", "6", "y", "g", "f", "", "", "", ""}}, {"y", {"t", "6", "7", "u", "h", "g", "", "", "", ""}}, {"u", {"y", "7", "8", "i", "j", "h", "", "", "", ""}}, {"i", {"u", "8", "9", "o", "k", "j", "", "", "", ""}}, {"o", {"i", "9", "0", "p", "l", "k", "", "", "", ""}}, {"p", {"o", "0", "minus", "lbrace", "colon", "l", "", "", "", ""}}, {"lbrace", {"p", "minus", "equal", "rbrace", "quote", "colon", "", "", "", ""}}, {"rbrace", {"lbrace", "equal", "bspace", "bslash", "enter", "quote", "", "", "", ""}}, {"bslash", {"rbrace", "bspace", "ins", "del", "enter", "", "", "", "", ""}}, {"del", {"bslash", "bspace", "ins", "home", "end", "enter", "", "", "", ""}}, {"end", {"del", "ins", "home", "pgup", "pgdn", "", "", "", "", ""}}, {"pgdn", {"end", "home", "pgup", "numlock", "num7", "num4", "", "", "", ""}}, {"num7", {"pgdn", "pgup", "numlock", "numslash", "num8", "num5", "num4", "", "", ""}}, {"num8", {"numlock", "numslash", "numstar", "num9", "num6", "num5", "num4", "", "", ""}}, {"num9", {"num8", "numslash", "numstar", "numminus", "numplus", "num6", "num5", "", "", ""}}, {"numplus", {"num9", "numstar", "numminus", "numenter", "num3", "num6", "", "", "", ""}},

    {"caps", {"tab", "q", "a", "lshift", "", "", "", "", "", ""}}, {"a", {"caps", "q", "w", "s", "z", "lshift", "", "", "", ""}}, {"s", {"a", "w", "e", "d", "x", "z", "", "", "", ""}}, {"d", {"s", "e", "r", "f", "c", "x", "", "", "", ""}}, {"f", {"d", "r", "t", "g", "v", "c", "", "", "", ""}}, {"g", {"f", "t", "y", "h", "b", "v", "", "", "", ""}}, {"h", {"g", "y", "u", "j", "n", "b", "", "", "", ""}}, {"j", {"h", "u", "i", "k", "m", "n", "", "", "", ""}}, {"k", {"j", "i", "o", "l", "comma", "m", "", "", "", ""}}, {"l", {"k", "o", "p", "colon", "dot", "comma", "", "", "", ""}}, {"colon", {"l", "p", "lbrace", "quote", "slash", "dot", "", "", "", ""}}, {"quote", {"colon", "lbrace", "rbrace", "enter", "rshift", "slash", "", "", "", ""}}, {"enter", {"quote", "rbrace", "bslash", "del", "rshift", "", "", "", "", ""}}, {"num4", {"pgdn", "num7", "num8", "num5", "num2", "num1", "", "", "", ""}}, {"num5", {"num4", "num7", "num8", "num9", "num6", "num3", "num2", "num1", "", ""}}, {"num6", {"num5", "num8", "num9", "numplus", "numenter", "num3", "num2", "", "", ""}},

    {"lshift", {"caps", "a", "z", "lctrl", "lwin", "", "", "", "", ""}}, {"z", {"lshift", "a", "s", "x", "lalt", "lwin", "", "", "", ""}}, {"x", {"z", "s", "d", "c", "space", "lalt", "", "", "", ""}}, {"c", {"x", "d", "f", "v", "space", "", "", "", "", ""}}, {"v", {"c", "f", "g", "b", "space", "", "", "", "", ""}}, {"b", {"v", "g", "h", "n", "space", "", "", "", "", ""}}, {"n", {"b", "h", "j", "m", "space", "", "", "", "", ""}}, {"m", {"n", "j", "k", "comma", "space", "", "", "", "", ""}}, {"comma", {"m", "k", "l", "dot", "ralt", "space", "", "", "", ""}}, {"dot", {"comma", "l", "colon", "slash", "ralt", "space", "", "", "", ""}}, {"slash", {"dot", "colon", "quote", "rshift", "rwin", "ralt", "", "", "", ""}}, {"rshift", {"slash", "quote", "enter", "left", "rctrl", "rmenu", "rwin", "", "", ""}}, {"up", {"left", "down", "right", "", "", "", "", "", "", ""}}, {"num1", {"num4", "num5", "num2", "num0", "", "", "", "", "", ""}}, {"num2", {"num1", "num4", "num5", "num6", "num3", "numdot", "num0", "", "", ""}}, {"num3", {"num2", "num5", "num6", "numplus", "numenter", "numdot", "num0", "", "", ""}}, {"numenter", {"num3", "num6", "numplus", "numdot", "", "", "", "", "", ""}}, 
    
    {"lctrl", {"lshift", "lwin", "", "", "", "", "", "", "", ""}}, {"lwin", {"lctrl", "lshift", "z", "lalt", "", "", "", "", "", ""}}, {"lalt", {"lwin", "z", "x", "space", "", "", "", "", "", ""}}, {"space", {"lalt", "x", "c", "v", "b", "n", "m", "comma", "dot", "ralt"}}, {"ralt", {"space", "comma", "dot", "slash", "rwin", "", "", "", "", ""}}, {"rwin", {"ralt", "slash", "rshift", "rmenu", "", "", "", "", "", ""}}, {"rmenu", {"rwin", "rshift", "rctrl", "", "", "", "", "", "", ""}}, {"rctrl", {"rmenu", "rshift", "left", "", "", "", "", "", "", ""}}, {"left", {"rctrl", "rshift", "up", "down", "", "", "", "", "", ""}}, {"down", {"left", "up", "down", "right", "", "", "", "", "", ""}}, {"right", {"down", "up", "num1", "num0", "", "", "", "", "", ""}}, {"num0", {"right", "num1", "num2", "num3", "numdot", "", "", "", "", ""}}, {"numdot", {"num0", "num1", "num2", "num3", "numenter", "", "", "", "", ""}}
};

//adjacencygraph using gridy mapping
adjacencynode gridygraph[108] = {
    {"esc", {"f1", "grave", "1", "2", "", "", "", "", "", ""}}, {"f1", {"esc", "f2", "grave", "1", "2", "3", "", "", "", ""}}, {"f2", {"f1", "f3", "2", "3", "4", "1", "", "", "", ""}}, {"f3", {"f2", "f4", "3", "4", "5", "", "", "", "", ""}}, {"f4", {"f3", "f5", "4", "5", "6", "", "", "", "", ""}}, {"f5", {"f4", "f6", "5", "6", "7", "", "", "", "", ""}}, {"f6", {"f5", "f7", "7", "8", "6", "", "", "", "", ""}}, {"f7", {"f6", "f8", "8", "9", "7", "", "", "", "", ""}}, {"f8", {"f7", "f9", "9", "0", "minus", "8", "", "", "", ""}}, {"f9", {"f8", "f10", "0", "minus", "equal", "9", "", "", "", ""}}, {"f10", {"f9", "f11", "minus", "equal", "bspace", "", "", "", "", ""}}, {"f11", {"f10", "f12", "equal", "bspace", "", "", "", "", "", ""}}, {"f12", {"f11", "prtscn", "bspace", "", "", "", "", "", "", ""}}, {"prtscn", {"f12", "scroll", "bspace", "ins", "home", "", "", "", "", ""}}, {"scroll", {"prtscn", "pause", "ins", "home", "pgup", "", "", "", "", ""}}, {"pause", {"scroll", "stop", "home", "pgup", "numlock", "", "", "", "", ""}}, {"stop", {"pause", "prev", "pgup", "numlock", "numslash", "", "", "", "", ""}}, {"prev", {"stop", "play", "numlock", "numslash", "numstar", "", "", "", "", ""}}, {"play", {"prev", "next", "numslash", "numstar", "numminus", "", "", "", "", ""}}, {"next", {"play", "numstar", "numminus", "", "", "", "", "", "", ""}},
    
    {"grave", {"esc", "f1", "1", "tab", "q", "", "", "", "", ""}}, {"1", {"esc", "f1", "grave", "2", "tab", "q", "w", "f2", "", ""}}, {"2", {"f1", "f2", "1", "3", "q", "w", "e", "esc", "f3", ""}}, {"3", {"f2", "f3", "2", "4", "w", "e", "r", "f1", "f4", ""}}, {"4", {"f3", "f4", "3", "5", "e", "r", "t", "f2", "f5", ""}}, {"5", {"f4", "f5", "4", "6", "r", "t", "y", "f3", "", ""}}, {"6", {"f4", "f5", "5", "7", "t", "y", "u", "f6", "", ""}}, {"7", {"f5", "f6", "6", "8", "y", "u", "i", "f7", "", ""}}, {"8", {"f6", "f7", "7", "9", "u", "i", "o", "f8", "", ""}}, {"9", {"f7", "f8", "8", "0", "i", "o", "p", "f9", "", ""}}, {"0", {"f8", "f9", "9", "minus", "o", "p", "lbrace", "", "", ""}}, {"minus", {"f8", "f9", "f10", "0", "equal", "p", "lbrace", "rbrace", "", ""}}, {"equal", {"f9", "f10", "f11", "minus", "bspace", "lbrace", "rbrace", "bslash", "", ""}}, {"bspace", {"f10", "f11", "f12", "prtscn", "equal", "ins", "rbrace", "bslash", "del", ""}}, {"ins", {"f12", "prtscn", "scroll", "bspace", "home", "bslash", "del", "end", "", ""}}, {"home", {"prtscn", "scroll", "pause", "ins", "pgup", "del", "end", "pgdn", "", ""}}, {"pgup", {"scroll", "pause", "stop", "home", "numlock", "end", "pgdn", "num7", "", ""}}, {"numlock", {"pause", "stop", "prev", "pgup", "numslash", "pgdn", "num7", "num8", "", ""}}, {"numslash", {"stop", "prev", "play", "numlock", "numstar", "num7", "num8", "num9", "", ""}}, {"numstar", {"prev", "play", "next", "numslash", "numminus", "num8", "num9", "numplus", "", ""}}, {"numminus", {"play", "next", "num9", "numplus", "", "", "", "", "", ""}},

    {"tab", {"grave", "1", "q", "caps", "a", "2", "", "", "", ""}}, {"q", {"tab", "1", "2", "w", "a", "caps", "grave", "s", "", ""}}, {"w", {"q", "2", "3", "e", "s", "a", "1", "d", "", ""}}, {"e", {"w", "3", "4", "r", "d", "s", "2", "f", "", ""}}, {"r", {"e", "4", "5", "t", "f", "d", "3", "g", "", ""}}, {"t", {"r", "5", "6", "y", "g", "f", "4", "h", "", ""}}, {"y", {"t", "6", "7", "u", "h", "g", "5", "j", "", ""}}, {"u", {"y", "7", "8", "i", "j", "h", "6", "k", "", ""}}, {"i", {"u", "8", "9", "o", "k", "j", "7", "l", "", ""}}, {"o", {"i", "9", "0", "p", "l", "k", "8", "colon", "", ""}}, {"p", {"o", "0", "minus", "lbrace", "colon", "l", "9", "quote", "", ""}}, {"lbrace", {"p", "minus", "equal", "rbrace", "quote", "colon", "0", "enter", "", ""}}, {"rbrace", {"lbrace", "equal", "bspace", "bslash", "enter", "quote", "minus", "", "", ""}}, {"bslash", {"rbrace", "bspace", "ins", "del", "enter", "equal", "", "", "", ""}}, {"del", {"bslash", "bspace", "ins", "home", "end", "enter", "", "", "", ""}}, {"end", {"del", "ins", "home", "pgup", "pgdn", "", "", "", "", ""}}, {"pgdn", {"end", "home", "pgup", "numlock", "num7", "num4", "", "", "", ""}}, {"num7", {"pgdn", "pgup", "numlock", "numslash", "num8", "num5", "num4", "", "", ""}}, {"num8", {"numlock", "numslash", "numstar", "num9", "num6", "num5", "num4", "", "", ""}}, {"num9", {"num8", "numslash", "numstar", "numminus", "numplus", "num6", "num5", "", "", ""}}, {"numplus", {"num9", "numstar", "numminus", "numenter", "num3", "num6", "", "", "", ""}},

    {"caps", {"tab", "q", "a", "lshift", "z", "", "", "", "", ""}}, {"a", {"caps", "q", "w", "s", "z", "lshift", "tab", "x", "", ""}}, {"s", {"a", "w", "e", "d", "x", "z", "q", "c", "", ""}}, {"d", {"s", "e", "r", "f", "c", "x", "w", "v", "", ""}}, {"f", {"d", "r", "t", "g", "v", "c", "e", "b", "", ""}}, {"g", {"f", "t", "y", "h", "b", "v", "r", "n", "", ""}}, {"h", {"g", "y", "u", "j", "n", "b", "t", "m", "", ""}}, {"j", {"h", "u", "i", "k", "m", "n", "y", "comma", "", ""}}, {"k", {"j", "i", "o", "l", "comma", "m", "u", "dot", "", ""}}, {"l", {"k", "o", "p", "colon", "dot", "comma", "i", "slash", "", ""}}, {"colon", {"l", "p", "lbrace", "quote", "slash", "dot", "o", "rshift", "", ""}}, {"quote", {"colon", "lbrace", "rbrace", "enter", "rshift", "slash", "p", "", "", ""}}, {"enter", {"quote", "rbrace", "bslash", "del", "rshift", "", "", "", "", ""}}, {"num4", {"pgdn", "num7", "num8", "num5", "num2", "num1", "", "", "", ""}}, {"num5", {"num4", "num7", "num8", "num9", "num6", "num3", "num2", "num1", "", ""}}, {"num6", {"num5", "num8", "num9", "numplus", "numenter", "num3", "num2", "", "", ""}},

    {"lshift", {"caps", "a", "z", "lctrl", "lwin", "lalt", "", "", "", ""}}, {"z", {"lshift", "a", "s", "x", "lalt", "lwin", "caps", "space", "", ""}}, {"x", {"z", "s", "d", "c", "space", "lalt", "a", "", "", ""}}, {"c", {"x", "d", "f", "v", "space", "s", "", "", "", ""}}, {"v", {"c", "f", "g", "b", "space", "d", "", "", "", ""}}, {"b", {"v", "g", "h", "n", "space", "f", "", "", "", ""}}, {"n", {"b", "h", "j", "m", "space", "g", "", "", "", ""}}, {"m", {"n", "j", "k", "comma", "space", "h", "", "", "", ""}}, {"comma", {"m", "k", "l", "dot", "ralt", "space", "j", "", "", ""}}, {"dot", {"comma", "l", "colon", "slash", "ralt", "space", "k", "rwin", "", ""}}, {"slash", {"dot", "colon", "quote", "rshift", "rwin", "ralt", "l", "rmenu", "", ""}}, {"rshift", {"slash", "quote", "enter", "left", "rctrl", "rmenu", "rwin", "colon", "", ""}}, {"up", {"left", "down", "right", "", "", "", "", "", "", ""}}, {"num1", {"num4", "num5", "num2", "num0", "", "", "", "", "", ""}}, {"num2", {"num1", "num4", "num5", "num6", "num3", "numdot", "num0", "", "", ""}}, {"num3", {"num2", "num5", "num6", "numplus", "numenter", "numdot", "num0", "", "", ""}}, {"numenter", {"num3", "num6", "numplus", "numdot", "", "", "", "", "", ""}}, 
    
    {"lctrl", {"lshift", "lwin", "", "", "", "", "", "", "", ""}}, {"lwin", {"lctrl", "lshift", "z", "lalt", "", "", "", "", "", ""}}, {"lalt", {"lwin", "z", "x", "space", "", "", "", "", "", ""}}, {"space", {"lalt", "x", "c", "v", "b", "n", "m", "comma", "dot", "ralt"}}, {"ralt", {"space", "comma", "dot", "slash", "rwin", "", "", "", "", ""}}, {"rwin", {"ralt", "slash", "rshift", "rmenu", "", "", "", "", "", ""}}, {"rmenu", {"rwin", "rshift", "rctrl", "", "", "", "", "", "", ""}}, {"rctrl", {"rmenu", "rshift", "left", "", "", "", "", "", "", ""}}, {"left", {"rctrl", "rshift", "up", "down", "", "", "", "", "", ""}}, {"down", {"left", "up", "down", "right", "", "", "", "", "", ""}}, {"right", {"down", "up", "num1", "num0", "", "", "", "", "", ""}}, {"num0", {"right", "num1", "num2", "num3", "numdot", "", "", "", "", ""}}, {"numdot", {"num0", "num1", "num2", "num3", "numenter", "", "", "", "", ""}}
};

// translate a keyname to an adjacencygraph index
// returns 108 if fails to find
int name2num(char* keyname){
    int i = -1;
    do { i++; } while(i < 108 && strcmp(keyname, adjacencygraph[i].name));
    if (i == 108) {
        //this key wasn't named correctly
        fprintf(stderr, "Recheck the key graph, something wasn't named right: %s", keyname);
    }
    return i;
}

// messy precompute instead of messier preset data
void choosemap(ckb_runctx* context) {
    int count = context->keycount;
    for (int i = 0; i < count; i++) {
        ckb_key* key = context->keys + i;
        for (int j = 0; j < 108; j++) {
            if (!strcmp(key->name, adjacencygraph[j].name)) {
                keymap[j] = (int) i;
                for (int k = 0; k < 10; k++) {
                    if (strcmp(adjacencygraph[j].neighbors[k], "")) {
                        neighbors[j].address[k] = name2num(adjacencygraph[j].neighbors[k]);
                    } else {
                        //Don't map blank keynames to Esc
                        neighbors[j].address[k] = -1;
                    }
                }
            }
        }
    }
}

// load user settings
void ckb_parameter(ckb_runctx* context, const char* name, const char* value) {
    CKB_PARSE_AGRADIENT("lcolor", &lcolor) {}
    CKB_PARSE_DOUBLE("growdelay", &growdelay) {}
    CKB_PARSE_BOOL("refresh", &refreshing) {}
    CKB_PARSE_BOOL("gridy", &gridymap) {}
    tng = growdelay;
    //choose keymap
    if (!strcmp(name,"gridy")) { 
        if (gridymap) {
            memcpy(adjacencygraph, gridygraph, sizeof(gridygraph));
        }
        //params come after initial init unfortunately
        choosemap(context);
    }
}

// render a single key
void draw_key_state(ckb_key *key, int s) {
    float a, r, g, b;
    ckb_grad_color(&a, &r, &g, &b, &lcolor, (s ? roundf(tng*100.f/growdelay) : 0.f));
    key->a = roundf(a);
    key->r = roundf(r);
    key->g = roundf(g);
    key->b = roundf(b);
}

// update the board for user input
void ckb_keypress(ckb_runctx* context, ckb_key* key, int x, int y, int state) {
    // optionally give the user more time to edit their board
    if (refreshing > 0) { tng = growdelay; }
    for (int i = 0; i < 108; i++) {
        if (!strcmp(key->name, adjacencygraph[i].name) && state){
            keystate[i] = !keystate[i];
            return;
        }
    }
}

// This is the game loop
void ckb_time(ckb_runctx* context, double delta) {
    // track clock speed
    tng--;
    if (tng <= 0) {
        // Grow/Die each cell this tick
        tng = growdelay;
        // record the current living cells
        int livmap[108];
        memcpy(livmap, keystate, sizeof(keystate));
        // compute changes before ticking
        for (int i = 0; i < 108; i++) {
            int neighborhood = 0;
            // count the active keys near this key
            for (int j = 0; j < 10; j++) {
                //skip vacant neighbors
                if(neighbors[i].address[j] < 0) break;
                // finally use the precompute to make the game loop less soupy
                if (keystate[neighbors[i].address[j]]) {
                    neighborhood++;
                }
            }
            if (neighborhood == 2) continue;
            livmap[i] = (neighborhood == 3);
        }
        // tick changes
        memcpy(keystate, livmap, sizeof(keystate));
    }
    // render
    for (int i = 0; i < 108; i++) {
        ckb_key* key = context->keys + keymap[i];
        draw_key_state(key, keystate[i]);
    }
}

// need params to decide which graph to initialize
void ckb_init(ckb_runctx* context) {

}

// define unused builtins
void ckb_start(ckb_runctx* context, int state) {

}
 
int ckb_frame(ckb_runctx* context) {
    return 0;
}

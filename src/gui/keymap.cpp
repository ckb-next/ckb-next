#include <clocale>
#include <QMap>
#include "keymap.h"
#include <QObject>
#include <QPolygon>
#include <QRect>

const Key KeyMap::emptyKey = {nullptr, nullptr, nullptr, QPolygon {}, false, false};

// Normal key size
#define NS 12, 12

// Lightbar LED size
#define LBS 17, 6

// K55 Zone Size
#define ZS 97, 75

// K55 PRO Zone Size
#define ZSP 59, 75

// Key positions (K95 - English)
// This is the master key map that includes ANSI, ISO and JP-106 layouts - use patchANSI(), patchISO() or patchJP106() to finalize it
static const Key K95Keys[] = {
    {nullptr, nullptr, "mr", QRect {38, 0, NS}, true, true}, {nullptr, nullptr, "m1", QRect {50, 0, NS}, true, true}, {nullptr, nullptr, "m2", QRect {62, 0, NS}, true, true}, {nullptr, nullptr, "m3", QRect {74, 0, NS}, true, true}, {nullptr,  "Brightness", "light", QRect {222, 0, NS}, true, true}, {nullptr,  "Windows Lock", "lock", QRect {234, 0, NS}, true, true}, {nullptr,  "Mute", "mute", QRect {273, 0, 13, 8}, true, true}, {nullptr,  "Volume Up", "volup", QRect {290, -2, 18, 6}, false, true}, {nullptr,  "Volume down", "voldn", QRect {290, 2, 18, 6}, false, true},
    {nullptr, nullptr, "g1", QRect {0, 14, NS}, true, true}, {nullptr, nullptr, "g2", QRect {11, 14, NS}, true, true}, {nullptr, nullptr, "g3", QRect {22, 14, NS}, true, true}, {nullptr,  "Esc", "esc", QRect {38, 14, NS}, true, true}, {nullptr, nullptr, "f1", QRect {58, 14, NS}, true, true}, {nullptr, nullptr, "f2", QRect {70, 14, NS}, true, true}, {nullptr, nullptr, "f3", QRect {82, 14, NS}, true, true}, {nullptr, nullptr, "f4", QRect {94, 14, NS}, true, true}, {nullptr, nullptr, "f5", QRect {114, 14, NS}, true, true}, {nullptr, nullptr, "f6", QRect {126, 14, NS}, true, true}, {nullptr, nullptr, "f7", QRect {138, 14, NS}, true, true}, {nullptr, nullptr, "f8", QRect {150, 14, NS}, true, true}, {nullptr, nullptr, "f9", QRect {170, 14, NS}, true, true}, {nullptr, nullptr, "f10", QRect {182, 14, NS}, true, true}, {nullptr, nullptr, "f11", QRect {194, 14, NS}, true, true}, {nullptr, nullptr, "f12", QRect {206, 14, NS}, true, true}, {nullptr,  "Print Screen\nSysRq", "prtscn", QRect {222, 14, NS}, true, true}, {nullptr,  "Scroll Lock", "scroll", QRect {234, 14, NS}, true, true}, {nullptr,  "Pause\nBreak", "pause", QRect {246, 14, NS}, true, true}, {nullptr,  "Stop", "stop", QRect {262, 14, 12, 8}, true, true}, {nullptr,  "Previous", "prev", QRect {273, 14, 13, 8}, true, true}, {nullptr,  "Play/Pause", "play", QRect {285, 14, 13, 8}, true, true}, {nullptr,  "Next", "next", QRect {296, 14, 12, 8}, true, true},
    {nullptr, nullptr, "g4", QRect {0, 25, NS}, true, true}, {nullptr, nullptr, "g5", QRect {11, 25, NS}, true, true}, {nullptr, nullptr, "g6", QRect {22, 25, NS}, true, true}, {nullptr,  "`", "grave", QRect {38, 27, NS}, true, true}, {nullptr, nullptr, "1", QRect {50, 27, NS}, true, true}, {nullptr, nullptr, "2", QRect {62, 27, NS}, true, true}, {nullptr, nullptr, "3", QRect {74, 27, NS}, true, true}, {nullptr, nullptr, "4", QRect {86, 27, NS}, true, true}, {nullptr, nullptr, "5", QRect {98, 27, NS}, true, true}, {nullptr, nullptr, "6", QRect {110, 27, NS}, true, true}, {nullptr, nullptr, "7", QRect {122, 27, NS}, true, true}, {nullptr, nullptr, "8", QRect {134, 27, NS}, true, true}, {nullptr, nullptr, "9", QRect {146, 27, NS}, true, true}, {nullptr, nullptr, "0", QRect {158, 27, NS}, true, true}, {nullptr,  "-", "minus", QRect {170, 27, NS}, true, true}, {nullptr,  "=", "equal", QRect {182, 27, NS}, true, true}, {nullptr,  "¥", "yen", QRect {194, 27, NS}, true, true}, {nullptr,  "Backspace", "bspace", QRect {200, 27, 24, 12}, true, true}, {nullptr,  "Insert", "ins", QRect {222, 27, NS}, true, true}, {nullptr,  "Home", "home", QRect {234, 27, NS}, true, true}, {nullptr,  "Page Up", "pgup", QRect {246, 27, NS}, true, true}, {nullptr,  "Num Lock", "numlock", QRect {261, 27, NS}, true, true}, {nullptr,  "NumPad /", "numslash", QRect {273, 27, NS}, true, true}, {nullptr,  "NumPad *", "numstar", QRect {285, 27, NS}, true, true}, {nullptr,  "NumPad -", "numminus", QRect {297, 27, NS}, true, true},
    {nullptr, nullptr, "g7", QRect {0, 39, NS}, true, true}, {nullptr, nullptr, "g8", QRect {11, 39, NS}, true, true}, {nullptr, nullptr, "g9", QRect {22, 39, NS}, true, true}, {nullptr,  "Tab", "tab", QRect {41, 39, 18, 12}, true, true}, {nullptr, nullptr, "q", QRect {56, 39, NS}, true, true}, {nullptr, nullptr, "w", QRect {68, 39, NS}, true, true}, {nullptr, nullptr, "e", QRect {80, 39, NS}, true, true}, {nullptr, nullptr, "r", QRect {92, 39, NS}, true, true}, {nullptr, nullptr, "t", QRect {104, 39, NS}, true, true}, {nullptr, nullptr, "y", QRect {116, 39, NS}, true, true}, {nullptr, nullptr, "u", QRect {128, 39, NS}, true, true}, {nullptr, nullptr, "i", QRect {140, 39, NS}, true, true}, {nullptr, nullptr, "o", QRect {152, 39, NS}, true, true}, {nullptr, nullptr, "p", QRect {164, 39, NS}, true, true}, {nullptr,  "[", "lbrace", QRect {176, 39, NS}, true, true}, {nullptr,  "]", "rbrace", QRect {188, 39, NS}, true, true}, {nullptr,  "\\", "bslash", QRect {203, 39, 18, 12}, true, true}, {nullptr,  "Enter", "enter", QRect {203, 45, 18, 24}, true, true}, {nullptr,  "Delete", "del", QRect {222, 39, NS}, true, true}, {nullptr,  "End", "end", QRect {234, 39, NS}, true, true}, {nullptr,  "Page Down", "pgdn", QRect {246, 39, NS}, true, true}, {nullptr,  "NumPad 7", "num7", QRect {261, 39, NS}, true, true}, {nullptr,  "NumPad 8", "num8", QRect {273, 39, NS}, true, true}, {nullptr,  "NumPad 9", "num9", QRect {285, 39, NS}, true, true}, {nullptr,  "NumPad +", "numplus", QRect {297, 45, 12, 24}, true, true},
    {nullptr, nullptr, "g10", QRect {0, 50, NS}, true, true}, {nullptr, nullptr, "g11", QRect {11, 50, NS}, true, true}, {nullptr, nullptr, "g12", QRect {22, 50, NS}, true, true}, {nullptr,  "Caps Lock", "caps", QRect {42, 51, 20, 12}, true, true}, {nullptr, nullptr, "a", QRect {59, 51, NS}, true, true}, {nullptr, nullptr, "s", QRect {71, 51, NS}, true, true}, {nullptr, nullptr, "d", QRect {83, 51, NS}, true, true}, {nullptr, nullptr, "f", QRect {95, 51, NS}, true, true}, {nullptr, nullptr, "g", QRect {107, 51, NS}, true, true}, {nullptr, nullptr, "h", QRect {119, 51, NS}, true, true}, {nullptr, nullptr, "j", QRect {131, 51, NS}, true, true}, {nullptr, nullptr, "k", QRect {143, 51, NS}, true, true}, {nullptr, nullptr, "l", QRect {155, 51, NS}, true, true}, {nullptr,  ";", "colon", QRect {167, 51, NS}, true, true}, {nullptr,  "'", "quote", QRect {179, 51, NS}, true, true}, {nullptr,  "#", "hash", QRect {191, 51, NS}, true, true}, {nullptr,  "NumPad 4", "num4", QRect {261, 51, NS}, true, true}, {nullptr,  "NumPad 5", "num5", QRect {273, 51, NS}, true, true}, {nullptr,  "NumPad 6", "num6", QRect {285, 51, NS}, true, true},
    {nullptr, nullptr, "g13", QRect {0, 64, NS}, true, true}, {nullptr, nullptr, "g14", QRect {11, 64, NS}, true, true}, {nullptr, nullptr, "g15", QRect {22, 64, NS}, true, true}, {nullptr,  "Left Shift", "lshift", QRect {39, 63, 14, 12}, true, true}, {"bslash", "\\", "bslash_iso", QRect {53, 63, NS}, true, true}, {nullptr, nullptr, "z", QRect {65, 63, NS}, true, true}, {nullptr, nullptr, "x", QRect {77, 63, NS}, true, true}, {nullptr, nullptr, "c", QRect {89, 63, NS}, true, true}, {nullptr, nullptr, "v", QRect {101, 63, NS}, true, true}, {nullptr, nullptr, "b", QRect {113, 63, NS}, true, true}, {nullptr, nullptr, "n", QRect {125, 63, NS}, true, true}, {nullptr, nullptr, "m", QRect {137, 63, NS}, true, true}, {nullptr,  ",", "comma", QRect {149, 63, NS}, true, true}, {nullptr,  ".", "dot", QRect {161, 63, NS}, true, true}, {nullptr,  "/", "slash", QRect {173, 63, NS}, true, true}, {nullptr,  "_", "ro", QRect {185, 63, NS}, true, true}, {nullptr,  "Right Shift", "rshift", QRect {196, 63, 32, 12}, true, true}, {nullptr,  "Up", "up", QRect {234, 63, NS}, true, true}, {nullptr,  "NumPad 1", "num1", QRect {261, 63, NS}, true, true}, {nullptr,  "NumPad 2", "num2", QRect {273, 63, NS}, true, true}, {nullptr,  "NumPad 3", "num3", QRect {285, 63, NS}, true, true}, {nullptr,  "NumPad Enter", "numenter", QRect {297, 69, 12, 24}, true, true},
    {nullptr, nullptr, "g16", QRect {0, 75, NS}, true, true}, {nullptr, nullptr, "g17", QRect {11, 75, NS}, true, true}, {nullptr, nullptr, "g18", QRect {22, 75, NS}, true, true}, {nullptr,  "Left Ctrl", "lctrl", QRect {40, 75, 16, 12}, true, true}, {nullptr,  "Left Windows", "lwin", QRect {54, 75, NS}, true, true}, {nullptr,  "Left Alt", "lalt", QRect {67, 75, 14, 12}, true, true}, {nullptr,  "無変換", "muhenkan", QRect {80, 75, NS}, true, true}, {nullptr,  "Space", "space", QRect {116, 75, 84, 12}, true, true}, {nullptr,  "変換", "henkan", QRect {150, 75, NS}, true, true}, {nullptr,  "ひらがな カタカナ ローマ字", "katahira", QRect {162, 75, NS}, true, true}, {nullptr,  "Right Alt", "ralt", QRect {165, 75, 14, 12}, true, true}, {nullptr,  "Right Windows", "rwin", QRect {178, 75, NS}, true, true}, {nullptr,  "Menu", "rmenu", QRect {190, 75, NS}, true, true}, {nullptr,  "Right Ctrl", "rctrl", QRect {204, 75, 16, 12}, true, true}, {nullptr,  "Left", "left", QRect {222, 75, NS}, true, true}, {nullptr,  "Down", "down", QRect {234, 75, NS}, true, true}, {nullptr,  "Right", "right", QRect {246, 75, NS}, true, true}, {nullptr,  "NumPad 0", "num0", QRect {267, 75, 24, 12}, true, true}, {nullptr,  "NumPad .", "numdot", QRect {285, 75, NS}, true, true}
};
#define KEYCOUNT_K95 (sizeof(K95Keys) / sizeof(Key))


// ANSI layouts use a different Enter/LShift key
const QPolygon ansi_enter_pos {QRect {199, 51, 26, 12}};
const QPolygon ansi_lshift_pos {QRect {45, 63, 26, 12}};

// Layout "patches" for different regions
struct KeyPatch {
    const char* _storageName;
    const char* _friendlyName;
    const char* name;
};

static const KeyPatch patchDK[] = {
    {nullptr,  "§", "grave"}, {nullptr,  "+", "minus"}, {nullptr,  "´", "equal"},
    {nullptr,  "Å", "lbrace"}, {nullptr,  "¨", "rbrace"},
    {nullptr,  "Æ", "colon"}, {nullptr,  "Ø", "quote"}, {nullptr,  "'", "hash"},
    {nullptr,  "<", "bslash_iso"}, {nullptr,  "-", "slash"},
};

static const KeyPatch patchEU[] = {
    {nullptr,  "\\ (R)", "hash"},
    {nullptr,  "\\ (L)", "bslash_iso"},
};

static const KeyPatch patchFR[] = {
    {"sup2", "²", "grave"}, {nullptr,  "&", "1"}, {nullptr,  "É", "2"}, {nullptr,  "\"", "3"}, {nullptr,  "'", "4"}, {nullptr,  "(", "5"}, {nullptr,  "-", "6"}, {nullptr,  "È", "7"}, {nullptr,  "_", "8"}, {nullptr,  "Ç", "9"}, {nullptr,  "À", "0"}, {"rparen", ")", "minus"},
    {"a", "A", "q"}, {"z", "Z", "w"}, {"caret", "^", "lbrace"}, {"dollar", "$", "rbrace"},
    {"q", "Q", "a"}, {"m", "M", "colon"}, {"percent", "Ù", "quote"}, {"star", "*", "hash"},
    {"angle", "<", "bslash_iso"}, {"w", "W", "z"}, {"comma", ",", "m"}, {"semicolon", ";", "comma"}, {"colon", ":", "dot"}, {"exclam", "!", "slash"},
};

static const KeyPatch patchDE[] = {
    {"caret", "^", "grave"}, {"ss", "ß", "minus"}, {"grave", "´", "equal"},
    {"z", "Z", "y"}, {"ue", "Ü", "lbrace"}, {"plus", "+", "rbrace"},
    {"oe", "Ö", "colon"}, {"ae", "Ä", "quote"},
    {"angle", "<", "bslash_iso"}, {"y", "Y", "z"}, {"minus", "-", "slash"}
};

static const KeyPatch patchIT[] = {
    {nullptr,  "\\", "grave"}, {nullptr,  "'", "minus"}, {nullptr,  "Ì", "equal"},
    {nullptr,  "È", "lbrace"}, {nullptr,  "+", "rbrace"},
    {nullptr,  "Ò", "colon"}, {nullptr,  "À", "quote"}, {nullptr,  "Ù", "hash"},
    {nullptr,  "<", "bslash_iso"}, {nullptr,  "-", "slash"},
};

static const KeyPatch patchNO[] = {
    {nullptr,  "§", "grave"}, {nullptr,  "+", "minus"}, {nullptr,  "´", "equal"},
    {nullptr,  "Å", "lbrace"}, {nullptr,  "¨", "rbrace"},
    {nullptr,  "Ø", "colon"}, {nullptr,  "Æ", "quote"}, {nullptr,  "'", "hash"},
    {nullptr,  "<", "bslash_iso"}, {nullptr,  "-", "slash"},
};

static const KeyPatch patchMX[] = {
    {nullptr,  "|", "grave"}, {nullptr,  "'", "minus"}, {nullptr,  "¿", "equal"},
    {nullptr,  "´", "lbrace"}, {nullptr,  "+", "rbrace"},
    {nullptr,  "Ñ", "colon"}, {nullptr,  "{", "quote"}, {nullptr,  "}", "hash"},
    {nullptr,  "<", "bslash_iso"}, {nullptr,  "-", "slash"},
};

static const KeyPatch patchPTBR[] = {
    {"quote", "'", "grave"},
    {"accent", "´", "lbrace"}, {"lbrace", "[", "rbrace"},
    {"cc", "Ç", "colon"}, {"tilde", "~", "quote"}, {"rbrace", "]", "hash"},
    {"colon", ";", "slash"}, {"slash", "/", "ro"}, {"numcomma", ",", "numdot"},
};

static const KeyPatch patchES[] = {
    {"oa", "º", "grave"}, {"quote", "'", "minus"}, {"lexclam", "¡", "equal"},
    {"grave", "`", "lbrace"}, {"plus", "+", "rbrace"},
    {"nn", "Ñ", "colon"}, {"accent", "´", "quote"}, {"cc", "Ç", "hash"},
    {"angle", "<", "bslash_iso"}, {"minus", "-", "slash"},
};

static const KeyPatch patchSE[] = {
    {"section", "§", "grave"}, {"plus", "+", "minus"}, {"grave", "´", "equal"},
    {"aa", "Å", "lbrace"}, {"umlaut", "¨", "rbrace"},
    {"oe", "Ö", "colon"}, {"ae", "Ä", "quote"}, {"quote", "'", "hash"},
    {"angle", "<", "bslash_iso"}, {"minus", "-", "slash"},
};

/*static const KeyPatch patchJP[] = {
    {"lbrace", "[", "rbrace"},
    {"rbrace", "]", "hash"},
};*/

static const KeyPatch patchDvorak[] = {
    {nullptr,  "[", "minus"}, {nullptr,  "]", "equal"},
    {nullptr,  "'", "q"}, {nullptr,  ",", "w"}, {nullptr,  ".", "e"}, {nullptr,  "P", "r"}, {nullptr,  "Y", "t"}, {nullptr,  "F", "y"}, {nullptr,  "G", "u"}, {nullptr,  "C", "i"}, {nullptr,  "R", "o"}, {nullptr,  "L", "p"}, {nullptr,  "/", "lbrace"}, {nullptr,  "=", "rbrace"},
    {nullptr,  "O", "s"}, {nullptr,  "E", "d"}, {nullptr,  "U", "f"}, {nullptr,  "I", "g"}, {nullptr,  "D", "h"}, {nullptr,  "H", "j"}, {nullptr,  "T", "k"}, {nullptr,  "N", "l"}, {nullptr,  "S", "colon"}, {nullptr,  "-", "quote"},
    {nullptr,  ";", "z"}, {nullptr,  "Q", "x"}, {nullptr,  "J", "c"}, {nullptr,  "K", "v"}, {nullptr,  "X", "b"}, {nullptr,  "B", "n"}, {nullptr,  "W", "comma"}, {nullptr,  "V", "dot"}, {nullptr,  "Z", "slash"},
};

// Apply a patch to a key map
#define patch(map, patches) _patch(map, patches, sizeof(patches)/sizeof(*patches))
static void _patch(QHash<QString, Key>& map, const KeyPatch* patches, int patchCount){
    for(const KeyPatch* p = patches; p < patches + patchCount; p++){
        Key& key = map[p->name];
        key._storageName = p->_storageName;
        key._friendlyName = p->_friendlyName;
    }
}

// Patch a key map for ANSI/ISO layout
static void patchnonJP106(QHash<QString, Key>& map){
    map.remove("yen");
    map.remove("henkan");
    map.remove("muhenkan");
    map.remove("katahira");
    map.remove("ro");
}
static inline void patchISO(QHash<QString, Key>& map){
    patchnonJP106(map);
    map.remove("bslash");
}
static inline void patchANSI(QHash<QString, Key>& map){
    patchnonJP106(map);
    map.remove("bslash_iso");
    map.remove("hash");
    map["enter"].pos = ansi_enter_pos;
    map["lshift"].pos = ansi_lshift_pos;
}

// It's better to have absolute layout patches
static const QPolygon short_bspace {QRect {206, 27, NS}};
static const QPolygon short_rshift {QRect {196 + 6, 63, 32 - 12, 12}};
static const QPolygon short_spacebar {QRect {116 - 1, 75, 84 - 26, 12}};
static const QPolygon jp_ralt {QRect {165 + 11, 75, 14 + 2, 12}};
static const QPolygon jp_fn {QRect {152 - 2, 75, 12 + 4, 12}};

static inline void patchJP106(QHash<QString, Key>& map){
    // First apply the ISO patch
    map.remove("bslash");

    // Resize Backspace
    map["bspace"].pos = short_bspace;

    // Resize RShift
    map["rshift"].pos = short_rshift;

    // Resize Spacebar
    map["space"].pos = short_spacebar;

    // Left shift
    map["lshift"].pos = ansi_lshift_pos;

    map.remove("bslash_iso");

    // Resize and move ralt to make space for the extra keys
    map["ralt"].pos = jp_ralt;

    // None of these layouts have rwin
    map.remove("rwin");
}
// Used to fix the Fn size and remove Alt when necessary in JP layouts for compatible devices
static inline void patchJP106fn(QHash<QString, Key>& map){
    if(map.contains("fn")){
        map.remove("ralt");
        map["fn"].pos = jp_fn;
    }
}
// Patch for ABNT 2 layout
static inline void patchABNT2(QHash<QString, Key>& map){
    map.remove("yen");
    map.remove("henkan");
    map.remove("muhenkan");
    map.remove("katahira");
    map.remove("bslash");

    // Resize RShift
    map["rshift"].pos = short_rshift;
}

// Total width/height
#define K95_WIDTH       298
#define K95_HEIGHT      76

#define K95P_HEIGHT      82
#define K95P_X_START     20
#define K95P_WIDTH       (K95_WIDTH - K95P_X_START + 1)

#define K100_HEIGHT      (K95P_HEIGHT + 6)
#define K100_WIDTH       (K95P_WIDTH + 10)

// K70 cuts off the G keys on the left, as well as MR/M1/M2/M3
#define K70_X_START     38
#define K70_WIDTH       (K95_WIDTH - K70_X_START)
#define K70_HEIGHT      K95_HEIGHT

#define K68_WIDTH       K70_WIDTH
#define K68_HEIGHT      K70_HEIGHT

// K65 additionally removes the numpad on the right, and has a different top row
#define K65_WIDTH       209
#define K65_HEIGHT      K70_HEIGHT

#define K65_MINI_WIDTH  162
#define K65_MINI_HEIGHT 48

// K63 is the same as the K65 in terms of size
#define K63_WIDTH       K65_WIDTH
#define K63_HEIGHT      K65_HEIGHT

// K60 has only six rows
#define K60_WIDTH       K70_WIDTH
#define K60_HEIGHT      62

// K60 PRO TKL has only six rows and no numpad on the right
#define K60_TKL_WIDTH   K65_WIDTH
#define K60_TKL_HEIGHT  K60_HEIGHT


static const Key K70TklTopRow[] = {
    {nullptr, "Stop", "stop", QRect {K70_X_START - 37, 0, 12, 8}, true, true},
    {nullptr, "Previous", "prev", QRect {K70_X_START - 26, 0, 12, 8}, true, true},
    {nullptr, "Play/Pause", "play", QRect {K70_X_START - 15, 0, 12, 8}, true, true},
    {nullptr, "Next", "next", QRect {K70_X_START - 4, 0, 12, 8}, true, true},
    {nullptr, "Logo", "logo", QRect {140 - K70_X_START, 0, 12, 12}, true, false},
    {nullptr, "Profile Switch", "profswitch", QRect {178 - K70_X_START, 0, 12, 8}, true, true},
    {nullptr, "Mute", "mute", QRect {222 - K70_X_START, 0, 12, 8}, true, true},
};
#define K70_TKL_TOP_COUNT (sizeof(K70TklTopRow) / sizeof(Key))

static const Key K68TopRow[] = {
    {nullptr,  "Volume Down", "voldn", QRect {285 - K70_X_START, 0, 13, 8}, true, true}, {nullptr,  "Volume Up", "volup", QRect {297 - K70_X_START, 0, 13, 8}, true, true},
};
#define K68_TOP_COUNT (sizeof(K68TopRow) / sizeof(Key))

static const Key K65TopRow[] = {
    {nullptr,  "Brightness", "light", QRect {164 - K70_X_START, 0, 12, 12}, true, true}, {nullptr,  "Mute", "mute", QRect {176 - K70_X_START, 0, 12, 12}, true, true}, {nullptr,  "Volume Down", "voldn", QRect {192 - K70_X_START, 0, 14, 8}, true, true}, {nullptr,  "Volume Up", "volup", QRect {205 - K70_X_START, 0, 14, 8}, true, true}, {nullptr,  "Windows Lock", "lock", QRect {222 - K70_X_START, 0, 12, 12}, true, true}
};
#define K65_TOP_COUNT (sizeof(K65TopRow) / sizeof(Key))

static const Key K63TopRow[] = {
    {nullptr,  "Stop", "stop", QRect {K70_X_START - 37, 0, 12, 8}, true, true}, {nullptr,  "Previous", "prev", QRect {K70_X_START - 26, 0, 12, 8}, true, true}, {nullptr,  "Play/Pause", "play", QRect {K70_X_START - 15, 0, 12, 8}, true, true}, {nullptr,  "Next", "next", QRect {K70_X_START - 4, 0, 12, 8}, true, true}, {nullptr,  "Brightness", "light", QRect {170 - K70_X_START, 0, 12, 12}, true, true}, {nullptr,  "Windows Lock", "lock", QRect {180 - K70_X_START, 0, 12, 12}, true, true}, {nullptr,  "Mute", "mute", QRect {222 - K70_X_START, 0, 13, 8}, true, true}, {nullptr,  "Volume Down", "voldn", QRect {234 - K70_X_START, 0, 13, 8}, true, true}, {nullptr,  "Volume Up", "volup", QRect {246 - K70_X_START, 0, 13, 8}, true, true}
};
#define K63_TOP_COUNT (sizeof(K63TopRow) / sizeof(Key))

static const Key K55Zones[] = {
    {nullptr,  "Zone 1", "zone1", QRect {46, 45, ZS}, true, false},
    {nullptr,  "Zone 2", "zone2", QRect {140, 45, ZS}, true, false},
    {nullptr,  "Zone 3", "zone3", QRect {233, 45, ZS}, true, false},
};
#define K55_ZONES (sizeof(K55Zones) / sizeof(Key))

static const Key K55PROZones[] = {
    {nullptr,  "Zone 1", "zone1", QRect {26, 45, ZSP}, true, false},
    {nullptr,  "Zone 2", "zone2", QRect {83, 45, ZSP}, true, false},
    {nullptr,  "Zone 3", "zone3", QRect {140, 45, ZSP}, true, false},
    {nullptr,  "Zone 4", "zone4", QRect {197, 45, ZSP}, true, false},
    {nullptr,  "Zone 5", "zone5", QRect {254, 45, ZSP}, true, false},
};
#define K55PRO_ZONES (sizeof(K55PROZones) / sizeof(Key))

// Strafe has side lights
#define KSTRAFE_X_START     12
#define KSTRAFE_WIDTH       (K70_WIDTH + (KSTRAFE_X_START * 2))
#define KSTRAFE_HEIGHT      K95_HEIGHT

static const Key KStrafeKeys[] = {
    {nullptr,  "Sidelight", "lsidel", QRect {0, KSTRAFE_HEIGHT/2, KSTRAFE_X_START, KSTRAFE_HEIGHT}, true, false},
    {nullptr,  "Sidelight", "rsidel", QRect {KSTRAFE_WIDTH, KSTRAFE_HEIGHT/2, KSTRAFE_X_START, KSTRAFE_HEIGHT}, true, false},
    {nullptr,  "Logo", "logo", QRect {KSTRAFE_X_START, 0, NS}, true, false},
    {nullptr,  "Function", "fn", QRect {152, 75, NS}, true, true}
};

// Mouse map - M65
static const Key M65Keys[] = {
    {nullptr,  "Left Mouse", "mouse1", QRect {8, 0, 14, 32}, false, true}, {nullptr,  "Right Mouse", "mouse2", QRect {30, 0, 14, 32}, false, true}, {nullptr,  "Middle Mouse", "mouse3", QRect {22, 8, 8, 7}, false, true},
    {nullptr,  "Wheel Up", "wheelup", QRect {22, 4, 8, 5}, false, true}, {nullptr,  "Wheel Down", "wheeldn", QRect {22, 14, 8, 5}, false, true}, {nullptr,  "Wheel Light", "front", QRect {22, 15, 8, 8}, true, false},
    {nullptr,  "DPI Up", "dpiup", QRect {22, 19, 8, 6}, false, true}, {nullptr,  "DPI Light", "dpi", QRect {22, 24, 8, 8}, true, false}, {nullptr,  "DPI Down", "dpidn", QRect {22, 31, 8, 6}, false, true},
    {nullptr,  "Forward", "mouse5", QRect {5, 24, 5, 9}, false, true}, {nullptr,  "Back", "mouse4", QRect {5, 33, 5, 10}, false, true}, {nullptr,  "Sniper", "sniper", QRect {0, 25, 5, 15}, false, true},
    {nullptr,  "Logo", "back", QRect {20, 54, 12, 12}, true, false}
};

#define M65_WIDTH       52
#define M65_HEIGHT      67

// Sabre
static const Key SabreKeys[] = {
    {nullptr,  "Left Mouse", "mouse1", QRect {8, 0, 14, 32}, false, true}, {nullptr,  "Right Mouse", "mouse2", QRect {30, 0, 14, 32}, false, true}, {nullptr,  "Middle Mouse", "mouse3", QRect {22, 9, 8, 7}, false, true}, {nullptr,  "Front light", "front", QRect {8, -2, 14, 8}, true, false },
    {nullptr,  "Wheel Up", "wheelup", QRect {22, 5, 8, 5}, false, true}, {nullptr,  "Wheel Down", "wheeldn", QRect {22, 15, 8, 5}, false, true}, {nullptr,  "Wheel Light", "wheel", QRect {22, 5, 8, 15}, true, false}, {nullptr,  "Extra button", "thumb1", QRect {22, 20, 8, 18}, false, true},
    {nullptr,  "DPI Up", "dpiup", QRect {5, 3, 5, 7}, false, true}, {nullptr,  "DPI Down", "dpidn", QRect {5, 10, 5, 7}, false, true}, {nullptr,  "DPI Light", "dpi", QRect {5, 4, 5, 12}, true, false},
    {nullptr,  "Forward", "mouse5", QRect {5, 24, 5, 9}, false, true}, {nullptr,  "Back", "mouse4", QRect {5, 33, 5, 10}, false, true},
    {nullptr,  "Logo", "back", QRect {21, 50, 12, 12}, true, false}
};

// Harpoon
static const Key HarpoonKeys[] = {
    {nullptr,  "Left Mouse", "mouse1", QRect {10, 5, 14, 26}, false, true}, {nullptr,  "Right Mouse", "mouse2", QRect {30, 5, 14, 26}, false, true}, {nullptr,  "Middle Mouse", "mouse3", QRect {25, 11, 6, 7}, false, true},
    {nullptr,  "Wheel Up", "wheelup", QRect {25, 7, 6, 5}, false, true}, {nullptr,  "Wheel Down", "wheeldn", QRect {25, 17, 6, 5}, false, true},
    {nullptr,  "DPI Cycle", "dpiup", QRect {25, 23, 6, 10}, false, true}, {nullptr,  "Logo Light", "dpi", QRect {18, 40, 20, 20}, true, false},
    {nullptr,  "Forward", "mouse5", QRect {3, 24, 5, 10}, false, true}, {nullptr,  "Back", "mouse4", QRect {3, 33, 5, 10}, false, true}
};

// Glaive
static const Key GlaiveKeys[] = {
    {nullptr,  "Left Mouse", "mouse1", QRect {17, 3, 14, 18}, false, true}, {nullptr,  "Right Mouse", "mouse2", QRect {37, 3, 14, 18}, false, true}, {nullptr,  "Middle Mouse", "mouse3", QRect {31, 9, 7, 7}, false, true}, {nullptr,  "Front light", "front", QRect {16, -5, 36, 8}, true, false },
    {nullptr,  "Wheel Up", "wheelup", QRect {31, 5, 7, 5}, false, true}, {nullptr,  "Wheel Down", "wheeldn", QRect {31, 15, 7, 5}, false, true}, {nullptr,  "Side Lights", "side", QRect {22, 24, 7, 20}, true, false},
    {nullptr,  "DPI Cycle", "dpiup", QRect {31, 19, 6, 12}, false, true}, {nullptr,  "Logo Light", "back", QRect {26, 45, 16, 16}, true, false},
    {nullptr,  "Forward", "mouse5", QRect {15, 22, 5, 11}, false, true}, {nullptr,  "Back", "mouse4", QRect {15, 32, 5, 11}, false, true}
};

// Katar
static const Key KatarKeys[] = {
    {nullptr,  "Left Mouse", "mouse1", QRect {7, 4, 14, 18}, false, true}, {nullptr,  "Right Mouse", "mouse2", QRect {28, 4, 14, 18}, false, true}, {nullptr,  "Middle Mouse", "mouse3", QRect {22, 9, 7, 7}, false, true},
    {nullptr,  "Wheel Up", "wheelup", QRect {22, 5, 7, 5}, false, true}, {nullptr,  "Wheel Down", "wheeldn", QRect {22, 15, 7, 5}, false, true},
    {nullptr,  "DPI Cycle", "dpiup", QRect {22, 19, 6, 12}, false, true}, {nullptr,  "Logo Light", "front", QRect {15, 43, 20, 20}, true, false},
};

// Katar PRO XT
static const Key KatarPROXTKeys[] = {
    {nullptr,  "Left Mouse",       "mouse1",    QRect {7,  4, 14, 18}, false, true},
    {nullptr,  "Right Mouse",      "mouse2",   QRect {28,  4, 14, 18}, false, true},
    {nullptr,  "Middle Mouse",     "mouse3",   QRect {22,  9,  7,  7}, false, true},
    {nullptr,  "Wheel Up",         "wheelup",  QRect {22,  5,  7,  5}, false, true},
    {nullptr,  "Wheel Down",       "wheeldn",  QRect {22, 15,  7,  5}, false, true},
    {nullptr,  "DPI Cycle",        "dpiup",    QRect {22, 19,  6, 12}, false, true},
    {nullptr,  "DPI Wheel Light",  "dpi",      QRect {21,  3,  8, 14}, true,  false},
    {nullptr,  "Forward",          "mouse5",    QRect {3, 24,  5, 10}, false, true},
    {nullptr,  "Back",             "mouse4",    QRect {3, 33,  5, 10}, false, true}
};

// Scimitar
static const Key ScimKeys[] = {
    {nullptr,  "Left Mouse", "mouse1", QRect {15, 0, 14, 32}, false, true}, {nullptr,  "Right Mouse", "mouse2", QRect {37, 0, 12, 32}, false, true}, {nullptr,  "Middle Mouse", "mouse3", QRect {29, 9, 8, 6}, false, true}, {nullptr,  "Front light", "front", QRect {37, 0, 12, 8}, true, false },
    {nullptr,  "Wheel Up", "wheelup", QRect {29, 3, 8, 6}, false, true}, {nullptr,  "Wheel Down", "wheeldn", QRect {29, 14, 8, 6}, false, true}, {nullptr,  "Wheel Light", "wheel", QRect {29, 3, 8, 17}, true, false},
    {nullptr,  "DPI Up", "dpiup", QRect {29, 19, 8, 9}, false, true}, {nullptr,  "DPI Light", "dpi", QRect {8, 12, 8, 4}, true, false}, {nullptr,  "DPI Down", "dpidn", QRect {29, 28, 8, 9}, false, true},
    {nullptr,  "Thumb light", "thumb", QRect {7, 21, 10, 24}, true, false},
    {nullptr,  "1", "thumb1", QRect {-6, 18, 7, 7}, false, true}, {nullptr,  "2", "thumb2", QRect {1, 18, 7, 7}, false, true}, {nullptr,  "3", "thumb3", QRect {8, 18, 7, 7}, false, true},
    {nullptr,  "4", "thumb4", QRect {-6, 25, 7, 7}, false, true}, {nullptr,  "5", "thumb5", QRect {1, 25, 7, 7}, false, true}, {nullptr,  "6", "thumb6", QRect {8, 25, 7, 7}, false, true},
    {nullptr,  "7", "thumb7", QRect {-6, 32, 7, 7}, false, true}, {nullptr,  "8", "thumb8", QRect {1, 32, 7, 7}, false, true}, {nullptr,  "9", "thumb9", QRect {8, 32, 7, 7}, false, true},
    {nullptr,  "10", "thumb10", QRect {-6, 39, 7, 7}, false, true}, {nullptr,  "11", "thumb11", QRect {1, 39, 7, 7}, false, true}, {nullptr,  "12", "thumb12", QRect {8, 39, 7, 7}, false, true},
    {nullptr,  "Logo", "back", QRect {26, 50, 16, 16}, true, false}
};

#define SCIM_WIDTH      66

// M95
static const Key M95Keys[] = {
    {nullptr,  "Left Mouse", "mouse1", QRect {42, 6, 14, 26}, false, true}, {nullptr,  "Right Mouse", "mouse2", QRect {63, 6, 14, 26}, false, true}, {nullptr,  "Middle Mouse", "mouse3", QRect {55, 8, 8, 7}, false, true},
    {nullptr,  "4", "thumb6", QRect {56, 28, 7, 12}, false, true},
    {nullptr,  "Wheel Up", "wheelup", QRect {55, 4, 8, 5}, false, true}, {nullptr,  "Wheel Down", "wheeldn", QRect {55, 14, 8, 5}, false, true}, {nullptr,  "DPI Up (5)", "dpiup", QRect {38, -1, 6, 9}, false, true}, {nullptr,  "DPI Down (6)", "dpidn", QRect {38, 6, 6, 9}, false, true},
    {nullptr,  "Forward (7)", "mouse6", QRect {-9, 14, 14, 8}, false, true}, {nullptr,  "Back (8)", "mouse7", QRect {3, 14, 14, 8}, false, true}, {nullptr,  "Sniper (14)", "sniper", QRect {-6, 20, 8, 14}, false, true},
    {nullptr,  "9", "thumb1", QRect {15, 14, 18, 8}, false, true}, {nullptr,  "15", "thumb7", QRect {15, 20, 10, 22}, false, true},
    {nullptr,  "10", "thumb2", QRect {23, 20, 10, 12}, false, true},{nullptr,  "11", "thumb3", QRect {23, 30, 10, 12}, false, true},
    {nullptr,  "12", "thumb4", QRect {15, 40, 18, 8}, false, true}, {nullptr,  "13", "thumb5", QRect {-1, 40, 18, 8}, false, true},
    {nullptr,  "Logo", "back", QRect {53, 55, 12, 12}, true, false}

};

#define M95_WIDTH      110

// M55
static const Key M55Keys[] = {
    {nullptr,  "Left Mouse",       "mouse1",    QRect {8,  4, 14, 18}, false, true},
    {nullptr,  "Right Mouse",      "mouse2",   QRect {31,  4, 14, 18}, false, true},
    {nullptr,  "Middle Mouse",     "mouse3",   QRect {24,  6,  7,  8}, false, true},
    {nullptr,  "Wheel Up",         "wheelup",  QRect {24,  3,  7,  5}, false, true},
    {nullptr,  "Wheel Down",       "wheeldn",  QRect {24, 13,  7,  5}, false, true},
    {nullptr,  "DPI Cycle",        "dpiup",    QRect {24, 19,  6, 10}, false, true},
    {nullptr,  "DPI Light",        "dpi",      QRect {23, 15,  8,  6},  true, false},
    {nullptr,  "Forward",          "mouse4",    QRect {6, 24,  5, 10}, false, true},
    {nullptr,  "Back",             "mouse5",    QRect {6, 33,  5, 10}, false, true},
    {nullptr,  "Forward (7)",      "mouse6",   QRect {43, 24,  5, 10}, false, true},
    {nullptr,  "Back (8)",         "mouse7",   QRect {43, 33,  5, 10}, false, true},
    {nullptr,  "Logo",             "back",     QRect {21, 50, 12, 12}, true,  false}
};

// K95 Platinum lightbar
static const Key K95PLbar[] = {
    {nullptr, "Top Light Bar 1", "topbar1", QRect {4, -3, LBS}, true, false}, {nullptr, "Top Light Bar 2", "topbar2", QRect {19, -3, LBS}, true, false}, {nullptr, "Top Light Bar 3", "topbar3", QRect {34, -3, LBS}, true, false}, {nullptr, "Top Light Bar 4", "topbar4", QRect {49, -3, LBS}, true, false}, {nullptr, "Top Light Bar 5", "topbar5", QRect {64, -3, LBS}, true, false}, {nullptr, "Top Light Bar 6", "topbar6", QRect {79, -3, LBS}, true, false},
    {nullptr, "Top Light Bar 7", "topbar7", QRect {94, -3, LBS}, true, false}, {nullptr, "Top Light Bar 8", "topbar8", QRect {109, -3, LBS}, true, false}, {nullptr, "Top Light Bar 9", "topbar9", QRect {124, -3, LBS}, true, false}, {nullptr, "Top Light Bar 10", "topbar10", QRect {139, -3, LBS}, true, false}, {nullptr, "Top Light Bar 11", "topbar11", QRect {154, -3, LBS}, true, false}, {nullptr, "Top Light Bar 12", "topbar12", QRect {169, -3, LBS}, true, false},
    {nullptr, "Top Light Bar 13", "topbar13", QRect {184, -3, LBS}, true, false}, {nullptr, "Top Light Bar 14", "topbar14", QRect {199, -3, LBS}, true, false}, {nullptr, "Top Light Bar 15", "topbar15", QRect {214, -3, LBS}, true, false}, {nullptr, "Top Light Bar 16", "topbar16", QRect {229, -3, LBS}, true, false}, {nullptr, "Top Light Bar 17", "topbar17", QRect {244, -3, LBS}, true, false}, {nullptr, "Top Light Bar 18", "topbar18", QRect {259, -3, LBS}, true, false},
    {nullptr, "Top Light Bar 19", "topbar19", QRect {274, -3, LBS}, true, false},
};
#define LBARCOUNT_K95P (sizeof(K95PLbar) / sizeof(Key))

// Mouse map - DARK CORE
static const Key DarkCoreKeys[] = {
    {nullptr,  "Left Mouse", "mouse1", QRect {8, 0, 14, 32}, false, true}, {nullptr,  "Right Mouse", "mouse2", QRect {30, 0, 14, 32}, false, true}, {nullptr,  "Middle Mouse", "mouse3", QRect {22, 8, 8, 7}, false, true},
    {nullptr,  "Wheel Up", "wheelup", QRect {22, 4, 8, 5}, false, true}, {nullptr,  "Wheel Down", "wheeldn", QRect {22, 14, 8, 5}, false, true}, {nullptr,  "Wheel Light", "wheel", QRect {22, 2, 8, 16}, true, false},
    {nullptr,  "DPI Up", "dpiup", QRect {0, 5, 8, 8}, false, true}, {nullptr,  "DPI Down", "dpidn", QRect {0, 12, 8, 8}, false, true},
    {nullptr,  "Forward", "mouse5", QRect {5, 24, 5, 9}, false, true}, {nullptr,  "Back", "mouse4", QRect {5, 33, 5, 10}, false, true}, {nullptr,  "Sniper", "sniper", QRect {0, 25, 5, 15}, false, true}, {nullptr,  "Side light", "side", QRect {0, 24, 10, 24}, true, false},
    {nullptr,  "Logo", "back", QRect {19, 50, 12, 12}, true, false}, {nullptr,  "Profile Switch", "profswitch", QRect {22, 20, 8, 12}, false, true}, {nullptr,  "DPI", "dpi", QRect {0, 0, 10, 24}, true,  false }
};

// Mouse map - DARK CORE RGB PRO
static const Key DarkCoreRgbProKeys[] = {
    {nullptr, "Left Mouse",     "mouse1",      QRect {8,  0, 14, 24}, false, true},
    {nullptr, "Right Mouse",    "mouse2",     QRect {25,  0, 14, 24}, false, true},
    {nullptr, "Middle Mouse",   "mouse3",     QRect {19,  8,  8,  7}, false, true},
    {nullptr, "Wheel Up",       "wheelup",    QRect {19,  4,  8,  5}, false, true},
    {nullptr, "Wheel Down",     "wheeldn",    QRect {19, 14,  8,  5}, false, true},
    {nullptr, "Wheel Light",    "wheel",      QRect {19,  5,  8, 10}, true,  false},
    {nullptr, "DPI Up",         "dpiup",       QRect {0,  5,  8,  8}, false, true},
    {nullptr, "DPI Down",       "dpidn",       QRect {0, 12,  8,  8}, false, true},
    {nullptr, "Forward",        "mouse5",      QRect {5, 24, 10,  9}, true,  true},
    {nullptr, "Back",           "mouse4",      QRect {5, 29,  5, 10}, true,  true},
    {nullptr, "Bar 3 light",    "bar3",        QRect {5, 35,  5,  8}, true,  false},
    {nullptr, "Bar 4 light",    "bar4",        QRect {5, 40,  5,  8}, true,  false},
    {nullptr, "Thumb light",    "thumb",       QRect {5, 55,  8,  8}, true,  false},
    {nullptr, "Side light",     "side",       QRect {30, 48,  8,  8}, true,  false},
    {nullptr, "Logo",           "back",       QRect {15, 48, 15, 15}, true,  false},
    {nullptr, "Profile",        "profdn",     QRect {19, 19,  8, 8},  false,  true},
    {nullptr, "DPI0",           "dpiw0",       QRect {19, 15,  8, 8},  true,  false},
    {nullptr, "DPI3",           "dpiw3",       QRect {10,  5,  8, 8},  true,  false},
    {nullptr, "DPI2",           "dpiw2",        QRect {5,  5,  8, 8},  true,  false},
    {nullptr, "DPI1",           "dpiw1",         QRect {0,  5,  8, 8},  true,  false},
};

// MM800 Polaris
#define POLARIS_V 6, 14
#define POLARIS_H 14, 6

static const Key PolarisZones[] = {
    {nullptr,  "Zone 1", "zone1", QRect {65, 4, POLARIS_V}, true, false}, {nullptr,  "Zone 2", "zone2", QRect {65, 16, POLARIS_V}, true, false}, {nullptr,  "Zone 3", "zone3", QRect {65, 28, POLARIS_V}, true, false}, {nullptr,  "Zone 4", "zone4", QRect {65, 40, POLARIS_V}, true, false},
    {nullptr,  "Zone 5", "zone5", QRect {65, 52, POLARIS_V}, true, false}, {nullptr,  "Zone 6", "zone6", QRect {43, 62, POLARIS_H}, true, false}, {nullptr,  "Zone 7", "zone7", QRect {31, 62, POLARIS_H}, true, false}, {nullptr,  "Zone 8", "zone8", QRect {19, 62, POLARIS_H}, true, false},
    {nullptr,  "Zone 9", "zone9", QRect {7, 62, POLARIS_H}, true, false}, {nullptr,  "Zone 10", "zone10", QRect {-5, 62, POLARIS_H}, true, false}, {nullptr,  "Zone 11", "zone11", QRect {-19, 52, POLARIS_V}, true, false}, {nullptr,  "Zone 12", "zone12", QRect {-19, 40, POLARIS_V}, true, false},
    {nullptr,  "Zone 13", "zone13", QRect {-19, 28, POLARIS_V}, true, false}, {nullptr,  "Zone 14", "zone14", QRect {-19, 16, POLARIS_V}, true, false}, {nullptr,  "Zone 15", "zone15", QRect {-19, 4, POLARIS_V}, true, false}
};

// MM700
#define MM700_WIDTH      110
#define MM700_HEIGHT     60
#define MM700_V 6, 62
#define MM700_H 62, 6

static const Key MM700Zones[] = {
    {nullptr,  "Zone 1", "zone1", QRect {-16, 1, MM700_H}, true, false},
    {nullptr,  "Zone 2", "zone2", QRect {119, 1, MM700_V}, true, false},
    {nullptr,  "Zone 3", "zone3", QRect {104, -5, NS}, true, false}
};

#define ST100_V 6,22
#define ST100_H 22,6

static const Key ST100Zones[] = {
    {nullptr,  "Zone 1", "zone1", QRect {16, 10, ST100_H}, true, false}, {nullptr,  "Zone 2", "zone2", QRect {36, 10, ST100_H}, true, false}, {nullptr,  "Zone 3", "zone3", QRect {52, 30, ST100_V}, true, false}, {nullptr,  "Zone 4", "zone4", QRect {52, 50, ST100_V}, true, false},
    {nullptr,  "Logo", "zone5", QRect {21, -1, NS}, true, false}, // Logo
    {nullptr,  "Zone 6", "zone6", QRect {16, 66, ST100_H}, true, false}, {nullptr,  "Zone 7", "zone7", QRect {-4, 50, ST100_V}, true, false}, {nullptr,  "Zone 8", "zone8", QRect {-4, 30, ST100_V}, true, false},
    {nullptr,  "Zone 9", "zone9", QRect {-4, 10, ST100_H}, true, false},
};

// Mouse map - Ironclaw
static const Key IronclawKeys[] = {
    // primary keys
    {nullptr,  "Left Mouse",    "mouse1",     QRect {12,  0, 12, 28}, false, true  },
    {nullptr,  "Right Mouse",   "mouse2",     QRect {31,  0, 12, 28}, false, true  },

    // center column keys
    {nullptr,  "Wheel Up",      "wheelup",    QRect {23,  3,  8,  7}, false, true  },
    {nullptr,  "Middle Mouse",  "mouse3",     QRect {23,  7,  8,  6}, false, true  },
    {nullptr,  "Wheel Down",    "wheeldn",    QRect {23, 12,  8,  7}, false, true  },
    {nullptr,  "Profile Cycle", "profswitch", QRect {23, 18,  9,  9}, false, true  },
    {nullptr,  "DPI Cycle",     "dpiup",      QRect {23, 26,  8,  9}, false, true  },

    // left side forward/back keys
    {nullptr,  "Forward",    "mouse5",      QRect {6, 20,  5, 12}, false, true  },
    {nullptr,  "Back",       "mouse4",      QRect {7, 32,  5, 12}, false, true  },

    // zones for LEDs
    {nullptr,  "Logo",          "back",       QRect {21, 50,  NS},    true,  false },
    {nullptr,  "Wheel",         "wheel",      QRect {23, 3,  8,  14}, true,  false },

    // need to add DPI LED, even if not directly configurable for indicator to work
    {nullptr,  "DPI",           "dpi",        QRect {10, 10,  8,  8}, true,  false }

};

// Nightsword
static const Key NightswordKeys[] = {
    //Primary buttons
    {nullptr,  "Left Mouse", "mouse1", QRect {14, 6, 12, 26}, false, true},
    {nullptr,  "Right Mouse", "mouse2", QRect {34, 6, 12, 26}, false, true},
    {nullptr,  "Wheel Up", "wheelup",      QRect {26, 5, 6, 5}, false, true},
    {nullptr,  "Middle Mouse", "mouse3",   QRect {26, 10, 6, 5}, false, true},
    {nullptr,  "Wheel Down", "wheeldn",    QRect {26, 15, 6, 5}, false, true},
    //Lighting zones
    {nullptr,  "Wheel Light", "wheel", QRect {26, 5, 8, 15}, true, false},
    {nullptr,  "Front light", "front", QRect {12, -2, 36, 8}, true, false },
    {nullptr,  "Logo", "back",         QRect {24, 50, 12, 12}, true, false},
    {nullptr,  "Back light", "side", QRect {12, 64, 36, 8}, true, false }, //Addition; side == bottom, under logo
    {nullptr,  "DPI Light", "dpi",     QRect {6, 11, 5, 12}, true, false}, // thumb side DPI indicator light
    //DPI keys
    {nullptr,  "DPI Up", "dpiup", QRect {12, 10, 5, 7}, false, true},
    {nullptr,  "DPI Down", "dpidn", QRect {12, 18, 5, 7}, false, true},
    //Profile keys
    {nullptr,  "Profile Up", "profswitch", QRect {26, 18,  6,  10}, false, true  },
    {nullptr,  "Profile Down", "profdn", QRect {26, 26,  6,  10}, false, true  },
    //Thumb keys (extra should be sniper)
    {nullptr,  "Forward", "mouse5", QRect {8, 21, 5, 9}, false, true},
    {nullptr,  "Back", "mouse4", QRect {8, 30, 5, 10}, false, true},
    {nullptr,  "Sniper", "sniper", QRect {0, 26, 8, 8}, false, true}
};

// Mouse map - Ironclaw RGB Wireless
static const Key IronclawWirelessKeys[] = {
    // primary keys
    {nullptr,  "Left Mouse",   "mouse1",   QRect {12,  0, 12, 28}, false, true  },
    {nullptr,  "Right Mouse",  "mouse2",   QRect {31,  0, 12, 28}, false, true  },

    // center column keys
    {nullptr,  "Wheel Up",     "wheelup",  QRect {23,  3,  8,  7}, false, true  },
    {nullptr,  "Middle Mouse",  "mouse3",  QRect {23,  7,  8,  6}, false, true  },
    {nullptr,  "Wheel Down",   "wheeldn",  QRect {23, 12,  8,  7}, false, true  },
    {nullptr,  "Profile Up",    "profup",  QRect {23, 18,  9,  9}, false, true  },
    {nullptr,  "Profile Dn",    "profdn",  QRect {23, 26,  8,  9}, false, true  },

    // left side forward/back keys
    {nullptr,  "DPI Up",        "dpiup",    QRect {6,  4,  6,  9}, false, true  },
    {nullptr,  "DPI Dn",        "dpidn",    QRect {6, 10,  6,  9}, false, true  },
    {nullptr,  "Forward",      "mouse5",    QRect {6, 20,  5, 12}, false, true  },
    {nullptr,  "Back",         "mouse4",    QRect {7, 32,  5, 12}, false, true  },
    {nullptr,  "Opt",          "optbtn",    QRect {9, 27,  6,  9}, false, true  },

    // zones for LEDs
    {nullptr,  "Front",        "front",     QRect {9,  1,  9,  9}, true, false  },
    {nullptr,  "Logo",         "back",     QRect {21, 50, NS},     true, false  },
    {nullptr,  "Wheel",        "wheel",    QRect {23,  3,  8, 14}, true, false  },

    // need to add DPI LED, even if not directly configurable for indicator to work
    {nullptr,  "DPI",           "dpi",        QRect {10, 10,  8,  8}, true,  false }

};

// Common patch for mice and other devices
// FIXME: Can we get rid of this? (By making keyboards act the same way.)
static inline void translateToTopLeftCoords(const Key* key, size_t keyLen, QHash<QString, Key>& map) {
    for(size_t i = 0; i < keyLen; i++) {
        // Keyboard keys are written from the center because that's where the LEDs are, but the mouse buttons are odd shapes so they're
        // written from the upper left
#warning "QSize/2 does float division and rounds up"
        QSize boundingRect = key[i].boundingRect().size();
        Key k = key[i];
#warning "FIXME"
        //k.translate(boundingRect.width()/2, boundingRect.height()/2);
        map[key[i].name] = k;
    }
}
#define TRANSLATE_TO_TOP_LEFT_COORDS(k) translateToTopLeftCoords(k, sizeof(k)/sizeof(*k), map)

static inline void replaceRwinWithFn(QHash<QString, Key>& map) {
    map["fn"] = KStrafeKeys[3];
    // This check is needed because jp layouts have no rwin
    if(map.contains("rwin")) {
        map["fn"].setRectX(map["rwin"].rectX());
        map.remove("rwin");
    }
}
// Map getter. Each model/layout pair only needs to be constructed once; after that, future KeyMaps can copy the existing maps.
#define N_MODELS    KeyMap::_MODEL_MAX
#define N_LAYOUTS   KeyMap::_LAYOUT_MAX
static QHash<QString, Key> K95BaseMap;
static QHash<QString, Key> standardMaps[N_MODELS][N_LAYOUTS];
static QHash<QString, Key> getMap(KeyMap::Model model, KeyMap::Layout layout){
    if(model < 0 || layout < 0 || model >= N_MODELS || layout >= N_LAYOUTS)
        return QHash<QString, Key>();
    // Return the map if it's already filled out
    QHash<QString, Key>& map = standardMaps[model][layout];
    if(!map.empty())
        return map;
    // Otherwise, create it
    switch(model){
    case KeyMap::K95:{
        // The K95 maps serve as bases for all the other keyboards
        // Fetch the master map, or create it if not yet done
        if(K95BaseMap.empty()){
            for(const Key* key = K95Keys; key < K95Keys + KEYCOUNT_K95; key++)
                K95BaseMap[key->name] = *key;
        }
        map = K95BaseMap;
        // Patch the map for the layout
        switch(layout){
        case KeyMap::DK:
            patch(map, patchDK);
            break;
        case KeyMap::EU_DVORAK:
            patch(map, patchDvorak);    // fall through
        case KeyMap::EU:
            patch(map, patchEU);
            break;
        case KeyMap::GB_DVORAK:
        case KeyMap::US_DVORAK:
            patch(map, patchDvorak);
            break;
        case KeyMap::FR:
            patch(map, patchFR);
            break;
        case KeyMap::DE:
            patch(map, patchDE);
            break;
        case KeyMap::IT:
            patch(map, patchIT);
            break;
        case KeyMap::NO:
            patch(map, patchNO);
            break;
        case KeyMap::MX:
            patch(map, patchMX);
            break;
        case KeyMap::PT_BR:
            patch(map, patchPTBR);
            break;
        case KeyMap::ES:
            patch(map, patchES);
            break;
        case KeyMap::SE:
            patch(map, patchSE);
            break;
        case KeyMap::JP:
            /*patch(map, patchJP);
            break;*/
        default:;
            // English QWERTY - no patch needed
        }
        if(KeyMap::isJP(layout))
            patchJP106(map);
        else if(KeyMap::isPTBR(layout))
            patchABNT2(map);
        else if(KeyMap::isISO(layout))
            patchISO(map);
        else
            patchANSI(map);
        // Done! return the map
        break;
    }
    case KeyMap::K95L:{
        // The K95 Legacy map is based on the K95
        // but with the light program button
        map = getMap(KeyMap::K95, layout);
        const Key lghtpgm = {nullptr,  "Lighting Programming", "lghtpgm", QRect {210, 0, NS}, true, true};
        map.insert(lghtpgm.name, lghtpgm);
        break;
    }

    case KeyMap::K95P:{
        // The K95 Platinum map is based on the K95
        map = getMap(KeyMap::K95, layout);

        // Remove excess G keys
        map.remove("g7");
        map.remove("g8");
        map.remove("g9");
        map.remove("g10");
        map.remove("g11");
        map.remove("g12");
        map.remove("g13");
        map.remove("g14");
        map.remove("g15");
        map.remove("g16");
        map.remove("g17");
        map.remove("g18");

        // Place the remaining G keys vertically
        static const char* gkeys[] = {"g1", "g2", "g3", "g4", "g5", "g6"};
        for(int i = 0; i < (int)(sizeof(gkeys)/sizeof(*gkeys)); i++)
            map[gkeys[i]].setRectPos(22, 14+(12*i));

        // Remove M keys
        map.remove("m1");
        map.remove("m2");
        map.remove("m3");
        map.remove("mr");

        // Add profile button
        map["profswitch"] = {nullptr,  "Profile Switch", "profswitch", QRect {64, 1, 11, 8}, true, true};

        // Centre Brightness/Winlock between F1 and F4
        map["light"].setRectX(75);
        map["lock"].setRectX(86);

        // Resize them
        map["light"].setRectHeight(8);
        map["lock"].setRectHeight(8);
        map["light"].setRectWidth(11);
        map["lock"].setRectWidth(11);

        // Mute is above Stop
        // Also move the buttons on the top a bit further down to centre them
        map["mute"].translate(-11, 1);
        map["voldn"].translate(-11, 1);
        map["volup"].translate(-11, 1);

        map["light"].translate(0, 1);
        map["lock"].translate(0, 1);

        // Shift all keys down (to make room for the lightbar), and to the left
        QMutableHashIterator<QString, Key> i(map);
        while(i.hasNext()){
            i.next();
            i.value().translate(-K95P_X_START, 6);
        }

        // Add lightbar
        for(const Key* key = K95PLbar; key < K95PLbar + LBARCOUNT_K95P; key++)
                map.insert(key->name, *key);

        break;
    }
    case KeyMap::K100:{
        map = getMap(KeyMap::K95P, layout);
        // Shift everything down except the existing topbar
        QMutableHashIterator<QString, Key> i(map);
        while(i.hasNext()){
            i.next();
            if(!i.key().contains("topbar"))
                i.value().translate(0, K100_HEIGHT - K95P_HEIGHT);

             i.value().translate((K100_WIDTH - K95P_WIDTH)/2, 0);
        }
        // Shrink the top lightbar and add the extra three items
        for(int j = 0; j < 19; j++){
            QString key = QString("topbar%1").arg(j + 1);
            map[key].setRectWidth(15); // maybe 15
            map[key].translate(-(j * 2 + 1), 0);
        }
        map["topbar20"] = {nullptr, "Top Light Bar 20", "topbar20", QRect {255, -3, 15, 6}, true, false};
        map["topbar21"] = {nullptr, "Top Light Bar 21", "topbar21", QRect {268, -3, 15, 6}, true, false};
        map["topbar22"] = {nullptr, "Top Light Bar 22", "topbar22", QRect {281, -3, 15, 6}, true, false};

        // Add the left and right bars
        map["leftbar1"] = {nullptr, "Left Light Bar 1", "leftbar1", QRect {-2, -2, 6, 9}, true, false};
        map["leftbar2"] = {nullptr, "Left Light Bar 2", "leftbar2", QRect {-2, 7, 6, 9}, true, false};
        map["leftbar3"] = {nullptr, "Left Light Bar 3", "leftbar3", QRect {-2, 16, 6, 9}, true, false};
        map["leftbar4"] = {nullptr, "Left Light Bar 4", "leftbar4", QRect {-2, 25, 6, 9}, true, false};
        map["leftbar5"] = {nullptr, "Left Light Bar 5", "leftbar5", QRect {-2, 34, 6, 9}, true, false};
        map["leftbar6"] = {nullptr, "Left Light Bar 6", "leftbar6", QRect {-2, 43, 6, 9}, true, false};
        map["leftbar7"] = {nullptr, "Left Light Bar 7", "leftbar7", QRect {-2, 52, 6, 9}, true, false};
        map["leftbar8"] = {nullptr, "Left Light Bar 8", "leftbar8", QRect {-2, 61, 6, 9}, true, false};
        map["leftbar9"] = {nullptr, "Left Light Bar 9", "leftbar9", QRect {-2, 70, 6, 9}, true, false};
        map["leftbar10"] = {nullptr, "Left Light Bar 10", "leftbar10", QRect {-2, 79, 6, 9}, true, false};
        map["leftbar11"] = {nullptr, "Left Light Bar 11", "leftbar11", QRect {-2, 88, 6, 9}, true, false};

        map["rightbar1"] = {nullptr, "Right Light Bar 1", "rightbar1", QRect {292, -2, 6, 9}, true, false};
        map["rightbar2"] = {nullptr, "Right Light Bar 2", "rightbar2", QRect {292, 7, 6, 9}, true, false};
        map["rightbar3"] = {nullptr, "Right Light Bar 3", "rightbar3", QRect {292, 16, 6, 9}, true, false};
        map["rightbar4"] = {nullptr, "Right Light Bar 4", "rightbar4", QRect {292, 25, 6, 9}, true, false};
        map["rightbar5"] = {nullptr, "Right Light Bar 5", "rightbar5", QRect {292, 34, 6, 9}, true, false};
        map["rightbar6"] = {nullptr, "Right Light Bar 6", "rightbar6", QRect {292, 43, 6, 9}, true, false};
        map["rightbar7"] = {nullptr, "Right Light Bar 7", "rightbar7", QRect {292, 52, 6, 9}, true, false};
        map["rightbar8"] = {nullptr, "Right Light Bar 8", "rightbar8", QRect {292, 61, 6, 9}, true, false};
        map["rightbar9"] = {nullptr, "Right Light Bar 9", "rightbar9", QRect {292, 70, 6, 9}, true, false};
        map["rightbar10"] = {nullptr, "Right Light Bar 10", "rightbar10", QRect {292, 79, 6, 9}, true, false};
        map["rightbar11"] = {nullptr, "Right Light Bar 11", "rightbar11", QRect {292, 88, 6, 9}, true, false};

        map["ctrlwheelb"] = {nullptr, "Control Wheel Button", "ctrlwheelb", map["light"].pos, true, true};
        map.remove("light");
        map["ctrlwheelb"].translate(0, -3);
        map["ctrlwheelb"].setRectHeight(map["ctrlwheelb"].rectWidth());

        map["profswitch"].extendRect(0, 1);
        map["profswitch"].translate(-1, 0);

        map["lock"].setRectHeight(map["profswitch"].rectHeight());
        map["mute"].setRectHeight(map["profswitch"].rectHeight());
        const int ctrlwheelb_y = map["ctrlwheelb"].rectY();
        map["mute"].setRectY(ctrlwheelb_y);
        map["profswitch"].setRectY(ctrlwheelb_y);
        map["lock"].setRectY(ctrlwheelb_y);
        map["volup"].setRectY(ctrlwheelb_y - 2);
        map["voldn"].setRectY(ctrlwheelb_y + 2);
        map["lock"].translate(1, 0);

        map["ctrlwheel1"] = {nullptr, "Control Wheel 22.5°",  "ctrlwheel1", QRect {60+2, 10, 8, 6}, true, false};
        map["ctrlwheel2"] = {nullptr, "Control Wheel 67.5°",  "ctrlwheel2", QRect {60+3, 10+1, 5, 8}, true, false};
        map["ctrlwheel3"] = {nullptr, "Control Wheel 112.5°", "ctrlwheel3", QRect {60+3, 10+2, 5, 8}, true, false};
        map["ctrlwheel4"] = {nullptr, "Control Wheel 157.5°", "ctrlwheel4", QRect {60+2, 10+3, 8, 6}, true, false};
        map["ctrlwheel5"] = {nullptr, "Control Wheel 202.5°", "ctrlwheel5", QRect {60+1, 10+3, 8, 6}, true, false};
        map["ctrlwheel6"] = {nullptr, "Control Wheel 247.5°", "ctrlwheel6", QRect {60, 10+2, 5, 8}, true, false};
        map["ctrlwheel7"] = {nullptr, "Control Wheel 292.5°", "ctrlwheel7", QRect {60, 10+1, 5, 8}, true, false};
        map["ctrlwheel8"] = {nullptr, "Control Wheel 337.5°", "ctrlwheel8", QRect {60+1, 10, 8, 6}, true, false};

        map["logoleft"] = {nullptr, "Logo Left", "logoleft", QRect {134, 10, 10, 10}, true, false};
        map["logo"] = {nullptr, "Logo", "logo", QRect {144, 10, 10, 10}, true, false};
        map["logoright"] = {nullptr, "Logo Right", "logoright", QRect {154, 10, 10, 10}, true, false};

        break;
    }
    case KeyMap::K70:{
        // The K70 maps are based on the K95 maps. However all the keys are shifted left and the G keys are removed
        map = getMap(KeyMap::K95, layout);
        QMutableHashIterator<QString, Key> i(map);
        while(i.hasNext()){
            i.next();
            // Move key to left. Remove it if it fell off the edge
            Key& k = i.value();
            k.translate(-K70_X_START, 0);
            if(k.rectX() < 0)
                i.remove();
        }
        // Remove the M buttons as well
        map.remove("mr");
        map.remove("m1");
        map.remove("m2");
        map.remove("m3");
        // Done!
        break;
    }
    case KeyMap::K70MK2:{
        // Same as K70 but buttons moved to the left, profswitch, and two logo lights (using m3, g11)
        map = getMap(KeyMap::K70, layout);
        // Starting from the right of F2
        // Add profswitch
        map["profswitch"] = {nullptr,  "Profile Switch", "profswitch", QRect {40, 0, 10, 8}, true, true};

        map["light"].setRectX(49);
        map["light"].setRectWidth(10);
        map["light"].setRectHeight(8);

        map["lock"].setRectX(58);
        map["lock"].setRectWidth(10);
        map["lock"].setRectHeight(8);

        // Add logo1 (m3)
        map["m3"] = {nullptr,  "Logo 1", "m3", QRect {118, 0, NS}, true, false};
        // Add logo2 (g11)
        map["g11"] = {nullptr,  "Logo 2", "g11", QRect {138, 0, NS}, true, false};

        // Move mute and volup/dn one block to the left
        map["mute"].translate(-10, 0);
        map["volup"].translate(-10, 0);
        map["voldn"].translate(-10, 0);
        break;
    }
    case KeyMap::K70_TKL:{
        // Same width as the K63 but with a top row more like the K70
        map = getMap(KeyMap::K63, layout);
        for(const Key* key = K70TklTopRow; key < K70TklTopRow + K70_TKL_TOP_COUNT; key++)
            map[key->name] = *key;

        map.remove("rwin");
        map["fn"] = KStrafeKeys[3];
        map["fn"].translate(-12, 0);
        map["light"].setRectX(190 - K70_X_START);
        map["light"].setRectHeight(8);
        map["lock"].setRectX(202 - K70_X_START);
        map["lock"].setRectHeight(8);
        break;
    }
    case KeyMap::STRAFE_MK2:{
        map = getMap(KeyMap::K70MK2, layout);
        // move everything right to make the space for the left sidelight
        QMutableHashIterator<QString, Key> i(map);
        while(i.hasNext()){
            i.next();
            i.value().translate(KSTRAFE_X_START, 0);
        }

        // Move profswitch, light and lwin two blocks to the left
        map["profswitch"].translate(-20, 0);
        map["light"].translate(-20, 0);
        map["lock"].translate(-20, 0);

        map["lsidel"] = KStrafeKeys[0];
        map["rsidel"] = KStrafeKeys[1];
        break;
    }
    case KeyMap::K68:{
        // Same as the K70, except volume up and down keys
        map = getMap(KeyMap::K70, layout);
        for(const Key* key = K68TopRow; key < K68TopRow + K68_TOP_COUNT; key++)
            map[key->name] = *key;

        break;
    }
    case KeyMap::K66:{
        // Same as the K68, except there's no brightness button
        map = getMap(KeyMap::K68, layout);
        map.remove("light");

        break;
    }
    case KeyMap::K65:{
        // The K65 maps additionally remove the numpad and have a modified top row
        map = getMap(KeyMap::K70, layout);
        QMutableHashIterator<QString, Key> i(map);
        while(i.hasNext()){
            i.next();
            if(i.value().rectX() >= K65_WIDTH)
                i.remove();
        }
        for(const Key* key = K65TopRow; key < K65TopRow + K65_TOP_COUNT; key++)
            map[key->name] = *key;

        map.remove("rwin");
        map["fn"] = KStrafeKeys[3];
        map["fn"].translate(-12, 0);

        // Done!
        break;
    }
    case KeyMap::K65_MINI:{
        map = getMap(KeyMap::K65, layout);

        // Move Esc so that it doesn't get deleted
        map["esc"].translate(0, 13);
        map.remove("grave");

        // Remove the whole top bar and shift everything up
        QMutableHashIterator<QString, Key> i(map);
        while(i.hasNext()){
            i.next();

            i.value().translate(-3, -27);
            if(i.value().rectY() < 0)
                i.remove();
            else if(i.value().rectX() > K65_MINI_WIDTH + 2)
                i.remove();
        }

        break;
    }
    case KeyMap::K63:{
        // Similar to the K65 but without the Fn key and a modified top row
        map = getMap(KeyMap::K70, layout);
        QMutableHashIterator<QString, Key> i(map);
        while(i.hasNext()){
            i.next();
            if(i.value().rectX() >= K65_WIDTH)
                i.remove();
        }
        for(const Key* key = K63TopRow; key < K63TopRow + K63_TOP_COUNT; key++)
            map[key->name] = *key;

        break;
    }
    case KeyMap::K63_WL:{
        // Same as K63 but with Fn
        map = getMap(KeyMap::K63, layout);

        map.remove("rwin");
        map["fn"] = KStrafeKeys[3];
        map["fn"].translate(-12, 0);

        break;
    }
    case KeyMap::K60:{
        map = getMap(KeyMap::K70, layout);
        map.remove("light");
        map.remove("lock");
        map.remove("mute");
        map.remove("volup");
        map.remove("voldn");
        map.remove("stop");
        map.remove("prev");
        map.remove("play");
        map.remove("next");

        // Replace rwin with Fn
        replaceRwinWithFn(map);

        QMutableHashIterator<QString, Key> i(map);
        while(i.hasNext()){
            i.next();
            i.value().translate(0, -14);
        }

        break;
    }
    case KeyMap::K60_TKL:{
        map = getMap(KeyMap::K63, layout);
        map.remove("light");
        map.remove("lock");
        map.remove("mute");
        map.remove("volup");
        map.remove("voldn");
        map.remove("stop");
        map.remove("prev");
        map.remove("play");
        map.remove("next");

        // Replace rwin with Fn
        replaceRwinWithFn(map);

        QMutableHashIterator<QString, Key> i(map);
        while(i.hasNext()){
            i.next();
            i.value().translate(0, -14);
        }
        break;
    }
    case KeyMap::K57_WL:{
        // Take the K95 map
        map = getMap(KeyMap::K95, layout);

        // Replace rwin with Fn
        replaceRwinWithFn(map);

        // Replace volume wheel
        map["voldn"] = {nullptr,  "Volume Down", "voldn", map["mute"].pos.translated(12, 0), true, true};
        map["volup"] = {nullptr,  "Volume Up", "volup", map["mute"].pos.translated(24, 0), true, true};

        // Fix up the G keys
        map.remove("g7");
        map.remove("g8");
        map.remove("g9");
        map.remove("g10");
        map.remove("g11");
        map.remove("g12");
        map.remove("g13");
        map.remove("g14");
        map.remove("g15");
        map.remove("g16");
        map.remove("g17");
        map.remove("g18");

        // Place the remaining G keys vertically
        static const char* gkeys[] = {"g1", "g2", "g3", "g4", "g5", "g6"};
        for(int i = 0; i < (int)(sizeof(gkeys)/sizeof(*gkeys)); i++)
            map[gkeys[i]].setRectPos(22, 14+(12*i));

        // Remove M keys
        map.remove("m1");
        map.remove("m2");
        map.remove("m3");

        // Move MR to the left of brightness
        map["mr"].pos = map["light"].pos.translated(-12, 0);

        // Shift all keys down (to make room for the lightbar), and to the left
        QMutableHashIterator<QString, Key> i(map);
        while(i.hasNext()){
            i.next();
            i.value().translate(-K95P_X_START, 0);
        }

        break;
    }
    case KeyMap::K55:{
        // The K55 map is based on the K95
        map = getMap(KeyMap::K95, layout);
        // Remove excess G keys
        map.remove("g7");
        map.remove("g8");
        map.remove("g9");
        map.remove("g10");
        map.remove("g11");
        map.remove("g12");
        map.remove("g13");
        map.remove("g14");
        map.remove("g15");
        map.remove("g16");
        map.remove("g17");
        map.remove("g18");

        // Place the remaining G keys vertically
        static const char* gkeys[] = {"g1", "g2", "g3", "g4", "g5", "g6"};
        for(int i = 0; i < (int)(sizeof(gkeys)/sizeof(*gkeys)); i++)
            map[gkeys[i]].setRectPos(22, 14+(12*i));

        // Remove M keys
        map.remove("m1");
        map.remove("m2");
        map.remove("m3");
        // Move MR to the left of the brightness button
        map["mr"].setRectX(210);
        // Resize and move Volume Up/Down
        map["voldn"].pos = QPolygon(QRect(285, 0, 13, 8));
        map["volup"].pos = QPolygon(QRect(297, 0, 13, 8));
        // Replace rwin with Fn
        replaceRwinWithFn(map);
        // Shift all keys to the left, and disable their LEDs
        QMutableHashIterator<QString, Key> i(map);
        while(i.hasNext()){
            i.next();
            i.value().translate(-K95P_X_START, 0);
            i.value().hasLed = false;
        }

        // https://github.com/ckb-next/ckb-next/issues/931
        map["lock"].hasLed = true;

        // Append the zones to the keymap
        for(const Key* key = K55Zones; key < K55Zones + K55_ZONES; key++)
            map[key->name] = *key;
        break;
    }
    case KeyMap::STRAFE:{
        // The Strafe RGB maps are based on the K70 map minus the media keys
        map = getMap(KeyMap::K70, layout);
        //move light and lock right
        map["light"].setRectX(285 - K70_X_START);
        //map["light"].hasLed=false;
        map["lock"].setRectX(297 - K70_X_START);
        //map["lock"].hasLed=false;
        // move everything right to make the space for the left sidelight
        QMutableHashIterator<QString, Key> i(map);
        while(i.hasNext()){
            i.next();
            i.value().translate(KSTRAFE_X_START, 0);
        }
        // Add Strafe lights and keys
        map["lsidel"] = KStrafeKeys[0];
        map["rsidel"] = KStrafeKeys[1];
        map["logo"] = KStrafeKeys[2];
        map["fn"] = KStrafeKeys[3];
        map.remove("rwin");
        // remove media controls
        map.remove("mute");
        map.remove("volup");
        map.remove("voldn");
        map.remove("stop");
        map.remove("prev");
        map.remove("play");
        map.remove("next");

        // Done!
        break;
    }
    case KeyMap::K55PRO:{
        // Keys are the same as the K55, but has 5 zones instead
        map = getMap(KeyMap::K55, layout);

        // Overwrite the zones
        for(const Key* key = K55PROZones; key < K55PROZones + K55PRO_ZONES; key++)
            map[key->name] = *key;
        break;
    }
    case KeyMap::M65:
        TRANSLATE_TO_TOP_LEFT_COORDS(M65Keys);
        break;

    case KeyMap::SABRE:
        TRANSLATE_TO_TOP_LEFT_COORDS(SabreKeys);
        break;

    case KeyMap::SCIMITAR:
        TRANSLATE_TO_TOP_LEFT_COORDS(ScimKeys);
        break;

    case KeyMap::HARPOON:
        TRANSLATE_TO_TOP_LEFT_COORDS(HarpoonKeys);
        break;

    case KeyMap::GLAIVE:
        TRANSLATE_TO_TOP_LEFT_COORDS(GlaiveKeys);
        break;

    case KeyMap::KATAR:
        TRANSLATE_TO_TOP_LEFT_COORDS(KatarKeys);
        break;

    case KeyMap::KATARPROXT:
        TRANSLATE_TO_TOP_LEFT_COORDS(KatarPROXTKeys);
        break;

    case KeyMap::DARKCORE:
        TRANSLATE_TO_TOP_LEFT_COORDS(DarkCoreKeys);
        break;

    case KeyMap::DARKCORERGBPRO:
        TRANSLATE_TO_TOP_LEFT_COORDS(DarkCoreRgbProKeys);
        break;

    case KeyMap::POLARIS:
        TRANSLATE_TO_TOP_LEFT_COORDS(PolarisZones);
        break;

    case KeyMap::MM700:
        TRANSLATE_TO_TOP_LEFT_COORDS(MM700Zones);
        break;

    case KeyMap::ST100:
        TRANSLATE_TO_TOP_LEFT_COORDS(ST100Zones);
        break;

    // Same as M65 but with the wheel zone instead of the front one
    case KeyMap::M65E:
        map = getMap(KeyMap::M65, layout);
        map["wheel"] = {nullptr,  "Wheel Light", "wheel", map["front"].pos, true, false};
        map.remove("front");
        break;

    case KeyMap::M95:
        TRANSLATE_TO_TOP_LEFT_COORDS(M95Keys);
        break;

    case KeyMap::IRONCLAW:
        TRANSLATE_TO_TOP_LEFT_COORDS(IronclawKeys);
        break;

    case KeyMap::NIGHTSWORD:
        TRANSLATE_TO_TOP_LEFT_COORDS(NightswordKeys);
        break;

    case KeyMap::IRONCLAW_W:
        TRANSLATE_TO_TOP_LEFT_COORDS(IronclawWirelessKeys);
        break;

    case KeyMap::GLAIVEPRO:
        // Based on the Glaive
        map = getMap(KeyMap::GLAIVE, layout);
        // Change dpiup back to DPI Up, and add dpidn
        map["dpiup"]._friendlyName = "DPI Up";
        map["dpiup"].setRectHeight(8);
        map["dpiup"].translate(0, -2);
        map["dpidn"] = {nullptr,  "DPI Down", "dpidn", QRect {34, 29, 6, 8}, false, true};
        break;

    case KeyMap::M55:
        TRANSLATE_TO_TOP_LEFT_COORDS(M55Keys);
        break;
    default:;    // <- stop GCC from complaining
    }

    if(KeyMap::isJP(layout))
        patchJP106fn(map);

    // test
    QMutableHashIterator<QString, Key> i(map);
    while(i.hasNext()){
        i.next();
        i.value().setRectHeight(i.value().rectHeight()-2);
        i.value().setRectWidth(i.value().rectWidth()-2);
    }

    // Map is finished, return result
    return map;
}

KeyMap::Layout KeyMap::locale(QList<QPair<int, QString>>* layouts){
    setlocale(LC_ALL, "");
    QString loc = setlocale(LC_CTYPE, nullptr);
    loc = loc.toLower().replace('_', '-');

    KeyMap::Layout layout = KeyMap::NO_LAYOUT;

    if(loc.startsWith("dk-"))
        layout = KeyMap::DK;
    else if(loc.startsWith("fr-"))
        layout = KeyMap::FR;
    else if(loc.startsWith("de-"))
        layout = KeyMap::DE;
    else if(loc.startsWith("it-"))
        layout = KeyMap::IT;
    else if(loc.startsWith("ja-"))
        layout = KeyMap::JP;
    else if(loc.startsWith("pl-"))
        layout = KeyMap::PL;
    else if(loc.startsWith("pt-br"))
        layout = KeyMap::PT_BR;
    else if(loc.startsWith("no-"))
        layout = KeyMap::NO;
    else if(loc.startsWith("es-es"))
        // Spain uses the ES layout
        layout = KeyMap::ES;
    else if(loc.startsWith("es-"))
        // Other Spanish locales use MX
        layout = KeyMap::MX;
    else if(loc.startsWith("se-"))
        layout = KeyMap::SE;
    else if(loc.startsWith("en-us") || loc.startsWith("en-au") || loc.startsWith("en-ca") || loc.startsWith("en-hk") || loc.startsWith("en-in") || loc.startsWith("en-nz") || loc.startsWith("en-ph") || loc.startsWith("en-sg") || loc.startsWith("en-za"))
        // Most English locales use US
        layout = KeyMap::US;
    else if(loc.startsWith("en-gb"))
        layout = KeyMap::GB;

    // Check if the hardware supports the detected layout
    for(int i = 0; i < layouts->count(); i++)
        if(layouts->at(i).first == (int)layout)
            return layout;

    // If no matching layout was found, pick one from the list
    KeyMap::Layout retlayout = (KeyMap::Layout)layouts->at(0).first;
    // Pick English as the default ISO layout
    if(retlayout == KeyMap::DK)
        retlayout = KeyMap::EU;

    return retlayout;
}

KeyMap::Layout KeyMap::getLayout(const QString& name){
    QString lower = name.toLower();
    if(lower == "dk")
        return DK;
    if(lower == "eu")
        return EU;
    if(lower == "eu_dvorak")
        return EU_DVORAK;
    if(lower == "gb_dvorak")
        return GB_DVORAK;
    if(lower == "us")
        return US;
    if(lower == "us_dvorak")
        return US_DVORAK;
    if(lower == "fr")
        return FR;
    if(lower == "de")
        return DE;
    if(lower == "it")
        return IT;
    if(lower == "no")
        return NO;
    if(lower == "jp")
        return JP;
    if(lower == "pl")
        return PL;
    if(lower == "pt_br")
        return PT_BR;
    if(lower == "mx")
        return MX;
    if(lower == "es")
        return ES;
    if(lower == "se")
        return SE;
    if(lower == "gb")
        return GB;
    return NO_LAYOUT;
}

QString KeyMap::getLayout(KeyMap::Layout layout){
    switch(layout){
    case DK:
        return "dk";
    case EU:
        return "eu";
    case EU_DVORAK:
        return "eu_dvorak";
    case GB:
        return "gb";
    case GB_DVORAK:
        return "gb_dvorak";
    case US:
        return "us";
    case US_DVORAK:
        return "us_dvorak";
    case FR:
        return "fr";
    case DE:
        return "de";
    case IT:
        return "it";
    case JP:
        return "jp";
    case NO:
        return "no";
    case PL:
        return "pl";
    case PT_BR:
        return "pt_br";
    case MX:
        return "mx";
    case ES:
        return "es";
    case SE:
        return "se";
    default:
        return "";
    }
}

QPair<int, QString> KeyMap::addToList(int i, const QStringList& list){
    return QPair<int, QString>(i, list.at(i));
}

const QStringList KeyMap::layoutList {
    "Danish", "English (ISO/European)", "English (ISO/European, Dvorak)", "English (United Kingdom)", "English (United Kingdom, Dvorak)",
     "English (United States)", "English (United States, Dvorak)", "French", "German", "Italian", "Japanese", "Norwegian", "Polish",
     "Portuguese (Brazil)", "Spanish (Latin America)", "Spanish (Spain)", "Swedish"
};
QList<QPair<int, QString>> KeyMap::layoutNames(const QString& layout){

    // Create a list containing the layouts supported by the current device
    QList<QPair<int, QString>> retlist;
    if(layout == "ansi")
        retlist << KeyMap::addToList(5, layoutList)
                << KeyMap::addToList(6, layoutList);
    else if(layout == "iso")
        retlist << KeyMap::addToList(0, layoutList)
                << KeyMap::addToList(1, layoutList)
                << KeyMap::addToList(2, layoutList)
                << KeyMap::addToList(3, layoutList)
                << KeyMap::addToList(4, layoutList)
                << KeyMap::addToList(7, layoutList)
                << KeyMap::addToList(8, layoutList)
                << KeyMap::addToList(9, layoutList)
                << KeyMap::addToList(11, layoutList)
                << KeyMap::addToList(12, layoutList)
                << KeyMap::addToList(14, layoutList)
                << KeyMap::addToList(15, layoutList)
                << KeyMap::addToList(16, layoutList);
    else if(layout == "abnt")
        retlist << KeyMap::addToList(13, layoutList);
    else if(layout == "jis")
        retlist << KeyMap::addToList(10, layoutList);
    else
        for(int i = 0; i < layoutList.count(); i++)
            retlist << KeyMap::addToList(i, layoutList);
    return retlist;
}

KeyMap::Model KeyMap::getModel(const QString& name){
    QString lower = name.toLower();
    if(lower == "k55")
        return K55;
    if(lower == "k57_wireless")
        return K57_WL;
    if(lower == "k60")
        return K60;
    if(lower == "k60_tkl")
        return K60_TKL;
    if(lower == "k63")
        return K63;
    if(lower == "k63_wireless")
        return K63_WL;
    if(lower == "k65")
        return K65;
    if(lower == "k66")
        return K66;
    if(lower == "k68")
        return K68;
    if(lower == "k70")
        return K70;
    if(lower == "k95")
        return K95;
    if(lower == "k95p")
        return K95P;
    if(lower == "strafe")
        return STRAFE;
    if(lower == "m55")
        return M55;
    if(lower == "m65")
        return M65;
    if(lower == "sabre")
        return SABRE;
    if(lower == "scimitar")
        return SCIMITAR;
    if(lower == "harpoon")
        return HARPOON;
    if(lower == "glaive")
        return GLAIVE;
    if(lower == "katar")
        return KATAR;
    if(lower == "katarproxt")
        return KATARPROXT;
    if(lower == "darkcore")
        return DARKCORE;
    if(lower == "dark_core_rgb_pro")
        return DARKCORERGBPRO;
    if(lower == "polaris")
        return POLARIS;
    if(lower == "mm700")
        return MM700;
    if(lower == "st100")
        return ST100;
    if(lower == "k70mk2")
        return K70MK2;
    if(lower == "k70tkl")
        return K70_TKL;
    if(lower == "strafe_mk2")
        return STRAFE_MK2;
    if(lower == "m65e")
        return M65E;
    if(lower == "m95")
        return M95;
    if(lower == "ironclaw")
        return IRONCLAW;
    if(lower == "nightsword")
        return NIGHTSWORD;
    if(lower == "ironclaw_wireless")
        return IRONCLAW_W;
    if(lower == "k95l")
        return K95L;
    if(lower == "glaivepro")
        return GLAIVEPRO;
    if(lower == "k55pro")
        return K55PRO;
    if(lower == "bragi_dongle")
        return BRAGI_DONGLE;
    if(lower == "k100")
        return K100;
    if(lower == "k65_mini")
        return K65_MINI;
    return NO_MODEL;
}

QString KeyMap::getModel(KeyMap::Model model){
    switch(model){
    case K55:
        return "k55";
    case K55PRO:
        return "k55pro";
    case K57_WL:
        return "k57_wireless";
    case K60:
        return "k60";
    case K60_TKL:
        return "k60_tkl";
    case K63:
        return "k63";
    case K63_WL:
        return "k63_wireless";
    case K65:
        return "k65";
    case K68:
        return "k68";
    case K66:
        return "k66";
    case K70:
        return "k70";
    case K95:
        return "k95";
    case K95P:
        return "k95P";
    case STRAFE:
        return "strafe";
    case M55:
        return "m55";
    case M65:
        return "m65";
    case SABRE:
        return "sabre";
    case SCIMITAR:
        return "scimitar";
    case HARPOON:
        return "harpoon";
    case GLAIVE:
        return "glaive";
    case KATAR:
        return "katar";
    case KATARPROXT:
        return "katarproxt";
    case DARKCORE:
        return "darkcore";
    case DARKCORERGBPRO:
        return "dark_core_rgb_pro";
    case POLARIS:
        return "polaris";
    case MM700:
        return "mm700";
    case ST100:
        return "st100";
    case K70MK2:
        return "k70mk2";
    case K70_TKL:
        return "k70tkl";
    case STRAFE_MK2:
        return "strafe_mk2";
    case M65E:
        return "m65e";
    case M95:
        return "m95";
    case IRONCLAW:
        return "ironclaw";
    case NIGHTSWORD:
        return "nightsword";
    case IRONCLAW_W:
        return "ironclaw_wireless";
    case K95L:
        return "k95l";
    case GLAIVEPRO:
        return "glaivepro";
    case BRAGI_DONGLE:
        return "bragi_dongle";
    case K100:
        return "k100";
    case K65_MINI:
        return "k65_mini";
    default:
        return "";
    }
}

KeyMap KeyMap::fromName(const QString &name){
    QStringList list = name.trimmed().split(" ");
    if(list.length() != 2)
        return KeyMap();
    return KeyMap(getModel(list[0]), getLayout(list[1]));
}

int KeyMap::modelWidth(Model model){
    switch(model){
    case K60:
        return K60_WIDTH;
    case K60_TKL:
        return K60_TKL_WIDTH;
    case K63:
    case K63_WL:
    case K70_TKL:
        return K63_WIDTH;
    case K65:
        return K65_WIDTH;
    case K65_MINI:
        return K65_MINI_WIDTH;
    case K66:
    case K68:
        return K68_WIDTH;
    case K70:
    case K70MK2:
        return K70_WIDTH;
    case K95:
    case K95L:
         return K95_WIDTH;
    case K55PRO:
        return K95P_WIDTH + 1; // FIXME
    case K95P:
    case K55:
    case K57_WL:
        return K95P_WIDTH;
    case K100:
        return K100_WIDTH;
    case STRAFE:
    case STRAFE_MK2:
        return KSTRAFE_WIDTH;
    case SCIMITAR:
        return SCIM_WIDTH;
    case M95:
        return M95_WIDTH;
    case M55:
    case M65:
    case M65E:
    case SABRE:
    case HARPOON:
    case GLAIVE:
    case KATAR:
    case KATARPROXT:
    case DARKCORE:
    case DARKCORERGBPRO:
    case POLARIS:
    case ST100:
    case IRONCLAW:
    case NIGHTSWORD:
    case GLAIVEPRO:
    case IRONCLAW_W:
        return M65_WIDTH;
    case MM700:
        return MM700_WIDTH;
    default:
        return 0;
    }
}

int KeyMap::modelHeight(Model model){
    switch(model){
    case K55:
    case K55PRO:
    case K57_WL:
    case K63:
    case K63_WL:
    case K65:
    case K66:
    case K68:
    case K70:
    case K70MK2:
    case K70_TKL:
    case K95:
    case K95L:
    case STRAFE:
    case STRAFE_MK2:
        return K95_HEIGHT;
    case K95P:
        return K95P_HEIGHT;
    case K100:
        return K100_HEIGHT;
    case K60:
        return K60_HEIGHT;
    case K60_TKL:
        return K60_TKL_HEIGHT;
    case K65_MINI:
        return K65_MINI_HEIGHT;
    case M55:
    case M65:
    case M65E:
    case SABRE:
    case SCIMITAR:
    case HARPOON:
    case GLAIVE:
    case KATAR:
    case KATARPROXT:
    case DARKCORE:
    case DARKCORERGBPRO:
    case POLARIS:
    case ST100:
    case M95:
    case IRONCLAW:
    case NIGHTSWORD:
    case IRONCLAW_W:
        return M65_HEIGHT;
    case MM700:
        return MM700_HEIGHT;
    default:
        return 0;
    }
}

KeyMap::KeyMap(Model _keyModel, Layout _keyLayout) :
    _keys(getMap(_keyModel, _keyLayout)),
    keyModel(_keyModel), keyLayout(_keyLayout),
    keyWidth(modelWidth(_keyModel)), keyHeight(modelHeight(_keyModel))
{}

KeyMap::KeyMap() :
     keyModel(NO_MODEL), keyLayout(NO_LAYOUT),
     keyWidth(0), keyHeight(0)
{}

QStringList KeyMap::byPosition() const {
    // Use QMaps to order the keys
    QMap<int, QMap<int, QString> > ordered;
    for(const Key& key: *this) {
        const QRect boundingRect = key.boundingRect();
        ordered[boundingRect.y()][boundingRect.x()] = key.name;
    }

    // Merge them into a single list
    QStringList result;
    QMapIterator<int, QMap<int, QString> > y(ordered);
    while(y.hasNext()){
        y.next();
        QMapIterator<int, QString> x(y.value());
        while(x.hasNext()){
            x.next();
            result << x.value();
        }
    }
    return result;
}

QString KeyMap::friendlyName(const QString& key, Layout layout){
    // Try K95 map first
    // FIXME: This is an odd function and probably should be refactored
    // it would probably be best to remove the friendly names from the maps and have a completely separate name->friendlyName store
    KeyMap map(K95, layout);
    if(map.contains(key))
        return map.key(key).friendlyName();

    // The only key missing from it should be Fn, which is found on STRAFE
    map = KeyMap(STRAFE, layout);
    if(map.contains(key))
        return map.key(key).friendlyName();

    // Light Program for the legacy K95
    map = KeyMap(K95L, layout);
    if(map.contains(key))
        return map.key(key).friendlyName();

    // Additionally, there are a handful of keys not present on any physical keyboard, but we need names for them
    if(key == "f13" || key == "f14" || key == "f15" || key == "f16" || key == "f17" || key == "f18" || key == "f19" || key == "f20"
        || key == "f21" || key == "f22" || key == "f23" || key == "f24")
        return key.toUpper();
    else if(key == "lightup")
        return "Screen Brightness Up";
    else if(key == "lightdn")
        return "Screen Brightness Down";
    else if(key == "eject")
        return QObject::tr("Eject");
    else if(key == "power")
        return QObject::tr("Power");
    else if(key == "wheellf")
        return QObject::tr("Wheel Left");
    else if(key == "wheelrg")
        return QObject::tr("Wheel Right");

    // All other names are found on mice
    map = KeyMap(SCIMITAR, layout);
    if(map.contains(key))
        return map.key(key).friendlyName();
    map = KeyMap(M65, layout);
    if(map.contains(key))
        return map.key(key).friendlyName();
    map = KeyMap(HARPOON, layout);
    if(map.contains(key))
        return map.key(key).friendlyName();
    map = KeyMap(IRONCLAW, layout);
    if(map.contains(key))
        return map.key(key).friendlyName();
    map = KeyMap(NIGHTSWORD, layout);
    if(map.contains(key))
        return map.key(key).friendlyName();
    map = KeyMap(IRONCLAW_W, layout);
    if(map.contains(key))
        return map.key(key).friendlyName();

    // Not found at all
    return "";
}

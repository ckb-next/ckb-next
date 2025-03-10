#include <clocale>
#include <QMap>
#include "keymap.h"
#include <QObject>

const Key KeyMap::emptyKey = {nullptr,nullptr,nullptr,0,0,0,0,0,0};

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
    {nullptr, nullptr, "mr", 38, 0, NS, true, true}, {nullptr, nullptr, "m1", 50, 0, NS, true, true}, {nullptr, nullptr, "m2", 62, 0, NS, true, true}, {nullptr, nullptr, "m3", 74, 0, NS, true, true}, {nullptr,  "Brightness", "light", 222, 0, NS, true, true}, {nullptr,  "Windows Lock", "lock", 234, 0, NS, true, true}, {nullptr,  "Mute", "mute", 273, 0, 13, 8, true, true}, {nullptr,  "Volume Up", "volup", 290, -2, 18, 6, false, true}, {nullptr,  "Volume down", "voldn", 290, 2, 18, 6, false, true},
    {nullptr, nullptr, "g1", 0, 14, NS, true, true}, {nullptr, nullptr, "g2", 11, 14, NS, true, true}, {nullptr, nullptr, "g3", 22, 14, NS, true, true}, {nullptr,  "Esc", "esc", 38, 14, NS, true, true}, {nullptr, nullptr, "f1", 58, 14, NS, true, true}, {nullptr, nullptr, "f2", 70, 14, NS, true, true}, {nullptr, nullptr, "f3", 82, 14, NS, true, true}, {nullptr, nullptr, "f4", 94, 14, NS, true, true}, {nullptr, nullptr, "f5", 114, 14, NS, true, true}, {nullptr, nullptr, "f6", 126, 14, NS, true, true}, {nullptr, nullptr, "f7", 138, 14, NS, true, true}, {nullptr, nullptr, "f8", 150, 14, NS, true, true}, {nullptr, nullptr, "f9", 170, 14, NS, true, true}, {nullptr, nullptr, "f10", 182, 14, NS, true, true}, {nullptr, nullptr, "f11", 194, 14, NS, true, true}, {nullptr, nullptr, "f12", 206, 14, NS, true, true}, {nullptr,  "Print Screen\nSysRq", "prtscn", 222, 14, NS, true, true}, {nullptr,  "Scroll Lock", "scroll", 234, 14, NS, true, true}, {nullptr,  "Pause\nBreak", "pause", 246, 14, NS, true, true}, {nullptr,  "Stop", "stop", 262, 14, 12, 8, true, true}, {nullptr,  "Previous", "prev", 273, 14, 13, 8, true, true}, {nullptr,  "Play/Pause", "play", 285, 14, 13, 8, true, true}, {nullptr,  "Next", "next", 296, 14, 12, 8, true, true},
    {nullptr, nullptr, "g4", 0, 25, NS, true, true}, {nullptr, nullptr, "g5", 11, 25, NS, true, true}, {nullptr, nullptr, "g6", 22, 25, NS, true, true}, {nullptr,  "`", "grave", 38, 27, NS, true, true}, {nullptr, nullptr, "1", 50, 27, NS, true, true}, {nullptr, nullptr, "2", 62, 27, NS, true, true}, {nullptr, nullptr, "3", 74, 27, NS, true, true}, {nullptr, nullptr, "4", 86, 27, NS, true, true}, {nullptr, nullptr, "5", 98, 27, NS, true, true}, {nullptr, nullptr, "6", 110, 27, NS, true, true}, {nullptr, nullptr, "7", 122, 27, NS, true, true}, {nullptr, nullptr, "8", 134, 27, NS, true, true}, {nullptr, nullptr, "9", 146, 27, NS, true, true}, {nullptr, nullptr, "0", 158, 27, NS, true, true}, {nullptr,  "-", "minus", 170, 27, NS, true, true}, {nullptr,  "=", "equal", 182, 27, NS, true, true}, {nullptr,  "¥", "yen", 194, 27, NS, true, true}, {nullptr,  "Backspace", "bspace", 200, 27, 24, 12, true, true}, {nullptr,  "Insert", "ins", 222, 27, NS, true, true}, {nullptr,  "Home", "home", 234, 27, NS, true, true}, {nullptr,  "Page Up", "pgup", 246, 27, NS, true, true}, {nullptr,  "Num Lock", "numlock", 261, 27, NS, true, true}, {nullptr,  "NumPad /", "numslash", 273, 27, NS, true, true}, {nullptr,  "NumPad *", "numstar", 285, 27, NS, true, true}, {nullptr,  "NumPad -", "numminus", 297, 27, NS, true, true},
    {nullptr, nullptr, "g7", 0, 39, NS, true, true}, {nullptr, nullptr, "g8", 11, 39, NS, true, true}, {nullptr, nullptr, "g9", 22, 39, NS, true, true}, {nullptr,  "Tab", "tab", 41, 39, 18, 12, true, true}, {nullptr, nullptr, "q", 56, 39, NS, true, true}, {nullptr, nullptr, "w", 68, 39, NS, true, true}, {nullptr, nullptr, "e", 80, 39, NS, true, true}, {nullptr, nullptr, "r", 92, 39, NS, true, true}, {nullptr, nullptr, "t", 104, 39, NS, true, true}, {nullptr, nullptr, "y", 116, 39, NS, true, true}, {nullptr, nullptr, "u", 128, 39, NS, true, true}, {nullptr, nullptr, "i", 140, 39, NS, true, true}, {nullptr, nullptr, "o", 152, 39, NS, true, true}, {nullptr, nullptr, "p", 164, 39, NS, true, true}, {nullptr,  "[", "lbrace", 176, 39, NS, true, true}, {nullptr,  "]", "rbrace", 188, 39, NS, true, true}, {nullptr,  "\\", "bslash", 203, 39, 18, 12, true, true}, {nullptr,  "Enter", "enter", 203, 45, 18, 24, true, true}, {nullptr,  "Delete", "del", 222, 39, NS, true, true}, {nullptr,  "End", "end", 234, 39, NS, true, true}, {nullptr,  "Page Down", "pgdn", 246, 39, NS, true, true}, {nullptr,  "NumPad 7", "num7", 261, 39, NS, true, true}, {nullptr,  "NumPad 8", "num8", 273, 39, NS, true, true}, {nullptr,  "NumPad 9", "num9", 285, 39, NS, true, true}, {nullptr,  "NumPad +", "numplus", 297, 45, 12, 24, true, true},
    {nullptr, nullptr, "g10", 0, 50, NS, true, true}, {nullptr, nullptr, "g11", 11, 50, NS, true, true}, {nullptr, nullptr, "g12", 22, 50, NS, true, true}, {nullptr,  "Caps Lock", "caps", 42, 51, 20, 12, true, true}, {nullptr, nullptr, "a", 59, 51, NS, true, true}, {nullptr, nullptr, "s", 71, 51, NS, true, true}, {nullptr, nullptr, "d", 83, 51, NS, true, true}, {nullptr, nullptr, "f", 95, 51, NS, true, true}, {nullptr, nullptr, "g", 107, 51, NS, true, true}, {nullptr, nullptr, "h", 119, 51, NS, true, true}, {nullptr, nullptr, "j", 131, 51, NS, true, true}, {nullptr, nullptr, "k", 143, 51, NS, true, true}, {nullptr, nullptr, "l", 155, 51, NS, true, true}, {nullptr,  ";", "colon", 167, 51, NS, true, true}, {nullptr,  "'", "quote", 179, 51, NS, true, true}, {nullptr,  "#", "hash", 191, 51, NS, true, true}, {nullptr,  "NumPad 4", "num4", 261, 51, NS, true, true}, {nullptr,  "NumPad 5", "num5", 273, 51, NS, true, true}, {nullptr,  "NumPad 6", "num6", 285, 51, NS, true, true},
    {nullptr, nullptr, "g13", 0, 64, NS, true, true}, {nullptr, nullptr, "g14", 11, 64, NS, true, true}, {nullptr, nullptr, "g15", 22, 64, NS, true, true}, {nullptr,  "Left Shift", "lshift", 39, 63, 14, 12, true, true}, {"bslash", "\\", "bslash_iso", 53, 63, NS, true, true}, {nullptr, nullptr, "z", 65, 63, NS, true, true}, {nullptr, nullptr, "x", 77, 63, NS, true, true}, {nullptr, nullptr, "c", 89, 63, NS, true, true}, {nullptr, nullptr, "v", 101, 63, NS, true, true}, {nullptr, nullptr, "b", 113, 63, NS, true, true}, {nullptr, nullptr, "n", 125, 63, NS, true, true}, {nullptr, nullptr, "m", 137, 63, NS, true, true}, {nullptr,  ",", "comma", 149, 63, NS, true, true}, {nullptr,  ".", "dot", 161, 63, NS, true, true}, {nullptr,  "/", "slash", 173, 63, NS, true, true}, {nullptr,  "_", "ro", 185, 63, NS, true, true}, {nullptr,  "Right Shift", "rshift", 196, 63, 32, 12, true, true}, {nullptr,  "Up", "up", 234, 63, NS, true, true}, {nullptr,  "NumPad 1", "num1", 261, 63, NS, true, true}, {nullptr,  "NumPad 2", "num2", 273, 63, NS, true, true}, {nullptr,  "NumPad 3", "num3", 285, 63, NS, true, true}, {nullptr,  "NumPad Enter", "numenter", 297, 69, 12, 24, true, true},
    {nullptr, nullptr, "g16", 0, 75, NS, true, true}, {nullptr, nullptr, "g17", 11, 75, NS, true, true}, {nullptr, nullptr, "g18", 22, 75, NS, true, true}, {nullptr,  "Left Ctrl", "lctrl", 40, 75, 16, 12, true, true}, {nullptr,  "Left Windows", "lwin", 54, 75, NS, true, true}, {nullptr,  "Left Alt", "lalt", 67, 75, 14, 12, true, true}, {nullptr,  "無変換", "muhenkan", 80, 75, NS, true, true}, {nullptr,  "Space", "space", 116, 75, 84, 12, true, true}, {nullptr,  "変換", "henkan", 150, 75, NS, true, true}, {nullptr,  "ひらがな カタカナ ローマ字", "katahira", 162, 75, NS, true, true}, {nullptr,  "Right Alt", "ralt", 165, 75, 14, 12, true, true}, {nullptr,  "Right Windows", "rwin", 178, 75, NS, true, true}, {nullptr,  "Menu", "rmenu", 190, 75, NS, true, true}, {nullptr,  "Right Ctrl", "rctrl", 204, 75, 16, 12, true, true}, {nullptr,  "Left", "left", 222, 75, NS, true, true}, {nullptr,  "Down", "down", 234, 75, NS, true, true}, {nullptr,  "Right", "right", 246, 75, NS, true, true}, {nullptr,  "NumPad 0", "num0", 267, 75, 24, 12, true, true}, {nullptr,  "NumPad .", "numdot", 285, 75, NS, true, true}
};
#define KEYCOUNT_K95 (sizeof(K95Keys) / sizeof(Key))


// ANSI layouts use a different Enter/LShift key
#define ANSI_ENTER_X    199
#define ANSI_ENTER_Y    51
#define ANSI_ENTER_W    26
#define ANSI_ENTER_H    12
#define ANSI_LSHIFT_X   45
#define ANSI_LSHIFT_W   26

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
#define PATCH_COUNT(patch) (sizeof(patch) / sizeof(KeyPatch))
#define patch(map, patches) _patch(map, patches, PATCH_COUNT(patches))
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
    Key& enter = map["enter"];
    enter.x = ANSI_ENTER_X;
    enter.y = ANSI_ENTER_Y;
    enter.width = ANSI_ENTER_W;
    enter.height = ANSI_ENTER_H;
    Key& lshift = map["lshift"];
    lshift.x = ANSI_LSHIFT_X;
    lshift.width = ANSI_LSHIFT_W;
}
static inline void patchJP106(QHash<QString, Key>& map){
    // First apply the ISO patch
    map.remove("bslash");

    // Resize Backspace
    Key& bspace = map["bspace"];
    bspace.width -= 12;
    bspace.x = 206;

    // Resize RShift
    Key& rshift = map["rshift"];
    rshift.width -= 12;
    rshift.x += 6;

    // Resize Spacebar
    Key& space = map["space"];
    space.width -= 26;
    space.x -= 1;

    // Left shift
    Key& lshift = map["lshift"];
    lshift.x = ANSI_LSHIFT_X;
    lshift.width = ANSI_LSHIFT_W;

    map.remove("bslash_iso");

    // Resize and move ralt to make space for the extra keys
    Key& ralt = map["ralt"];
    ralt.x += 11;
    ralt.width += 2;

    // None of these layouts have rwin
    map.remove("rwin");
}
// Used to fix the Fn size and remove Alt when necessary in JP layouts for compatible devices
static inline void patchJP106fn(QHash<QString, Key>& map){
    if(map.contains("fn")){
        map.remove("ralt");
        Key& fn = map["fn"];
        fn.width += 4;
        fn.x -= 2;
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
    Key& rshift = map["rshift"];
    rshift.width -= 12;
    rshift.x += 6;
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
    {nullptr, "Stop", "stop", K70_X_START - 37, 0, 12, 8, true, true},
    {nullptr, "Previous", "prev", K70_X_START - 26, 0, 12, 8, true, true},
    {nullptr, "Play/Pause", "play", K70_X_START - 15, 0, 12, 8, true, true},
    {nullptr, "Next", "next", K70_X_START - 4, 0, 12, 8, true, true},
    {nullptr, "Logo", "logo", 140 - K70_X_START, 0, 12, 12, true, false},
    {nullptr, "Profile Switch", "profswitch", 178 - K70_X_START, 0, 12, 8, true, true},
    {nullptr, "Mute", "mute", 222 - K70_X_START, 0, 12, 8, true, true},
};
#define K70_TKL_TOP_COUNT (sizeof(K70TklTopRow) / sizeof(Key))

static const Key K68TopRow[] = {
    {nullptr,  "Volume Down", "voldn", 285 - K70_X_START, 0, 13, 8, true, true}, {nullptr,  "Volume Up", "volup", 297 - K70_X_START, 0, 13, 8, true, true},
};
#define K68_TOP_COUNT (sizeof(K68TopRow) / sizeof(Key))

static const Key K65TopRow[] = {
    {nullptr,  "Brightness", "light", 164 - K70_X_START, 0, 12, 12, true, true}, {nullptr,  "Mute", "mute", 176 - K70_X_START, 0, 12, 12, true, true}, {nullptr,  "Volume Down", "voldn", 192 - K70_X_START, 0, 14, 8, true, true}, {nullptr,  "Volume Up", "volup", 205 - K70_X_START, 0, 14, 8, true, true}, {nullptr,  "Windows Lock", "lock", 222 - K70_X_START, 0, 12, 12, true, true}
};
#define K65_TOP_COUNT (sizeof(K65TopRow) / sizeof(Key))

static const Key K63TopRow[] = {
    {nullptr,  "Stop", "stop", K70_X_START - 37, 0, 12, 8, true, true}, {nullptr,  "Previous", "prev", K70_X_START - 26, 0, 12, 8, true, true}, {nullptr,  "Play/Pause", "play", K70_X_START - 15, 0, 12, 8, true, true}, {nullptr,  "Next", "next", K70_X_START - 4, 0, 12, 8, true, true}, {nullptr,  "Brightness", "light", 170 - K70_X_START, 0, 12, 12, true, true}, {nullptr,  "Windows Lock", "lock", 180 - K70_X_START, 0, 12, 12, true, true}, {nullptr,  "Mute", "mute", 222 - K70_X_START, 0, 13, 8, true, true}, {nullptr,  "Volume Down", "voldn", 234 - K70_X_START, 0, 13, 8, true, true}, {nullptr,  "Volume Up", "volup", 246 - K70_X_START, 0, 13, 8, true, true}
};
#define K63_TOP_COUNT (sizeof(K63TopRow) / sizeof(Key))

static const Key K55Zones[] = {
    {nullptr,  "Zone 1", "zone1", 46, 45, ZS, true, false},
    {nullptr,  "Zone 2", "zone2", 140, 45, ZS, true, false},
    {nullptr,  "Zone 3", "zone3", 233, 45, ZS, true, false},
};
#define K55_ZONES (sizeof(K55Zones) / sizeof(Key))

static const Key K55PROZones[] = {
    {nullptr,  "Zone 1", "zone1", 26, 45, ZSP, true, false},
    {nullptr,  "Zone 2", "zone2", 83, 45, ZSP, true, false},
    {nullptr,  "Zone 3", "zone3", 140, 45, ZSP, true, false},
    {nullptr,  "Zone 4", "zone4", 197, 45, ZSP, true, false},
    {nullptr,  "Zone 5", "zone5", 254, 45, ZSP, true, false},
};
#define K55PRO_ZONES (sizeof(K55PROZones) / sizeof(Key))

// Strafe has side lights
#define KSTRAFE_X_START     12
#define KSTRAFE_WIDTH       (K70_WIDTH + (KSTRAFE_X_START * 2))
#define KSTRAFE_HEIGHT      K95_HEIGHT

static const Key KStrafeKeys[] = {
    {nullptr,  "Sidelight", "lsidel", 0, KSTRAFE_HEIGHT/2, KSTRAFE_X_START, KSTRAFE_HEIGHT, true, false},
    {nullptr,  "Sidelight", "rsidel", KSTRAFE_WIDTH, KSTRAFE_HEIGHT/2, KSTRAFE_X_START, KSTRAFE_HEIGHT, true, false},
    {nullptr,  "Logo", "logo", KSTRAFE_X_START, 0, NS, true, false},
    {nullptr,  "Function", "fn", 152, 75, NS, true, true}
};

// Mouse map - M65
static const Key M65Keys[] = {
    {nullptr,  "Left Mouse", "mouse1", 8, 0, 14, 32, false, true}, {nullptr,  "Right Mouse", "mouse2", 30, 0, 14, 32, false, true}, {nullptr,  "Middle Mouse", "mouse3", 22, 8, 8, 7, false, true},
    {nullptr,  "Wheel Up", "wheelup", 22, 4, 8, 5, false, true}, {nullptr,  "Wheel Down", "wheeldn", 22, 14, 8, 5, false, true}, {nullptr,  "Wheel Light", "front", 22, 15, 8, 8, true, false},
    {nullptr,  "DPI Up", "dpiup", 22, 19, 8, 6, false, true}, {nullptr,  "DPI Light", "dpi", 22, 24, 8, 8, true, false}, {nullptr,  "DPI Down", "dpidn", 22, 31, 8, 6, false, true},
    {nullptr,  "Forward", "mouse5", 5, 24, 5, 9, false, true}, {nullptr,  "Back", "mouse4", 5, 33, 5, 10, false, true}, {nullptr,  "Sniper", "sniper", 0, 25, 5, 15, false, true},
    {nullptr,  "Logo", "back", 20, 54, 12, 12, true, false}
};
#define KEYCOUNT_M65    (sizeof(M65Keys) / sizeof(Key))

#define M65_WIDTH       52
#define M65_HEIGHT      67

// Sabre
static const Key SabreKeys[] = {
    {nullptr,  "Left Mouse", "mouse1", 8, 0, 14, 32, false, true}, {nullptr,  "Right Mouse", "mouse2", 30, 0, 14, 32, false, true}, {nullptr,  "Middle Mouse", "mouse3", 22, 9, 8, 7, false, true}, {nullptr,  "Front light", "front", 8, -2, 14, 8, true, false },
    {nullptr,  "Wheel Up", "wheelup", 22, 5, 8, 5, false, true}, {nullptr,  "Wheel Down", "wheeldn", 22, 15, 8, 5, false, true}, {nullptr,  "Wheel Light", "wheel", 22, 5, 8, 15, true, false}, {nullptr,  "Extra button", "thumb1", 22, 20, 8, 18, false, true},
    {nullptr,  "DPI Up", "dpiup", 5, 3, 5, 7, false, true}, {nullptr,  "DPI Down", "dpidn", 5, 10, 5, 7, false, true}, {nullptr,  "DPI Light", "dpi", 5, 4, 5, 12, true, false},
    {nullptr,  "Forward", "mouse5", 5, 24, 5, 9, false, true}, {nullptr,  "Back", "mouse4", 5, 33, 5, 10, false, true},
    {nullptr,  "Logo", "back", 21, 50, 12, 12, true, false}
};
#define KEYCOUNT_SABRE  (sizeof(SabreKeys) / sizeof(Key))

#define SABRE_WIDTH     M65_WIDTH
#define SABRE_HEIGHT    M65_HEIGHT

// Harpoon
static const Key HarpoonKeys[] = {
    {nullptr,  "Left Mouse", "mouse1", 10, 5, 14, 26, false, true}, {nullptr,  "Right Mouse", "mouse2", 30, 5, 14, 26, false, true}, {nullptr,  "Middle Mouse", "mouse3", 25, 11, 6, 7, false, true},
    {nullptr,  "Wheel Up", "wheelup", 25, 7, 6, 5, false, true}, {nullptr,  "Wheel Down", "wheeldn", 25, 17, 6, 5, false, true},
    {nullptr,  "DPI Cycle", "dpiup", 25, 23, 6, 10, false, true}, {nullptr,  "Logo Light", "dpi", 18, 40, 20, 20, true, false},
    {nullptr,  "Forward", "mouse5", 3, 24, 5, 10, false, true}, {nullptr,  "Back", "mouse4", 3, 33, 5, 10, false, true}
};
#define KEYCOUNT_HARPOON  (sizeof(HarpoonKeys) / sizeof(Key))

#define HARPOON_WIDTH     M65_WIDTH
#define HARPOON_HEIGHT    M65_HEIGHT

// Glaive
static const Key GlaiveKeys[] = {
    {nullptr,  "Left Mouse", "mouse1", 17, 3, 14, 18, false, true}, {nullptr,  "Right Mouse", "mouse2", 37, 3, 14, 18, false, true}, {nullptr,  "Middle Mouse", "mouse3", 31, 9, 7, 7, false, true}, {nullptr,  "Front light", "front", 16, -5, 36, 8, true, false },
    {nullptr,  "Wheel Up", "wheelup", 31, 5, 7, 5, false, true}, {nullptr,  "Wheel Down", "wheeldn", 31, 15, 7, 5, false, true}, {nullptr,  "Side Lights", "side", 22, 24, 7, 20, true, false},
    {nullptr,  "DPI Cycle", "dpiup", 31, 19, 6, 12, false, true}, {nullptr,  "Logo Light", "back", 26, 45, 16, 16, true, false},
    {nullptr,  "Forward", "mouse5", 15, 22, 5, 11, false, true}, {nullptr,  "Back", "mouse4", 15, 32, 5, 11, false, true}
};
#define KEYCOUNT_GLAIVE  (sizeof(GlaiveKeys) / sizeof(Key))

#define GLAIVE_WIDTH     M65_WIDTH
#define GLAIVE_HEIGHT    M65_HEIGHT

// Katar
static const Key KatarKeys[] = {
    {nullptr,  "Left Mouse", "mouse1", 7, 4, 14, 18, false, true}, {nullptr,  "Right Mouse", "mouse2", 28, 4, 14, 18, false, true}, {nullptr,  "Middle Mouse", "mouse3", 22, 9, 7, 7, false, true},
    {nullptr,  "Wheel Up", "wheelup", 22, 5, 7, 5, false, true}, {nullptr,  "Wheel Down", "wheeldn", 22, 15, 7, 5, false, true},
    {nullptr,  "DPI Cycle", "dpiup", 22, 19, 6, 12, false, true}, {nullptr,  "Logo Light", "front", 15, 43, 20, 20, true, false},
};
#define KEYCOUNT_KATAR  (sizeof(KatarKeys) / sizeof(Key))

// Katar PRO XT
static const Key KatarPROXTKeys[] = {
    {nullptr,  "Left Mouse",       "mouse1",    7,  4, 14, 18, false, true},
    {nullptr,  "Right Mouse",      "mouse2",   28,  4, 14, 18, false, true},
    {nullptr,  "Middle Mouse",     "mouse3",   22,  9,  7,  7, false, true},
    {nullptr,  "Wheel Up",         "wheelup",  22,  5,  7,  5, false, true},
    {nullptr,  "Wheel Down",       "wheeldn",  22, 15,  7,  5, false, true},
    {nullptr,  "DPI Cycle",        "dpiup",    22, 19,  6, 12, false, true},
    {nullptr,  "DPI Wheel Light",  "dpi",      21,  3,  8, 14, true,  false},
    {nullptr,  "Forward",          "mouse5",    3, 24,  5, 10, false, true},
    {nullptr,  "Back",             "mouse4",    3, 33,  5, 10, false, true}
};
#define KEYCOUNT_KATAR_PROXT  (sizeof(KatarPROXTKeys) / sizeof(Key))

// Scimitar
static const Key ScimKeys[] = {
    {nullptr,  "Left Mouse", "mouse1", 15, 0, 14, 32, false, true}, {nullptr,  "Right Mouse", "mouse2", 37, 0, 12, 32, false, true}, {nullptr,  "Middle Mouse", "mouse3", 29, 9, 8, 6, false, true}, {nullptr,  "Front light", "front", 37, 0, 12, 8, true, false },
    {nullptr,  "Wheel Up", "wheelup", 29, 3, 8, 6, false, true}, {nullptr,  "Wheel Down", "wheeldn", 29, 14, 8, 6, false, true}, {nullptr,  "Wheel Light", "wheel", 29, 3, 8, 17, true, false},
    {nullptr,  "DPI Up", "dpiup", 29, 19, 8, 9, false, true}, {nullptr,  "DPI Light", "dpi", 8, 12, 8, 4, true, false}, {nullptr,  "DPI Down", "dpidn", 29, 28, 8, 9, false, true},
    {nullptr,  "Thumb light", "thumb", 7, 21, 10, 24, true, false},
    {nullptr,  "1", "thumb1", -6, 18, 7, 7, false, true}, {nullptr,  "2", "thumb2", 1, 18, 7, 7, false, true}, {nullptr,  "3", "thumb3", 8, 18, 7, 7, false, true},
    {nullptr,  "4", "thumb4", -6, 25, 7, 7, false, true}, {nullptr,  "5", "thumb5", 1, 25, 7, 7, false, true}, {nullptr,  "6", "thumb6", 8, 25, 7, 7, false, true},
    {nullptr,  "7", "thumb7", -6, 32, 7, 7, false, true}, {nullptr,  "8", "thumb8", 1, 32, 7, 7, false, true}, {nullptr,  "9", "thumb9", 8, 32, 7, 7, false, true},
    {nullptr,  "10", "thumb10", -6, 39, 7, 7, false, true}, {nullptr,  "11", "thumb11", 1, 39, 7, 7, false, true}, {nullptr,  "12", "thumb12", 8, 39, 7, 7, false, true},
    {nullptr,  "Logo", "back", 26, 50, 16, 16, true, false}
};
#define KEYCOUNT_SCIM   (sizeof(ScimKeys) / sizeof(Key))

#define SCIM_WIDTH      66
#define SCIM_HEIGHT     M65_HEIGHT

// M95
static const Key M95Keys[] = {
    {nullptr,  "Left Mouse", "mouse1", 42, 6, 14, 26, false, true}, {nullptr,  "Right Mouse", "mouse2", 63, 6, 14, 26, false, true}, {nullptr,  "Middle Mouse", "mouse3", 55, 8, 8, 7, false, true},
    {nullptr,  "4", "thumb6", 56, 28, 7, 12, false, true},
    {nullptr,  "Wheel Up", "wheelup", 55, 4, 8, 5, false, true}, {nullptr,  "Wheel Down", "wheeldn", 55, 14, 8, 5, false, true}, {nullptr,  "DPI Up (5)", "dpiup", 38, -1, 6, 9, false, true}, {nullptr,  "DPI Down (6)", "dpidn", 38, 6, 6, 9, false, true},
    {nullptr,  "Forward (7)", "mouse6", -9, 14, 14, 8, false, true}, {nullptr,  "Back (8)", "mouse7", 3, 14, 14, 8, false, true}, {nullptr,  "Sniper (14)", "sniper", -6, 20, 8, 14, false, true},
    {nullptr,  "9", "thumb1", 15, 14, 18, 8, false, true}, {nullptr,  "15", "thumb7", 15, 20, 10, 22, false, true},
    {nullptr,  "10", "thumb2", 23, 20, 10, 12, false, true},{nullptr,  "11", "thumb3", 23, 30, 10, 12, false, true},
    {nullptr,  "12", "thumb4", 15, 40, 18, 8, false, true}, {nullptr,  "13", "thumb5", -1, 40, 18, 8, false, true},
    {nullptr,  "Logo", "back", 53, 55, 12, 12, true, false}

};
#define KEYCOUNT_M95    (sizeof(M95Keys) / sizeof(Key))

#define M95_WIDTH      110

// M55
static const Key M55Keys[] = {
    {nullptr,  "Left Mouse",       "mouse1",    8,  4, 14, 18, false, true},
    {nullptr,  "Right Mouse",      "mouse2",   31,  4, 14, 18, false, true},
    {nullptr,  "Middle Mouse",     "mouse3",   24,  6,  7,  8, false, true},
    {nullptr,  "Wheel Up",         "wheelup",  24,  3,  7,  5, false, true},
    {nullptr,  "Wheel Down",       "wheeldn",  24, 13,  7,  5, false, true},
    {nullptr,  "DPI Cycle",        "dpiup",    24, 19,  6, 10, false, true},
    {nullptr,  "DPI Light",        "dpi",      23, 15,  8,  6, true,  false},
    {nullptr,  "Forward",          "mouse4",    6, 24,  5, 10, false, true},
    {nullptr,  "Back",             "mouse5",    6, 33,  5, 10, false, true},
    {nullptr,  "Forward (7)",      "mouse6",   43, 24,  5, 10, false, true},
    {nullptr,  "Back (8)",         "mouse7",   43, 33,  5, 10, false, true},
    {nullptr,  "Logo",             "back",     21, 50, 12, 12, true,  false}
};
#define KEYCOUNT_M55    (sizeof(M55Keys) / sizeof(Key))

// K95 Platinum lightbar
static const Key K95PLbar[] = {
    {nullptr, "Top Light Bar 1", "topbar1", 4, -3, LBS, true, false}, {nullptr, "Top Light Bar 2", "topbar2", 19, -3, LBS, true, false}, {nullptr, "Top Light Bar 3", "topbar3", 34, -3, LBS, true, false}, {nullptr, "Top Light Bar 4", "topbar4", 49, -3, LBS, true, false}, {nullptr, "Top Light Bar 5", "topbar5", 64, -3, LBS, true, false}, {nullptr, "Top Light Bar 6", "topbar6", 79, -3, LBS, true, false},
    {nullptr, "Top Light Bar 7", "topbar7", 94, -3, LBS, true, false}, {nullptr, "Top Light Bar 8", "topbar8", 109, -3, LBS, true, false}, {nullptr, "Top Light Bar 9", "topbar9", 124, -3, LBS, true, false}, {nullptr, "Top Light Bar 10", "topbar10", 139, -3, LBS, true, false}, {nullptr, "Top Light Bar 11", "topbar11", 154, -3, LBS, true, false}, {nullptr, "Top Light Bar 12", "topbar12", 169, -3, LBS, true, false},
    {nullptr, "Top Light Bar 13", "topbar13", 184, -3, LBS, true, false}, {nullptr, "Top Light Bar 14", "topbar14", 199, -3, LBS, true, false}, {nullptr, "Top Light Bar 15", "topbar15", 214, -3, LBS, true, false}, {nullptr, "Top Light Bar 16", "topbar16", 229, -3, LBS, true, false}, {nullptr, "Top Light Bar 17", "topbar17", 244, -3, LBS, true, false}, {nullptr, "Top Light Bar 18", "topbar18", 259, -3, LBS, true, false},
    {nullptr, "Top Light Bar 19", "topbar19", 274, -3, LBS, true, false},
};
#define LBARCOUNT_K95P (sizeof(K95PLbar) / sizeof(Key))

// Mouse map - DARK CORE
static const Key DarkCoreKeys[] = {
    {nullptr,  "Left Mouse", "mouse1", 8, 0, 14, 32, false, true}, {nullptr,  "Right Mouse", "mouse2", 30, 0, 14, 32, false, true}, {nullptr,  "Middle Mouse", "mouse3", 22, 8, 8, 7, false, true},
    {nullptr,  "Wheel Up", "wheelup", 22, 4, 8, 5, false, true}, {nullptr,  "Wheel Down", "wheeldn", 22, 14, 8, 5, false, true}, {nullptr,  "Wheel Light", "wheel", 22, 2, 8, 16, true, false},
    {nullptr,  "DPI Up", "dpiup", 0, 5, 8, 8, false, true}, {nullptr,  "DPI Down", "dpidn", 0, 12, 8, 8, false, true},
    {nullptr,  "Forward", "mouse5", 5, 24, 5, 9, false, true}, {nullptr,  "Back", "mouse4", 5, 33, 5, 10, false, true}, {nullptr,  "Sniper", "sniper", 0, 25, 5, 15, false, true}, {nullptr,  "Side light", "side", 0, 24, 10, 24, true, false},
    {nullptr,  "Logo", "back", 19, 50, 12, 12, true, false}, {nullptr,  "Profile Switch", "profswitch", 22, 20, 8, 12, false, true}, {nullptr,  "DPI", "dpi", 0, 0, 10, 24, true,  false }
};
#define KEYCOUNT_DARKCORE    (sizeof(DarkCoreKeys) / sizeof(Key))

// Mouse map - DARK CORE RGB PRO
static const Key DarkCoreRgbProKeys[] = {
    {nullptr, "Left Mouse",     "mouse1",      8,  0, 14, 24, false, true},
    {nullptr, "Right Mouse",    "mouse2",     25,  0, 14, 24, false, true},
    {nullptr, "Middle Mouse",   "mouse3",     19,  8,  8,  7, false, true},
    {nullptr, "Wheel Up",       "wheelup",    19,  4,  8,  5, false, true},
    {nullptr, "Wheel Down",     "wheeldn",    19, 14,  8,  5, false, true},
    {nullptr, "Wheel Light",    "wheel",      19,  5,  8, 10, true,  false},
    {nullptr, "DPI Up",         "dpiup",       0,  5,  8,  8, false, true},
    {nullptr, "DPI Down",       "dpidn",       0, 12,  8,  8, false, true},
    {nullptr, "Forward",        "mouse5",      5, 24, 10,  9, true,  true},
    {nullptr, "Back",           "mouse4",      5, 29,  5, 10, true,  true},
    {nullptr, "Bar 3 light",    "bar3",        5, 35,  5,  8, true,  false},
    {nullptr, "Bar 4 light",    "bar4",        5, 40,  5,  8, true,  false},
    {nullptr, "Thumb light",    "thumb",       5, 55,  8,  8, true,  false},
    {nullptr, "Side light",     "side",       30, 48,  8,  8, true,  false},
    {nullptr, "Logo",           "back",       15, 48, 15, 15, true,  false},
    {nullptr, "Profile",        "profdn",     19, 19,  8, 8,  false,  true},
    {nullptr, "DPI0",           "dpiw0",       19, 15,  8, 8,  true,  false},
    {nullptr, "DPI3",           "dpiw3",       10,  5,  8, 8,  true,  false},
    {nullptr, "DPI2",           "dpiw2",        5,  5,  8, 8,  true,  false},
    {nullptr, "DPI1",           "dpiw1",         0,  5,  8, 8,  true,  false},
};
#define KEYCOUNT_DARKCORERGBPRO    (sizeof(DarkCoreRgbProKeys) / sizeof(Key))

// MM800 Polaris
#define POLARIS_V 6, 14
#define POLARIS_H 14, 6

static const Key PolarisZones[] = {
    {nullptr,  "Zone 1", "zone1", 65, 4, POLARIS_V, true, false}, {nullptr,  "Zone 2", "zone2", 65, 16, POLARIS_V, true, false}, {nullptr,  "Zone 3", "zone3", 65, 28, POLARIS_V, true, false}, {nullptr,  "Zone 4", "zone4", 65, 40, POLARIS_V, true, false},
    {nullptr,  "Zone 5", "zone5", 65, 52, POLARIS_V, true, false}, {nullptr,  "Zone 6", "zone6", 43, 62, POLARIS_H, true, false}, {nullptr,  "Zone 7", "zone7", 31, 62, POLARIS_H, true, false}, {nullptr,  "Zone 8", "zone8", 19, 62, POLARIS_H, true, false},
    {nullptr,  "Zone 9", "zone9", 7, 62, POLARIS_H, true, false}, {nullptr,  "Zone 10", "zone10", -5, 62, POLARIS_H, true, false}, {nullptr,  "Zone 11", "zone11", -19, 52, POLARIS_V, true, false}, {nullptr,  "Zone 12", "zone12", -19, 40, POLARIS_V, true, false},
    {nullptr,  "Zone 13", "zone13", -19, 28, POLARIS_V, true, false}, {nullptr,  "Zone 14", "zone14", -19, 16, POLARIS_V, true, false}, {nullptr,  "Zone 15", "zone15", -19, 4, POLARIS_V, true, false}
};
#define KEYCOUNT_POLARIS   (sizeof(PolarisZones) / sizeof(Key))

#define ST100_V 6,22
#define ST100_H 22,6

static const Key ST100Zones[] = {
    {nullptr,  "Zone 1", "zone1", 16, 10, ST100_H, true, false}, {nullptr,  "Zone 2", "zone2", 36, 10, ST100_H, true, false}, {nullptr,  "Zone 3", "zone3", 52, 30, ST100_V, true, false}, {nullptr,  "Zone 4", "zone4", 52, 50, ST100_V, true, false},
    {nullptr,  "Logo", "zone5", 21, -1, NS, true, false}, // Logo
    {nullptr,  "Zone 6", "zone6", 16, 66, ST100_H, true, false}, {nullptr,  "Zone 7", "zone7", -4, 50, ST100_V, true, false}, {nullptr,  "Zone 8", "zone8", -4, 30, ST100_V, true, false},
    {nullptr,  "Zone 9", "zone9", -4, 10, ST100_H, true, false},
};
#define KEYCOUNT_ST100     (sizeof(ST100Zones) / sizeof(Key))

// Mouse map - Ironclaw
static const Key IronclawKeys[] = {
    // primary keys
    {nullptr,  "Left Mouse",    "mouse1",     12,  0, 12, 28, false, true  },
    {nullptr,  "Right Mouse",   "mouse2",     31,  0, 12, 28, false, true  },

    // center column keys
    {nullptr,  "Wheel Up",      "wheelup",    23,  3,  8,  7, false, true  },
    {nullptr,  "Middle Mouse",  "mouse3",     23,  7,  8,  6, false, true  },
    {nullptr,  "Wheel Down",    "wheeldn",    23, 12,  8,  7, false, true  },
    {nullptr,  "Profile Cycle", "profswitch", 23, 18,  9,  9, false, true  },
    {nullptr,  "DPI Cycle",     "dpiup",      23, 26,  8,  9, false, true  },

    // left side forward/back keys
    {nullptr,  "Forward",    "mouse5",      6, 20,  5, 12, false, true  },
    {nullptr,  "Back",       "mouse4",      7, 32,  5, 12, false, true  },

    // zones for LEDs
    {nullptr,  "Logo",          "back",       21, 50,  NS,    true,  false },
    {nullptr,  "Wheel",         "wheel",      23, 3,  8,  14, true,  false },

    // need to add DPI LED, even if not directly configurable for indicator to work
    {nullptr,  "DPI",           "dpi",        10, 10,  8,  8, true,  false }

};
#define KEYCOUNT_IRONCLAW    (sizeof(IronclawKeys) / sizeof(Key))

#define IRONCLAW_WIDTH       52
#define IRONCLAW_HEIGHT      67

// Nightsword
static const Key NightswordKeys[] = {
    //Primary buttons
    {nullptr,  "Left Mouse", "mouse1", 14, 6, 12, 26, false, true},
    {nullptr,  "Right Mouse", "mouse2", 34, 6, 12, 26, false, true},
    {nullptr,  "Wheel Up", "wheelup",      26, 5, 6, 5, false, true},
    {nullptr,  "Middle Mouse", "mouse3",   26, 10, 6, 5, false, true},
    {nullptr,  "Wheel Down", "wheeldn",    26, 15, 6, 5, false, true},
    //Lighting zones
    {nullptr,  "Wheel Light", "wheel", 26, 5, 8, 15, true, false},
    {nullptr,  "Front light", "front", 12, -2, 36, 8, true, false },
    {nullptr,  "Logo", "back",         24, 50, 12, 12, true, false},
    {nullptr,  "Back light", "side", 12, 64, 36, 8, true, false }, //Addition; side == bottom, under logo
    {nullptr,  "DPI Light", "dpi",     6, 11, 5, 12, true, false}, // thumb side DPI indicator light
    //DPI keys
    {nullptr,  "DPI Up", "dpiup", 12, 10, 5, 7, false, true},
    {nullptr,  "DPI Down", "dpidn", 12, 18, 5, 7, false, true},
    //Profile keys
    {nullptr,  "Profile Up", "profswitch", 26, 18,  6,  10, false, true  },
    {nullptr,  "Profile Down", "profdn", 26, 26,  6,  10, false, true  },
    //Thumb keys (extra should be sniper)
    {nullptr,  "Forward", "mouse5", 8, 21, 5, 9, false, true},
    {nullptr,  "Back", "mouse4", 8, 30, 5, 10, false, true},
    {nullptr,  "Sniper", "sniper", 0, 26, 8, 8, false, true}
};
#define KEYCOUNT_NIGHTSWORD  (sizeof(NightswordKeys) / sizeof(Key))

#define NIGHTSWORD_WIDTH     M65_WIDTH
#define NIGHTSWORD_HEIGHT    M65_HEIGHT


// Mouse map - Ironclaw RGB Wireless
static const Key IronclawWirelessKeys[] = {
    // primary keys
    {nullptr,  "Left Mouse",   "mouse1",   12,  0, 12, 28, false, true  },
    {nullptr,  "Right Mouse",  "mouse2",   31,  0, 12, 28, false, true  },

    // center column keys
    {nullptr,  "Wheel Up",     "wheelup",  23,  3,  8,  7, false, true  },
    {nullptr,  "Middle Mouse",  "mouse3",  23,  7,  8,  6, false, true  },
    {nullptr,  "Wheel Down",   "wheeldn",  23, 12,  8,  7, false, true  },
    {nullptr,  "Profile Up",    "profup",  23, 18,  9,  9, false, true  },
    {nullptr,  "Profile Dn",    "profdn",  23, 26,  8,  9, false, true  },

    // left side forward/back keys
    {nullptr,  "DPI Up",        "dpiup",    6,  4,  6,  9, false, true  },
    {nullptr,  "DPI Dn",        "dpidn",    6, 10,  6,  9, false, true  },
    {nullptr,  "Forward",      "mouse5",    6, 20,  5, 12, false, true  },
    {nullptr,  "Back",         "mouse4",    7, 32,  5, 12, false, true  },
    {nullptr,  "Opt",          "optbtn",    9, 27,  6,  9, false, true  },

    // zones for LEDs
    {nullptr,  "Front",        "front",     9,  1,  9,  9, true, false  },
    {nullptr,  "Logo",         "back",     21, 50, NS,     true, false  },
    {nullptr,  "Wheel",        "wheel",    23,  3,  8, 14, true, false  },

    // need to add DPI LED, even if not directly configurable for indicator to work
    {nullptr,  "DPI",           "dpi",        10, 10,  8,  8, true,  false }

};
#define KEYCOUNT_IRONCLAW_W    (sizeof(IronclawWirelessKeys) / sizeof(Key))

#define IRONCLAW_W_WIDTH       52
#define IRONCLAW_W_HEIGHT      67

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
        const Key lghtpgm = {nullptr,  "Lighting Programming", "lghtpgm", 210, 0, NS, true, true};
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
        map["g1"].x = 22;
        map["g2"].x = 22;
        map["g3"].x = 22;
        map["g4"].x = 22;
        map["g5"].x = 22;
        map["g6"].x = 22;
        //map["g1"].y = 14+(12*0);
        map["g2"].y = 26; //14+(12*1)
        map["g3"].y = 38; //14+(12*2)
        map["g4"].y = 50; //14+(12*3)
        map["g5"].y = 62; //14+(12*4)
        map["g6"].y = 74; //14+(12*5)
        // Remove M keys
        map.remove("m1");
        map.remove("m2");
        map.remove("m3");
        map.remove("mr");
        // Add profile button
        map["profswitch"] = {nullptr,  "Profile Switch", "profswitch", 64, 1, 11, 8, true, true};
        // Centre Brightness/Winlock between F1 and F4
        map["light"].x = 75;
        map["lock"].x = 86;
        // Resize them
        map["light"].height = 8;
        map["lock"].height = 8;
        map["light"].width = 11;
        map["lock"].width = 11;
        // Mute is above Stop
        map["mute"].x -= 11;
        map["voldn"].x -= 11;
        map["volup"].x -= 11;
        // Move the buttons on the top a bit further down to centre them
        map["light"].y += 1;
        map["lock"].y += 1;
        map["mute"].y += 1;
        map["voldn"].y += 1;
        map["volup"].y += 1;
        // Shift all keys down (to make room for the lightbar), and to the left
        QMutableHashIterator<QString, Key> i(map);
        while(i.hasNext()){
            i.next();
            i.value().x -= K95P_X_START;
            i.value().y += 6;
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
                i.value().y += (K100_HEIGHT - K95P_HEIGHT);

             i.value().x += (K100_WIDTH - K95P_WIDTH)/2;
        }
        // Shrink the top lightbar and add the extra three items
        for(int j = 0; j < 19; j++){
            QString key = QString("topbar%1").arg(j + 1);
            map[key].width = 15; // maybe 15
            map[key].x -= j * 2 + 1;
        }
        map["topbar20"] = {nullptr, "Top Light Bar 20", "topbar20", 255, -3, 15, 6, true, false};
        map["topbar21"] = {nullptr, "Top Light Bar 21", "topbar21", 268, -3, 15, 6, true, false};
        map["topbar22"] = {nullptr, "Top Light Bar 22", "topbar22", 281, -3, 15, 6, true, false};

        // Add the left and right bars
        map["leftbar1"] = {nullptr, "Left Light Bar 1", "leftbar1", -2, -2, 6, 9, true, false};
        map["leftbar2"] = {nullptr, "Left Light Bar 2", "leftbar2", -2, 7, 6, 9, true, false};
        map["leftbar3"] = {nullptr, "Left Light Bar 3", "leftbar3", -2, 16, 6, 9, true, false};
        map["leftbar4"] = {nullptr, "Left Light Bar 4", "leftbar4", -2, 25, 6, 9, true, false};
        map["leftbar5"] = {nullptr, "Left Light Bar 5", "leftbar5", -2, 34, 6, 9, true, false};
        map["leftbar6"] = {nullptr, "Left Light Bar 6", "leftbar6", -2, 43, 6, 9, true, false};
        map["leftbar7"] = {nullptr, "Left Light Bar 7", "leftbar7", -2, 52, 6, 9, true, false};
        map["leftbar8"] = {nullptr, "Left Light Bar 8", "leftbar8", -2, 61, 6, 9, true, false};
        map["leftbar9"] = {nullptr, "Left Light Bar 9", "leftbar9", -2, 70, 6, 9, true, false};
        map["leftbar10"] = {nullptr, "Left Light Bar 10", "leftbar10", -2, 79, 6, 9, true, false};
        map["leftbar11"] = {nullptr, "Left Light Bar 11", "leftbar11", -2, 88, 6, 9, true, false};

        map["rightbar1"] = {nullptr, "Right Light Bar 1", "rightbar1", 292, -2, 6, 9, true, false};
        map["rightbar2"] = {nullptr, "Right Light Bar 2", "rightbar2", 292, 7, 6, 9, true, false};
        map["rightbar3"] = {nullptr, "Right Light Bar 3", "rightbar3", 292, 16, 6, 9, true, false};
        map["rightbar4"] = {nullptr, "Right Light Bar 4", "rightbar4", 292, 25, 6, 9, true, false};
        map["rightbar5"] = {nullptr, "Right Light Bar 5", "rightbar5", 292, 34, 6, 9, true, false};
        map["rightbar6"] = {nullptr, "Right Light Bar 6", "rightbar6", 292, 43, 6, 9, true, false};
        map["rightbar7"] = {nullptr, "Right Light Bar 7", "rightbar7", 292, 52, 6, 9, true, false};
        map["rightbar8"] = {nullptr, "Right Light Bar 8", "rightbar8", 292, 61, 6, 9, true, false};
        map["rightbar9"] = {nullptr, "Right Light Bar 9", "rightbar9", 292, 70, 6, 9, true, false};
        map["rightbar10"] = {nullptr, "Right Light Bar 10", "rightbar10", 292, 79, 6, 9, true, false};
        map["rightbar11"] = {nullptr, "Right Light Bar 11", "rightbar11", 292, 88, 6, 9, true, false};

        map["ctrlwheelb"] = map["light"];
        map["ctrlwheelb"].name = "ctrlwheelb";
        map["ctrlwheelb"]._friendlyName = "Control Wheel Button";
        map["ctrlwheelb"].height = map["ctrlwheelb"].width;
        map["ctrlwheelb"].y -= 3;

        map["profswitch"].height += 1;
        map["lock"].height = map["mute"].height = map["profswitch"].height;
        map["mute"].y = map["profswitch"].y = map["lock"].y = map["ctrlwheelb"].y;
        map["volup"].y = map["ctrlwheelb"].y - 2;
        map["voldn"].y = map["ctrlwheelb"].y + 2;
        map["profswitch"].x -= 1;
        map["lock"].x += 1;

        map["ctrlwheel1"] = {nullptr, "Control Wheel 22.5°",  "ctrlwheel1", 60+2, 10, 8, 6, true, false};
        map["ctrlwheel2"] = {nullptr, "Control Wheel 67.5°",  "ctrlwheel2", 60+3, 10+1, 5, 8, true, false};
        map["ctrlwheel3"] = {nullptr, "Control Wheel 112.5°", "ctrlwheel3", 60+3, 10+2, 5, 8, true, false};
        map["ctrlwheel4"] = {nullptr, "Control Wheel 157.5°", "ctrlwheel4", 60+2, 10+3, 8, 6, true, false};
        map["ctrlwheel5"] = {nullptr, "Control Wheel 202.5°", "ctrlwheel5", 60+1, 10+3, 8, 6, true, false};
        map["ctrlwheel6"] = {nullptr, "Control Wheel 247.5°", "ctrlwheel6", 60, 10+2, 5, 8, true, false};
        map["ctrlwheel7"] = {nullptr, "Control Wheel 292.5°", "ctrlwheel7", 60, 10+1, 5, 8, true, false};
        map["ctrlwheel8"] = {nullptr, "Control Wheel 337.5°", "ctrlwheel8", 60+1, 10, 8, 6, true, false};

        map["logoleft"] = {nullptr, "Logo Left", "logoleft", 134, 10, 10, 10, true, false};
        map["logo"] = {nullptr, "Logo", "logo", 144, 10, 10, 10, true, false};
        map["logoright"] = {nullptr, "Logo Right", "logoright", 154, 10, 10, 10, true, false};

        map.remove("light");
        break;
    }
    case KeyMap::K70:{
        // The K70 maps are based on the K95 maps. However all the keys are shifted left and the G keys are removed
        map = getMap(KeyMap::K95, layout);
        QMutableHashIterator<QString, Key> i(map);
        while(i.hasNext()){
            i.next();
            // Move key to left. Remove it if it fell off the edge
            if((i.value().x -= K70_X_START) < 0)
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
        map["profswitch"] = {nullptr,  "Profile Switch", "profswitch", 40, 0, 10, 8, true, true};

        map["light"].x = 49;
        map["light"].width = 10;
        map["light"].height = 8;

        map["lock"].x = 58;
        map["lock"].width = 10;
        map["lock"].height = 8;

        // Add logo1 (m3)
        map["m3"] = {nullptr,  "Logo 1", "m3", 118, 0, NS, true, false};
        // Add logo2 (g11)
        map["g11"] = {nullptr,  "Logo 2", "g11", 138, 0, NS, true, false};

        // Move mute and volup/dn one block to the left
        map["mute"].x -= 10;
        map["volup"].x -= 10;
        map["voldn"].x -= 10;
        break;
    }
    case KeyMap::K70_PRO:{
        // Same as K70 MK2 but right Windows key is a function key
        map = getMap(KeyMap::K70MK2, layout);

        map["fn"] = KStrafeKeys[3];
        map["fn"].x = map["rwin"].x;
        map.remove("rwin");

        // these keys are all the way to left
        map["profswitch"].x = 0;
        map["light"].x = 9;
        map["lock"].x = 18;

        // these keys are all the way to the right
        map["mute"].x += 10;
        map["volup"].x += 10;
        map["voldn"].x += 10;

        map["topbar1"] = {nullptr, "Logo", "topbar1", 128, 0, NS, true, false};
        map.remove("m3");
        map.remove("g11");

        break;
    }
    case KeyMap::K70_TKL:{
        // Same width as the K63 but with a top row more like the K70
        map = getMap(KeyMap::K63, layout);
        for(const Key* key = K70TklTopRow; key < K70TklTopRow + K70_TKL_TOP_COUNT; key++)
            map[key->name] = *key;

        map.remove("rwin");
        map["fn"] = KStrafeKeys[3];
        map["fn"].x -= 12;
        map["light"].x = 190 - K70_X_START;
        map["light"].height = 8;
        map["lock"].x = 202 - K70_X_START;
        map["lock"].height = 8;
        break;
    }
    case KeyMap::STRAFE_MK2:{
        map = getMap(KeyMap::K70MK2, layout);
        // move everything right to make the space for the left sidelight
        QMutableHashIterator<QString, Key> i(map);
        while(i.hasNext()){
            i.next();
            i.value().x += KSTRAFE_X_START;
        }

        // Move profswitch, light and lwin two blocks to the left
        map["profswitch"].x -=20;
        map["light"].x -= 20;
        map["lock"].x -= 20;

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
            if(i.value().x >= K65_WIDTH)
                i.remove();
        }
        for(const Key* key = K65TopRow; key < K65TopRow + K65_TOP_COUNT; key++)
            map[key->name] = *key;

        map.remove("rwin");
        map["fn"] = KStrafeKeys[3];
        map["fn"].x -= 12;

        // Done!
        break;
    }
    case KeyMap::K65_MINI:{
        map = getMap(KeyMap::K65, layout);

        // Move Esc so that it doesn't get deleted
        map["esc"].y += 13;
        map.remove("grave");

        // Remove the whole top bar and shift everything up
        QMutableHashIterator<QString, Key> i(map);
        while(i.hasNext()){
            i.next();

            if((i.value().y -= 27) < 0)
                i.remove();
            else if((i.value().x -= 3) > K65_MINI_WIDTH + 2)
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
            if(i.value().x >= K65_WIDTH)
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
        map["fn"].x -= 12;

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
        map["fn"] = KStrafeKeys[3];
        map["fn"].x = map["rwin"].x;
        map.remove("rwin");

        QMutableHashIterator<QString, Key> i(map);
        while(i.hasNext()){
            i.next();
            i.value().y -= 14;
        }

        break;
    }
    case KeyMap::K60_TKL:{
        map = getMap(KeyMap::K63, layout);
        // QMutableHashIterator<QString, Key> i(map);
        // while(i.hasNext()){
        //     i.next();
        //     // Move key to left. Remove it if it fell off the edge
        //     if((i.value().x -= K70_X_START) < 0)
        //         i.remove();
        // }
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
        map["fn"] = KStrafeKeys[3];
        map["fn"].x = map["rwin"].x;
        map.remove("rwin");

        QMutableHashIterator<QString, Key> i(map);
        while(i.hasNext()){
            i.next();
            i.value().y -= 14;
        }
        break;
    }
    case KeyMap::K57_WL:{
        // Take the K95 map
        map = getMap(KeyMap::K95, layout);

        // Replace rwin with Fn
        map["fn"] = KStrafeKeys[3];
        map["fn"].x = map["rwin"].x;
        map.remove("rwin");

        // Replace volume wheel
        map["voldn"] = {nullptr,  "Volume Down", "voldn", static_cast<short>(map["mute"].x + 12), 0, map["mute"].width, map["mute"].height, true, true};
        map["volup"] = {nullptr,  "Volume Up", "volup", static_cast<short>(map["mute"].x + 24), 0, map["mute"].width, map["mute"].height, true, true};

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
        map["g1"].x = 22;
        map["g2"].x = 22;
        map["g3"].x = 22;
        map["g4"].x = 22;
        map["g5"].x = 22;
        map["g6"].x = 22;
        map["g2"].y = 26; //14+(12*1)
        map["g3"].y = 38; //14+(12*2)
        map["g4"].y = 50; //14+(12*3)
        map["g5"].y = 62; //14+(12*4)
        map["g6"].y = 74; //14+(12*5)

        // Remove M keys
        map.remove("m1");
        map.remove("m2");
        map.remove("m3");

        // Move MR to the left of brightness
        map["mr"].x = map["light"].x - 12;

        // Shift all keys down (to make room for the lightbar), and to the left
        QMutableHashIterator<QString, Key> i(map);
        while(i.hasNext()){
            i.next();
            i.value().x -= K95P_X_START;
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
        map["g1"].x = 22;
        map["g2"].x = 22;
        map["g3"].x = 22;
        map["g4"].x = 22;
        map["g5"].x = 22;
        map["g6"].x = 22;
        //map["g1"].y = 14+(12*0);
        map["g2"].y = 26; //14+(12*1)
        map["g3"].y = 38; //14+(12*2)
        map["g4"].y = 50; //14+(12*3)
        map["g5"].y = 62; //14+(12*4)
        map["g6"].y = 74; //14+(12*5)
        // Remove M keys
        map.remove("m1");
        map.remove("m2");
        map.remove("m3");
        // Move MR to the left of the brightness button
        map["mr"].x = 210;
        // Resize and move Volume Up/Down
        map["voldn"].x = 285;
        map["voldn"].y = 0;
        map["voldn"].width = 13;
        map["voldn"].height = 8;
        map["volup"].x = 297;
        map["volup"].y = 0;
        map["volup"].width = 13;
        map["volup"].height = 8;
        // Replace rwin with Fn
        map["fn"] = KStrafeKeys[3];
        map["fn"].x = map["rwin"].x;
        map.remove("rwin");
        // Shift all keys to the left, and disable their LEDs
        QMutableHashIterator<QString, Key> i(map);
        while(i.hasNext()){
            i.next();
            i.value().x -= K95P_X_START;
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
        map["light"].x=285 - K70_X_START;
        //map["light"].hasLed=false;
        map["lock"].x=297 - K70_X_START;
        //map["lock"].hasLed=false;
        // move everything right to make the space for the left sidelight
        QMutableHashIterator<QString, Key> i(map);
        while(i.hasNext()){
            i.next();
            i.value().x += KSTRAFE_X_START;
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
    case KeyMap::M65:{
        // M65 isn't a keyboard; all mouse maps are unique.
        for(const Key* key = M65Keys; key < M65Keys + KEYCOUNT_M65; key++){
            // Keyboard keys are written from the center because that's where the LEDs are, but the mouse buttons are odd shapes so they're
            // written from the upper left
            Key translatedKey = *key;
            translatedKey.x += translatedKey.width / 2;
            translatedKey.y += translatedKey.height / 2;
            map[key->name] = translatedKey;
        }
        // Mice also have no layout patches - no other changes necessary
        break;
    }
    case KeyMap::SABRE:{
        // Sabre mouse
        for(const Key* key = SabreKeys; key < SabreKeys + KEYCOUNT_SABRE; key++){
            // Like the M65, the keys are upper-left justified
            Key translatedKey = *key;
            translatedKey.x += translatedKey.width / 2;
            translatedKey.y += translatedKey.height / 2;
            map[key->name] = translatedKey;
        }
        break;
    }
    case KeyMap::SCIMITAR:{
        // Scimitar mouse
        for(const Key* key = ScimKeys; key < ScimKeys + KEYCOUNT_SCIM; key++){
            Key translatedKey = *key;
            translatedKey.x += translatedKey.width / 2;
            translatedKey.y += translatedKey.height / 2;
            map[key->name] = translatedKey;
        }
        break;
    }
    case KeyMap::HARPOON:{
        // Harpoon mouse
        for(const Key* key = HarpoonKeys; key < HarpoonKeys + KEYCOUNT_HARPOON; key++){
            Key translatedKey = *key;
            translatedKey.x += translatedKey.width / 2;
            translatedKey.y += translatedKey.height / 2;
            map[key->name] = translatedKey;
        }
        break;
    }
    case KeyMap::GLAIVE:{
        // Glaive mouse
        for(const Key* key = GlaiveKeys; key < GlaiveKeys + KEYCOUNT_GLAIVE; key++){
            Key translatedKey = *key;
            translatedKey.x += translatedKey.width / 2;
            translatedKey.y += translatedKey.height / 2;
            map[key->name] = translatedKey;
        }
        break;
    }
    case KeyMap::KATAR:{
        // Katar
        for(const Key* key = KatarKeys; key < KatarKeys + KEYCOUNT_KATAR; key++){
            Key translatedKey = *key;
            translatedKey.x += translatedKey.width / 2;
            translatedKey.y += translatedKey.height / 2;
            map[key->name] = translatedKey;
        }
        break;
    }
    case KeyMap::KATARPROXT:{
        // Katar
        for(const Key* key = KatarPROXTKeys; key < KatarPROXTKeys + KEYCOUNT_KATAR_PROXT; key++){
            Key translatedKey = *key;
            translatedKey.x += translatedKey.width / 2;
            translatedKey.y += translatedKey.height / 2;
            map[key->name] = translatedKey;
        }
        break;
    }
    case KeyMap::DARKCORE:{
        // Dark Core
        for(const Key* key = DarkCoreKeys; key < DarkCoreKeys + KEYCOUNT_DARKCORE; key++){
            Key translatedKey = *key;
            translatedKey.x += translatedKey.width / 2;
            translatedKey.y += translatedKey.height / 2;
            map[key->name] = translatedKey;
        }
        break;
    }
    case KeyMap::DARKCORERGBPRO:{
        // Dark Core
        for(const Key* key = DarkCoreRgbProKeys; key < DarkCoreRgbProKeys + KEYCOUNT_DARKCORERGBPRO; key++){
            Key translatedKey = *key;
            translatedKey.x += translatedKey.width / 2;
            translatedKey.y += translatedKey.height / 2;
            map[key->name] = translatedKey;
        }
        break;
    }
    case KeyMap::POLARIS:{
        // MM800 Polaris Mousepad
        for(const Key* key = PolarisZones; key < PolarisZones + KEYCOUNT_POLARIS; key++){
            Key translatedKey = *key;
            translatedKey.x += translatedKey.width / 2;
            translatedKey.y += translatedKey.height / 2;
            map[key->name] = translatedKey;
        }
        break;
    }
    case KeyMap::ST100:{
        for(const Key* key = ST100Zones; key < ST100Zones + KEYCOUNT_ST100; key++){
            Key translatedKey = *key;
            translatedKey.x += translatedKey.width / 2;
            translatedKey.y += translatedKey.height / 2;
            map[key->name] = translatedKey;
        }
        break;
    }
    // Same as M65 but with the wheel zone instead of the front one
    case KeyMap::M65E:{
        map = getMap(KeyMap::M65, layout);
        map["wheel"] = {nullptr,  "Wheel Light", "wheel", map["front"].x, map["front"].y, map["front"].width, map["front"].height, true, false};
        map.remove("front");
        break;
    }
    case KeyMap::M95:{
        for(const Key* key = M95Keys; key < M95Keys + KEYCOUNT_M95; key++){
            Key translatedKey = *key;
            translatedKey.x += translatedKey.width / 2;
            translatedKey.y += translatedKey.height / 2;
            map[key->name] = translatedKey;
        }
        break;
    }
    case KeyMap::IRONCLAW:{
        // M65 isn't a keyboard; all mouse maps are unique.
        for(const Key* key = IronclawKeys; key < IronclawKeys + KEYCOUNT_IRONCLAW; key++){
            // Keyboard keys are written from the center because that's where the LEDs are, but the mouse buttons are odd shapes so they're
            // written from the upper left
            Key translatedKey = *key;
            translatedKey.x += translatedKey.width / 2;
            translatedKey.y += translatedKey.height / 2;
            map[key->name] = translatedKey;
        }
        // Mice also have no layout patches - no other changes necessary
        break;
    }
    case KeyMap::NIGHTSWORD:{
        // cloned from IRONCLAW above
        for(const Key* key = NightswordKeys; key < NightswordKeys + KEYCOUNT_NIGHTSWORD; key++){
            Key translatedKey = *key;
            translatedKey.x += translatedKey.width / 2;
            translatedKey.y += translatedKey.height / 2;
            map[key->name] = translatedKey;
        }
        // Mice also have no layout patches - no other changes necessary
        break;
    }
    case KeyMap::IRONCLAW_W:{
        // M65 isn't a keyboard; all mouse maps are unique.
        for(const Key* key = IronclawWirelessKeys; key < IronclawWirelessKeys + KEYCOUNT_IRONCLAW_W; key++){
            // Keyboard keys are written from the center because that's where the LEDs are, but the mouse buttons are odd shapes so they're
            // written from the upper left
            Key translatedKey = *key;
            translatedKey.x += translatedKey.width / 2;
            translatedKey.y += translatedKey.height / 2;
            map[key->name] = translatedKey;
        }
        // Mice also have no layout patches - no other changes necessary
        break;
    }
    case KeyMap::GLAIVEPRO:{
        // Based on the Glaive
        map = getMap(KeyMap::GLAIVE, layout);
        // Change dpiup back to DPI Up, and add dpidn
        map["dpiup"]._friendlyName = "DPI Up";
        map["dpiup"].height = 8;
        map["dpiup"].y -= 2;
        map["dpidn"] = {nullptr,  "DPI Down", "dpidn", 34, 29, 6, 8, false, true};
        break;
    }
    case KeyMap::M55:{
        // m55
        for(const Key* key = M55Keys; key < M55Keys + KEYCOUNT_M55; key++){
            Key translatedKey = *key;
            translatedKey.x += translatedKey.width / 2;
            translatedKey.y += translatedKey.height / 2;
            map[key->name] = translatedKey;
        }
        break;
    }
    default:;    // <- stop GCC from complaining
    }

    if(KeyMap::isJP(layout))
        patchJP106fn(map);

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
    if(lower == "k70pro")
        return K70_PRO;
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
    case K70_PRO:
        return "k70pro";
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
    case K70_PRO:
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
    case K70_PRO:
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
    case GLAIVEPRO:
        return M65_HEIGHT;
    case IRONCLAW_W:
        return M65_HEIGHT;
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
    for(const Key& key: *this)
        ordered[key.y][key.x] = key.name;

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

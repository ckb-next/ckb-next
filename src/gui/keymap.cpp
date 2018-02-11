#include <clocale>
#include <QMap>
#include "keymap.h"

// Normal key size
#define NS 12, 12

// Lightbar LED size
#define LBS 17, 6

// K55 Zone Size
#define ZS 97, 75

// Key positions (K95 - English)
// This is the master key map that includes ANSI, ISO and JP-106 layouts - use patchANSI(), patchISO() or patchJP106() to finalize it
static const Key K95Keys[] = {
    {0, 0, "mr", 38, 0, NS, true, true}, {0, 0, "m1", 50, 0, NS, true, true}, {0, 0, "m2", 62, 0, NS, true, true}, {0, 0, "m3", 74, 0, NS, true, true}, {0, "Brightness", "light", 222, 0, NS, true, true}, {0, "Windows Lock", "lock", 234, 0, NS, true, true}, {0, "Mute", "mute", 273, 0, 13, 8, true, true}, {0, "Volume Up", "volup", 290, -2, 18, 6, false, true}, {0, "Volume down", "voldn", 290, 2, 18, 6, false, true},
    {0, 0, "g1", 0, 14, NS, true, true}, {0, 0, "g2", 11, 14, NS, true, true}, {0, 0, "g3", 22, 14, NS, true, true}, {0, "Esc", "esc", 38, 14, NS, true, true}, {0, 0, "f1", 58, 14, NS, true, true}, {0, 0, "f2", 70, 14, NS, true, true}, {0, 0, "f3", 82, 14, NS, true, true}, {0, 0, "f4", 94, 14, NS, true, true}, {0, 0, "f5", 114, 14, NS, true, true}, {0, 0, "f6", 126, 14, NS, true, true}, {0, 0, "f7", 138, 14, NS, true, true}, {0, 0, "f8", 150, 14, NS, true, true}, {0, 0, "f9", 170, 14, NS, true, true}, {0, 0, "f10", 182, 14, NS, true, true}, {0, 0, "f11", 194, 14, NS, true, true}, {0, 0, "f12", 206, 14, NS, true, true}, {0, "Print Screen\nSysRq", "prtscn", 222, 14, NS, true, true}, {0, "Scroll Lock", "scroll", 234, 14, NS, true, true}, {0, "Pause\nBreak", "pause", 246, 14, NS, true, true}, {0, "Stop", "stop", 262, 14, 12, 8, true, true}, {0, "Previous", "prev", 273, 14, 13, 8, true, true}, {0, "Play/Pause", "play", 285, 14, 13, 8, true, true}, {0, "Next", "next", 296, 14, 12, 8, true, true},
    {0, 0, "g4", 0, 25, NS, true, true}, {0, 0, "g5", 11, 25, NS, true, true}, {0, 0, "g6", 22, 25, NS, true, true}, {0, "`", "grave", 38, 27, NS, true, true}, {0, 0, "1", 50, 27, NS, true, true}, {0, 0, "2", 62, 27, NS, true, true}, {0, 0, "3", 74, 27, NS, true, true}, {0, 0, "4", 86, 27, NS, true, true}, {0, 0, "5", 98, 27, NS, true, true}, {0, 0, "6", 110, 27, NS, true, true}, {0, 0, "7", 122, 27, NS, true, true}, {0, 0, "8", 134, 27, NS, true, true}, {0, 0, "9", 146, 27, NS, true, true}, {0, 0, "0", 158, 27, NS, true, true}, {0, "-", "minus", 170, 27, NS, true, true}, {0, "=", "equal", 182, 27, NS, true, true}, {0, "¥", "yen", 194, 27, NS, true, true}, {0, "Backspace", "bspace", 200, 27, 24, 12, true, true}, {0, "Insert", "ins", 222, 27, NS, true, true}, {0, "Home", "home", 234, 27, NS, true, true}, {0, "Page Up", "pgup", 246, 27, NS, true, true}, {0, "Num Lock", "numlock", 261, 27, NS, true, true}, {0, "NumPad /", "numslash", 273, 27, NS, true, true}, {0, "NumPad *", "numstar", 285, 27, NS, true, true}, {0, "NumPad -", "numminus", 297, 27, NS, true, true},
    {0, 0, "g7", 0, 39, NS, true, true}, {0, 0, "g8", 11, 39, NS, true, true}, {0, 0, "g9", 22, 39, NS, true, true}, {0, "Tab", "tab", 41, 39, 18, 12, true, true}, {0, 0, "q", 56, 39, NS, true, true}, {0, 0, "w", 68, 39, NS, true, true}, {0, 0, "e", 80, 39, NS, true, true}, {0, 0, "r", 92, 39, NS, true, true}, {0, 0, "t", 104, 39, NS, true, true}, {0, 0, "y", 116, 39, NS, true, true}, {0, 0, "u", 128, 39, NS, true, true}, {0, 0, "i", 140, 39, NS, true, true}, {0, 0, "o", 152, 39, NS, true, true}, {0, 0, "p", 164, 39, NS, true, true}, {0, "[", "lbrace", 176, 39, NS, true, true}, {0, "]", "rbrace", 188, 39, NS, true, true}, {0, "\\", "bslash", 203, 39, 18, 12, true, true}, {0, "Enter", "enter", 203, 39, 18, 24, true, true}, {0, "Delete", "del", 222, 39, NS, true, true}, {0, "End", "end", 234, 39, NS, true, true}, {0, "Page Down", "pgdn", 246, 39, NS, true, true}, {0, "NumPad 7", "num7", 261, 39, NS, true, true}, {0, "NumPad 8", "num8", 273, 39, NS, true, true}, {0, "NumPad 9", "num9", 285, 39, NS, true, true}, {0, "NumPad +", "numplus", 297, 45, 12, 24, true, true},
    {0, 0, "g10", 0, 50, NS, true, true}, {0, 0, "g11", 11, 50, NS, true, true}, {0, 0, "g12", 22, 50, NS, true, true}, {0, "Caps Lock", "caps", 42, 51, 20, 12, true, true}, {0, 0, "a", 59, 51, NS, true, true}, {0, 0, "s", 71, 51, NS, true, true}, {0, 0, "d", 83, 51, NS, true, true}, {0, 0, "f", 95, 51, NS, true, true}, {0, 0, "g", 107, 51, NS, true, true}, {0, 0, "h", 119, 51, NS, true, true}, {0, 0, "j", 131, 51, NS, true, true}, {0, 0, "k", 143, 51, NS, true, true}, {0, 0, "l", 155, 51, NS, true, true}, {0, ";", "colon", 167, 51, NS, true, true}, {0, "'", "quote", 179, 51, NS, true, true}, {0, "#", "hash", 191, 51, NS, true, true}, {0, "NumPad 4", "num4", 261, 51, NS, true, true}, {0, "NumPad 5", "num5", 273, 51, NS, true, true}, {0, "NumPad 6", "num6", 285, 51, NS, true, true},
    {0, 0, "g13", 0, 64, NS, true, true}, {0, 0, "g14", 11, 64, NS, true, true}, {0, 0, "g15", 22, 64, NS, true, true}, {0, "Left Shift", "lshift", 39, 63, 14, 12, true, true}, {"bslash", "\\", "bslash_iso", 53, 63, NS, true, true}, {0, 0, "z", 65, 63, NS, true, true}, {0, 0, "x", 77, 63, NS, true, true}, {0, 0, "c", 89, 63, NS, true, true}, {0, 0, "v", 101, 63, NS, true, true}, {0, 0, "b", 113, 63, NS, true, true}, {0, 0, "n", 125, 63, NS, true, true}, {0, 0, "m", 137, 63, NS, true, true}, {0, ",", "comma", 149, 63, NS, true, true}, {0, ".", "dot", 161, 63, NS, true, true}, {0, "/", "slash", 173, 63, NS, true, true}, {0, "_", "ro", 185, 63, NS, true, true}, {0, "Right Shift", "rshift", 196, 63, 32, 12, true, true}, {0, "Up", "up", 234, 63, NS, true, true}, {0, "NumPad 1", "num1", 261, 63, NS, true, true}, {0, "NumPad 2", "num2", 273, 63, NS, true, true}, {0, "NumPad 3", "num3", 285, 63, NS, true, true}, {0, "NumPad Enter", "numenter", 297, 69, 12, 24, true, true},
    {0, 0, "g16", 0, 75, NS, true, true}, {0, 0, "g17", 11, 75, NS, true, true}, {0, 0, "g18", 22, 75, NS, true, true}, {0, "Left Ctrl", "lctrl", 40, 75, 16, 12, true, true}, {0, "Left Windows", "lwin", 54, 75, NS, true, true}, {0, "Left Alt", "lalt", 67, 75, 14, 12, true, true}, {0, "無変換", "muhenkan", 80, 75, NS, true, true}, {0, "Space", "space", 116, 75, 84, 12, true, true}, {0, "変換", "henkan", 150, 75, NS, true, true}, {0, "ひらがな カタカナ ローマ字", "katahira", 162, 75, NS, true, true}, {0, "Right Alt", "ralt", 165, 75, 14, 12, true, true}, {0, "Right Windows", "rwin", 178, 75, NS, true, true}, {0, "Menu", "rmenu", 190, 75, NS, true, true}, {0, "Right Ctrl", "rctrl", 204, 75, 16, 12, true, true}, {0, "Left", "left", 222, 75, NS, true, true}, {0, "Down", "down", 234, 75, NS, true, true}, {0, "Right", "right", 246, 75, NS, true, true}, {0, "NumPad 0", "num0", 267, 75, 24, 12, true, true}, {0, "NumPad .", "numdot", 285, 75, NS, true, true}
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
    {0, "§", "grave"}, {0, "+", "minus"}, {0, "´", "equal"},
    {0, "Å", "lbrace"}, {0, "¨", "rbrace"},
    {0, "Æ", "colon"}, {0, "Ø", "quote"}, {0, "'", "hash"},
    {0, "<", "bslash_iso"}, {0, "-", "slash"},
};

static const KeyPatch patchEU[] = {
    {0, "\\ (R)", "hash"},
    {0, "\\ (L)", "bslash_iso"},
};

static const KeyPatch patchFR[] = {
    {"sup2", "²", "grave"}, {0, "&", "1"}, {0, "É", "2"}, {0, "\"", "3"}, {0, "'", "4"}, {0, "(", "5"}, {0, "-", "6"}, {0, "È", "7"}, {0, "_", "8"}, {0, "Ç", "9"}, {0, "À", "0"}, {"rparen", ")", "minus"},
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
    {0, "\\", "grave"}, {0, "'", "minus"}, {0, "Ì", "equal"},
    {0, "È", "lbrace"}, {0, "+", "rbrace"},
    {0, "Ò", "colon"}, {0, "À", "quote"}, {0, "Ù", "hash"},
    {0, "<", "bslash_iso"}, {0, "-", "slash"},
};

static const KeyPatch patchNO[] = {
    {0, "§", "grave"}, {0, "+", "minus"}, {0, "´", "equal"},
    {0, "Å", "lbrace"}, {0, "¨", "rbrace"},
    {0, "Ø", "colon"}, {0, "Æ", "quote"}, {0, "'", "hash"},
    {0, "<", "bslash_iso"}, {0, "-", "slash"},
};

static const KeyPatch patchMX[] = {
    {0, "|", "grave"}, {0, "'", "minus"}, {0, "¿", "equal"},
    {0, "´", "lbrace"}, {0, "+", "rbrace"},
    {0, "Ñ", "colon"}, {0, "{", "quote"}, {0, "}", "hash"},
    {0, "<", "bslash_iso"}, {0, "-", "slash"},
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
    {0, "[", "minus"}, {0, "]", "equal"},
    {0, "'", "q"}, {0, ",", "w"}, {0, ".", "e"}, {0, "P", "r"}, {0, "Y", "t"}, {0, "F", "y"}, {0, "G", "u"}, {0, "C", "i"}, {0, "R", "o"}, {0, "L", "p"}, {0, "/", "lbrace"}, {0, "=", "rbrace"},
    {0, "O", "s"}, {0, "E", "d"}, {0, "U", "f"}, {0, "I", "g"}, {0, "D", "h"}, {0, "H", "j"}, {0, "T", "k"}, {0, "N", "l"}, {0, "S", "colon"}, {0, "-", "quote"},
    {0, ";", "z"}, {0, "Q", "x"}, {0, "J", "c"}, {0, "K", "v"}, {0, "X", "b"}, {0, "B", "n"}, {0, "W", "comma"}, {0, "V", "dot"}, {0, "Z", "slash"},
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
static void patchISO(QHash<QString, Key>& map){
    patchnonJP106(map);
    map.remove("bslash");
}
static void patchANSI(QHash<QString, Key>& map){
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
static void patchJP106(QHash<QString, Key>& map){
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
static void patchJP106fn(QHash<QString, Key>& map){
    if(map.contains("fn")){
        map.remove("ralt");
        Key& fn = map["fn"];
        fn.width += 4;
        fn.x -= 2;
    }
}
// Patch for ABNT 2 layout
static void patchABNT2(QHash<QString, Key>& map){
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

// K70 cuts off the G keys on the left, as well as MR/M1/M2/M3
#define K70_X_START     38
#define K70_WIDTH       (K95_WIDTH - K70_X_START)
#define K70_HEIGHT      K95_HEIGHT

#define K68_WIDTH       K70_WIDTH
#define K68_HEIGHT      K70_HEIGHT

// K65 additionally removes the numpad on the right, and has a different top row
#define K65_WIDTH       209
#define K65_HEIGHT      K70_HEIGHT

// K63 is the same as the K65 in terms of size
#define K63_WIDTH       K65_WIDTH
#define K63_HEIGHT      K65_HEIGHT

static const Key K68TopRow[] = {
    {0, "Volume Down", "voldn", 285 - K70_X_START, 0, 13, 8, true, true}, {0, "Volume Up", "volup", 297 - K70_X_START, 0, 13, 8, true, true},
};
#define K68_TOP_COUNT (sizeof(K68TopRow) / sizeof(Key))

static const Key K65TopRow[] = {
    {0, "Brightness", "light", 164 - K70_X_START, 0, 12, 12, true, true}, {0, "Mute", "mute", 176 - K70_X_START, 0, 12, 12, true, true}, {0, "Volume Down", "voldn", 192 - K70_X_START, 0, 14, 8, true, true}, {0, "Volume Up", "volup", 205 - K70_X_START, 0, 14, 8, true, true}, {0, "Windows Lock", "lock", 222 - K70_X_START, 0, 12, 12, true, true}
};
#define K65_TOP_COUNT (sizeof(K65TopRow) / sizeof(Key))

static const Key K63TopRow[] = {
    {0, "Stop", "stop", K70_X_START - 37, 0, 12, 8, true, true}, {0, "Previous", "prev", K70_X_START - 26, 0, 12, 8, true, true}, {0, "Play/Pause", "play", K70_X_START - 15, 0, 12, 8, true, true}, {0, "Next", "next", K70_X_START - 4, 0, 12, 8, true, true}, {0, "Brightness", "light", 170 - K70_X_START, 0, 12, 12, true, true}, {0, "Windows Lock", "lock", 180 - K70_X_START, 0, 12, 12, true, true}, {0, "Mute", "mute", 222 - K70_X_START, 0, 13, 8, true, true}, {0, "Volume Down", "voldn", 234 - K70_X_START, 0, 13, 8, true, true}, {0, "Volume Up", "volup", 246 - K70_X_START, 0, 13, 8, true, true}
};
#define K63_TOP_COUNT (sizeof(K63TopRow) / sizeof(Key))

static const Key K55Zones[] = {
    {0, "Zone 1", "zone1", 46, 45, ZS, true, false},
    {0, "Zone 2", "zone2", 140, 45, ZS, true, false},
    {0, "Zone 3", "zone3", 233, 45, ZS, true, false},
};
#define K55_ZONES (sizeof(K55Zones) / sizeof(Key))


// Strafe has side lights
#define KSTRAFE_X_START     12
#define KSTRAFE_WIDTH       (K70_WIDTH + (KSTRAFE_X_START * 2))
#define KSTRAFE_HEIGHT      K95_HEIGHT

static const Key KStrafeKeys[] = {
    {0, "Sidelight", "lsidel", 0, KSTRAFE_HEIGHT/2, KSTRAFE_X_START, KSTRAFE_HEIGHT, true, false},
    {0, "Sidelight", "rsidel", KSTRAFE_WIDTH, KSTRAFE_HEIGHT/2, KSTRAFE_X_START, KSTRAFE_HEIGHT, true, false},
    {0, "Logo", "logo", KSTRAFE_X_START, 0, NS, true, false},
    {0, "Function", "fn", 152, 75, NS, true, true}
};

// Mouse map - M65
static const Key M65Keys[] = {
    {0, "Left Mouse", "mouse1", 8, 0, 14, 32, false, true}, {0, "Right Mouse", "mouse2", 30, 0, 14, 32, false, true}, {0, "Middle Mouse", "mouse3", 22, 8, 8, 7, false, true},
    {0, "Wheel Up", "wheelup", 22, 4, 8, 5, false, true}, {0, "Wheel Down", "wheeldn", 22, 14, 8, 5, false, true}, {0, "Wheel Light", "front", 22, 15, 8, 8, true, false},
    {0, "DPI Up", "dpiup", 22, 19, 8, 6, false, true}, {0, "DPI Light", "dpi", 22, 24, 8, 8, true, false}, {0, "DPI Down", "dpidn", 22, 31, 8, 6, false, true},
    {0, "Forward", "mouse5", 5, 24, 5, 9, false, true}, {0, "Back", "mouse4", 5, 33, 5, 10, false, true}, {0, "Sniper", "sniper", 0, 25, 5, 15, false, true},
    {0, "Logo", "back", 14, 55, 24, 12, true, false}
};
#define KEYCOUNT_M65    (sizeof(M65Keys) / sizeof(Key))

#define M65_WIDTH       52
#define M65_HEIGHT      67

// Sabre
static const Key SabreKeys[] = {
    {0, "Left Mouse", "mouse1", 8, 0, 14, 32, false, true}, {0, "Right Mouse", "mouse2", 30, 0, 14, 32, false, true}, {0, "Middle Mouse", "mouse3", 22, 9, 8, 7, false, true}, {0, "Front light", "front", 8, -2, 14, 8, true, false },
    {0, "Wheel Up", "wheelup", 22, 5, 8, 5, false, true}, {0, "Wheel Down", "wheeldn", 22, 15, 8, 5, false, true}, {0, "Wheel Light", "wheel", 22, 5, 8, 15, true, false}, {0, "Extra button", "thumb1", 22, 20, 8, 18, false, true},
    {0, "DPI Up", "dpiup", 5, 3, 5, 7, false, true}, {0, "DPI Down", "dpidn", 5, 10, 5, 7, false, true}, {0, "DPI Light", "dpi", 5, 4, 5, 12, true, false},
    {0, "Forward", "mouse5", 5, 24, 5, 9, false, true}, {0, "Back", "mouse4", 5, 33, 5, 10, false, true},
    {0, "Logo", "back", 14, 50, 24, 12, true, false}
};
#define KEYCOUNT_SABRE  (sizeof(SabreKeys) / sizeof(Key))

#define SABRE_WIDTH     M65_WIDTH
#define SABRE_HEIGHT    M65_HEIGHT

// Harpoon
static const Key HarpoonKeys[] = {
    {0, "Left Mouse", "mouse1", 10, 5, 14, 26, false, true}, {0, "Right Mouse", "mouse2", 30, 5, 14, 26, false, true}, {0, "Middle Mouse", "mouse3", 25, 11, 6, 7, false, true},
    {0, "Wheel Up", "wheelup", 25, 7, 6, 5, false, true}, {0, "Wheel Down", "wheeldn", 25, 17, 6, 5, false, true},
    {0, "DPI Cycle", "dpiup", 25, 23, 6, 10, false, true}, {0, "Logo Light", "dpi", 17, 40, 20, 20, true, false},
    {0, "Forward", "mouse5", 3, 24, 5, 10, false, true}, {0, "Back", "mouse4", 3, 33, 5, 10, false, true}
};
#define KEYCOUNT_HARPOON  (sizeof(HarpoonKeys) / sizeof(Key))

#define HARPOON_WIDTH     M65_WIDTH
#define HARPOON_HEIGHT    M65_HEIGHT

// Glaive
static const Key GlaiveKeys[] = {
    {0, "Left Mouse", "mouse1", 17, 3, 14, 18, false, true}, {0, "Right Mouse", "mouse2", 37, 3, 14, 18, false, true}, {0, "Middle Mouse", "mouse3", 31, 9, 7, 7, false, true}, {0, "Front light", "front", 16, -5, 36, 8, true, false },
    {0, "Wheel Up", "wheelup", 31, 5, 7, 5, false, true}, {0, "Wheel Down", "wheeldn", 31, 15, 7, 5, false, true}, {0, "Side Lights", "side", 22, 24, 7, 20, true, false},
    {0, "DPI Cycle", "dpiup", 31, 19, 6, 12, false, true}, {0, "Logo Light", "back", 24, 43, 20, 20, true, false},
    {0, "Forward", "mouse5", 15, 22, 5, 11, false, true}, {0, "Back", "mouse4", 15, 32, 5, 11, false, true}
};
#define KEYCOUNT_GLAIVE  (sizeof(GlaiveKeys) / sizeof(Key))

#define GLAIVE_WIDTH     M65_WIDTH
#define GLAIVE_HEIGHT    M65_HEIGHT

// Katar
static const Key KatarKeys[] = {
    {0, "Left Mouse", "mouse1", 7, 4, 14, 18, false, true}, {0, "Right Mouse", "mouse2", 28, 4, 14, 18, false, true}, {0, "Middle Mouse", "mouse3", 22, 9, 7, 7, false, true},
    {0, "Wheel Up", "wheelup", 22, 5, 7, 5, false, true}, {0, "Wheel Down", "wheeldn", 22, 15, 7, 5, false, true}, 
    {0, "DPI Cycle", "dpiup", 22, 19, 6, 12, false, true}, {0, "Logo Light", "front", 15, 43, 20, 20, true, false},
};
#define KEYCOUNT_KATAR  (sizeof(KatarKeys) / sizeof(Key))

// Scimitar
static const Key ScimKeys[] = {
    {0, "Left Mouse", "mouse1", 8, 0, 14, 32, false, true}, {0, "Right Mouse", "mouse2", 30, 0, 12, 32, false, true}, {0, "Middle Mouse", "mouse3", 22, 9, 8, 6, false, true}, {0, "Front light", "front", 30, 0, 12, 8, true, false },
    {0, "Wheel Up", "wheelup", 22, 3, 8, 6, false, true}, {0, "Wheel Down", "wheeldn", 22, 14, 8, 6, false, true}, {0, "Wheel Light", "wheel", 22, 3, 8, 17, true, false},
    {0, "DPI Up", "dpiup", 22, 19, 8, 9, false, true}, {0, "DPI Light", "dpi", 1, 12, 8, 4, true, false}, {0, "DPI Down", "dpidn", 22, 28, 8, 9, false, true},
    {0, "Thumb light", "thumb", 0, 21, 10, 24, true, false},
    {0, "1", "thumb1", -13, 18, 7, 7, false, true}, {0, "2", "thumb2", -6, 18, 7, 7, false, true}, {0, "3", "thumb3", 1, 18, 7, 7, false, true},
    {0, "4", "thumb4", -13, 25, 7, 7, false, true}, {0, "5", "thumb5", -6, 25, 7, 7, false, true}, {0, "6", "thumb6", 1, 25, 7, 7, false, true},
    {0, "7", "thumb7", -13, 32, 7, 7, false, true}, {0, "8", "thumb8", -6, 32, 7, 7, false, true}, {0, "9", "thumb9", 1, 32, 7, 7, false, true},
    {0, "10", "thumb10", -13, 39, 7, 7, false, true}, {0, "11", "thumb11", -6, 39, 7, 7, false, true}, {0, "12", "thumb12", 1, 39, 7, 7, false, true},
    {0, "Logo", "back", 14, 50, 24, 16, true, false}
};
#define KEYCOUNT_SCIM   (sizeof(ScimKeys) / sizeof(Key))

#define SCIM_WIDTH      M65_WIDTH
#define SCIM_HEIGHT     M65_HEIGHT

// M95
static const Key M95Keys[] = {
    {0, "Left Mouse", "mouse1", 9, 6, 14, 26, false, true}, {0, "Right Mouse", "mouse2", 30, 6, 14, 26, false, true}, {0, "Middle Mouse", "mouse3", 22, 8, 8, 7, false, true},
    {0, "4", "thumb6", 23, 28, 7, 12, false, true},
    {0, "Wheel Up", "wheelup", 22, 4, 8, 5, false, true}, {0, "Wheel Down", "wheeldn", 22, 14, 8, 5, false, true}, {0, "DPI Up (5)", "dpiup", 5, -1, 6, 9, false, true}, {0, "DPI Down (6)", "dpidn", 5, 6, 6, 9, false, true},
    {0, "Forward (7)", "mouse6", -42, 14, 14, 8, false, true}, {0, "Back (8)", "mouse7", -30, 14, 14, 8, false, true}, {0, "Sniper (14)", "sniper", -40, 20, 8, 14, false, true},
    {0, "9", "thumb1", -18, 14, 18, 8, false, true}, {0, "15", "thumb7", -18, 20, 10, 20, false, true},
    {0, "10", "thumb2", -10, 21, 10, 11, false, true},{0, "11", "thumb3", -10, 30, 10, 11, false, true},
    {0, "12", "thumb4", -18, 38, 18, 8, false, true}, {0, "13", "thumb5", -34, 38, 18, 8, false, true},
    {0, "Logo", "back", 20, 55, 12, 12, true, false}

};
#define KEYCOUNT_M95    (sizeof(M95Keys) / sizeof(Key))

// K95 Platinum lightbar
static const Key K95PLbar[] = {
    {0, 0, "topbar1", 4, -3, LBS, true, false}, {0, 0, "topbar2", 19, -3, LBS, true, false}, {0, 0, "topbar3", 34, -3, LBS, true, false}, {0, 0, "topbar4", 49, -3, LBS, true, false}, {0, 0, "topbar5", 64, -3, LBS, true, false}, {0, 0, "topbar6", 79, -3, LBS, true, false},
    {0, 0, "topbar7", 94, -3, LBS, true, false}, {0, 0, "topbar8", 109, -3, LBS, true, false}, {0, 0, "topbar9", 124, -3, LBS, true, false}, {0, 0, "topbar10", 139, -3, LBS, true, false}, {0, 0, "topbar11", 154, -3, LBS, true, false}, {0, 0, "topbar12", 169, -3, LBS, true, false},
    {0, 0, "topbar13", 184, -3, LBS, true, false}, {0, 0, "topbar14", 199, -3, LBS, true, false}, {0, 0, "topbar15", 214, -3, LBS, true, false}, {0, 0, "topbar16", 229, -3, LBS, true, false}, {0, 0, "topbar17", 244, -3, LBS, true, false}, {0, 0, "topbar18", 259, -3, LBS, true, false},
    {0, 0, "topbar19", 274, -3, LBS, true, false},
};
#define LBARCOUNT_K95P (sizeof(K95PLbar) / sizeof(Key))

// Mouse map - DARK CORE
static const Key DarkCoreKeys[] = {
    {0, "Left Mouse", "mouse1", 8, 0, 14, 32, false, true}, {0, "Right Mouse", "mouse2", 30, 0, 14, 32, false, true}, {0, "Middle Mouse", "mouse3", 22, 8, 8, 7, false, true},
    {0, "Wheel Up", "wheelup", 22, 4, 8, 5, false, true}, {0, "Wheel Down", "wheeldn", 22, 14, 8, 5, false, true}, {0, "Wheel Light", "zone3", 22, 15, 8, 8, true, false},
    {0, "DPI Up", "dpiup", 22, 19, 8, 6, false, true}, {0, "DPI Light", "dpi", 22, 24, 8, 8, true, false}, {0, "DPI Down", "dpidn", 22, 31, 8, 6, false, true},
    {0, "Forward", "mouse5", 5, 24, 5, 9, false, true}, {0, "Back", "mouse4", 5, 33, 5, 10, false, true}, {0, "Sniper", "sniper", 0, 25, 5, 15, false, true},
    // Stubs
    {0, "Alt Forward", "thumb1", 10, 44, 5, 9, false, true}, {0, "Alt Back", "thumb2", 10, 53, 5, 10, false, true},
    {0, "Logo", "zone1", 0, 48, 38, 12, true, false}, {0, "DPI Cycle", "dpiup", 31, 39, 6, 12, false, true},
    {0, "Side light", "zone2", 0, 21, 10, 24, true, false},


};
#define KEYCOUNT_DARKCORE    (sizeof(DarkCoreKeys) / sizeof(Key))

// MM800 Polaris
#define POLARIS_V 6, 14
#define POLARIS_H 14, 6

static const Key PolarisZones[] = {
    {0, "Zone 1", "zone1", 65, 4, POLARIS_V, true, false}, {0, "Zone 2", "zone2", 65, 16, POLARIS_V, true, false}, {0, "Zone 3", "zone3", 65, 28, POLARIS_V, true, false}, {0, "Zone 4", "zone4", 65, 40, POLARIS_V, true, false},
    {0, "Zone 5", "zone5", 65, 52, POLARIS_V, true, false}, {0, "Zone 6", "zone6", 43, 62, POLARIS_H, true, false}, {0, "Zone 7", "zone7", 31, 62, POLARIS_H, true, false}, {0, "Zone 8", "zone8", 19, 62, POLARIS_H, true, false},
    {0, "Zone 9", "zone9", 7, 62, POLARIS_H, true, false}, {0, "Zone 10", "zone10", -5, 62, POLARIS_H, true, false}, {0, "Zone 11", "zone11", -19, 52, POLARIS_V, true, false}, {0, "Zone 12", "zone12", -19, 40, POLARIS_V, true, false},
    {0, "Zone 13", "zone13", -19, 28, POLARIS_V, true, false}, {0, "Zone 14", "zone14", -19, 16, POLARIS_V, true, false}, {0, "Zone 15", "zone15", -19, 4, POLARIS_V, true, false}
};
#define KEYCOUNT_POLARIS   (sizeof(PolarisZones) / sizeof(Key))

#define ST100_V 6,22
#define ST100_H 22,6

static const Key ST100Zones[] = {
    {0, "Zone 1", "zone1", 16, 10, ST100_H, true, false}, {0, "Zone 2", "zone2", 36, 10, ST100_H, true, false}, {0, "Zone 3", "zone3", 52, 30, ST100_V, true, false}, {0, "Zone 4", "zone4", 52, 50, ST100_V, true, false},
    {0, "Logo", "zone5", 21, -1, NS, true, false}, // Logo
    {0, "Zone 6", "zone6", 16, 66, ST100_H, true, false}, {0, "Zone 7", "zone7", -4, 50, ST100_V, true, false}, {0, "Zone 8", "zone8", -4, 30, ST100_V, true, false},
    {0, "Zone 9", "zone9", -4, 10, ST100_H, true, false},
};
#define KEYCOUNT_ST100     (sizeof(ST100Zones) / sizeof(Key))

// Mouse map - Ironclaw
static const Key IronclawKeys[] = {
    // primary keys
    {0, "Left Mouse",    "mouse1",     12,  0, 12, 28, false, true  },
    {0, "Right Mouse",   "mouse2",     31,  0, 12, 28, false, true  },
    
    // center column keys
    {0, "Wheel Up",      "wheelup",    23,  3,  8,  7, false, true  },
    {0, "Middle Mouse",  "mouse3",     23,  7,  8,  6, false, true  },
    {0, "Wheel Down",    "wheeldn",    23, 12,  8,  7, false, true  },
    {0, "Profile Cycle", "profswitch", 23, 18,  9,  9, false, true  },
    {0, "DPI Cycle",     "dpiup",      23, 26,  8,  9, false, true  },
    
    // left side forward/back keys
    {0, "Forward",    "mouse5",      6, 20,  5, 12, false, true  },
    {0, "Back",       "mouse4",      7, 32,  5, 12, false, true  },

    // zones for LEDs
    {0, "Logo",          "back",       21, 50,  NS,    true,  false },
    {0, "Wheel",         "wheel",      23, 3,  8,  14, true,  false },
    
    // need to add DPI LED, even if not directly configurable for indicator to work
    {0, "DPI",           "dpi",        10, 10,  8,  8, true,  false }

};
#define KEYCOUNT_IRONCLAW    (sizeof(IronclawKeys) / sizeof(Key))

#define IRONCLAW_WIDTH       52
#define IRONCLAW_HEIGHT      67

// Nightsword
static const Key NightswordKeys[] = {
    //Primary buttons
    {0, "Left Mouse", "mouse1", 14, 6, 12, 26, false, true},
    {0, "Right Mouse", "mouse2", 34, 6, 12, 26, false, true},
    {0, "Wheel Up", "wheelup",      26, 5, 6, 5, false, true},
    {0, "Middle Mouse", "mouse3",   26, 10, 6, 5, false, true},
    {0, "Wheel Down", "wheeldn",    26, 15, 6, 5, false, true},
    //Lighting zones
    {0, "Wheel Light", "wheel", 26, 5, 8, 15, true, false},
    {0, "Front light", "front", 12, -2, 36, 8, true, false },
    {0, "Logo", "back",         24, 50, 12, 12, true, false},
    {0, "Back light", "side", 12, 64, 36, 8, true, false }, //Addition; side == bottom, under logo
    {0, "DPI Light", "dpi",     6, 11, 5, 12, true, false}, // thumb side DPI indicator light
    //DPI keys
    {0, "DPI Up", "dpiup", 12, 10, 5, 7, false, true},
    {0, "DPI Down", "dpidn", 12, 18, 5, 7, false, true},
    //Profile keys
    {0, "Profile Up", "profswitch", 26, 18,  6,  10, false, true  },
    {0, "Profile Down", "profdn", 26, 26,  6,  10, false, true  },
    //Thumb keys (extra should be sniper)
    {0, "Forward", "mouse5", 8, 21, 5, 9, false, true},
    {0, "Back", "mouse4", 8, 30, 5, 10, false, true},
    {0, "Sniper", "sniper", 0, 26, 8, 8, false, true}
};
#define KEYCOUNT_NIGHTSWORD  (sizeof(NightswordKeys) / sizeof(Key))

#define NIGHTSWORD_WIDTH     M65_WIDTH
#define NIGHTSWORD_HEIGHT    M65_HEIGHT

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
        map["profswitch"] = {0, "Profile Switch", "profswitch", 64, 1, 11, 8, true, true};
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
        map["profswitch"] = {0, "Profile Switch", "profswitch", 40, 0, 10, 8, true, true};

        map["light"].x = 49;
        map["light"].width = 10;
        map["light"].height = 8;

        map["lock"].x = 58;
        map["lock"].width = 10;
        map["lock"].height = 8;

        // Add logo1 (m3)
        map["m3"] = {0, "Logo 1", "m3", 118, 0, NS, true, false};
        // Add logo2 (g11)
        map["g11"] = {0, "Logo 2", "g11", 138, 0, NS, true, false};

        // Move mute and volup/dn one block to the left
        map["mute"].x -= 10;
        map["volup"].x -= 10;
        map["voldn"].x -= 10;
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

        if(KeyMap::isJP(layout))
            patchJP106fn(map);

        // Done!
        break;
    }
    case KeyMap::K63:{
        // Same as the K65 but without the Fn key
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
        if(KeyMap::isJP(layout))
            patchJP106fn(map);
        // Shift all keys to the left, and disable their LEDs
        QMutableHashIterator<QString, Key> i(map);
        while(i.hasNext()){
            i.next();
            i.value().x -= K95P_X_START;
            i.value().hasLed = false;
        }

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

        if(KeyMap::isJP(layout))
            patchJP106fn(map);

        // Done!
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
        map["wheel"] = {0, "Wheel Light", "wheel", map["front"].x, map["front"].y, map["front"].width, map["front"].height, true, false};
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
    default:;    // <- stop GCC from complaining
    }
    // Map is finished, return result
    return map;
}

KeyMap::Layout KeyMap::locale(QList<QPair<int, QString>>* layouts){
    setlocale(LC_ALL, "");
    QString loc = setlocale(LC_CTYPE, 0);
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

QPair<int, QString> KeyMap::addToList(int i, QStringList* list){
    return QPair<int, QString>(i, list->at(i));
}

QList<QPair<int, QString>> KeyMap::layoutNames(QString layout){
    QStringList list;
    list << "Danish"
         << "English (ISO/European)"
         << "English (ISO/European, Dvorak)"
         << "English (United Kingdom)"
         << "English (United Kingdom, Dvorak)"
         << "English (United States)"
         << "English (United States, Dvorak)"
         << "French"
         << "German"
         << "Italian"
         << "Japanese"
         << "Norwegian"
         << "Polish"
         << "Portuguese (Brazil)"
         << "Spanish (Latin America)"
         << "Spanish (Spain)"
         << "Swedish";

    // Create a list containing the layouts supported by the current device
    QList<QPair<int, QString>> retlist;
    if(layout == "ansi")
        retlist << KeyMap::addToList(5, &list)
                << KeyMap::addToList(6, &list);
    else if(layout == "iso")
        retlist << KeyMap::addToList(0, &list)
                << KeyMap::addToList(1, &list)
                << KeyMap::addToList(2, &list)
                << KeyMap::addToList(3, &list)
                << KeyMap::addToList(4, &list)
                << KeyMap::addToList(7, &list)
                << KeyMap::addToList(8, &list)
                << KeyMap::addToList(9, &list)
                << KeyMap::addToList(11, &list)
                << KeyMap::addToList(12, &list)
                << KeyMap::addToList(14, &list)
                << KeyMap::addToList(15, &list)
                << KeyMap::addToList(16, &list);
    else if(layout == "abnt")
        retlist << KeyMap::addToList(13, &list);
    else if(layout == "jis")
        retlist << KeyMap::addToList(10, &list);
    else
        for(int i = 0; i < list.count(); i++)
            retlist << KeyMap::addToList(i, &list);
    return retlist;
}

KeyMap::Model KeyMap::getModel(const QString& name){
    QString lower = name.toLower();
    if(lower == "k55")
        return K55;
    if(lower == "k63")
        return K63;
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
    if(lower == "darkcore")
        return DARKCORE;
    if(lower == "polaris")
        return POLARIS;
    if(lower == "st100")
        return ST100;
    if(lower == "k70mk2")
        return K70MK2;
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
    return NO_MODEL;
}

QString KeyMap::getModel(KeyMap::Model model){
    switch(model){
    case K55:
        return "k55";
    case K63:
        return "k63";
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
    case DARKCORE:
        return "darkcore";
    case POLARIS:
        return "polaris";
    case ST100:
        return "st100";
    case K70MK2:
        return "k70mk2";
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
    case K63:
        return K63_WIDTH;
    case K65:
        return K65_WIDTH;
    case K66:
    case K68:
        return K68_WIDTH;
    case K70:
    case K70MK2:
        return K70_WIDTH;
    case K95:
         return K95_WIDTH;
    case K95P:
    case K55:
        return K95P_WIDTH;
    case STRAFE:
    case STRAFE_MK2:
        return KSTRAFE_WIDTH;
    case M65:
    case M65E:
    case SABRE:
    case SCIMITAR:
    case HARPOON:
    case GLAIVE:
    case KATAR:
    case DARKCORE:
    case POLARIS:
    case ST100:
    case M95:
    case IRONCLAW:
    case NIGHTSWORD:
        return M65_WIDTH;
    default:
        return 0;
    }
}

int KeyMap::modelHeight(Model model){
    switch(model){
    case K55:
    case K63:
    case K65:
    case K66:
    case K68:
    case K70:
    case K70MK2:
    case K95:
    case STRAFE:
    case STRAFE_MK2:
        return K95_HEIGHT;
    case K95P:
        return K95P_HEIGHT;
    case M65:
    case M65E:
    case SABRE:
    case SCIMITAR:
    case HARPOON:
    case GLAIVE:
    case KATAR:
    case DARKCORE:
    case POLARIS:
    case ST100:
    case M95:
    case IRONCLAW:
    case NIGHTSWORD:
        return M65_HEIGHT;
    default:
        return 0;
    }
}

KeyMap::KeyMap(Model _keyModel, Layout _keyLayout) :
    _keys(getMap(_keyModel, _keyLayout)),
    keyWidth(modelWidth(_keyModel)), keyHeight(modelHeight(_keyModel)),
    keyModel(_keyModel), keyLayout(_keyLayout)
{}

KeyMap::KeyMap() :
     keyWidth(0), keyHeight(0),
     keyModel(NO_MODEL), keyLayout(NO_LAYOUT)
{}

QStringList KeyMap::byPosition() const {
    // Use QMaps to order the keys
    QMap<int, QMap<int, QString> > ordered;
    QHashIterator<QString, Key> i(*this);
    while(i.hasNext()){
        i.next();
        const Key& key = i.value();
        ordered[key.y][key.x] = i.key();
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
        return map[key].friendlyName();

    // The only key missing from it should be Fn, which is found on STRAFE
    map = KeyMap(STRAFE, layout);
    if(map.contains(key))
        return map[key].friendlyName();

    // Additionally, there are a handful of keys not present on any physical keyboard, but we need names for them
    if(key == "f13" || key == "f14" || key == "f15" || key == "f16" || key == "f17" || key == "f18" || key == "f19" || key == "f20"
        || key == "f21" || key == "f22" || key == "f23" || key == "f24")
        return key.toUpper();
    else if(key == "lightup")
        return "Screen Brightness Up";
    else if(key == "lightdn")
        return "Screen Brightness Down";
    else if(key == "eject" || key == "power")
        return key[0].toUpper() + key.mid(1);   // capitalize first letter

    // All other names are found on mice
    map = KeyMap(SCIMITAR, layout);
    if(map.contains(key))
        return map[key].friendlyName();
    map = KeyMap(M65, layout);
    if(map.contains(key))
        return map[key].friendlyName();
    map = KeyMap(HARPOON, layout);
    if(map.contains(key))
        return map[key].friendlyName();
    map = KeyMap(IRONCLAW, layout);
    if(map.contains(key))
        return map[key].friendlyName();
    map = KeyMap(NIGHTSWORD, layout);
    if(map.contains(key))
        return map[key].friendlyName();

    // Not found at all
    return "";
}

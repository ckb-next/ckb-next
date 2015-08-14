#include <clocale>
#include <QMap>
#include "keymap.h"

// Normal key size
#define NS 12, 12

// Key positions (K95 - English)
// This is the master key map that includes both ANSI and ISO layouts - use ansiPatch() or isoPatch() to finalize it
static const Key K95Keys[] = {
    {0, 0, "mr", 38, 0, NS, true, true}, {0, 0, "m1", 50, 0, NS, true, true}, {0, 0, "m2", 62, 0, NS, true, true}, {0, 0, "m3", 74, 0, NS, true, true}, {0, "Brightness", "light", 222, 0, NS, true, true}, {0, "Windows Lock", "lock", 234, 0, NS, true, true}, {0, "Mute", "mute", 273, 0, 13, 8, true, true}, {0, "Volume Up", "volup", 290, -2, 18, 6, false, true}, {0, "Volume down", "voldn", 290, 2, 18, 6, false, true},
    {0, 0, "g1", 0, 14, NS, true, true}, {0, 0, "g2", 11, 14, NS, true, true}, {0, 0, "g3", 22, 14, NS, true, true}, {0, "Esc", "esc", 38, 14, NS, true, true}, {0, 0, "f1", 58, 14, NS, true, true}, {0, 0, "f2", 70, 14, NS, true, true}, {0, 0, "f3", 82, 14, NS, true, true}, {0, 0, "f4", 94, 14, NS, true, true}, {0, 0, "f5", 114, 14, NS, true, true}, {0, 0, "f6", 126, 14, NS, true, true}, {0, 0, "f7", 138, 14, NS, true, true}, {0, 0, "f8", 150, 14, NS, true, true}, {0, 0, "f9", 170, 14, NS, true, true}, {0, 0, "f10", 182, 14, NS, true, true}, {0, 0, "f11", 194, 14, NS, true, true}, {0, 0, "f12", 206, 14, NS, true, true}, {0, "Print Screen\nSysRq", "prtscn", 222, 14, NS, true, true}, {0, "Scroll Lock", "scroll", 234, 14, NS, true, true}, {0, "Pause\nBreak", "pause", 246, 14, NS, true, true}, {0, "Stop", "stop", 262, 14, 12, 8, true, true}, {0, "Previous", "prev", 273, 14, 13, 8, true, true}, {0, "Play/Pause", "play", 285, 14, 13, 8, true, true}, {0, "Next", "next", 296, 14, 12, 8, true, true},
    {0, 0, "g4", 0, 25, NS, true, true}, {0, 0, "g5", 11, 25, NS, true, true}, {0, 0, "g6", 22, 25, NS, true, true}, {0, "`", "grave", 38, 27, NS, true, true}, {0, 0, "1", 50, 27, NS, true, true}, {0, 0, "2", 62, 27, NS, true, true}, {0, 0, "3", 74, 27, NS, true, true}, {0, 0, "4", 86, 27, NS, true, true}, {0, 0, "5", 98, 27, NS, true, true}, {0, 0, "6", 110, 27, NS, true, true}, {0, 0, "7", 122, 27, NS, true, true}, {0, 0, "8", 134, 27, NS, true, true}, {0, 0, "9", 146, 27, NS, true, true}, {0, 0, "0", 158, 27, NS, true, true}, {0, "-", "minus", 170, 27, NS, true, true}, {0, "=", "equal", 182, 27, NS, true, true}, {0, "Backspace", "bspace", 200, 27, 24, 12, true, true}, {0, "Insert", "ins", 222, 27, NS, true, true}, {0, "Home", "home", 234, 27, NS, true, true}, {0, "Page Up", "pgup", 246, 27, NS, true, true}, {0, "Num Lock", "numlock", 261, 27, NS, true, true}, {0, "NumPad /", "numslash", 273, 27, NS, true, true}, {0, "NumPad *", "numstar", 285, 27, NS, true, true}, {0, "NumPad -", "numminus", 297, 27, NS, true, true},
    {0, 0, "g7", 0, 39, NS, true, true}, {0, 0, "g8", 11, 39, NS, true, true}, {0, 0, "g9", 22, 39, NS, true, true}, {0, "Tab", "tab", 41, 39, 18, 12, true, true}, {0, 0, "q", 56, 39, NS, true, true}, {0, 0, "w", 68, 39, NS, true, true}, {0, 0, "e", 80, 39, NS, true, true}, {0, 0, "r", 92, 39, NS, true, true}, {0, 0, "t", 104, 39, NS, true, true}, {0, 0, "y", 116, 39, NS, true, true}, {0, 0, "u", 128, 39, NS, true, true}, {0, 0, "i", 140, 39, NS, true, true}, {0, 0, "o", 152, 39, NS, true, true}, {0, 0, "p", 164, 39, NS, true, true}, {0, "[", "lbrace", 176, 39, NS, true, true}, {0, "]", "rbrace", 188, 39, NS, true, true}, {0, "\\", "bslash", 203, 39, 18, 12, true, true}, {0, "Enter", "enter", 203, 39, 18, 24, true, true}, {0, "Delete", "del", 222, 39, NS, true, true}, {0, "End", "end", 234, 39, NS, true, true}, {0, "Page Down", "pgdn", 246, 39, NS, true, true}, {0, "NumPad 7", "num7", 261, 39, NS, true, true}, {0, "NumPad 8", "num8", 273, 39, NS, true, true}, {0, "NumPad 9", "num9", 285, 39, NS, true, true}, {0, "NumPad +", "numplus", 297, 45, 12, 24, true, true},
    {0, 0, "g10", 0, 50, NS, true, true}, {0, 0, "g11", 11, 50, NS, true, true}, {0, 0, "g12", 22, 50, NS, true, true}, {0, "Caps Lock", "caps", 42, 51, 20, 12, true, true}, {0, 0, "a", 59, 51, NS, true, true}, {0, 0, "s", 71, 51, NS, true, true}, {0, 0, "d", 83, 51, NS, true, true}, {0, 0, "f", 95, 51, NS, true, true}, {0, 0, "g", 107, 51, NS, true, true}, {0, 0, "h", 119, 51, NS, true, true}, {0, 0, "j", 131, 51, NS, true, true}, {0, 0, "k", 143, 51, NS, true, true}, {0, 0, "l", 155, 51, NS, true, true}, {0, ";", "colon", 167, 51, NS, true, true}, {0, "'", "quote", 179, 51, NS, true, true}, {0, "#", "hash", 191, 51, NS, true, true}, {0, "NumPad 4", "num4", 261, 51, NS, true, true}, {0, "NumPad 5", "num5", 273, 51, NS, true, true}, {0, "NumPad 6", "num6", 285, 51, NS, true, true},
    {0, 0, "g13", 0, 64, NS, true, true}, {0, 0, "g14", 11, 64, NS, true, true}, {0, 0, "g15", 22, 64, NS, true, true}, {0, "Left Shift", "lshift", 39, 63, 14, 12, true, true}, {"bslash", "\\", "bslash_iso", 53, 63, NS, true, true}, {0, 0, "z", 65, 63, NS, true, true}, {0, 0, "x", 77, 63, NS, true, true}, {0, 0, "c", 89, 63, NS, true, true}, {0, 0, "v", 101, 63, NS, true, true}, {0, 0, "b", 113, 63, NS, true, true}, {0, 0, "n", 125, 63, NS, true, true}, {0, 0, "m", 137, 63, NS, true, true}, {0, ",", "comma", 149, 63, NS, true, true}, {0, ".", "dot", 161, 63, NS, true, true}, {0, "/", "slash", 173, 63, NS, true, true}, {0, "Right Shift", "rshift", 196, 63, 32, 12, true, true}, {0, "Up", "up", 234, 63, NS, true, true}, {0, "NumPad 1", "num1", 261, 63, NS, true, true}, {0, "NumPad 2", "num2", 273, 63, NS, true, true}, {0, "NumPad 3", "num3", 285, 63, NS, true, true}, {0, "NumPad Enter", "numenter", 297, 69, 12, 24, true, true},
    {0, 0, "g16", 0, 75, NS, true, true}, {0, 0, "g17", 11, 75, NS, true, true}, {0, 0, "g18", 22, 75, NS, true, true}, {0, "Left Ctrl", "lctrl", 40, 75, 16, 12, true, true}, {0, "Left Windows", "lwin", 54, 75, NS, true, true}, {0, "Left Alt", "lalt", 67, 75, 14, 12, true, true}, {0, "Space", "space", 116, 75, 84, 12, true, true}, {0, "Right Alt", "ralt", 165, 75, 14, 12, true, true}, {0, "Right Windows", "rwin", 178, 75, NS, true, true}, {0, "Menu", "rmenu", 190, 75, NS, true, true}, {0, "Right Ctrl", "rctrl", 204, 75, 16, 12, true, true}, {0, "Left", "left", 222, 75, NS, true, true}, {0, "Down", "down", 234, 75, NS, true, true}, {0, "Right", "right", 246, 75, NS, true, true}, {0, "NumPad 0", "num0", 267, 75, 24, 12, true, true}, {0, "NumPad .", "numdot", 285, 75, NS, true, true}
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
static void patchISO(QHash<QString, Key>& map){
    map.remove("bslash");
}
static void patchANSI(QHash<QString, Key>& map){
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

// Total width/height
#define K95_WIDTH       298
#define K95_HEIGHT      76

// K70 cuts off the G keys on the left, as well as MR/M1/M2/M3
#define K70_X_START     38
#define K70_WIDTH       (K95_WIDTH - K70_X_START)
#define K70_HEIGHT      K95_HEIGHT

// K65 additionally removes the numpad on the right, and has a different top row
#define K65_WIDTH       209
#define K65_HEIGHT      K70_HEIGHT

static const Key K65TopRow[] = {
    {0, "Brightness", "light", 164 - K70_X_START, 0, 12, 12, true, true}, {0, "Mute", "mute", 176 - K70_X_START, 0, 12, 12, true, true}, {0, "Volume Down", "voldn", 192 - K70_X_START, 0, 14, 8, true, true}, {0, "Volume Up", "volup", 205 - K70_X_START, 0, 14, 8, true, true}, {0, "Windows Lock", "lock", 222 - K70_X_START, 0, 12, 12, true, true}
};
#define K65_TOP_COUNT (sizeof(K65TopRow) / sizeof(Key))

// Mouse map
static const Key M65Keys[] = {
    {0, "Left Mouse", "mouse1", 8, 0, 14, 32, false, true}, {0, "Right Mouse", "mouse2", 30, 0, 14, 32, false, true}, {0, "Middle Mouse", "mouse3", 22, 8, 8, 7, false, true},
    {0, "Wheel Up", "wheelup", 22, 4, 8, 5, false, true}, {0, "Wheel Down", "wheeldn", 22, 14, 8, 5, false, true}, {0, "Wheel Light", "front", 22, 15, 8, 8, true, false},
    {0, "DPI Up", "dpiup", 22, 19, 8, 6, false, true}, {0, "DPI Light", "dpi", 22, 24, 8, 8, true, false}, {0, "DPI Down", "dpidn", 22, 31, 8, 6, false, true},
    {0, "Forward", "mouse5", 5, 24, 5, 9, false, true}, {0, "Back", "mouse4", 5, 33, 5, 10, false, true}, {0, "Sniper", "sniper", 0, 25, 5, 15, false, true},
    {0, "Logo", "back", 14, 55, 24, 12, true, false}
};
#define KEYCOUNT_M65 (sizeof(M65Keys) / sizeof(Key))

#define M65_WIDTH       52
#define M65_HEIGHT      67

// Map getter. Each model/layout pair only needs to be constructed once; after that, future KeyMaps can copy the existing maps.
#define N_MODELS    KeyMap::_MODEL_MAX
#define N_LAYOUTS   KeyMap::_LAYOUT_MAX
static QHash<QString, Key> K95BaseMap, M65BaseMap;
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
        case KeyMap::FR:
            patch(map, patchFR);
            break;
        case KeyMap::DE:
            patch(map, patchDE);
            break;
        case KeyMap::IT:
            patch(map, patchIT);
            break;
        case KeyMap::ES:
            patch(map, patchES);
            break;
        case KeyMap::SE:
            patch(map, patchSE);
            break;
        case KeyMap::GB_DVORAK:
        case KeyMap::US_DVORAK:
            patch(map, patchDvorak);
            break;
        default:;
            // English QWERTY - no patch needed
        }
        if(KeyMap::isISO(layout))
            patchISO(map);
        else
            patchANSI(map);
        // Done! return the map
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
        // Done!
        break;
    }
    case KeyMap::M65:{
        // M65 isn't a keyboard; it has its own base
        for(const Key* key = M65Keys; key < M65Keys + KEYCOUNT_M65; key++){
            // Keyboard keys are written from the center because that's where the LEDs are, but the mouse buttons are odd shapes so they're
            // written from the upper left
            Key translatedKey = *key;
            translatedKey.x += translatedKey.width / 2;
            translatedKey.y += translatedKey.height / 2;
            M65BaseMap[key->name] = translatedKey;
        }
        map = M65BaseMap;
        // Mice have no layout patches - no other changes necessary
        break;
    }
    default:;    // <- stop GCC from complaining
    }
    // Map is finished, return result
    return map;
}

KeyMap::Layout KeyMap::locale(){
    setlocale(LC_ALL, "");
    QString loc = setlocale(LC_CTYPE, 0);
    loc = loc.toLower().replace('_', '-');
    if(loc.startsWith("fr-"))
        return KeyMap::FR;
    else if(loc.startsWith("de-"))
        return KeyMap::DE;
    else if(loc.startsWith("it-"))
        return KeyMap::IT;
    else if(loc.startsWith("es-"))
        return KeyMap::ES;
    else if(loc.startsWith("se-"))
        return KeyMap::SE;
    else if(loc.startsWith("en-us") || loc.startsWith("en-au") || loc.startsWith("en-ca") || loc.startsWith("en-hk") || loc.startsWith("en-in") || loc.startsWith("en-nz") || loc.startsWith("en-ph") || loc.startsWith("en-sg") || loc.startsWith("en-za"))
        // Most English locales use US
        return KeyMap::US;
    else
        // Default to GB
        return KeyMap::GB;
}

KeyMap::Layout KeyMap::getLayout(const QString& name){
    QString lower = name.toLower();
    if(lower == "fr")
        return FR;
    if(lower == "de")
        return DE;
    if(lower == "it")
        return IT;
    if(lower == "es")
        return ES;
    if(lower == "se")
        return SE;
    if(lower == "gb")
        return GB;
    if(lower == "gb_dvorak")
        return GB_DVORAK;
    if(lower == "us")
        return US;
    if(lower == "us_dvorak")
        return US_DVORAK;
    return NO_LAYOUT;
}

QString KeyMap::getLayout(KeyMap::Layout layout){
    switch(layout){
    case FR:
        return "fr";
    case DE:
        return "de";
    case IT:
        return "it";
    case ES:
        return "es";
    case SE:
        return "se";
    case GB:
        return "gb";
    case GB_DVORAK:
        return "gb_dvorak";
    case US:
        return "us";
    case US_DVORAK:
        return "us_dvorak";
    default:
        return "";
    }
}

KeyMap::Model KeyMap::getModel(const QString& name){
    QString lower = name.toLower();
    if(lower == "k65")
        return K65;
    if(lower == "k70")
        return K70;
    if(lower == "k95")
        return K95;
    if(lower == "m65")
        return M65;
    return NO_MODEL;
}

QString KeyMap::getModel(KeyMap::Model model){
    switch(model){
    case K65:
        return "k65";
    case K70:
        return "k70";
    case K95:
        return "k95";
    case M65:
        return "m65";
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

KeyMap::KeyMap(Model _keyModel, Layout _keyLayout) :
    _keys(getMap(_keyModel, _keyLayout)),
    keyWidth(_keyModel == K95 ? K95_WIDTH : _keyModel == K70 ? K70_WIDTH : _keyModel == K65 ? K65_WIDTH : _keyModel == M65 ? M65_WIDTH : 0),
    keyHeight(_keyModel == K95 || _keyModel == K70 || _keyModel == K65 ? K95_HEIGHT : _keyModel == M65 ? M65_HEIGHT : 0),
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
    KeyMap map(K95, layout);
    if(map.contains(key))
        return map[key].friendlyName();
    // If that didn't work, try M65
    map = KeyMap(M65, layout);
    if(map.contains(key))
        return map[key].friendlyName();
    return "";
}

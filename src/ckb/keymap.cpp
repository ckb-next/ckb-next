#include "keymap.h"

// Normal size
#define NS 12, 12

#ifdef __APPLE__
#define WIN_NAME "Cmd"
#else
#define WIN_NAME "Super"
#endif

// Key positions (K95)
static KeyPos K95PosUS[] = {
    {0, "mr", 38, 0, NS}, {0, "m1", 50, 0, NS}, {0, "m2", 62, 0, NS}, {0, "m3", 74, 0, NS}, {"Brightness", "light", 222, 0, NS}, {"Windows Lock", "lock", 234, 0, NS}, {"Mute", "mute", 273, 0, 13, 8},
    {0, "g1", 0, 14, NS}, {0, "g2", 11, 14, NS}, {0, "g3", 22, 14, NS}, {"Esc", "esc", 38, 14, NS}, {0, "f1", 58, 14, NS}, {0, "f2", 70, 14, NS}, {0, "f3", 82, 14, NS}, {0, "f4", 94, 14, NS}, {0, "f5", 114, 14, NS}, {0, "f6", 126, 14, NS}, {0, "f7", 138, 14, NS}, {0, "f8", 150, 14, NS}, {0, "f9", 170, 14, NS}, {0, "f10", 182, 14, NS}, {0, "f11", 194, 14, NS}, {0, "f12", 206, 14, NS}, {"Print Screen\nSysRq", "prtscn", 222, 14, NS}, {"Scroll Lock", "scroll", 234, 14, NS}, {"Pause\nBreak", "pause", 246, 14, NS}, {"Stop", "stop", 262, 14, 12, 8}, {"Previous", "prev", 273, 14, 13, 8}, {"Play/Pause", "play", 285, 14, 13, 8}, {"Next", "next", 296, 14, 12, 8},
    {0, "g4", 0, 25, NS}, {0, "g5", 11, 25, NS}, {0, "g6", 22, 25, NS}, {"`", "grave", 38, 27, NS}, {0, "1", 50, 27, NS}, {0, "2", 62, 27, NS}, {0, "3", 74, 27, NS}, {0, "4", 86, 27, NS}, {0, "5", 98, 27, NS}, {0, "6", 110, 27, NS}, {0, "7", 122, 27, NS}, {0, "8", 134, 27, NS}, {0, "9", 146, 27, NS}, {0, "0", 158, 27, NS}, {"-", "minus", 170, 27, NS}, {"=", "equal", 182, 27, NS}, {"Backspace", "bspace", 200, 27, 24, 12}, {"Insert", "ins", 222, 27, NS}, {"Home", "home", 234, 27, NS}, {"Page Up", "pgup", 246, 27, NS}, {"Num Lock", "numlock", 261, 27, NS}, {"NumPad /", "numslash", 273, 27, NS}, {"NumPad *", "numstar", 285, 27, NS}, {"NumPad -", "numminus", 297, 27, NS},
    {0, "g7", 0, 39, NS}, {0, "g8", 11, 39, NS}, {0, "g9", 22, 39, NS}, {"Tab", "tab", 41, 39, 18, 12}, {0, "q", 56, 39, NS}, {0, "w", 68, 39, NS}, {0, "e", 80, 39, NS}, {0, "r", 92, 39, NS}, {0, "t", 104, 39, NS}, {0, "y", 116, 39, NS}, {0, "u", 128, 39, NS}, {0, "i", 140, 39, NS}, {0, "o", 152, 39, NS}, {0, "p", 164, 39, NS}, {"[", "lbrace", 176, 39, NS}, {"]", "rbrace", 188, 39, NS}, {"\\", "bslash", 203, 39, 18, 12}, {"Delete", "del", 222, 39, NS}, {"End", "end", 234, 39, NS}, {"Page Down", "pgdn", 246, 39, NS}, {"NumPad 7", "num7", 261, 39, NS}, {"NumPad 8", "num8", 273, 39, NS}, {"NumPad 9", "num9", 285, 39, NS}, {"NumPad +", "numplus", 297, 45, 12, 24},
    {0, "g10", 0, 50, NS}, {0, "g11", 11, 50, NS}, {0, "g12", 22, 50, NS}, {"Caps Lock", "caps", 42, 51, 20, 12}, {0, "a", 59, 51, NS}, {0, "s", 71, 51, NS}, {0, "d", 83, 51, NS}, {0, "f", 95, 51, NS}, {0, "g", 107, 51, NS}, {0, "h", 119, 51, NS}, {0, "j", 131, 51, NS}, {0, "k", 143, 51, NS}, {0, "l", 155, 51, NS}, {";", "colon", 167, 51, NS}, {"'", "quote", 179, 51, NS}, {"Enter", "enter", 199, 51, 26, 12}, {"NumPad 4", "num4", 261, 51, NS}, {"NumPad 5", "num5", 273, 51, NS}, {"NumPad 6", "num6", 285, 51, NS},
    {0, "g13", 0, 64, NS}, {0, "g14", 11, 64, NS}, {0, "g15", 22, 64, NS}, {"Shift", "lshift", 45, 63, 26, 12}, {0, "z", 65, 63, NS}, {0, "x", 77, 63, NS}, {0, "c", 89, 63, NS}, {0, "v", 101, 63, NS}, {0, "b", 113, 63, NS}, {0, "n", 125, 63, NS}, {0, "m", 137, 63, NS}, {",", "comma", 149, 63, NS}, {".", "dot", 161, 63, NS}, {"/", "slash", 173, 63, NS}, {"Right Shift", "rshift", 196, 63, 32, 12}, {"Up", "up", 234, 63, NS}, {"NumPad 1", "num1", 261, 63, NS}, {"NumPad 2", "num2", 273, 63, NS}, {"NumPad 3", "num3", 285, 63, NS}, {"NumPad Enter", "numenter", 297, 69, 12, 24},
    {0, "g16", 0, 75, NS}, {0, "g17", 11, 75, NS}, {0, "g18", 22, 75, NS}, {"Ctrl", "lctrl", 40, 75, 16, 12}, {WIN_NAME, "lwin", 54, 75, NS}, {"Alt", "lalt", 67, 75, 14, 12}, {"Space", "space", 116, 75, 84, 12}, {"Right Alt", "ralt", 165, 75, 14, 12}, {"Right " WIN_NAME, "rwin", 178, 75, NS}, {"Menu", "rmenu", 190, 75, NS}, {"Right Ctrl", "rctrl", 204, 75, 16, 12}, {"Left", "left", 222, 75, NS}, {"Down", "down", 234, 75, NS}, {"Right", "right", 246, 75, NS}, {"NumPad 0", "num0", 267, 75, 24, 12}, {"NumPad .", "numdot", 285, 75, NS},
};

static KeyPos K95PosUK[] = {
    {0, "mr", 38, 0, NS}, {0, "m1", 50, 0, NS}, {0, "m2", 62, 0, NS}, {0, "m3", 74, 0, NS}, {"Brightness", "light", 222, 0, NS}, {"Windows Lock", "lock", 234, 0, NS}, {"Mute", "mute", 273, 0, 13, 8},
    {0, "g1", 0, 14, NS}, {0, "g2", 11, 14, NS}, {0, "g3", 22, 14, NS}, {"Esc", "esc", 38, 14, NS}, {0, "f1", 58, 14, NS}, {0, "f2", 70, 14, NS}, {0, "f3", 82, 14, NS}, {0, "f4", 94, 14, NS}, {0, "f5", 114, 14, NS}, {0, "f6", 126, 14, NS}, {0, "f7", 138, 14, NS}, {0, "f8", 150, 14, NS}, {0, "f9", 170, 14, NS}, {0, "f10", 182, 14, NS}, {0, "f11", 194, 14, NS}, {0, "f12", 206, 14, NS}, {"Print Screen\nSysRq", "prtscn", 222, 14, NS}, {"Scroll Lock", "scroll", 234, 14, NS}, {"Pause\nBreak", "pause", 246, 14, NS}, {"Stop", "stop", 262, 14, 12, 8}, {"Previous", "prev", 273, 14, 13, 8}, {"Play/Pause", "play", 285, 14, 13, 8}, {"Next", "next", 296, 14, 12, 8},
    {0, "g4", 0, 25, NS}, {0, "g5", 11, 25, NS}, {0, "g6", 22, 25, NS}, {"`", "grave", 38, 27, NS}, {0, "1", 50, 27, NS}, {0, "2", 62, 27, NS}, {0, "3", 74, 27, NS}, {0, "4", 86, 27, NS}, {0, "5", 98, 27, NS}, {0, "6", 110, 27, NS}, {0, "7", 122, 27, NS}, {0, "8", 134, 27, NS}, {0, "9", 146, 27, NS}, {0, "0", 158, 27, NS}, {"-", "minus", 170, 27, NS}, {"=", "equal", 182, 27, NS}, {"Backspace", "bspace", 200, 27, 24, 12}, {"Insert", "ins", 222, 27, NS}, {"Home", "home", 234, 27, NS}, {"Page Up", "pgup", 246, 27, NS}, {"Num Lock", "numlock", 261, 27, NS}, {"NumPad /", "numslash", 273, 27, NS}, {"NumPad *", "numstar", 285, 27, NS}, {"NumPad -", "numminus", 297, 27, NS},
    {0, "g7", 0, 39, NS}, {0, "g8", 11, 39, NS}, {0, "g9", 22, 39, NS}, {"Tab", "tab", 41, 39, 18, 12}, {0, "q", 56, 39, NS}, {0, "w", 68, 39, NS}, {0, "e", 80, 39, NS}, {0, "r", 92, 39, NS}, {0, "t", 104, 39, NS}, {0, "y", 116, 39, NS}, {0, "u", 128, 39, NS}, {0, "i", 140, 39, NS}, {0, "o", 152, 39, NS}, {0, "p", 164, 39, NS}, {"[", "lbrace", 176, 39, NS}, {"]", "rbrace", 188, 39, NS}, {"Enter", "enter", 203, 39, 18, 24}, {"Delete", "del", 222, 39, NS}, {"End", "end", 234, 39, NS}, {"Page Down", "pgdn", 246, 39, NS}, {"NumPad 7", "num7", 261, 39, NS}, {"NumPad 8", "num8", 273, 39, NS}, {"NumPad 9", "num9", 285, 39, NS}, {"NumPad +", "numplus", 297, 45, 12, 24},
    {0, "g10", 0, 50, NS}, {0, "g11", 11, 50, NS}, {0, "g12", 22, 50, NS}, {"Caps Lock", "caps", 42, 51, 20, 12}, {0, "a", 59, 51, NS}, {0, "s", 71, 51, NS}, {0, "d", 83, 51, NS}, {0, "f", 95, 51, NS}, {0, "g", 107, 51, NS}, {0, "h", 119, 51, NS}, {0, "j", 131, 51, NS}, {0, "k", 143, 51, NS}, {0, "l", 155, 51, NS}, {";", "colon", 167, 51, NS}, {"'", "quote", 179, 51, NS}, {0, "hash", 191, 51, NS}, {"NumPad 4", "num4", 261, 51, NS}, {"NumPad 5", "num5", 273, 51, NS}, {"NumPad 6", "num6", 285, 51, NS},
    {0, "g13", 0, 64, NS}, {0, "g14", 11, 64, NS}, {0, "g15", 22, 64, NS}, {"Shift", "lshift", 39, 63, 14, 12}, {"\\", "bslash", 53, 63, NS}, {0, "z", 65, 63, NS}, {0, "x", 77, 63, NS}, {0, "c", 89, 63, NS}, {0, "v", 101, 63, NS}, {0, "b", 113, 63, NS}, {0, "n", 125, 63, NS}, {0, "m", 137, 63, NS}, {",", "comma", 149, 63, NS}, {".", "dot", 161, 63, NS}, {"/", "slash", 173, 63, NS}, {"Right Shift", "rshift", 196, 63, 32, 12}, {"Up", "up", 234, 63, NS}, {"NumPad 1", "num1", 261, 63, NS}, {"NumPad 2", "num2", 273, 63, NS}, {"NumPad 3", "num3", 285, 63, NS}, {"NumPad Enter", "numenter", 297, 69, 12, 24},
    {0, "g16", 0, 75, NS}, {0, "g17", 11, 75, NS}, {0, "g18", 22, 75, NS}, {"Ctrl", "lctrl", 40, 75, 16, 12}, {WIN_NAME, "lwin", 54, 75, NS}, {"Alt", "lalt", 67, 75, 14, 12}, {"Space", "space", 116, 75, 84, 12}, {"Right Alt", "ralt", 165, 75, 14, 12}, {"Right " WIN_NAME, "rwin", 178, 75, NS}, {"Menu", "rmenu", 190, 75, NS}, {"Right Ctrl", "rctrl", 204, 75, 16, 12}, {"Left", "left", 222, 75, NS}, {"Down", "down", 234, 75, NS}, {"Right", "right", 246, 75, NS}, {"NumPad 0", "num0", 267, 75, 24, 12}, {"NumPad .", "numdot", 285, 75, NS},
};

#define KEYCOUNT_K95_US (sizeof(K95PosUS)/sizeof(KeyPos))
#define KEYCOUNT_K95_UK (sizeof(K95PosUK)/sizeof(KeyPos))

#define K95_WIDTH       298
#define K95_HEIGHT      76

static KeyPos* K70PosUS = 0;
static KeyPos* K70PosUK = 0;

#define K70_COUNT_DIFF  22 // 18 G keys, MR, M1, M2, M3
#define KEYCOUNT_K70_US (KEYCOUNT_K95_US - K70_COUNT_DIFF)
#define KEYCOUNT_K70_UK (KEYCOUNT_K95_UK - K70_COUNT_DIFF)

#define K70_X_START     38
#define K70_WIDTH       (K95_WIDTH - K70_X_START)
#define K70_HEIGHT      K95_HEIGHT

KeyMap KeyMap::standard(KeyMap::Model model, KeyMap::Layout layout){
    if(model == K70){
        // Build the K70 tables if not done yet
        if(!K70PosUS){
            K70PosUS = new KeyPos[KEYCOUNT_K70_US];
            K70PosUK = new KeyPos[KEYCOUNT_K70_UK];
            // Build US table
            int j = 0;
            KeyPos* keyIn = K95PosUS, *keyOut = K70PosUS;
            for(uint i = 0; i < KEYCOUNT_K95_US; i++){
                // Remove anything before X-start (G keys) as well as the M keys
                if(keyIn[i].x < K70_X_START || !strcmp(keyIn[i].name, "mr")
                        || !strcmp(keyIn[i].name, "m1") || !strcmp(keyIn[i].name, "m2") || !strcmp(keyIn[i].name, "m3"))
                    continue;
                keyOut[j] = keyIn[i];
                keyOut[j].x -= K70_X_START;
                j++;
            }
            // Do the same thing for the UK table
            j = 0;
            keyIn = K95PosUK;
            keyOut = K70PosUK;
            for(uint i = 0; i < KEYCOUNT_K95_UK; i++){
                if(keyIn[i].x < K70_X_START || !strcmp(keyIn[i].name, "mr")
                        || !strcmp(keyIn[i].name, "m1") || !strcmp(keyIn[i].name, "m2") || !strcmp(keyIn[i].name, "m3"))
                    continue;
                keyOut[j] = keyIn[i];
                keyOut[j].x -= K70_X_START;
                j++;
            }
        }
        if(layout == US)
            return KeyMap(KEYCOUNT_K70_US, K70_WIDTH, K70PosUS);
        else
            return KeyMap(KEYCOUNT_K70_UK, K70_WIDTH, K70PosUK);
    }
    // K95
    if(layout == US)
        return KeyMap(KEYCOUNT_K95_US, K95_WIDTH, K95PosUS);
    else
        return KeyMap(KEYCOUNT_K95_UK, K95_WIDTH, K95PosUK);
}

KeyMap::KeyMap(uint _keyCount, uint _width, const KeyPos* _positions) :
    keyCount(_keyCount), keyWidth(_width), keyHeight(K95_HEIGHT), positions(_positions), rgb(new QRgb[_keyCount]())
{}

KeyMap::KeyMap() :
    keyCount(0), keyWidth(0), keyHeight(0), positions(0), rgb(0)
{}

KeyMap::~KeyMap(){
    delete[] rgb;
}

const KeyMap& KeyMap::operator = (const KeyMap& rhs){
    if(keyCount < rhs.keyCount){
        delete[] rgb;
        rgb = new QRgb[rhs.keyCount]();
    }
    keyCount = rhs.keyCount;
    keyWidth = rhs.keyWidth;
    keyHeight = rhs.keyHeight;
    positions = rhs.positions;
    memcpy(rgb, rhs.rgb, keyCount * sizeof(QRgb));
    return rhs;
}

KeyMap::KeyMap(const KeyMap& rhs) :
    keyCount(rhs.keyCount), keyWidth(rhs.keyWidth), keyHeight(rhs.keyHeight), positions(rhs.positions), rgb(new QRgb[keyCount]())
{
    memcpy(rgb, rhs.rgb, keyCount * sizeof(QRgb));
}

// Gets or sets values
const KeyPos* KeyMap::key(uint index) const {
    return positions + index;
}

const KeyPos* KeyMap::key(const QString& name) const {
    uint c = count();
    const char* cname = name.toLatin1();
    for(uint i = 0; i < c; i++){
        if(!strcmp(positions[i].name, cname))
            return positions + i;
    }
    return 0;
}

QColor KeyMap::color(uint index) const {
    return QColor::fromRgb(rgb[index]);
}

QColor KeyMap::color(const QString& name) const {
    uint c = count();
    const char* cname = name.toLatin1();
    for(uint i = 0; i < c; i++){
        if(!strcmp(positions[i].name, cname))
            return QColor::fromRgb(rgb[i]);
    }
    return QColor();
}

void KeyMap::color(uint index, const QColor& newColor) {
    rgb[index] = newColor.rgb();
}

void KeyMap::color(const QString& name, const QColor& newColor){
    uint c = count();
    const char* cname = name.toLatin1();
    for(uint i = 0; i < c; i++){
        if(!strcmp(positions[i].name, cname))
            rgb[i] = newColor.rgb();
    }
}

// Sets the color for the entire keyboard
void KeyMap::colorAll(const QColor& newColor){
    QRgb clr = newColor.rgb();
    uint c = count();
    for(uint i = 0; i < c; i++){
        rgb[i] = clr;
    }
}

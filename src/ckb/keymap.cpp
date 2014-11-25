#include "keymap.h"

// Normal size
#define NS 12, 12

// Key positions (K95)
static KeyPos K95PosUS[] = {
    {"mr", 38, 0, NS}, {"m1", 50, 0, NS}, {"m2", 62, 0, NS}, {"m3", 74, 0, NS}, {"light", 222, 0, NS}, {"lock", 234, 0, NS}, {"mute", 273, 0, 13, 8},
    {"g1", 0, 14, NS}, {"g2", 11, 14, NS}, {"g3", 22, 14, NS}, {"esc", 38, 14, NS}, {"f1", 58, 14, NS}, {"f2", 70, 14, NS}, {"f3", 82, 14, NS}, {"f4", 94, 14, NS}, {"f5", 114, 14, NS}, {"f6", 126, 14, NS}, {"f7", 138, 14, NS}, {"f8", 150, 14, NS}, {"f9", 170, 14, NS}, {"f10", 182, 14, NS}, {"f11", 194, 14, NS}, {"f12", 206, 14, NS}, {"prtscn", 222, 14, NS}, {"scroll", 234, 14, NS}, {"pause", 246, 14, NS}, {"stop", 262, 14, 12, 8}, {"prev", 273, 14, 13, 8}, {"play", 285, 14, 13, 8}, {"next", 296, 14, 12, 8},
    {"g4", 0, 25, NS}, {"g5", 11, 25, NS}, {"g6", 22, 25, NS}, {"grave", 38, 27, NS}, {"1", 50, 27, NS}, {"2", 62, 27, NS}, {"3", 74, 27, NS}, {"4", 86, 27, NS}, {"5", 98, 27, NS}, {"6", 110, 27, NS}, {"7", 122, 27, NS}, {"8", 134, 27, NS}, {"9", 146, 27, NS}, {"0", 158, 27, NS}, {"minus", 170, 27, NS}, {"equal", 182, 27, NS}, {"bspace", 200, 27, 24, 12}, {"ins", 222, 27, NS}, {"home", 234, 27, NS}, {"pgup", 246, 27, NS}, {"numlock", 261, 27, NS}, {"numslash", 273, 27, NS}, {"numstar", 285, 27, NS}, {"numminus", 297, 27, NS},
    {"g7", 0, 39, NS}, {"g8", 11, 39, NS}, {"g9", 22, 39, NS}, {"tab", 41, 39, 18, 12}, {"q", 56, 39, NS}, {"w", 68, 39, NS}, {"e", 80, 39, NS}, {"r", 92, 39, NS}, {"t", 104, 39, NS}, {"y", 116, 39, NS}, {"u", 128, 39, NS}, {"i", 140, 39, NS}, {"o", 152, 39, NS}, {"p", 164, 39, NS}, {"lbrace", 176, 39, NS}, {"rbrace", 188, 39, NS}, {"bslash", 203, 39, 18, 12}, {"del", 222, 39, NS}, {"end", 234, 39, NS}, {"pgdn", 246, 39, NS}, {"num7", 261, 39, NS}, {"num8", 273, 39, NS}, {"num9", 285, 39, NS}, {"numplus", 297, 45, 12, 24},
    {"g10", 0, 50, NS}, {"g11", 11, 50, NS}, {"g12", 22, 50, NS}, {"caps", 42, 51, 20, 12}, {"a", 59, 51, NS}, {"s", 71, 51, NS}, {"d", 83, 51, NS}, {"f", 95, 51, NS}, {"g", 107, 51, NS}, {"h", 119, 51, NS}, {"j", 131, 51, NS}, {"k", 143, 51, NS}, {"l", 155, 51, NS}, {"colon", 167, 51, NS}, {"quote", 179, 51, NS}, {"enter", 199, 51, 26, 12}, {"num4", 261, 51, NS}, {"num5", 273, 51, NS}, {"num6", 285, 51, NS},
    {"g13", 0, 64, NS}, {"g14", 11, 64, NS}, {"g15", 22, 64, NS}, {"lshift", 45, 63, 26, 12}, {"z", 65, 63, NS}, {"x", 77, 63, NS}, {"c", 89, 63, NS}, {"v", 101, 63, NS}, {"b", 113, 63, NS}, {"n", 125, 63, NS}, {"m", 137, 63, NS}, {"comma", 149, 63, NS}, {"dot", 161, 63, NS}, {"slash", 173, 63, NS}, {"rshift", 196, 63, 32, 12}, {"up", 234, 63, NS}, {"num1", 261, 63, NS}, {"num2", 273, 63, NS}, {"num3", 285, 63, NS}, {"numenter", 297, 69, 12, 24},
    {"g16", 0, 75, NS}, {"g17", 11, 75, NS}, {"g18", 22, 75, NS}, {"lctrl", 40, 75, 16, 12}, {"lwin", 54, 75, NS}, {"lalt", 67, 75, 14, 12}, {"space", 116, 75, 84, 12}, {"ralt", 165, 75, 14, 12}, {"rwin", 178, 75, NS}, {"rmenu", 190, 75, NS}, {"rctrl", 204, 75, 16, 12}, {"left", 222, 75, NS}, {"down", 234, 75, NS}, {"right", 246, 75, NS}, {"num0", 267, 75, 24, 12}, {"numdot", 285, 75, NS},
};

static KeyPos K95PosUK[] = {
    {"mr", 38, 0, NS}, {"m1", 50, 0, NS}, {"m2", 62, 0, NS}, {"m3", 74, 0, NS}, {"light", 222, 0, NS}, {"lock", 234, 0, NS}, {"mute", 273, 0, 13, 8},
    {"g1", 0, 14, NS}, {"g2", 11, 14, NS}, {"g3", 22, 14, NS}, {"esc", 38, 14, NS}, {"f1", 58, 14, NS}, {"f2", 70, 14, NS}, {"f3", 82, 14, NS}, {"f4", 94, 14, NS}, {"f5", 114, 14, NS}, {"f6", 126, 14, NS}, {"f7", 138, 14, NS}, {"f8", 150, 14, NS}, {"f9", 170, 14, NS}, {"f10", 182, 14, NS}, {"f11", 194, 14, NS}, {"f12", 206, 14, NS}, {"prtscn", 222, 14, NS}, {"scroll", 234, 14, NS}, {"pause", 246, 14, NS}, {"stop", 262, 14, 12, 8}, {"prev", 273, 14, 13, 8}, {"play", 285, 14, 13, 8}, {"next", 296, 14, 12, 8},
    {"g4", 0, 25, NS}, {"g5", 11, 25, NS}, {"g6", 22, 25, NS}, {"grave", 38, 27, NS}, {"1", 50, 27, NS}, {"2", 62, 27, NS}, {"3", 74, 27, NS}, {"4", 86, 27, NS}, {"5", 98, 27, NS}, {"6", 110, 27, NS}, {"7", 122, 27, NS}, {"8", 134, 27, NS}, {"9", 146, 27, NS}, {"0", 158, 27, NS}, {"minus", 170, 27, NS}, {"equal", 182, 27, NS}, {"bspace", 200, 27, 24, 12}, {"ins", 222, 27, NS}, {"home", 234, 27, NS}, {"pgup", 246, 27, NS}, {"numlock", 261, 27, NS}, {"numslash", 273, 27, NS}, {"numstar", 285, 27, NS}, {"numminus", 297, 27, NS},
    {"g7", 0, 39, NS}, {"g8", 11, 39, NS}, {"g9", 22, 39, NS}, {"tab", 41, 39, 18, 12}, {"q", 56, 39, NS}, {"w", 68, 39, NS}, {"e", 80, 39, NS}, {"r", 92, 39, NS}, {"t", 104, 39, NS}, {"y", 116, 39, NS}, {"u", 128, 39, NS}, {"i", 140, 39, NS}, {"o", 152, 39, NS}, {"p", 164, 39, NS}, {"lbrace", 176, 39, NS}, {"rbrace", 188, 39, NS}, {"enter", 203, 39, 18, 24}, {"del", 222, 39, NS}, {"end", 234, 39, NS}, {"pgdn", 246, 39, NS}, {"num7", 261, 39, NS}, {"num8", 273, 39, NS}, {"num9", 285, 39, NS}, {"numplus", 297, 45, 12, 24},
    {"g10", 0, 50, NS}, {"g11", 11, 50, NS}, {"g12", 22, 50, NS}, {"caps", 42, 51, 20, 12}, {"a", 59, 51, NS}, {"s", 71, 51, NS}, {"d", 83, 51, NS}, {"f", 95, 51, NS}, {"g", 107, 51, NS}, {"h", 119, 51, NS}, {"j", 131, 51, NS}, {"k", 143, 51, NS}, {"l", 155, 51, NS}, {"colon", 167, 51, NS}, {"quote", 179, 51, NS}, {"hash", 191, 51, NS}, {"num4", 261, 51, NS}, {"num5", 273, 51, NS}, {"num6", 285, 51, NS},
    {"g13", 0, 64, NS}, {"g14", 11, 64, NS}, {"g15", 22, 64, NS}, {"lshift", 39, 63, 14, 12}, {"bslash", 53, 63, NS}, {"z", 65, 63, NS}, {"x", 77, 63, NS}, {"c", 89, 63, NS}, {"v", 101, 63, NS}, {"b", 113, 63, NS}, {"n", 125, 63, NS}, {"m", 137, 63, NS}, {"comma", 149, 63, NS}, {"dot", 161, 63, NS}, {"slash", 173, 63, NS}, {"rshift", 196, 63, 32, 12}, {"up", 234, 63, NS}, {"num1", 261, 63, NS}, {"num2", 273, 63, NS}, {"num3", 285, 63, NS}, {"numenter", 297, 69, 12, 24},
    {"g16", 0, 75, NS}, {"g17", 11, 75, NS}, {"g18", 22, 75, NS}, {"lctrl", 40, 75, 16, 12}, {"lwin", 54, 75, NS}, {"lalt", 67, 75, 14, 12}, {"space", 116, 75, 84, 12}, {"ralt", 165, 75, 14, 12}, {"rwin", 178, 75, NS}, {"rmenu", 190, 75, NS}, {"rctrl", 204, 75, 16, 12}, {"left", 222, 75, NS}, {"down", 234, 75, NS}, {"right", 246, 75, NS}, {"num0", 267, 75, 24, 12}, {"numdot", 285, 75, NS},
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
                keyOut[j].name = keyIn[i].name;
                keyOut[j].x = keyIn[i].x - K70_X_START;
                keyOut[j].y = keyIn[i].y;
                keyOut[j].width = keyIn[i].width;
                keyOut[j].height = keyIn[i].height;
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
                keyOut[j].name = keyIn[i].name;
                keyOut[j].x = keyIn[i].x - K70_X_START;
                keyOut[j].y = keyIn[i].y;
                keyOut[j].width = keyIn[i].width;
                keyOut[j].height = keyIn[i].height;
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

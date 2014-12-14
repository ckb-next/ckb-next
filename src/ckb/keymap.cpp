#include "keymap.h"

// Normal size
#define NS 12, 12

#ifdef __APPLE__
#define WIN_NAME "Cmd"
#else
#define WIN_NAME "Super"
#endif

#define KEYCOUNT_K95_104 133
#define KEYCOUNT_K95_105 134

// Key positions (K95)
extern const KeyPos K95PosFR[KEYCOUNT_K95_105];
extern const KeyPos K95PosDE[KEYCOUNT_K95_105];
extern const KeyPos K95PosSE[KEYCOUNT_K95_105];
extern const KeyPos K95PosGB[KEYCOUNT_K95_105];
extern const KeyPos K95PosUS[KEYCOUNT_K95_104];

struct KeyLayout {
    const KeyPos* positions;
    uint count;
};

static const KeyLayout K95Pos[] = {
    { K95PosFR, sizeof(K95PosSE) / sizeof(KeyPos) },
    { K95PosDE, sizeof(K95PosSE) / sizeof(KeyPos) },
    { K95PosSE, sizeof(K95PosSE) / sizeof(KeyPos) },
    { K95PosGB, sizeof(K95PosGB) / sizeof(KeyPos) },
    { K95PosUS, sizeof(K95PosUS) / sizeof(KeyPos) }
};

static const uint KEYLAYOUT_COUNT = sizeof(K95Pos) / sizeof(KeyLayout);

#define K95_WIDTH       298
#define K95_HEIGHT      76

#define K70_COUNT_DIFF  22 // 18 G keys, MR, M1, M2, M3

static KeyLayout K70Pos[KEYLAYOUT_COUNT] = {
    { 0, K95Pos[0].count - K70_COUNT_DIFF },
    { 0, K95Pos[1].count - K70_COUNT_DIFF },
    { 0, K95Pos[2].count - K70_COUNT_DIFF },
};

#define K70_X_START     38
#define K70_WIDTH       (K95_WIDTH - K70_X_START)
#define K70_HEIGHT      K95_HEIGHT

KeyMap::Layout KeyMap::getLayout(const QString& name){
    QString lower = name.toLower();
    if(lower == "fr")
        return FR;
    if(lower == "de")
        return DE;
    if(lower == "se")
        return SE;
    if(lower == "gb")
        return GB;
    if(lower == "us")
        return US;
    return NO_LAYOUT;
}

QString KeyMap::getLayout(KeyMap::Layout layout){
    switch(layout){
    case FR:
        return "fr";
    case DE:
        return "de";
    case SE:
        return "se";
    case GB:
        return "gb";
    case US:
        return "us";
    default:
        return "";
    }
}

KeyMap::Model KeyMap::getModel(const QString& name){
    QString lower = name.toLower();
    if(lower == "k70")
        return K70;
    if(lower == "k95")
        return K95;
    return NO_MODEL;
}

QString KeyMap::getModel(KeyMap::Model model){
    switch(model){
    case K70:
        return "k70";
    case K95:
        return "k95";
    default:
        return "";
    }
}

KeyMap KeyMap::standard(KeyMap::Model model, KeyMap::Layout layout){
    if(model == NO_MODEL || layout == NO_LAYOUT)
        return KeyMap();
    if(model == K70){
        // Build the K70 tables if not done yet
        if(!K70Pos[0].positions){
            for(uint keyboard = 0; keyboard < KEYLAYOUT_COUNT; keyboard++){
                uint j = 0;
                uint count = K95Pos[keyboard].count;
                const KeyPos* keyIn = K95Pos[keyboard].positions;
                KeyPos* keyOut = new KeyPos[K70Pos[keyboard].count];
                K70Pos[keyboard].positions = keyOut;
                for(uint i = 0; i < count; i++){
                    // Remove anything before X-start (G keys) as well as the M keys
                    if(keyIn[i].x < K70_X_START || !strcmp(keyIn[i].name, "mr")
                            || !strcmp(keyIn[i].name, "m1") || !strcmp(keyIn[i].name, "m2") || !strcmp(keyIn[i].name, "m3"))
                        continue;
                    keyOut[j] = keyIn[i];
                    keyOut[j].x -= K70_X_START;
                    j++;
                }
            }
        }
        // Return K70 positions
        return KeyMap(model, layout, K70Pos[layout].count, K70_WIDTH, K70Pos[layout].positions);
    }
    // Return K95 positions
    return KeyMap(model, layout, K95Pos[layout].count, K95_WIDTH, K95Pos[layout].positions);
}

KeyMap KeyMap::fromName(const QString &name){
    QStringList list = name.trimmed().split(" ");
    if(list.length() != 2)
        return KeyMap();
    return standard(getModel(list[0]), getLayout(list[1]));
}

QString KeyMap::name() const {
    return (getModel(model()) + " " + getLayout(layout())).toUpper();
}

KeyMap::KeyMap(Model _keyModel, Layout _keyLayout, uint _keyCount, uint _width, const KeyPos* _positions) :
    positions(_positions), keyCount(_keyCount), keyWidth(_width), keyHeight(K95_HEIGHT),
    keyModel(_keyModel), keyLayout(_keyLayout)
{}

KeyMap::KeyMap() :
     positions(0), keyCount(0), keyWidth(0), keyHeight(0),
     keyModel(NO_MODEL), keyLayout(NO_LAYOUT)
{}

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

int KeyMap::index(const QString& name) const {
    uint c = count();
    const char* cname = name.toLatin1();
    for(uint i = 0; i < c; i++){
        if(!strcmp(positions[i].name, cname))
            return i;
    }
    return -1;
}

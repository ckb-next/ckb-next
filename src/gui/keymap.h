#ifndef KEYMAP_H
#define KEYMAP_H

#include <QColor>
#include <QHash>

// Key information
struct Key {
    // Name stored in settings (this is here due to a bad design decision - it will be removed later)
    const char*         _storageName;
    inline const char*  storageName() const { return _storageName ? _storageName : name; }
    // Key name
    const char* _friendlyName;
    const char* name;
    // LED position, measured roughly in 16th inches. Most keys are 3/4" apart (12x12).
    short x, y;
    short width, height;
    // Whether or not the key has an LED, scancode, or both
    bool hasLed;
    bool hasScan;

    // Friendly name with optional OS-based labels
    inline QString friendlyName(bool os = true) const {
        if(os){
#ifdef Q_OS_MACOS
            if(!strcmp(name, "lwin")) return "Left Cmd";
            if(!strcmp(name, "rwin")) return "Right Cmd";
            if(!strcmp(name, "lalt")) return "Left Option";
            if(!strcmp(name, "ralt")) return "Right Option";
            if(!strcmp(name, "prtscn")) return "F13";
            if(!strcmp(name, "scroll")) return "F14";
            if(!strcmp(name, "pause")) return "F15";
            if(!strcmp(name, "ins")) return "Help";
            if(!strcmp(name, "numlock")) return "Clear";
#elif defined(Q_OS_LINUX)
            if(!strcmp(name, "lwin")) return "Left Super";
            if(!strcmp(name, "rwin")) return "Right Super";
#endif
        }
        return _friendlyName ? _friendlyName : QString(name).toUpper();
    }

    // Convert to bool -> key exists?
    inline operator bool () const { return name != nullptr; }
    inline bool operator !() const { return !(bool)*this; }
};

// Key layout/device info class
class KeyMap {
public:
    // Do not reorder these.
    // Doing so will break the profile import/export
    enum Model {
        NO_MODEL = -1,
        K55,
        K63,
        K65,
        K68,
        K70,
        K95,
        K95P,
        STRAFE,
        M65,
        SABRE,
        SCIMITAR,
        HARPOON,
        GLAIVE,
        KATAR,
        POLARIS,
        ST100,
        K70MK2,
        STRAFE_MK2,
        K66,
        M65E,
        M95,
        IRONCLAW,
        NIGHTSWORD,
        DARKCORE,
        IRONCLAW_W,
        KATARPROXT,
        K95L,
        GLAIVEPRO,
        M55,
        K60,
        K57_WL,
        K63_WL,
        K55PRO,
        DARKCORERGBPRO,
        K60_TKL,
        BRAGI_DONGLE,
        K100,
        K65_MINI,
        K70_TKL,
        K70_PRO,
        _MODEL_MAX
    };
    // Key layouts (ordered alphabetically by name)
    enum Layout {
        NO_LAYOUT = -1,
        DK,                 // Danish
        EU,                 // English (EU)
        EU_DVORAK,
        GB,                 // English (UK)
        GB_DVORAK,
        US,                 // English (US)
        US_DVORAK,
        FR,                 // French
        DE,                 // German
        IT,                 // Italian
        JP,                 // Japanese
        NO,                 // Norwegian
        PL,                 // Polish (identical to US)
        PT_BR,              // Portuguese (Brazil)
        MX,                 // Spanish (Mexico/Latin America)
        ES,                 // Spanish (Spain)
        SE,                 // Swedish
        _LAYOUT_MAX
    };
    // Human-readable names of each layout
    static QList<QPair<int, QString> > layoutNames(const QString& layout);
    // ISO (105-key) or ANSI (104-key)?
    inline static bool  isISO(Layout layout)    { return layout != US && layout != US_DVORAK && layout != PL; }
    inline bool         isISO() const           { return isISO(keyLayout); }

    // JP (106-key)?
    inline static bool  isJP(Layout layout)    { return layout == JP; }
    inline bool         isJP() const           { return isJP(keyLayout); }

    // PT_BR?
    inline static bool  isPTBR(Layout layout)    { return layout == PT_BR; }
    inline bool         isPTBR() const           { return isPTBR(keyLayout); }

    // Auto-detects layout from system locale
    static Layout       locale(QList<QPair<int, QString>>*layouts);

    // Type of device
    inline static bool  isKeyboard(Model model)     { return !isMouse(model) && !isMousepad(model) && !isHeadsetStand(model) && model != NO_MODEL; }
    inline bool         isKeyboard() const          { return isKeyboard(keyModel); }
    inline static bool  isMouse(Model model)        { return model == M55 || model == M65 || model == SABRE || model == SCIMITAR || model == HARPOON || model == GLAIVE || model == KATAR || model == KATARPROXT || model == M65E || model == M95 || model == IRONCLAW || model == NIGHTSWORD || model == DARKCORE || model == DARKCORERGBPRO || model == IRONCLAW_W || model == GLAIVEPRO; }
    inline bool         isMouse() const             { return isMouse(keyModel); }
    inline static bool  isMousepad(Model model)     { return model == POLARIS; }
    inline bool         isMousepad() const          { return isMousepad(keyModel); }
    inline static bool  isHeadsetStand(Model model) { return model == ST100; }
    inline bool         isHeadsetStand() const      { return isHeadsetStand(keyModel); }

    inline static bool hasLights(Model model)       { return !(model == M95 || model == K66 || model == BRAGI_DONGLE); }
    inline bool        hasLights() const            { return hasLights(keyModel); }

    // Creates a blank key map
    KeyMap();
    // Creates a standard key map
    KeyMap(Model _keyModel, Layout _keyLayout);
    static KeyMap   fromName(const QString& name);
    // Gets a layout by name or name by layout
    static Layout   getLayout(const QString& name);
    static QString  getLayout(Layout layout);
    inline QString  strLayout() const { return getLayout(keyLayout); }
    // Gets a model by name or name by model
    static Model    getModel(const QString& name);
    static QString  getModel(Model model);
    inline QString  strModel() const { return getModel(keyModel); }

    // Keyboard model and layout
    inline Model    model() const   { return keyModel; }
    inline Layout   layout() const  { return keyLayout; }
    inline QString  name() const    { return (strModel() + " " + strLayout()).toUpper(); }

    // Number of keys in the keymap
    inline int count() const   { return _keys.count(); }
    // Keyboard total width
    inline short width() const   { return keyWidth; }
    // Keyboard total height
    inline short height() const  { return keyHeight; }

    // Keys by name
    inline Key  key(const QString& name) const          { return _keys.value(name, emptyKey); }
    inline Key& operator[](const QString& name)         { return _keys[name]; }
    inline bool contains(const QString& name) const     { return _keys.contains(name); }
    inline QHash<QString, Key>::const_iterator begin() const { return _keys.cbegin(); }
    inline QHash<QString, Key>::const_iterator end() const { return _keys.cend(); }
    QStringList     keys() const                        { return _keys.keys(); }
    QList<Key>      positions() const                   { return _keys.values(); }
    // Key name to/from storage name. Returns the given name if not found.
    inline QString toStorage(const QString& name) {
        const char* storage = key(name).storageName();
        if(!storage)
            return name;
        return storage;
    }
    inline QString fromStorage(const QString& storage) {
        for(const Key& key : *this){
            if(key.storageName() == storage)
                return key.name;
        }
        return storage;
    }

    // Keys by position (top to bottom, left to right)
    QStringList byPosition() const;

    // Friendly key name on any device
    static QString friendlyName(const QString& key, Layout layout = US);

    static const QStringList layoutList;

private:
    static const Key emptyKey;
    static int modelWidth(Model model);
    static int modelHeight(Model model);

    QHash<QString, Key> _keys;
    Model keyModel :8;
    Layout keyLayout :8;

    static QPair<int, QString> addToList(int i, const QStringList& list);
protected:
    short keyWidth, keyHeight;

};

struct KeyMapDebug : KeyMap {
    using KeyMap::KeyMap;
    using KeyMap::keyWidth;
    using KeyMap::keyHeight;
};

#endif // KEYMAP_H

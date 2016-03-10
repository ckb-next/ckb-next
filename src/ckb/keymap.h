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
#ifdef Q_OS_MACX
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
    inline operator bool () const { return name != 0; }
    inline bool operator !() const { return !(bool)*this; }
};

// Key layout/device info class
class KeyMap {
public:
    enum Model {
        NO_MODEL = -1,
        // Keyboard models
        K65,
        K70,
        K95,
        STRAFE,
        // Mouse models
        M65,
        SABRE,
        SCIMITAR,
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
        NO,                 // Norwegian
        PL,                 // Polish (identical to US)
        MX,                 // Spanish (Mexico/Latin America)
        ES,                 // Spanish (Spain)
        SE,                 // Swedish
        _LAYOUT_MAX
    };
    // Human-readable names of each layout
    static QStringList layoutNames();
    // ISO (105-key) or ANSI (104-key)?
    inline static bool  isISO(Layout layout)    { return layout != US && layout != US_DVORAK && layout != PL; }
    inline bool         isISO() const           { return isISO(keyLayout); }
    // Auto-detects layout from system locale
    static Layout       locale();

    // Keyboard or mouse?
    inline static bool  isKeyboard(Model model) { return !isMouse(model) && model != NO_MODEL; }
    inline bool         isKeyboard() const      { return isKeyboard(keyModel); }
    inline static bool  isMouse(Model model)    { return model == M65 || model == SABRE || model == SCIMITAR; }
    inline bool         isMouse() const         { return isMouse(keyModel); }

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
    inline uint count() const   { return _keys.count(); }
    // Keyboard total width
    inline uint width() const   { return keyWidth; }
    // Keyboard total height
    inline uint height() const  { return keyHeight; }

    // Keys by name
    inline Key  key(const QString& name) const          { Key empty = {0,0,0,0,0,0,0,0,0}; return _keys.value(name, empty); }
    inline Key  operator[](const QString& name) const   { return key(name); }
    inline bool contains(const QString& name) const     { return _keys.contains(name); }
    // List all key names/values
    inline const QHash<QString, Key>&   map() const                         { return _keys; }
    inline operator                     const QHash<QString, Key>& () const { return _keys; }
    QStringList     keys() const                        { return _keys.keys(); }
    QList<Key>      positions() const                   { return _keys.values(); }
    // Key name to/from storage name. Returns the given name if not found.
    inline QString  toStorage(const QString& name)      { const char* storage = key(name).storageName(); if(!storage) return name; return storage; }
    inline QString  fromStorage(const QString& storage) { QHashIterator<QString, Key> i(*this); while(i.hasNext()) { i.next(); const char* s = i.value().storageName(); if(s == storage) return i.value().name; } return storage; }

    // Keys by position (top to bottom, left to right)
    QStringList byPosition() const;

    // Friendly key name on any device
    static QString friendlyName(const QString& key, Layout layout = US);

private:
    static int modelWidth(Model model);
    static int modelHeight(Model model);

    QHash<QString, Key> _keys;
    short keyWidth, keyHeight;
    Model keyModel :8;
    Layout keyLayout :8;
};

#endif // KEYMAP_H

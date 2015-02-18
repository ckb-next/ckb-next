#ifndef KEYMAP_H
#define KEYMAP_H

#include <QColor>

// Key information
struct KeyPos {
    // Key name
    const char* _friendlyName;
    const char* name;
    // LED position, measured roughly in 16th inches. Most keys are 3/4" apart.
    int x, y;
    int width, height;

    // Swap Cmd/Ctrl on OSX
    static bool osxCmdSwap;

    inline QString friendlyName(bool os = true) const {
        if(os){
#ifdef Q_OS_MACX
            if(osxCmdSwap){
                if(!strcmp(name, "lctrl")) return "Left Cmd";
                if(!strcmp(name, "rctrl")) return "Right Cmd";
                if(!strcmp(name, "lwin")) return "Left Ctrl";
                if(!strcmp(name, "rwin")) return "Right Ctrl";
            } else {
                if(!strcmp(name, "lwin")) return "Left Cmd";
                if(!strcmp(name, "rwin")) return "Right Cmd";
            }
            if(!strcmp(name, "lalt")) return "Left Option";
            if(!strcmp(name, "ralt")) return "Right Option";
            if(!strcmp(name, "prtscn")) return "F13";
            if(!strcmp(name, "scroll")) return "F14";
            if(!strcmp(name, "pause")) return "F15";
            if(!strcmp(name, "ins")) return "Help";
            if(!strcmp(name, "numlock")) return "Clear";
#endif
#ifdef Q_OS_LINUX
            if(!strcmp(name, "lwin")) return "Left Super";
            if(!strcmp(name, "rwin")) return "Right Super";
#endif
        }
        return _friendlyName ? _friendlyName : QString(name).toUpper();
    }
};

// Key lighting/layout class
class KeyMap {
public:
    // Key layouts
    enum Layout {
        NO_LAYOUT = -1,
        FR,
        DE,
        SE,
        GB,
        GB_DVORAK,
        US,
        US_DVORAK
    };
    // Keyboard models
    enum Model {
        NO_MODEL = -1,
        K65,
        K70,
        K95,
    };

    // Copies a standard key map
    static KeyMap standard(Model model, Layout layout);
    static KeyMap fromName(const QString& name);
    // Gets a layout by name or name by layout
    static Layout getLayout(const QString& name);
    static QString getLayout(Layout layout);
    // Returns a layout name to send to the daemon
    static inline QString getLayoutHw(Layout layout) { return getLayout(layout).split("_")[0]; }
    // Gets a model by name or name by model
    static Model getModel(const QString& name);
    static QString getModel(Model model);

    // Keyboard model and layout
    inline Model model() const { return keyModel; }
    inline Layout layout() const { return keyLayout; }
    QString name() const;

    // Number of keys in the keymap
    inline uint count() const { return keyCount; }
    // Keyboard total width
    inline uint width() const { return keyWidth; }
    // Keyboard total height
    inline uint height() const { return keyHeight; }

    // Gets key info. Returns null if key not found.
    const KeyPos* key(uint index) const;
    const KeyPos* key(const QString& name) const;
    int index(const QString& name) const;

    // List of all key names
    QStringList allKeys() const;

    KeyMap();

private:
    const KeyPos* positions;
    uint keyCount :16, keyWidth :16, keyHeight :16;
    Model keyModel :4;
    Layout keyLayout :4;
    KeyMap(Model _keyModel, Layout _keyLayout, uint _keyCount, uint _width, const KeyPos* _positions);
};

#endif // KEYMAP_H

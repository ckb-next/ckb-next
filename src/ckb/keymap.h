#ifndef KEYMAP_H
#define KEYMAP_H

#include <QColor>

// Key information
struct KeyPos {
    // Key name
    const char* friendlyName;
    const char* name;
    // LED position, measured roughly in 16th inches. Most keys are 3/4" apart.
    int x, y;
    int width, height;
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
        US
    };
    // Keyboard models
    enum Model {
        NO_MODEL = -1,
        K70,
        K95,
    };

    // Copies a standard key map
    static KeyMap standard(Model model, Layout layout);
    static KeyMap fromName(const QString& name);
    // Gets a layout by name or name by layout
    static Layout getLayout(const QString& name);
    static QString getLayout(Layout layout);
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

    KeyMap();

private:
    const KeyPos* positions;
    uint keyCount, keyWidth, keyHeight;
    Model keyModel;
    Layout keyLayout;
    KeyMap(Model _keyModel, Layout _keyLayout, uint _keyCount, uint _width, const KeyPos* _positions);
};

#endif // KEYMAP_H

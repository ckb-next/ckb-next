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
    uint keyCount, keyWidth, keyHeight;
    const KeyPos* positions;
    QRgb* rgb;
    KeyMap(uint _keyCount, uint _width, const KeyPos* _positions);
public:
    // Key layouts
    enum Layout {
        US,
        UK
    };
    // Keyboard models
    enum Model {
        K70,
        K95,
    };

    // Copies a standard key layout
    static KeyMap standard(Model model, Layout layout);

    // Number of keys in the keymap
    inline uint count() const { return keyCount; }
    // Keyboard total width
    inline uint width() const { return keyWidth; }
    // Keyboard total height
    inline uint height() const { return keyHeight; }

    // Gets or sets values
    const KeyPos* key(uint index) const;
    const KeyPos* key(const QString& name) const;
    QColor color(uint index) const;
    QColor color(const QString& name) const;
    void color(uint index, const QColor& newColor);
    void color(const QString& name, const QColor& newColor);

    // Sets the color for the entire keyboard
    void colorAll(const QColor& newColor);

    KeyMap();
    ~KeyMap();
    const KeyMap& operator = (const KeyMap& rhs);
    KeyMap(const KeyMap& rhs);
};

#endif // KEYMAP_H

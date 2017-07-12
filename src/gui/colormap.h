#ifndef COLORMAP_H
#define COLORMAP_H

#include <QHash>
#include <QRgb>
#include "keymap.h"

// Qt-based color map for use by classes outside KbLight
typedef QHash<QString, QRgb>                QColorMap;
typedef QHashIterator<QString, QRgb>        QColorMapIterator;
typedef QMutableHashIterator<QString, QRgb> QMutableColorMapIterator;

// ColorMap provides a flat, fast-access array for storing color values on a keyboard. Keys are sorted by name.

class ColorMap
{
public:
    ColorMap();
    ~ColorMap();
    ColorMap(const ColorMap& rhs);
    const ColorMap& operator=(const ColorMap& rhs);

    // Initialize the color map with the given keys. Color values are initialized to transparent black.
    // This may be called more than once; the existing color set will be erased.
    void init(const KeyMap& map);
    // Erase current color values
    void clear();

    // Flat key -> color map
    int                 count() const       { return _count; }
    const char* const*  keyNames() const    { return _keyNames; }
    QRgb*               colors()            { return _colors; }
    const QRgb*         colors() const      { return _colors; }

    // Finds a color by key name. Returns null if the key isn't in the map.
    QRgb*       colorForName(const char* name);
    const QRgb* colorForName(const char* name) const;

private:
    void alloc(int count);
    void deAlloc();

    const char** _keyNames;
    QRgb* _colors;
    int _count, _mapCount;
};

#endif // COLORMAP_H

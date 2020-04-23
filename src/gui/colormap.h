#ifndef COLORMAP_H
#define COLORMAP_H

#include <QHash>
#include <QRgb>
#include "keymap.h"

#include <map>

// Qt-based color map for use by classes outside KbLight
typedef QHash<QString, QRgb>                QColorMap;
typedef QHashIterator<QString, QRgb>        QColorMapIterator;
typedef QMutableHashIterator<QString, QRgb> QMutableColorMapIterator;

// ColorMap provides a flat, fast-access array for storing color values on a keyboard. Keys are sorted by name.

class ColorMap
{
public:
    ColorMap() = default;
    ColorMap(const ColorMap& rhs) = default;
    ColorMap& operator=(const ColorMap& rhs) = default;
    friend inline bool operator==(const ColorMap& r, const ColorMap& l)
    {
      return r._colors == l._colors;
    }

    // Initialize the color map with the given keys. Color values are initialized to transparent black.
    // This may be called more than once; the existing color set will be erased.
    void init(const KeyMap& map);
    // Erase current color values
    void clear();

    int                 count() const       { return _colors.size(); }

    // Finds a color by key name. Returns null if the key isn't in the map.
    QRgb*       colorForName(const char* name);
    const QRgb* colorForName(const char* name) const;

  std::map<std::string, QRgb> _colors;
};

#endif // COLORMAP_H

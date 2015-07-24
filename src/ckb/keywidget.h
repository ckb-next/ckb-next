#ifndef RGBWIDGET_H
#define RGBWIDGET_H

#include <QBitArray>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QWidget>
#include "keymap.h"

class KeyWidget : public QWidget
{
    Q_OBJECT
public:
    // New key widget. rgbMode = true to display colors, false to display key names
    explicit KeyWidget(QWidget *parent = 0, bool rgbMode = true);
    inline bool     rgbMode() { return _rgbMode; }
    inline void     rgbMode(bool newRgbMode) { _rgbMode = newRgbMode; update(); }

    // Key map
    const KeyMap&   map() const                         { return keyMap; }
    void            map(const KeyMap& newMap);
    // Key -> color map (must contain exactly the keys in the key map)
    typedef QHash<QString, QRgb> ColorMap;
    const ColorMap& colorMap() const                    { return _colorMap; }
    void            colorMap(const ColorMap& newColorMap);
    // Key -> binding map
    typedef QHash<QString, QString> BindMap;
    const BindMap&  bindMap() const                     { return _bindMap; }
    void            bindMap(const BindMap& newBindMap);

    // Set current selection (highlighted in blue)
    void setSelection(const QStringList& keys);
    void selectAll();
    void clearSelection();

    // Set animated keys (highlighted in green)
    void setAnimation(const QStringList& keys);
    void setAnimationToSelection();
    void clearAnimation();

public slots:
    // Sets display colors. Pass an empty map to clear.
    // These will be displayed instead of the regular color map, if supplied.
    void displayColorMap(ColorMap newDisplayMap);

signals:
    // Emitted when the selection is changed.
    void selectionChanged(QStringList selected);

private:
    KeyMap keyMap;
    ColorMap _colorMap, _displayColorMap;
    BindMap _bindMap;

    QBitArray selection;
    QBitArray newSelection;
    QBitArray animation;
    int mouseDownX, mouseDownY;
    int mouseCurrentX, mouseCurrentY;
    enum {
        NONE,
        SET,
        ADD,
        SUBTRACT,
        TOGGLE,
    } mouseDownMode;
    bool _rgbMode;

    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

    // Get drawing scale/offset. drawX = (keymapX + offsetX) * scale
    void drawInfo(float& scale, float& offsetX, float& offsetY);
};

#endif // RGBWIDGET_H

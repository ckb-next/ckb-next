#ifndef RGBWIDGET_H
#define RGBWIDGET_H

#include <QBitArray>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QWidget>
#include "keymap.h"

class RgbWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RgbWidget(QWidget *parent = 0);

    // Key map
    const KeyMap& map() const { return keyMap; }
    void map(const KeyMap& newMap);
    // Key -> color map
    const QHash<QString, QRgb>& colorMap() const { return _colorMap; }
    void colorMap(const QHash<QString, QRgb>& newColorMap);

    // Set current selection
    void setSelection(const QStringList& keys);
    void clearSelection();

    // Set animated keys
    void setAnimation(const QStringList& keys);
    void setAnimationToSelection();
    void clearAnimation();

signals:
    // Emitted when the selection is changed.
    // selectedColor is the color of the selected keys or invalid color if none selected or not all keys are the same
    void selectionChanged(QColor selectedColor, QStringList selected);

private:
    KeyMap keyMap;
    QHash<QString, QRgb> _colorMap;

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

    // Determine the color of the selected keys (invalid color if there are any conflicts)
    QColor selectedColor();

    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
};

#endif // RGBWIDGET_H

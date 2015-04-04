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
    inline bool rgbMode() { return _rgbMode; }
    inline void rgbMode(bool newRgbMode) { _rgbMode = newRgbMode; update(); }

    // Key map
    const KeyMap& map() const { return keyMap; }
    void map(const KeyMap& newMap);
    // Key -> color map (must contain exactly the keys in the key map)
    const QHash<QString, QRgb>& colorMap() const { return _colorMap; }
    void colorMap(const QHash<QString, QRgb>& newColorMap);
    // Key -> binding map
    const QHash<QString, QString>& bindMap() const { return _bindMap; }
    void bindMap(const QHash<QString, QString>& newBindMap);

    // Set current selection (highlighted in blue)
    void setSelection(const QStringList& keys);
    void selectAll();
    void clearSelection();

    // Set animated keys (highlighted in green)
    void setAnimation(const QStringList& keys);
    void setAnimationToSelection();
    void clearAnimation();

signals:
    // Emitted when the selection is changed.
    void selectionChanged(QStringList selected);

private:
    KeyMap keyMap;
    QHash<QString, QRgb> _colorMap;
    QHash<QString, QString> _bindMap;

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
};

#endif // RGBWIDGET_H

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

    const KeyMap& map() const { return keyMap; }
    void map(const KeyMap& newMap);

    void set(const QString& name, const QColor& color);
    void set(const QColor& color);
    void setSelected(const QColor& color);

    void setSelection(const QStringList& keys);
    void clearSelection();

    void setAnimation(const QStringList& keys);
    void setAnimationToSelection();
    void clearAnimation();

signals:
    void selectionChanged(QColor selectedColor, QStringList selected);

private:
    KeyMap keyMap;

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

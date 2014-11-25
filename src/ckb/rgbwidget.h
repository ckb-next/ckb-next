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

signals:
    void selectionChanged(QColor selectedColor, int selectedCount);

private:
    KeyMap keyMap;

    void paintEvent(QPaintEvent*);

    QBitArray selection;
    QBitArray newSelection;
    int mouseDownX, mouseDownY;
    int mouseCurrentX, mouseCurrentY;
    enum {
        NONE,
        SET,
        ADD,
        SUBTRACT
    } mouseDownMode;

    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
};

#endif // RGBWIDGET_H

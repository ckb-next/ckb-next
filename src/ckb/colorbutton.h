#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include <QPushButton>

class ColorButton : public QPushButton
{
    Q_OBJECT
public:
    explicit ColorButton(QWidget *parent = 0);

    inline const QColor& color() { return _color; }
    void color(const QColor& newColor);

signals:
    void colorChanged(QColor newColor);

private slots:
    void pickColor();

private:
    QColor _color;

    void updateImage();
};

#endif // COLORBUTTON_H

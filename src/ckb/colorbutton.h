#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include <QPushButton>

class ColorButton : public QPushButton
{
    Q_OBJECT
public:
    explicit ColorButton(QWidget* parent = 0, bool allowAlpha = false);

    inline const QColor& color() { return _color; }
    void color(const QColor& newColor);

signals:
    void colorChanged(QColor newColor);

private slots:
    void pickColor();

private:
    QColor _color;
    bool _alpha;

    void updateImage();
};

#endif // COLORBUTTON_H

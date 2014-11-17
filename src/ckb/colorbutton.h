#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include <QPushButton>

class ColorButton : public QPushButton
{
    Q_OBJECT
public:
    explicit ColorButton(QWidget *parent = 0);

    QColor color;
    void updateImage();

signals:
    void colorChanged(QColor newColor);

private slots:
    void pickColor();
};

#endif // COLORBUTTON_H

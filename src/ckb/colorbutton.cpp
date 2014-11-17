#include "colorbutton.h"
#include <QColorDialog>
#include <QPainter>

ColorButton::ColorButton(QWidget *parent) :
    QPushButton(parent), color(QColor(255, 255, 255))
{
    connect(this, SIGNAL(clicked()), this, SLOT(pickColor()));
    updateImage();
}

void ColorButton::updateImage(){
    const int w = 12, h = 12;
    QImage image(w, h, QImage::Format_RGB888);
    QPainter painter(&image);
    painter.setPen(Qt::NoPen);
    painter.fillRect(0, 0, w, h, QColor(0, 0, 0));
    painter.fillRect(1, 1, w - 2, h - 2, color);
    setIcon(QIcon(QPixmap::fromImage(image)));
    setText(" " + color.name().toUpper());
}

void ColorButton::pickColor(){
    QColor newColor = QColorDialog::getColor(color, this);
    if(newColor.isValid()){
        color = newColor;
        updateImage();
        emit colorChanged(color);
    }
}

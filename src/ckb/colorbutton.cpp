#include "colorbutton.h"
#include <QColorDialog>
#include <QPainter>

ColorButton::ColorButton(QWidget *parent) :
    QPushButton(parent)
{
    connect(this, SIGNAL(clicked()), this, SLOT(pickColor()));
    updateImage();
}

void ColorButton::color(const QColor& newColor){
    _color = newColor;
    updateImage();
}

void ColorButton::updateImage(){
    if(!_color.isValid()){
        setIcon(QIcon());
        setText("Change color...");
        return;
    }
    const int w = 12, h = 12;
    QImage image(w, h, QImage::Format_RGB888);
    QPainter painter(&image);
    painter.setPen(Qt::NoPen);
    painter.fillRect(0, 0, w, h, QColor(0, 0, 0));
    painter.fillRect(1, 1, w - 2, h - 2, _color);
    setIcon(QIcon(QPixmap::fromImage(image)));
    setText(" " + _color.name().toUpper());
}

void ColorButton::pickColor(){
    QColor newColor = QColorDialog::getColor(_color, this);
    if(newColor.isValid()){
        _color = newColor;
        updateImage();
        emit colorChanged(_color);
    }
}

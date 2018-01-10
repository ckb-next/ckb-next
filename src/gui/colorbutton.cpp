#include "colorbutton.h"
#include <QColorDialog>
#include <QPainter>

ColorButton::ColorButton(QWidget* parent, bool allowAlpha) :
    QPushButton(parent), _alpha(allowAlpha), _setLabel(true), _bigIcons(false)
{
    setAutoDefault(false);
    setDefault(false);
    updateImage();
    // Pick color on click (use queued connection so that any on_*_clicked() events can be processed first)
    connect(this, SIGNAL(clicked()), this, SLOT(pickColor()), Qt::QueuedConnection);
}

void ColorButton::color(const QColor& newColor){
    _color = newColor;
    if(!_alpha)
        _color.setAlpha(255);
    updateImage();
}

void ColorButton::allowAlpha(bool newAllowAlpha){
    _alpha = newAllowAlpha;
    updateImage();
}

void ColorButton::bigIcons(bool newBigIcons){
    _bigIcons = newBigIcons;
    updateImage();
}

void ColorButton::updateImage(){
    if(!_color.isValid()){
        setIcon(QIcon());
        if(_setLabel)
            setText("Change color...");
        return;
    }
    const int w = 24, h = _bigIcons ? 24 : 12;
    QImage image(w, h, QImage::Format_RGB888);
    QPainter painter(&image);
    painter.setPen(Qt::NoPen);
    painter.fillRect(0, 0, w, h, QColor(0, 0, 0));
    if(_alpha && _color.alpha() != 255){
        painter.fillRect(1, 1, w / 2 - 1, h / 2 - 1, QColor(255, 255, 255));
        painter.fillRect(w / 2, 1, w / 2 - 1, h / 2 - 1, QColor(192, 192, 192));
        painter.fillRect(1, h / 2, w / 2 - 1, h / 2 - 1, QColor(192, 192, 192));
        painter.fillRect(w / 2, h / 2, w / 2 - 1, h / 2 - 1, QColor(255, 255, 255));
    }
    painter.fillRect(1, 1, w - 2, h - 2, _color);
    setIcon(QIcon(QPixmap::fromImage(image)));
    if(_setLabel){
        QString text = QString(" (%1, %2, %3)").arg(_color.red()).arg(_color.green()).arg(_color.blue());
        if(_alpha)
            text += QString(", %4%").arg(QString::number(_color.alphaF() * 100., 'f', 0));
        setText(text);
    }
}

void ColorButton::pickColor(){
    QColor newColor = QColorDialog::getColor(_color, this, QString(), QColorDialog::ColorDialogOptions(_alpha ? QColorDialog::ShowAlphaChannel : 0));
    if(newColor.isValid()){
        _color = newColor;
        updateImage();
        emit colorChanged(_color);
    }
}

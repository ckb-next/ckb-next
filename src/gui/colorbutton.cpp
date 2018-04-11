#include "colorbutton.h"
#include <QColorDialog>
#include <QPainter>
#include <QMessageBox>
#include <QProcessEnvironment>
#include "ckbsettings.h"

ColorButton::ColorButton(QWidget* parent, bool allowAlpha) :
    QPushButton(parent), _alpha(allowAlpha), _setLabel(true), _bigIcons(false)
{
#if QT_VERSION >= 0x050700
    // Make sure the version warning function is called before there is any attempt to draw the colour picker.
    connect(this, SIGNAL(clicked()), this, SLOT(versionWarning()), Qt::DirectConnection);
#endif
    setAutoDefault(false);
    setDefault(false);
    updateImage();
    // Pick color on click (use queued connection so that any on_*_clicked() events can be processed first)
    connect(this, SIGNAL(clicked()), this, SLOT(pickColor()), Qt::QueuedConnection);
#if QT_VERSION >= 0x050700
    // If the widget was drawn, mark as dismissed.
    connect(this, SIGNAL(clicked()), this, SLOT(versionWarningDismissed()), Qt::QueuedConnection);
#endif
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

void ColorButton::versionWarning(){
    CkbSettings settings("Program");
    if(!settings.value("QtWarningDismissed").toBool()){
        QProcessEnvironment procEnv = QProcessEnvironment::systemEnvironment();
        QString qpaTheme = procEnv.value("QT_QPA_PLATFORMTHEME", QString("")).toLower();
        // Whitelist
        QStringList qpaThemeWhiteList = QStringList() << "qt5ct" << "kde" << "appmenu-qt5";
        if(qpaTheme.isEmpty() || !qpaThemeWhiteList.contains(qpaTheme)){
            QMessageBox qtWarning(this);
            qtWarning.setTextFormat(Qt::RichText);
            qtWarning.setIcon(QMessageBox::Warning);
            qtWarning.setText(tr("A system configuration that can lead to instability issues with this software has been detected.<br><br>"
                                 "If this application locks up after clicking the OK button below, please refer to "
                                 "<a href=\"https://github.com/ckb-next/ckb-next/wiki/Troubleshooting#glib-critical-errors-and-lock-up-at-colour-chooser\">https://github.com/ckb-next/ckb-next/wiki/Troubleshooting#glib-critical-errors-and-lock-up-at-colour-chooser</a>"));
            qtWarning.setStandardButtons(QMessageBox::Ok);
            qtWarning.exec();
        }
    }
}

void ColorButton::versionWarningDismissed(){
    CkbSettings settings("Program");
    if(!settings.value("QtWarningDismissed").toBool())
        settings.setValue("QtWarningDismissed", true);
}

#ifndef KBLIGHTWIDGET_H
#define KBLIGHTWIDGET_H

#include <QWidget>
#include <QColor>
#include <QFile>
#include <QResizeEvent>
#include "rgbwidget.h"

namespace Ui {
class KbLightWidget;
}

class KbLightWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KbLightWidget(QWidget *parent = 0);
    ~KbLightWidget();

    QColor fgColor;
    bool active, forceLight;

    RgbWidget* rgbWidget;

    void frameUpdate(QFile& cmd, int modenumber);
    void close(QFile& cmd, int modenumber);

public slots:
    void newSelection(QColor selectedColor, int selectedCount);
    void changeBG(QColor newColor);
    void changeFG(QColor newColor);

private slots:
    void on_brightnessBox_currentIndexChanged(int index);

    void on_animBox_currentIndexChanged(int index);

private:
    void animSolid(QFile& cmd, float light);
    void animWave(QFile& cmd, float light);
    void animRipple(QFile& cmd, float light);

    Ui::KbLightWidget *ui;
    friend class KbWidget;
};

#endif // KBLIGHTWIDGET_H

#ifndef KBLIGHTWIDGET_H
#define KBLIGHTWIDGET_H

#include <QWidget>
#include <QColor>
#include <QFile>
#include <QResizeEvent>
#include "kblight.h"
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

    RgbWidget* rgbWidget;

    void setLight(KbLight* newLight);

private slots:
    void newSelection(QColor selectedColor, int selectedCount);
    void changeBG(QColor newColor);
    void changeFG(QColor newColor);

    void on_brightnessBox_currentIndexChanged(int index);
    void on_animBox_currentIndexChanged(int index);
    void on_inactiveCheck_stateChanged(int arg1);
    void on_inactiveLevelBox_currentIndexChanged(int index);

private:
    KbLight* light;

    Ui::KbLightWidget *ui;
    friend class KbWidget;
};

#endif // KBLIGHTWIDGET_H

#ifndef KBLIGHTWIDGET_H
#define KBLIGHTWIDGET_H

#include <QWidget>
#include <QColor>
#include <QFile>
#include <QResizeEvent>
#include "kblight.h"
#include "keywidget.h"

namespace Ui {
class KbLightWidget;
}

class KbLightWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KbLightWidget(QWidget *parent = 0);
    ~KbLightWidget();

    void setLight(KbLight* newLight);

private slots:
    void updateLight();
    void newSelection(QStringList selection);
    void changeColor(QColor newColor);
    void changeAnim(KbAnim* newAnim);
    void changeAnimKeys(QStringList keys);

    void on_brightnessBox_currentIndexChanged(int index);
    void on_animButton_clicked();

private:
    KbLight* light;

    QStringList currentSelection;

    Ui::KbLightWidget *ui;
    friend class KbWidget;
};

#endif // KBLIGHTWIDGET_H

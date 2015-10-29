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
    void setMonochrome();

private slots:
    void updateLight();
    void newSelection(QStringList selection);
    void changeColor(QColor newColor);
    void changeAnim(KbAnim* newAnim);
    void changeAnimKeys(QStringList keys);

    void on_brightnessBox_activated(int index);
    void on_animButton_clicked();

    void on_bgButton_clicked();

    void on_showAnimBox_clicked(bool checked);

    void toggleSidelight(); //strafe

private:
    KbLight* light;
    QStringList currentSelection;

    Ui::KbLightWidget *ui;
};

#endif // KBLIGHTWIDGET_H

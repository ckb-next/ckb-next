#ifndef KBLIGHTWIDGET_H
#define KBLIGHTWIDGET_H

#include <QWidget>
#include <QFile>
#include <QColor>

namespace Ui {
class KbLightWidget;
}

class KbLightWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KbLightWidget(QWidget *parent = 0);
    ~KbLightWidget();

    QColor fgColor, bgColor;
    bool active, forceLight;

public slots:
    void frameUpdate(QFile& cmd, int modenumber, int layout);
    void close(QFile& cmd, int modenumber);
    void changeFG(QColor newColor);
    void changeBG(QColor newColor);

private slots:
    void on_brightnessBox_currentIndexChanged(int index);

    void on_animBox_currentIndexChanged(int index);

private:
    Ui::KbLightWidget *ui;
    friend class KbWidget;
};

#endif // KBLIGHTWIDGET_H

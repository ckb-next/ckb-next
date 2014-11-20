#ifndef KBWIDGET_H
#define KBWIDGET_H

#include <QWidget>
#include <QFile>

namespace Ui {
class KbWidget;
}

class KbWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KbWidget(QWidget *parent, const QString& path);
    ~KbWidget();

    QString devpath, cmdpath, notifypath;
    QString serial;
    QString model;
    QString firmware;

    int notifyNumber;

    QColor fgColor, bgColor;

public slots:
    void frameUpdate();
    void changeFG(QColor newColor);
    void changeBG(QColor newColor);

private slots:
    void on_layoutBox_currentIndexChanged(int index);
    void on_brightnessBox_currentIndexChanged(int index);

private:
    Ui::KbWidget *ui;

    void getCmd(QFile& file);
    void readInput();
};

#endif // KBWIDGET_H

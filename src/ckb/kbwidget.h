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

    int notifyNumber;

    QColor fgColor, bgColor;

public slots:
    void frameUpdate();
    void changeFG(QColor newColor);
    void changeBG(QColor newColor);

private slots:
    void on_lightCheckbox_clicked(bool checked);

    void on_layoutBox_currentIndexChanged(int index);

private:
    Ui::KbWidget *ui;

    void getCmd(QFile& file);
    void readInput();
};

#endif // KBWIDGET_H

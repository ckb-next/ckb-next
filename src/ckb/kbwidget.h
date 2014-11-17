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

    QString devpath;
    QString serial;
    QString model;

    QFile cmd;

    QColor fg, bg;

public slots:
    void frameUpdate();
    void changeFG(QColor newColor);
    void changeBG(QColor newColor);

private slots:
    void on_lightCheckbox_clicked(bool checked);

private:
    Ui::KbWidget *ui;
};

#endif // KBWIDGET_H

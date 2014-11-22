#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>

namespace Ui {
class SettingsWidget;
}

class SettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsWidget(QWidget *parent = 0);
    ~SettingsWidget();

private slots:
    void on_pushButton_clicked();

    void on_fpsBox_currentIndexChanged(const QString &arg1);

private:
    Ui::SettingsWidget *ui;
    friend class MainWindow;
};

#endif // SETTINGSWIDGET_H

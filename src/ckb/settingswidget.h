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

    void setStatus(const QString& text);

private slots:
    void on_pushButton_clicked();

    void on_fpsBox_activated(const QString &arg1);
    void on_animScanButton_clicked();

    void on_capsBox_activated(int index);
    void on_shiftBox_activated(int index);
    void on_ctrlBox_activated(int index);
    void on_altBox_activated(int index);
    void on_winBox_activated(int index);

    void on_autoFWBox_clicked(bool checked);

private:
    Ui::SettingsWidget *ui;
    friend class MainWindow;

    void updateModifiers();
};

#endif // SETTINGSWIDGET_H

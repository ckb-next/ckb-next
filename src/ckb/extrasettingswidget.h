#ifndef EXTRASETTINGSWIDGET_H
#define EXTRASETTINGSWIDGET_H

#include <QDialog>

namespace Ui {
class ExtraSettingsWidget;
}

class ExtraSettingsWidget : public QDialog
{
    Q_OBJECT

public:
    explicit ExtraSettingsWidget(QWidget *parent = 0);
    ~ExtraSettingsWidget();

    void pollUpdates();

private slots:
    void on_trayBox_clicked(bool checked);
    void on_brightnessBox_clicked(bool checked);
    void on_animScanButton_clicked();
    void on_fpsBox_valueChanged(int arg1);
    void on_ditherBox_clicked(bool checked);

    void on_mAccelBox_clicked(bool checked);
    void on_sAccelBox_clicked(bool checked);
    void on_sSpeedBox_valueChanged(int arg1);

private:
    Ui::ExtraSettingsWidget *ui;
};

#endif // EXTRASETTINGSWIDGET_H

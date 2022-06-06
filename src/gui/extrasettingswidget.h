#ifndef EXTRASETTINGSWIDGET_H
#define EXTRASETTINGSWIDGET_H

#include <QWidget>

namespace Ui {
class ExtraSettingsWidget;
}

class ExtraSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ExtraSettingsWidget(QWidget *parent = nullptr);
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
    void on_startDelayBox_clicked(bool checked);
    void on_previewBox_clicked(bool checked);
    void on_detailsBtn_clicked();
    void on_timerBox_clicked(bool checked);
    void on_timerMinBox_editingFinished();
    void on_monochromeBox_toggled(bool checked);

private:
    Ui::ExtraSettingsWidget *ui;
};

#endif // EXTRASETTINGSWIDGET_H

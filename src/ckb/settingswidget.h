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

    // Set text labels
    void setVersion(const QString& version);
    void setStatus(const QString& text);

    // Current FPS
    int frameRate() const;

    // Poll for setting updates and save (if necessary)
    void pollUpdates();

private slots:
    void on_pushButton_clicked();

    void on_fpsBox_valueChanged(int framerate);
    void on_animScanButton_clicked();

    void on_capsBox_activated(int index);
    void on_shiftBox_activated(int index);
    void on_ctrlBox_activated(int index);
    void on_altBox_activated(int index);
    void on_winBox_activated(int index);

    void on_brightnessBox_clicked(bool checked);
    void on_autoFWBox_clicked(bool checked);
    void on_trayBox_clicked(bool checked);
    void on_loginItemBox_clicked(bool checked);
    void on_layoutBox_currentIndexChanged(int index);
    void on_ditherBox_clicked(bool checked);

private:
    Ui::SettingsWidget *ui;
    friend class MainWindow;

    void updateModifiers();
};

#endif // SETTINGSWIDGET_H

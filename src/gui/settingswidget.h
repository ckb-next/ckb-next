#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>
#include "extrasettingswidget.h"
#include <QProcess>
#include <QProgressDialog>
#include "ckbversionnumber.h"

namespace Ui {
class SettingsWidget;
}

class SettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsWidget(QWidget *parent = nullptr);
    ~SettingsWidget();

    // Set text labels
    void setVersion(const CkbVersionNumber& version);
    void setStatus(const QString& text);

    // Poll for setting updates and save (if necessary)
    void pollUpdates();

    void enableUpdateButton();
    void setUpdateButtonText(const QString& text);

signals:
    void checkForUpdates();

private slots:
    void on_pushButton_clicked();
    void on_capsBox_activated(int index);
    void on_shiftBox_activated(int index);
    void on_ctrlBox_activated(int index);
    void on_altBox_activated(int index);
    void on_winBox_activated(int index);
    void on_autoFWBox_clicked(bool checked);
    void on_aboutQt_clicked();
    void on_generateReportButton_clicked();
    void devDetectFinished(int retVal);
    void devDetectDestroyed();
    void on_autoUpdBox_clicked(bool checked);
    void on_pushButton_2_clicked();
    void on_uninstallButton_clicked();
    void on_hiDPIBox_clicked(bool checked);

private:
    QProcess* devDetect;
    QProgressDialog* devDetectProgress;
    Ui::SettingsWidget *ui;
    friend class MainWindow;
    ExtraSettingsWidget* extra;
    void updateModifiers();
};

#endif // SETTINGSWIDGET_H

#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>
#include "extrasettingswidget.h"

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

    // Poll for setting updates and save (if necessary)
    void pollUpdates();

private slots:
    void on_pushButton_clicked();

    void on_capsBox_activated(int index);
    void on_shiftBox_activated(int index);
    void on_ctrlBox_activated(int index);
    void on_altBox_activated(int index);
    void on_winBox_activated(int index);

    void on_autoFWBox_clicked(bool checked);
    void on_loginItemBox_clicked(bool checked);
    void on_layoutBox_activated(int index);
    void showLayoutDialog();

    void on_extraButton_clicked();

private:
    Ui::SettingsWidget *ui;
    friend class MainWindow;

    ExtraSettingsWidget* extra;

    void updateModifiers();
};

#endif // SETTINGSWIDGET_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCloseEvent>
#include <QMainWindow>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QTimer>
#include "kbwidget.h"
#include "settingswidget.h"

#ifdef USE_LIBAPPINDICATOR
#define signals_BACKUP signals
#undef signals
extern "C" {
  #include <libappindicator/app-indicator.h>
  #include <gtk/gtk.h>
}
#define signals signals_BACKUP
#undef signals_BACKUP
#endif // USE_LIBAPPINDICATOR

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    static MainWindow* mainWindow;

    void toggleTrayIcon(bool visible);

private:
    SettingsWidget* settingsWidget;
    QList<KbWidget*> kbWidgets;

    QAction* restoreAction;
    QAction* closeAction;

#ifdef USE_LIBAPPINDICATOR
    bool                unityDesktop;
    AppIndicator*       indicator;
    GtkWidget*          indicatorMenu;
    GtkWidget*          indicatorMenuQuitItem;
    GtkWidget*          indicatorMenuRestoreItem;
#endif // USE_LIBAPPINDICATOR
    QMenu*              trayIconMenu;
    QSystemTrayIcon*    trayIcon;

    void closeEvent(QCloseEvent *event);

public slots:
    void showWindow();
    void stateChange(Qt::ApplicationState state);
    void quitApp();

private slots:
    void addDevice(Kb* device);
    void removeDevice(Kb* device);
    void updateVersion();
    void checkFwUpdates();

    void timerTick();
    void iconClicked(QSystemTrayIcon::ActivationReason reason);
    void cleanup();
    void showFwUpdateNotification(QWidget* widget, float version);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H

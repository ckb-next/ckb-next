#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCloseEvent>
#include <QMainWindow>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QTimer>
#include "kbwidget.h"
#include "settingswidget.h"
#include <QSocketNotifier>

#ifndef DISABLE_UPDATER
#include "ckbupdater.h"
#endif

#ifdef USE_LIBAPPINDICATOR
// 'signals' has to be undefined as GTK has its own signal mechanism
#undef signals
extern "C" {
  #include <libappindicator/app-indicator.h>
  #include <gtk/gtk.h>
}
// Redefine QT signals as per qtbase/src/corelib/kernel/qobjectdefs.h
#define signals Q_SIGNALS
#endif

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
    static int signalHandlerFd[2];
    static void PosixSignalHandler(int signal);
    QIcon getIcon();

private:
    SettingsWidget* settingsWidget;
    QList<KbWidget*> kbWidgets;
    QAction* restoreAction;
    QAction* closeAction;
    QAction* changeTrayIconAction;

#ifdef USE_LIBAPPINDICATOR
    bool                useAppindicator;
    AppIndicator*       indicator;
    GtkWidget*          indicatorMenu;
    GtkWidget*          indicatorMenuQuitItem;
    GtkWidget*          indicatorMenuRestoreItem;
#endif

    QMenu*              trayIconMenu;
    QSystemTrayIcon*    trayIcon;

    void closeEvent(QCloseEvent *event);

public slots:
    void showWindow();
    void stateChange(Qt::ApplicationState state);
    void quitApp();
    void checkForCkbUpdates();
    void changeTrayIconToMonochrome();
    void changeTrayIconToRGB();


private slots:
    void addDevice(Kb* device);
    void removeDevice(Kb* device);
    void updateVersion();
    void checkFwUpdates();
    void timerTick();
    void iconClicked(QSystemTrayIcon::ActivationReason reason);
    void cleanup();
    void showFwUpdateNotification(QWidget* widget, float version);
    void QSignalHandler();
    void checkedForNewVer(QString ver, QString changelog);

private:
    Ui::MainWindow *ui;
    QSocketNotifier* sigNotifier;
#ifndef DISABLE_UPDATER
    CkbUpdater* updater;
#endif
};

#endif // MAINWINDOW_H

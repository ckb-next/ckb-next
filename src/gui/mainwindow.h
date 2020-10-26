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
#include "ckbsystemtrayicon.h"

#ifndef DISABLE_UPDATER
#include "ckbupdater.h"
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
    void syncTrayIcon();
    void setTabsEnabled(bool e);

private:
    SettingsWidget* settingsWidget;
    QList<KbWidget*> kbWidgets;
    QAction* restoreAction;
    QAction* closeAction;
    QMenu*              trayIconMenu;
    CkbSystemTrayIcon*  trayIcon;
    void closeEvent(QCloseEvent *event);

#if defined(Q_OS_MACOS) && !defined(OS_MAC_LEGACY)
    QTimer*             catalinaTimer;
    typedef enum {
       REQUEST_STATE_NOT_SET = -1,
       REQUEST_SUCCEEDED = 0,
       REQUEST_ALREADY_ALLOWED,
       REQUEST_ERROR
    } hid_req_ret;
    hid_req_ret prevHidRet = REQUEST_STATE_NOT_SET;
    bool catalinaAgentStarted = false;

#endif
    static QIcon getIcon();
    static QString getIconName();

public slots:
    void showWindow();
    void stateChange(Qt::ApplicationState state);
    void quitApp();
    void checkForCkbUpdates();
    void handleTrayScrollEvt(int delta, Qt::Orientation orientation);

signals:
    void switchToProfileCLI(QString profile);
    void switchToModeCLI(QString mode);

private slots:
    void addDevice(Kb* device);
    void removeDevice(Kb* device);
    void updateVersion();
    void checkFwUpdates();
    void timerTick();
    void cleanup();
    void showFwUpdateNotification(QWidget* widget, float version);
    void QSignalHandler();
    void checkedForNewVer(QString ver, QString changelog);
#if defined(Q_OS_MACOS) && !defined(OS_MAC_LEGACY)
    void appleRequestHidTimer();
#endif
    void iconClicked(QSystemTrayIcon::ActivationReason reason);
private:
    Ui::MainWindow *ui;
    QSocketNotifier* sigNotifier;
#ifndef DISABLE_UPDATER
    CkbUpdater* updater;
#endif

signals:
    void trayIconScrolled(bool up);
};

#endif // MAINWINDOW_H

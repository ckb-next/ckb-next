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

// ckb version info (populated at load)
extern float ckbGuiVersion;
extern float ckbDaemonVersion;
// String (e.g. "alpha-v0.0.1" or "0.0.1") -> float
inline float PARSE_CKB_VERSION(QString version){
    // Remove extraneous info (anything after a +, anything non-numeric)
    QStringList dots = version.replace(QRegExp("\\+.+"), "").replace(QRegExp("[^\\d\\.]"), "").split(".");
    float base = 1.f;
    float res = 0.f;
    // A number like "1.2.3" will result in 1.0203
    // NB: will fail if a point version goes over 99 or if using more than two dots. floats can only reliably encode 7 decimal digits.
    foreach(const QString& dot, dots){
        res += dot.toFloat() * base;
        base /= 100.f;
    }
    return res;
}

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void toggleTrayIcon(bool visible);
    void scanKeyboards();
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
    QString             iconPath;
#endif // USE_LIBAPPINDICATOR
    QMenu*              trayIconMenu;
    QSystemTrayIcon*    trayIcon;

    void closeEvent(QCloseEvent *event);

    static MainWindow* mainWindow;

public slots:
    void showWindow();
    void quitApp();

private slots:
    void timerTick();
    void iconClicked(QSystemTrayIcon::ActivationReason reason);
    void cleanup();
    void showFwUpdateNotification(QWidget* widget, float version);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H

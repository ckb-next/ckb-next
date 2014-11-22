#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCloseEvent>
#include <QMainWindow>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QTimer>
#include "kbwidget.h"
#include "settingswidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void scanKeyboards();
    SettingsWidget* settingsWidget;
    QList<KbWidget*> kbWidgets;

    QAction* restoreAction;
    QAction* closeAction;

    QMenu* trayIconMenu;
    QSystemTrayIcon* trayIcon;

    void closeEvent(QCloseEvent *event);

public slots:
    void timerTick();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H

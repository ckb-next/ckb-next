#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QTimer>
#include "kbwidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QTimer* eventTimer;

    void scanKeyboards();
    QWidget* noKbWidget;
    QList<KbWidget*> kbWidgets;

    QAction* restoreAction;
    QAction* closeAction;

    QMenu* trayIconMenu;
    QSystemTrayIcon* trayIcon;

    void closeEvent(QCloseEvent *event);

public slots:
    void timerTick();
    void about();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_settingswidget.h"
#include <QSharedMemory>
#include <QMessageBox>

extern QSharedMemory appShare;

static const QString configLabel = "Settings";
#ifndef __APPLE__
QString devpath = "/dev/input/ckb%1";
#else
QString devpath = "/tmp/ckb%1";
#endif

QTimer* eventTimer = 0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    trayIconMenu = new QMenu(this);
    restoreAction = new QAction(tr("Restore"), this);
    closeAction = new QAction(tr("Exit"), this);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addAction(closeAction);
    trayIcon = new QSystemTrayIcon(QIcon(":/img/ckb-logo.png"), this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->show();

    connect(ui->actionExit, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(closeAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(show()));

    eventTimer = new QTimer(this);
    connect(eventTimer, SIGNAL(timeout()), this, SLOT(timerTick()));
    eventTimer->start(1000 / 60);

    QCoreApplication::setOrganizationName("ckb");

    ui->tabWidget->addTab(settingsWidget = new SettingsWidget(this), configLabel);

    scanKeyboards();
}

void MainWindow::scanKeyboards(){
    QString rootdev = devpath.arg(0);
    QFile connected(rootdev + "/connected");
    if(!connected.open(QIODevice::ReadOnly)){
        // No root controller - remove all keyboards
        while(ui->tabWidget->count() > 1)
            ui->tabWidget->removeTab(0);
        foreach(KbWidget* w, kbWidgets)
            w->deleteLater();
        kbWidgets.clear();
        return;
    }

    // Scan connected devices
    foreach(KbWidget* w, kbWidgets)
        w->disconnect = true;
    QString line;
    while((line = connected.readLine().trimmed()) != ""){
        QString dev = line.split(" ")[0].trimmed();
        if(dev == "")
            break;
        // Connected already?
        KbWidget* widget = 0;
        foreach(KbWidget* w, kbWidgets){
            if(w->devpath == dev){
                widget = w;
                widget->disconnect = false;
                break;
            }
        }
        if(widget)
            continue;
        // Add the keyboard
        widget = new KbWidget(this, dev, "Devices");
        if(widget->cmdpath == ""){
            delete widget;
            continue;
        }
        kbWidgets.append(widget);
        ui->tabWidget->insertTab(ui->tabWidget->count() - 1, widget, widget->model);
        connect(eventTimer, SIGNAL(timeout()), widget, SLOT(frameUpdate()));
    }
    connected.close();

    // Remove any devices not found in the connected list
    QList<KbWidget*> kbWidgets2 = kbWidgets;
    foreach(KbWidget* w, kbWidgets2){
        if(w->disconnect){
            int i = kbWidgets.indexOf(w);
            ui->tabWidget->removeTab(i);
            kbWidgets.removeAt(i);
            w->deleteLater();
        }
    }

    int count = kbWidgets.count();
    if(count == 0)
        settingsWidget->ui->devicesLabel->setText("No devices connected");
    else if(count == 1)
        settingsWidget->ui->devicesLabel->setText("1 device connected");
    else
        settingsWidget->ui->devicesLabel->setText(QString("%1 devices connected").arg(count));
}

void MainWindow::closeEvent(QCloseEvent *event){
    QMessageBox::information(this, "ckb", "ckb will still run in the background.\nTo close it, choose Exit from the tray menu\nor click \"Quit ckb\" on the Settings screen.");
    hide();
    event->ignore();
}

void MainWindow::timerTick(){
    // Check if another instance requested this in the foreground
    if(appShare.lock()){
        void* data = appShare.data();
        if((QString)QByteArray((const char*)data) == "Open"){
            show();
            raise();
            activateWindow();
        }
        // Remove the request
        *(char*)data = 0;
        appShare.unlock();
    }
    // Scan for connected/disconnected keyboards
    scanKeyboards();
}

MainWindow::~MainWindow(){
    delete ui;
}

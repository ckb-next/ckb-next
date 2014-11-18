#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSharedMemory>
#include <QMessageBox>

extern QSharedMemory appShare;

static const QString noKbMsg = "No keyboard detected";
#ifndef __APPLE__
static const QString devpath = "/dev/input/ckb%1";
#else
static const QString devpath = "/tmp/ckb%1";
#endif

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

    ui->tabWidget->addTab(noKbWidget = new QWidget(this), noKbMsg);

    connect(ui->actionExit, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));
    connect(closeAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(show()));

    eventTimer = new QTimer(this);
    connect(eventTimer, SIGNAL(timeout()), this, SLOT(timerTick()));
    eventTimer->start(16);

    scanKeyboards();
}

void MainWindow::scanKeyboards(){
    QString rootdev = devpath.arg(0);
    QFile connected(rootdev + "/connected");
    if(!connected.open(QIODevice::ReadOnly)){
        // No root controller
        if(ui->tabWidget->currentWidget() != noKbWidget){
            while(ui->tabWidget->count() > 0)
                ui->tabWidget->removeTab(0);
            ui->tabWidget->addTab(noKbWidget, noKbMsg);
        }
        foreach(KbWidget* w, kbWidgets)
            w->deleteLater();
        kbWidgets.clear();
    }
    // Check if any currently-connected keyboards have been removed
    QList<KbWidget*> kbWidgets2 = kbWidgets;
    foreach(KbWidget* w, kbWidgets2){
        if(w->cmdpath == ""){
            w->deleteLater();
            ui->tabWidget->removeTab(kbWidgets.indexOf(w));
            kbWidgets.removeAll(w);
        }
    }
    QString line;
    int count = kbWidgets.count();
    while((line = connected.readLine()) != ""){
        QString dev = line.split(" ")[0];
        if(dev == "")
            break;
        // Connected already?
        KbWidget* widget = 0;
        foreach(KbWidget* w, kbWidgets){
            if(w->devpath == dev){
                widget = w;
                break;
            }
        }
        if(widget)
            continue;
        // Add the keyboard
        widget = new KbWidget(this, dev);
        if(widget->cmdpath == ""){
            delete widget;
            continue;
        }
        count++;
        kbWidgets.append(widget);
        ui->tabWidget->addTab(widget, widget->model);
        connect(eventTimer, SIGNAL(timeout()), widget, SLOT(frameUpdate()));
    }
    connected.close();
    if(count == 0){
        // No keyboards found
        if(ui->tabWidget->currentWidget() != noKbWidget){
            while(ui->tabWidget->count() > 0)
                ui->tabWidget->removeTab(0);
            ui->tabWidget->addTab(noKbWidget, noKbMsg);
        }
        foreach(KbWidget* w, kbWidgets)
            w->deleteLater();
        kbWidgets.clear();
    } else if(ui->tabWidget->widget(0) == noKbWidget){
        // Keyboards found
        // Remove "no keyboard detected" if it's there
        ui->tabWidget->removeTab(0);
    }
}

void MainWindow::closeEvent(QCloseEvent *event){
    QMessageBox::information(this, "ckb", "ckb will still run in the background.\nTo close it, use the Exit option from the menu.");
    hide();
    event->ignore();
}

void MainWindow::timerTick(){
    // Check if another instance requested this in the foreground
    if(appShare.lock()){
        void* data = appShare.data();
        if((QString)QByteArray((const char*)data) == "Open")
            show();
        // Remove the request
        *(char*)data = 0;
        appShare.unlock();
    }
    scanKeyboards();
}

void MainWindow::about(){
    QMessageBox::information(this, "About ckb", "ckb v0.0.2\nhttps://github.com/ccMSC/ckb\n\nLicensed under GPL v2");
}

MainWindow::~MainWindow(){
    delete ui;
}

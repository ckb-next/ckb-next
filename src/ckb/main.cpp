#include "mainwindow.h"
#include <QApplication>
#include <QSharedMemory>
#include <cstring>

QSharedMemory appShare("ckb");

bool isRunning(){
    if(!appShare.create(16)){
        if(!appShare.attach() || !appShare.lock())
            return false;
        void* data = appShare.data();
        snprintf((char*)data, 16, "Open");
        appShare.unlock();
        return true;
    }
    return false;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    if(isRunning())
        return 0;
    MainWindow w;
    w.show();

    return a.exec();
}

#include "mainwindow.h"
#include <QApplication>
#include <QSharedMemory>
#include <cstring>

QSharedMemory appShare("ckb");

#ifdef Q_OS_MACX
// App Nap is an OSX feature designed to save power by putting background apps to sleep.
// However, it interferes with ckb's animations, so we want it off
extern "C" void disableAppNap();
#endif

bool isRunning(bool showWindow){
    if(!appShare.create(16)){
        if(!appShare.attach() || !appShare.lock())
            return false;
        if(showWindow){
            void* data = appShare.data();
            snprintf((char*)data, 16, "Open");
        }
        appShare.unlock();
        return true;
    }
    return false;
}

int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    bool background = qApp->arguments().contains("--background");
    if(isRunning(!background)){
        printf("ckb is already running. Exiting.\n");
        return 0;
    }
#ifdef Q_OS_MACX
    disableAppNap();
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
    // Enable HiDPI support
    qApp->setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    MainWindow w;
    if(!background)
        w.show();

    return a.exec();
}

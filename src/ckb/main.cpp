#include "mainwindow.h"
#include <QApplication>
#include <QSharedMemory>
#include <cstring>

QSharedMemory appShare("ckb");

#ifdef __APPLE__
// App Nap is an OSX feature designed to save power by putting background apps to sleep.
// However, it interferes with ckb's animations, so we want it off
extern "C" void disableAppNap();
#endif

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
#ifdef __APPLE__
    disableAppNap();
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
    // Enable HiDPI support
    qApp->setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    MainWindow w;
    if(!qApp->arguments().contains("--background"))
        w.show();

    return a.exec();
}

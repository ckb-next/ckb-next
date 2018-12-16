#include "daemonpipe.h"
#include <QLocalSocket>
// This file should only be included on Windows

/*void DaemonPipe::initPipe(QString str) {
    this->connectToServer(str);
}*/

bool DaemonPipe::open(int fd, QIODevice::OpenMode mode, QFileDevice::FileHandleFlags handleFlags)
{
    this->connectToServer(pipeName, mode);
    qDebug() << this->errorString();
    return true;
}

#include "daemonpipe.h"
#include <QLocalSocket>
// This file should only be included on Windows
#ifdef Q_OS_WIN32

bool DaemonPipe::open(int fd, QIODevice::OpenMode mode, QFileDevice::FileHandleFlags handleFlags)
{
    this->connectToServer(pipeName, mode);
    this->waitForConnected(500);
    qDebug() << this->errorString();
    return this->isOpen();
}

QByteArray DaemonPipe::readLine(qint64 maxSize)
{
    while(!this->waitForReadyRead() && !this->isOpen());
    if(!this->isOpen())
        return QByteArray();
    return QLocalSocket::readLine(maxSize);
}

#endif

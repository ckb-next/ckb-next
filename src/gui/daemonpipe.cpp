#include "daemonpipe.h"
#include <QDir>
#include <QLocalSocket>
// This file should only be included on Windows
#ifdef Q_OS_WIN32

bool DaemonPipe::open(int fd, QIODevice::OpenMode mode, QFileDevice::FileHandleFlags handleFlags)
{
    this->connectToServer(pipeName, mode);
    this->waitForConnected(500);
    if(this->error())
        qDebug() << this->errorString();
    return this->isOpen();
}

QByteArray DaemonPipe::readLine(qint64 maxSize)
{
    while(!this->waitForReadyRead());
    if(!this->isOpen())
        return QByteArray();

    return QLocalSocket::readLine(maxSize);
}

bool DaemonPipe::pipeExists(QString pipe)
{
    pipe = fixPathForWin32(pipe);
    QDir dir("\\\\.\\pipe\\");
    QFileInfoList list = dir.entryInfoList();
    for(int i = 0; i < list.count(); i++)
    {
        QString candidate = QDir::toNativeSeparators(list.at(i).absoluteFilePath());
        if(candidate == pipe)
        {
            qDebug() << "Match found for" << candidate;
            return true;
        }
    }
    return false;}

QString DaemonPipe::fixPathForWin32(QString name)
{
    QString pipe = name.mid(name.lastIndexOf("ckb")).replace(QChar('/'),QChar('\\'));
    pipe.prepend("\\\\.\\pipe\\ckb-next\\");
    qDebug() << name << "-->" << pipe;
    return pipe;
}
#endif

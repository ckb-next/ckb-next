#ifndef DAEMONPIPE_H
#define DAEMONPIPE_H
#include <QFile>
#include <QLocalSocket>
#include <QByteArray>

#ifdef Q_OS_WIN
class DaemonPipe : public QLocalSocket {
#else
class DaemonPipe : public QFile {
#endif
Q_OBJECT
public:
#ifdef Q_OS_WIN
    DaemonPipe(const QString &name) : QLocalSocket() {
        pipeName = fixPathForWin32(name);
        qDebug() << "Opening" << pipeName;
    }
    inline bool open(QIODevice::OpenMode mode) { return open(0, mode, 0); }
    bool open(int fd, QIODevice::OpenMode mode, QFileDevice::FileHandleFlags handleFlags);
    QByteArray readLine(qint64 maxSize = 0);
    static bool pipeExists(QString pipe);
    static QString fixPathForWin32(QString name);

private:
    QString pipeName;
#else
    DaemonPipe(const QString &name) : QFile(name) {}
#endif
};

#endif // DAEMONPIPE_H

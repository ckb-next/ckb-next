#ifndef CKBUPDATER_H
#define CKBUPDATER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class CkbUpdater : public QObject
{
    Q_OBJECT
public:
    explicit CkbUpdater(QObject *parent = 0);
    void checkForNewVersion();

private:
    QNetworkAccessManager* _accessManager;
    QString getVersionString(QByteArray* bytes);
    QString _latestVerStr;
    QString parseChangeLog(QByteArray* bytes);

signals:
    void checkedForNewVer(QString ver, QString changelog);

private slots:
    void finished(QNetworkReply* reply);
};

#endif // CKBUPDATER_H

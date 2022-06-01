#ifndef CKBUPDATER_H
#define CKBUPDATER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "ckbversionnumber.h"

class CkbUpdater : public QObject
{
    Q_OBJECT
public:
    explicit CkbUpdater(QObject *parent = nullptr);
    void checkForNewVersion();

private:
    QNetworkAccessManager* _accessManager;
    CkbVersionNumber getLatestVersion(const QByteArray& bytes);
    CkbVersionNumber latestVer;
    QString parseChangeLog(const QByteArray& bytes);

signals:
    void checkedForNewVer(QString ver, QString changelog);

private slots:
    void finished(QNetworkReply* reply);
};

#endif // CKBUPDATER_H

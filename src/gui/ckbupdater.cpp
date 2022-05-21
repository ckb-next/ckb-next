#include "ckbupdater.h"
#include "kbmanager.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include "ckbversionnumber.h"

CkbUpdater::CkbUpdater(QObject *parent) : QObject(parent), _accessManager(new QNetworkAccessManager(this)), latestVer(CkbVersionNumber()){
    connect(_accessManager, &QNetworkAccessManager::finished, this, &CkbUpdater::finished); // FIXME: use finished() on QNetworkReply instead
}

void CkbUpdater::checkForNewVersion(){
    _accessManager->get(QNetworkRequest(QUrl("https://raw.githubusercontent.com/ckb-next/ckb-next/master/VERSION.cmake")));
}

// Read the VERSION. If there's a newer one, fetch the changelog and parse it.
void CkbUpdater::finished(QNetworkReply* reply){
    if(reply->error() == QNetworkReply::NoError) {
        QByteArray bytes = reply->readAll();

        if(reply->url().url().endsWith("VERSION.cmake")) {
            latestVer = getLatestVersion(bytes);
            CkbVersionNumber currentVer = KbManager::ckbGuiVersion();

            if(latestVer > currentVer)
                _accessManager->get(QNetworkRequest(QUrl("https://raw.githubusercontent.com/ckb-next/ckb-next/master/CHANGELOG.md")));
            else
                emit checkedForNewVer(QString(), QString());
        } else
            emit checkedForNewVer(latestVer.toString(), parseChangeLog(bytes));
    } else
        emit checkedForNewVer(QString(), QString());
    reply->deleteLater();
}

CkbVersionNumber CkbUpdater::getLatestVersion(const QByteArray& bytes){
    //return CkbVersionNumber(QLatin1String("99.99.99")); // Uncomment to force the update message to pop up

    QString str(bytes);

    QRegularExpression re("ckb-next_VERSION +\"([0-9\\.]+)\"", QRegularExpression::MultilineOption);
    QRegularExpressionMatch match = re.match(str);
    if(match.hasMatch())
        return CkbVersionNumber(match.captured(1));
    return CkbVersionNumber();
}

QString CkbUpdater::parseChangeLog(const QByteArray& bytes){
    QList<QByteArray> bytelist = bytes.split('\n');

    bool append = false;
    QString changelog;
    for(int i = 0; i < bytelist.count(); i++) {
        if(bytelist.at(i).startsWith("## [v")) {
            if(append)
                break;
            append = true;
            continue;
        }
        if(bytelist.at(i).startsWith("[Full Changelog]")) {
            i++;
            continue;
        }
        if(append) {
            changelog.append(bytelist.at(i));
            changelog.append('\n');
        }
    }
    return changelog;
}

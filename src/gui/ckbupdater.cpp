#include "ckbupdater.h"
#include "kbmanager.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

CkbUpdater::CkbUpdater(QObject *parent) : QObject(parent), _accessManager(new QNetworkAccessManager(this)), _latestVerStr(QString()){
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
            _latestVerStr = getVersionString(&bytes);
            float latestVer = KbManager::parseVersionString(_latestVerStr);
            float currentVer = KbManager::ckbGuiVersionF();

            if(latestVer <= currentVer) {
                emit checkedForNewVer(QString(), QString());
            } else
                _accessManager->get(QNetworkRequest(QUrl("https://raw.githubusercontent.com/ckb-next/ckb-next/master/CHANGELOG.md")));
        } else
            emit checkedForNewVer(_latestVerStr, parseChangeLog(&bytes));

    } else
        emit checkedForNewVer(QString(), QString());
    reply->deleteLater();
}

QString CkbUpdater::getVersionString(QByteArray* bytes){
    // return "0.3.2"; // Uncomment and set a higher version to force the update message to pop up
    QString ret;

    if(bytes->count() > 1000)
        return ret;

    QString str(*bytes);

    QRegularExpression re("ckb-next_VERSION +\"([0-9\\.]+)\"", QRegularExpression::MultilineOption);
    QRegularExpressionMatch match = re.match(str);
    if(match.hasMatch())
        ret = match.captured(1);
    return ret;
}

QString CkbUpdater::parseChangeLog(QByteArray* bytes){
    QByteArray lbytes = bytes->left(4000); // Don't read too much data
    QList<QByteArray> bytelist = lbytes.split('\n');

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

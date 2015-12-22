#include "kbfirmware.h"
#include "kbmanager.h"
#include "quazip/quazip.h"
#include "quazip/quazipfile.h"
#include <QDateTime>
#include <QDebug>

// Auto check: 1 hr
static const quint64 AUTO_CHECK_TIME = 60 * 60 * 1000;

KbFirmware KbFirmware::instance;
KbFirmware::FW::FW() : fwVersion(0.f), ckbVersion(0.f) {}

KbFirmware::KbFirmware() :
    lastCheck(0), lastFinished(0), networkManager(0), tableDownload(0), hasGPG(UNKNOWN)
{
}

KbFirmware::~KbFirmware(){
    //delete networkManager;
    // ^ This can cause a crash (QT bug?)
}

void KbFirmware::initManager(){
    if(networkManager)
        return;
    networkManager = new QNetworkAccessManager();
}

bool KbFirmware::_checkUpdates(){
    initManager();
    quint64 now = QDateTime::currentMSecsSinceEpoch();
    if(now < lastCheck + AUTO_CHECK_TIME)
        return false;
    tableDownload = networkManager->get(QNetworkRequest(QUrl("https://raw.githubusercontent.com/ccMSC/ckb/master/FIRMWARE")));
    connect(tableDownload, SIGNAL(finished()), this, SLOT(downloadFinished()));
    lastCheck = now;
    return true;
}

void KbFirmware::processDownload(QNetworkReply* reply){
    if(reply->error() != QNetworkReply::NoError)
        return;
    // Update last check
    lastCheck = lastFinished = QDateTime::currentMSecsSinceEpoch();
    QByteArray data = reply->readAll();
    // Don't do anything if this is the same as the last version downloaded
    QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
    if(hash == fwTableHash)
        return;
    fwTableHash = hash;
    if(hasGPG == UNKNOWN){
        // Check for a GPG installation
        QProcess gpg;
        gpg.start("gpg", QStringList("--version"));
        gpg.waitForFinished();
        if(gpg.error() == QProcess::FailedToStart)
            // No GPG install
            hasGPG = NO;
        else {
            QString output = QString::fromUtf8(gpg.readAll());
            // Must support RSA keys and SHA256
            if(output.contains("RSA", Qt::CaseInsensitive) && output.contains("SHA256", Qt::CaseInsensitive))
                hasGPG = YES;
            else
                hasGPG = NO;
        }
        if(!hasGPG)
            qDebug() << "No GPG detected, signature verification disabled";
    }
    if(hasGPG){
        // If GPG is available, check the signature on the file before proceeding.
        QDir tmp = QDir::temp();
        // Save file to a temporary path. Include PID to avoid conflicts
        qint64 pid = QCoreApplication::applicationPid();
        QString fwPath = tmp.absoluteFilePath(QString("ckb-%1-firmware").arg(pid));
        QFile firmware(fwPath);
        if(!firmware.open(QIODevice::WriteOnly)
                || firmware.write(data) != data.length()){
            qDebug() << "Failed to write firmware file to temporary location, aborting firmware check";
            return;
        }
        firmware.close();
        // Write GPG key
        QString keyPath = tmp.absoluteFilePath(QString("ckb-%1-key.gpg").arg(pid));
        if(!QFile::copy(":/bin/msckey.gpg", keyPath)){
            firmware.remove();
            qDebug() << "Failed to write GPG key to temporary location, aborting firmware check";
            return;
        }
        // Check signature
        QProcess gpg;
        gpg.start("gpg", QStringList("--no-default-keyring") << "--keyring" << keyPath << "--verify" << fwPath);
        gpg.waitForFinished();
        // Clean up temp files
        tmp.remove(fwPath);
        tmp.remove(keyPath);
        if(gpg.error() != QProcess::UnknownError || gpg.exitCode() != 0){
            qDebug() << "GPG couldn't verify firmware signature:";
            qDebug() << gpg.readAllStandardOutput();
            qDebug() << gpg.readAllStandardError();
            return;
        }
        // Signature good, proceed to update database
    }
    fwTable.clear();
    QStringList lines = QString::fromUtf8(data).split("\n");
    bool scan = false;
    foreach(QString line, lines){
        // Collapse whitespace
        line.replace(QRegExp("\\s+"), " ").remove(QRegExp("^\\s")).remove(QRegExp("\\s$"));
        // Skip empty or commented-out lines
        if(line.length() == 0 || line.at(0) == '#')
            continue;
        // Don't read anything until the entries begin and don't read anything after they end
        if(!scan){
            if(line == "!BEGIN FW ENTRIES")
                scan = true;
            else
                continue;
        }
        if(line == "!END FW ENTRIES")
            break;
        QStringList components = line.split(" ");
        if(components.length() != 7)
            continue;
        // "VENDOR-PRODUCT"
        QString device = components[0].toUpper() + "-" + components[1].toUpper();
        FW fw;
        fw.fwVersion = components[2].toFloat();                             // Firmware blob version
        fw.url = QUrl::fromPercentEncoding(components[3].toLatin1());       // URL to zip file
        fw.ckbVersion = KbManager::parseVersionString(components[4]);       // Minimum ckb version
        fw.fileName = QUrl::fromPercentEncoding(components[5].toLatin1());  // Name of file inside zip
        fw.hash = QByteArray::fromHex(components[6].toLatin1());            // SHA256 of file inside zip
        // Update entry
        fwTable[device] = fw;
    }
    qDebug() << "Downloaded new firmware list." << fwTable.count() << "entries found.";
}

void KbFirmware::downloadFinished(){
    if(!tableDownload)
        return;
    processDownload(tableDownload);
    tableDownload->deleteLater();
    tableDownload = 0;
    emit downloaded();
}

static QString tableName(const QString& features){
    QStringList components = features.split(" ");
    if(components.length() < 2)
        return "";
    // First two components are vendor and model
    QString vendorModel = components[0].toUpper() + "-" + components[1].toUpper();
    // Add "RGB" on RGB boards
    if(features.contains("rgb") && !features.contains("monochrome"))
        vendorModel += "RGB";
    return vendorModel;
}

float KbFirmware::_latestForBoard(const QString& features, bool waitForComplete){
    if((tableDownload || checkUpdates()) && waitForComplete){
        // If waiting is desired, enter an event loop and stay here until the download is finished
        QEventLoop loop(this);
        connect(this, SIGNAL(downloaded()), &loop, SLOT(quit()));
        loop.exec();
    }
    // Find this board
    QString name = tableName(features);
    FW info = fwTable.value(name);
    if(info.hash.isEmpty())
        return 0.f;
    // Don't return the new version if the current ckb doesn't support it
    if(info.ckbVersion > KbManager::ckbGuiVersionF() || info.ckbVersion > KbManager::ckbDaemonVersionF())
        return -1.f;
    return info.fwVersion;
}

QByteArray KbFirmware::_fileForBoard(const QString& features){
    QString name = tableName(features);
    FW info = fwTable.value(name);
    if(info.hash.isEmpty())
        return "";
    // Download zip from URL. Wait for it to finish.
    QNetworkReply* reply = networkManager->get(QNetworkRequest(QUrl(info.url)));
    QEventLoop loop(this);
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    // Download finished, process data
    if(reply->error() != QNetworkReply::NoError)
        return "";
    QByteArray zipData = reply->readAll();
    QBuffer buffer(&zipData);
    // Open zip archive
    QuaZip zip(&buffer);
    if(!zip.open(QuaZip::mdUnzip))
        return "";
    // Find the desired file
    if(!zip.setCurrentFile(info.fileName, QuaZip::csInsensitive))
        return "";
    QuaZipFile binFile(&zip);
    if(!binFile.open(QIODevice::ReadOnly))
        return "";
    QByteArray binary = binFile.readAll();
    // Check the hash
    if(QCryptographicHash::hash(binary, QCryptographicHash::Sha256) != info.hash)
        return "";
    return binary;
}

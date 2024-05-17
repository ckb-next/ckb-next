#include "kbfirmware.h"
#include "kbmanager.h"
#include <quazip.h>
#include <quazipfile.h>
#include "monotonicclock.h"
#include <QDebug>

// Auto check: 6 hr
static const qint64 AUTO_CHECK_TIME = 60 * 60 * 6000;

KbFirmware::FW::FW() : fwVersion(CkbVersionNumber()), ckbVersion(CkbVersionNumber()) {}

KbFirmware::KbFirmware() :
    lastCheck(0), lastFinished(0), tableDownload(nullptr), hasGPG(GPG_UNKNOWN)
{
    // Disable bearer polling. This corrects an issue with latency spikes when
    // using WiFi. The problem and workaround are described here:
    // https://lostdomain.org/2017/06/17/qt-qnetworkaccessmanager-causing-latency-spikes-on-wifi/
    qputenv("QT_BEARER_POLL_TIMEOUT", QByteArray::number(-1));
    networkManager = new QNetworkAccessManager(this);
    // Try to find GPG. gpg2 first
    _gpg = new QProcess();
    _gpg->setProgram("gpg2");
    _gpg->setArguments(QStringList("--version"));
    _gpg->start();
    _gpg->waitForFinished();
    if(_gpg->error() == QProcess::FailedToStart){
        // Try gpg now
        _gpg->setProgram("gpg");
        _gpg->start();
        _gpg->waitForFinished();
        if(_gpg->error() == QProcess::FailedToStart){
            hasGPG = GPG_NO;
            qDebug() << "No GPG2 or GPG detected. Signature verification disabled.";
        }
    }

    // If set to NO it means there was no binary found
    // So don't bother checking capabilities
    if(hasGPG == GPG_UNKNOWN){
        QString output = QString::fromUtf8(_gpg->readAll());
        // Must support RSA keys and SHA256
        if(output.contains("RSA", Qt::CaseInsensitive) && output.contains("SHA512", Qt::CaseInsensitive)){
            hasGPG = GPG_YES;
        } else {
            hasGPG = GPG_NO;
            qDebug() << _gpg->program() << "does not support RSA and SHA512. Signature verification disabled.";
        }
    }
}

KbFirmware::~KbFirmware(){
    networkManager->deleteLater();
    _gpg->deleteLater();
}

bool KbFirmware::checkUpdates(){
    qint64 now = MonotonicClock::msecs();
    if(now < lastCheck + AUTO_CHECK_TIME)
        return false;
    tableDownload = networkManager->get(QNetworkRequest(QUrl("https://raw.githubusercontent.com/ckb-next/ckb-next/master/FIRMWARE")));
    connect(tableDownload, SIGNAL(finished()), this, SLOT(downloadFinished()));
    lastCheck = now;
    return true;
}

void KbFirmware::processDownload(QNetworkReply* reply){
    if(reply->error() != QNetworkReply::NoError)
        return;
    // Update last check
    lastCheck = lastFinished = MonotonicClock::msecs();
    QByteArray data = reply->readAll();
    // Don't do anything if this is the same as the last version downloaded
    QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
    if(hash == fwTableHash)
        return;
    fwTableHash = hash;

    // If GPG is available, check the signature on the file before proceeding.
    if(hasGPG){
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
        if(!QFile::copy(":/bin/ckb-next-key.gpg", keyPath)){
            firmware.remove();
            qDebug() << "Failed to write GPG key to temporary location, aborting firmware check";
            return;
        }
        // Check signature
        _gpg->setArguments(QStringList("--no-default-keyring") << "--keyring" << keyPath << "--verify" << fwPath);
        _gpg->start();
        _gpg->waitForFinished();
        // Clean up temp files
        tmp.remove(fwPath);
        tmp.remove(keyPath);
        if(_gpg->error() != QProcess::UnknownError || _gpg->exitCode() != 0){
            qDebug() << "GPG couldn't verify firmware signature:";
            qDebug() << _gpg->readAllStandardOutput();
            qDebug() << _gpg->readAllStandardError();
            return;
        }
        // Signature good, proceed to update database
    }
    fwTable.clear();
    QStringList lines = QString::fromUtf8(data).split("\n");
    bool scan = false;
    foreach(QString line, lines){
        // Collapse whitespace
        line.replace(QRegularExpression("\\s+"), " ").remove(QRegularExpression("^\\s")).remove(QRegularExpression("\\s$"));
        // Skip empty or commented-out lines
        if(line.length() == 0 || line.at(0) == '#')
            continue;
        // Don't read anything until the entries begin and don't read anything after they end
        if(!scan){
            if(line == "!BEGIN FW ENTRIES")
                scan = true;

            continue;
        }
        if(line == "!END FW ENTRIES")
            break;
        QStringList components = line.split(" ");
        int len = components.length();
        if(len != 8){
            qWarning() << "Found" << len << "components in the firmware list.";
            if(len < 8)
                continue;
        }

        FW fw;
        fw.fwVersion = CkbVersionNumber(components[2]);                     // Firmware blob version
        fw.url = QUrl::fromPercentEncoding(components[3].toLatin1());       // URL to zip file
        fw.ckbVersion = CkbVersionNumber(components[4]);                    // Minimum ckb version
        fw.fileName = QUrl::fromPercentEncoding(components[5].toLatin1());  // Name of file inside zip
        fw.hash = QByteArray::fromHex(components[6].toLatin1());            // SHA256 of file inside zip
        fw.productID = components[7].toUShort(nullptr, 16);                 // Hex productID assigned to this FW

        // If the current ckb doesn't support the new version, mark it as such
        if(fw.ckbVersion > KbManager::ckbGuiVersion() || fw.ckbVersion > KbManager::ckbDaemonVersion())
            fw.fwVersion.setCkbTooOld();

        // Update entry
        fwTable[QString::number(fw.productID)] = fw;
    }
    qDebug() << "Downloaded new firmware list." << fwTable.count() << "entries found.";
}

void KbFirmware::downloadFinished(){
    if(!tableDownload)
        return;
    processDownload(tableDownload);
    tableDownload->deleteLater();
    tableDownload = nullptr;
    emit downloaded();
}

CkbVersionNumber KbFirmware::versionForBoard(const ushort productID, bool waitForComplete) {
    if((tableDownload || checkUpdates()) && waitForComplete){
        // If waiting is desired, enter an event loop and stay here until the download is finished
        QEventLoop loop(this);
        connect(this, SIGNAL(downloaded()), &loop, SLOT(quit()));
        loop.exec();
    }
    // Find this board
    const FW& info = fwTable.value(QString::number(productID));
    if (info.hash.isEmpty())
        return CkbVersionNumber();

    return info.fwVersion;
}

QByteArray KbFirmware::dataForBoard(const ushort productID){
    const FW& info = fwTable.value(QString::number(productID));
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

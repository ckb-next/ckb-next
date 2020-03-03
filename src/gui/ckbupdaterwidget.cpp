#include "ckbupdaterwidget.h"
#include "ui_ckbupdaterwidget.h"
#include "kbmanager.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QBitmap>
#include <QCryptographicHash>
#include <QMessageBox>

CkbUpdaterDialog::CkbUpdaterDialog(const QString& ver, const QString& changelog, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CkbUpdaterWidget), _version(ver), _changelog(changelog), _manager(nullptr), _redirectCount(0), _url(QString("https://github.com/ckb-next/ckb-next/")), _quitApp(false){
    ui->setupUi(this);
    QPixmap pixmap = QPixmap(":/img/ckb-next.png").copy(QRect(0, 125, 512, 262)).scaledToWidth(70, Qt::SmoothTransformation);
    ui->iconLabel->setPixmap(pixmap);
    ui->iconLabel->setMask(pixmap.mask());
    ui->versionLabel->setText(QString("ckb-next v") + _version);
#ifdef Q_OS_MAC
    // dmg download
    _url.append("releases/download/v");
    _url.append(_version);
    _url.append("/ckb-next_v");
    _url.append(_version);
#ifdef OS_MAC_LEGACY
    _url.append("-legacy");
#endif
    _url.append(".dmg");
#else
    // Source download
    _url.append("archive/v");
    _url.append(_version);
    _url.append(".tar.gz");
#endif
    ui->changelogLabel->setText("<a href=\"https://github.com/ckb-next/ckb-next/releases/tag/v" + _version + "\" style=\"text-decoration: none;\">v" + _version + " full changelog</a><br /><a href=\"" + _url + "\" style=\"text-decoration: none;\">v" + _version + " manual download</a>");
    ui->changelogTextEdit->setPlainText(changelog);
    ui->progressBar->setValue(0);
    ui->progressBar->hide();
    qDebug() << _url;
}

CkbUpdaterDialog::~CkbUpdaterDialog(){
    if(_manager != nullptr)
        _manager->deleteLater();
    delete ui;
}

void CkbUpdaterDialog::on_remindMeLaterButton_clicked(){
    this->close();
}

void CkbUpdaterDialog::on_updateButton_clicked(){
    ui->remindMeLaterButton->setEnabled(false);
    ui->updateButton->setEnabled(false);
    ui->updateButton->setText(tr("Downloading"));
    if(_manager != nullptr)
        return;

    ui->progressBar->show();
    _manager = new QNetworkAccessManager(this);
    connect(_manager, &QNetworkAccessManager::finished, this, &CkbUpdaterDialog::downloadFinished);
    _reply = _manager->get(QNetworkRequest(QUrl(_url)));
    connect(_reply, &QNetworkReply::downloadProgress, this, &CkbUpdaterDialog::downloadProgress);
}

void CkbUpdaterDialog::downloadProgress(qint64 bytesReceived, qint64 bytesTotal){
    // Redirects are handled elsewhere
    if(_reply->hasRawHeader("Location"))
        return;

    ui->progressBar->setRange(0, bytesTotal);
    ui->progressBar->setValue(bytesReceived);
}

void CkbUpdaterDialog::downloadFinished(QNetworkReply* reply){
    ui->progressBar->setRange(0, 0);
    _redirectCount++;
    if(_redirectCount > 3){
        reply->deleteLater();
        return;
    }

    QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if(redirect.isEmpty()) {
        QByteArray download = reply->readAll(); // We can afford to keep the downloaded file in ram
        QCryptographicHash hash(QCryptographicHash::Sha256);
        hash.addData(download);
        QDir dir;
        dir.mkdir("/tmp/ckb-next/");
#ifdef Q_OS_MAC
        QString suffix(".dmg");
#else
        QString suffix(".tar.gz");
#endif
        QFile outFile(QString("/tmp/ckb-next/v") + _version + suffix);
        if(outFile.open(QFile::WriteOnly)) {
            outFile.write(download);
            outFile.close();
        }
        reply->deleteLater();

        QMessageBox::information(this, tr("Download Complete"),     tr("Update has been downloaded to /tmp/ckb-next/v")
                                                                    + _version + suffix
#ifndef Q_OS_MAC
                                                                    + tr("<br />You will need to manually compile the source code.")
#endif
                                                                    + tr("<br /><br />Optionally, for added security, please check that the following value exists at the bottom of <a href=\"https://github.com/ckb-next/ckb-next/releases/tag/v")
                                                                    + _version
                                                                    + tr("\">this page</a>.<br /><pre>%1</pre><br />Press OK to continue.").arg(QString(hash.result().toHex())));
        QProcess process;
#if defined(Q_OS_MAC)
        // Unmount any old ckb-next volume that might exist
        QStringList detachArgs("detach");
        detachArgs << QString("/Volumes/ckb-next/");
        process.execute("hdiutil", detachArgs);

        // Mount the new image
        QStringList attachArgs("attach");
        attachArgs << QString("/tmp/ckb-next/v") + _version + suffix;
        process.execute("hdiutil", attachArgs);

        // Open the mpkg inside
        process.execute("open", QStringList(QString("/Volumes/ckb-next/ckb-next.mpkg")));
#elif defined(Q_OS_LINUX)
        QStringList tarpit("-xzf");
        tarpit << "/tmp/ckb-next/v" + _version + suffix;
        tarpit << "-C";
        tarpit << "/tmp/ckb-next/";
        process.execute("tar", tarpit);
        process.execute("xdg-open", QStringList(QString("/tmp/ckb-next/ckb-next-") + _version ));
#endif
        // Close the GUI
        qApp->quit();
    } else {
        QString host = redirect.host();
        if(host != "github-production-release-asset-2e65be.s3.amazonaws.com" && host != "codeload.github.com") {
            reply->deleteLater();
            return;
        }
        _reply = _manager->get(QNetworkRequest(redirect));
        connect(_reply, &QNetworkReply::downloadProgress, this, &CkbUpdaterDialog::downloadProgress);
    }
}

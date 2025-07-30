#ifndef KBFIRMWARE_H
#define KBFIRMWARE_H

#include "kb.h"
#include <QObject>
#include <QtNetwork/QtNetwork>

// Class for managing/updating firmware on a device

class KbFirmware : public QObject
{
    Q_OBJECT
public:
    KbFirmware();
    ~KbFirmware();

    // Checks for firmware updates. Maximum of once per hour. Returns true if it actually checked or false if not.
    bool checkUpdates();

    // Whether or not the firmware table has been downloaded at all.
    inline bool hasDownloaded() { return lastFinished != 0; }

    // Latest firmware version for a keyboard model. Will check for updates automatically and return the latest known version.
    CkbVersionNumber versionForBoard(const ushort productID, bool waitForComplete = false);

    // Downloads and extracts the latest firmware for a keyboard. Returns an empty array on failure.
    QByteArray dataForBoard(const ushort productID);

    // Network manager object to use with QtNetwork
    QNetworkAccessManager* 	networkManager;

private:

    // Time initiated
    qint64 lastCheck, lastFinished;
    // Model -> firmware table
    struct FW {
        QString     url, fileName;
        QByteArray  hash;
        CkbVersionNumber fwVersion;
        CkbVersionNumber ckbVersion;
        ushort       productID;
        FW();

        // Explicit definition of copy and assignment operators to silence the compilation warnings.
        FW(const FW& fw) : productID(fw.productID) {};
        FW& operator=(const FW& other) { return *this; }
    };
    QMap<QString, FW>   fwTable;
    // SHA256 of last downloaded table (redundancy check)
    QByteArray          fwTableHash;

    // Current FW table download (null if nothing downloading)
    QNetworkReply* tableDownload;

    // Can GPG be used to verify signatures?
    enum { GPG_UNKNOWN = -1, GPG_NO, GPG_YES } hasGPG :2;
    QProcess*    _gpg;

signals:
    void downloaded();

private slots:
    void processDownload(QNetworkReply* reply);
    void downloadFinished();
};

#endif // KBFIRMWARE_H

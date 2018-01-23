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
    // Checks for firmware updates. Maximum of once per hour. Returns true if it actually checked or false if not.
    static inline bool          checkUpdates()                                                          { return instance._checkUpdates(); }

    // Whether or not the firmware table has been downloaded at all.
    static inline bool          hasDownloaded()                                                         { return instance.lastFinished != 0; }

    // Latest firmware version for a keyboard model. Will check for updates automatically and return the latest known version.
    // Zero if version unknown, -1.0 if ckb needs to be upgraded.
    static inline float         versionForBoard(const short productID, bool waitForComplete = false)  { return instance._latestForBoard(productID, waitForComplete); }

    // Downloads and extracts the latest firmware for a keyboard. Returns an empty array on failure.
    static inline QByteArray    dataForBoard(const short productID)                                   { return instance._fileForBoard(productID); }

    // Network manager object to use with QtNetwork
    QNetworkAccessManager* 	networkManager;

private:
    KbFirmware();
    ~KbFirmware();

    // Time initiated
    quint64 lastCheck, lastFinished;
    // Model -> firmware table
    struct FW {
        QString     url, fileName;
        QByteArray  hash;
        float       fwVersion, ckbVersion;
        short       productID;
        FW();
    };
    QMap<QString, FW>   fwTable;
    // SHA256 of last downloaded table (redundancy check)
    QByteArray          fwTableHash;

    // Current FW table download (null if nothing downloading)
    QNetworkReply* tableDownload;

    // Can GPG be used to verify signatures?
    enum { UNKNOWN = -1, NO, YES } hasGPG :2;

    // Singleton instance
    bool                _checkUpdates();
    float               _latestForBoard(const short productID, bool waitForComplete);
    QByteArray          _fileForBoard(const short productID);
    static KbFirmware   instance;

signals:
    void downloaded();

private slots:
    void processDownload(QNetworkReply* reply);
    void downloadFinished();
};

#endif // KBFIRMWARE_H

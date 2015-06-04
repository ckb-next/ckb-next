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
    static inline float         versionForBoard(const QString& features, bool waitForComplete = false)  { return instance._latestForBoard(features, waitForComplete); }

    // Downloads and extracts the latest firmware for a keyboard. Returns an empty array on failure.
    static inline QByteArray    dataForBoard(const QString& features)                                   { return instance._fileForBoard(features); }

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

        FW();
    };
    QMap<QString, FW>   fwTable;
    // SHA256 of last downloaded table (redundancy check)
    QByteArray          fwTableHash;

    // Network manager for downloading updates
    // It can't be declared as part of the object or it will misbehave. Has to be initialized on use.
    QNetworkAccessManager* networkManager;
    void initManager();
    // Current FW table download (null if nothing downloading)
    QNetworkReply* tableDownload;

    // Can GPG be used to verify signatures?
    enum { UNKNOWN = -1, NO, YES } hasGPG :2;

    // Singleton instance
    bool                _checkUpdates();
    float               _latestForBoard(const QString& features, bool waitForComplete);
    QByteArray          _fileForBoard(const QString& features);
    static KbFirmware   instance;

signals:
    void downloaded();

private slots:
    void processDownload(QNetworkReply* reply);
    void downloadFinished();
};

#endif // KBFIRMWARE_H

#ifndef KBMANAGER_H
#define KBMANAGER_H

#include <QObject>
#include <QTimer>
#include <cmath>
#include <QSet>
#include "kb.h"

// Class for managing keyboard devices. Handles scanning devices from the daemon and creating/destroying Kb objects for each device.

#define DAEMON_UNAVAILABLE_STR  "<unavailable>"

class KbManager : public QObject
{
    Q_OBJECT
public:
    // Call at startup
    static void init(QString guiVersion);
    // Call at shutdown
    static void stop();
    // Singleton instance. Signals are emitted from here. Created when init() is called.
    static inline KbManager* kbManager()    { return _kbManager; }

    // ckb version info
    static inline QString ckbGuiVersion()   { return _guiVersion; }
    static inline QString ckbDaemonVersion(){ return _daemonVersion; }
    // String (e.g. "alpha-v0.0.1" or "0.0.1") -> float. Daemon version returns INFINITY if not connected.
    static float        parseVersionString(QString version);
    static inline float ckbGuiVersionF()    { return parseVersionString(_guiVersion); }
    static inline float ckbDaemonVersionF() { return _daemonVersion == DAEMON_UNAVAILABLE_STR ? INFINITY : parseVersionString(_daemonVersion); }

    // List of all connected devices
    static const QSet<Kb*> devices()        { return _kbManager ? _kbManager->_devices : QSet<Kb*>(); }

    // Event timer for the driver. Created during init(). Starts ticking when fps() is called.
    // Use this for animations or other events which need to run at a high frame rate.
    static inline QTimer* eventTimer()      { return _kbManager ? _kbManager->_eventTimer : 0; }
    // Sets the frame rate for the event timer
    static void fps(int framerate);

    // Timer for scanning the driver/device list. May also be useful for periodic GUI events. Created during init(), always runs at 10FPS.
    static inline QTimer* scanTimer()       { return _kbManager ? _kbManager->_scanTimer : 0; }

signals:
    // A new device was connected.
    void kbConnected(Kb* device);
    // A device was disconnected. The Kb object will be deleted afterward.
    void kbDisconnected(Kb* device);

    // The daemon version info was updated (or the daemon was started/stopped)
    void versionUpdated();

private slots:
    void scanKeyboards();

private:
    static KbManager* _kbManager;
    static QString _guiVersion, _daemonVersion;

    explicit KbManager(QObject* parent = 0);

    QSet<Kb*> _devices;
    QTimer* _eventTimer, *_scanTimer;
};

#endif // KBMANAGER_H

#ifndef KBMANAGER_H
#define KBMANAGER_H

#include <QObject>
#include <QTimer>
#include <cmath>
#include <QSet>
#include "kb.h"

// Class for managing keyboard devices. Handles scanning devices from the daemon and creating/destroying Kb objects for each device.

class KbManager : public QObject
{
    Q_OBJECT
public:
    // Call at startup
    static void init(const QString& guiVersion);
    // Call at shutdown
    static void stop();
    // Singleton instance. Signals are emitted from here. Created when init() is called.
    static inline KbManager* kbManager()    { return _kbManager; }

    // ckb version info
    static inline CkbVersionNumber ckbGuiVersion()    { return _guiVersion; }
    static inline CkbVersionNumber ckbDaemonVersion() { return _daemonVersion; }
    //static inline float ckbDaemonVersionF() { return _daemonVersion == DAEMON_UNAVAILABLE_STR ? INFINITY : parseVersionString(_daemonVersion); }

    // List of all connected devices
    static const QSet<Kb*> devices()        { return _kbManager ? _kbManager->_devices : QSet<Kb*>(); }

    // Event timer for the driver. Created during init(). Starts ticking when fps() is called.
    // Use this for animations or other events which need to run at a high frame rate.
    static inline QTimer* eventTimer()      { return _kbManager ? _kbManager->_eventTimer : nullptr; }
    // Sets the frame rate for the event timer
    static void fps(int framerate);

    // Timer for scanning the driver/device list. May also be useful for periodic GUI events. Created during init(), always runs at 10FPS.
    static inline QTimer* scanTimer()       { return _kbManager ? _kbManager->_scanTimer : nullptr; }
    inline bool getDeviceTimerDimmed() { for(Kb* kb : _devices) if(kb->currentLight()->isTimerDimmed()) { return true; } return false; }

#ifdef USE_XCB_SCREENSAVER
    // Called to restart the idle timer
    static void setIdleTimer(bool enable);
#endif

public slots:
    void brightnessScroll(QPoint delta);
    void scanKeyboards();

signals:
    // A new device was connected.
    void kbConnected(Kb* device);
    // A device was disconnected. The Kb object will be deleted afterward.
    void kbDisconnected(Kb* device);

    // The daemon version info was updated (or the daemon was started/stopped)
    void versionUpdated();

private slots:
#ifdef USE_XCB_SCREENSAVER
    void idleTimerTick();
#endif

private:
    static KbManager* _kbManager;
    static CkbVersionNumber _guiVersion, _daemonVersion;

    explicit KbManager(QObject* parent = nullptr);

    QSet<Kb*> _devices;
    QTimer* _eventTimer, *_scanTimer, *_saveTimer;
#ifdef USE_XCB_SCREENSAVER
    static QTimer* _idleTimer;
#endif
    int getLastUsedDeviceIdleTime();
};

#endif // KBMANAGER_H

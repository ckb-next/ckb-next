#ifndef KB_H
#define KB_H

#include <QtGlobal>
#include <QObject>
#include <QFile>
#include <QThread>
#include <QTimer>

#include "kbprofile.h"
#include <QElapsedTimer>
#include <limits>
#include "batterysystemtrayicon.h"
#include "ckbversionnumber.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringView>
#endif

struct firmware_t {
    CkbVersionNumber app;
    CkbVersionNumber bld;
    CkbVersionNumber radioapp;
    CkbVersionNumber radiobld;
    void parse(const QString& str){
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QStringView view(str);
        view.truncate(view.size() - 1);
        // KeepEmptyParts is the default
        QList<QStringView> split = view.split(QChar('\n'));
#else
        // KeepEmptyParts is the default
        QVector<QStringRef> split = str.leftRef(str.size() - 1).split(QChar('\n'));
#endif
        // Old < 0.5.0 format (NXP and legacy)
        if(split.size() == 1){
            app = CkbVersionNumber(split.at(0).toString());
        } else if (split.size() >= 4) {
            app = CkbVersionNumber(split.at(0).toString());
            bld = CkbVersionNumber(split.at(1).toString());
            radioapp = CkbVersionNumber(split.at(2).toString());
            radiobld = CkbVersionNumber(split.at(3).toString());
        }
    }
};

// Class for managing devices
class Kb : public QThread
{
    Q_OBJECT
public:
    // USB model and serial number
    QString usbModel, usbSerial;
    // Device information
    QStringList features;

    enum pollrate_t {
        POLLRATE_UNKNOWN = -1,
        POLLRATE_8MS,
        POLLRATE_4MS,
        POLLRATE_2MS,
        POLLRATE_1MS,
        POLLRATE_05MS,
        POLLRATE_025MS,
        POLLRATE_01MS,
        POLLRATE_COUNT,
    };
    pollrate_t pollrate, maxpollrate;

    bool monochrome;
    ushort productID;
    bool hwload;
    bool adjrate;
    firmware_t firmware;

    // Keyboard model
    inline KeyMap::Model    model() const                       { return _model; }
    bool                    isKeyboard() const                  { return KeyMap::isKeyboard(_model); }
    bool                    isMouse() const                     { return KeyMap::isMouse(_model); }
    bool                    isMousepad() const                  { return KeyMap::isMousepad(_model); }
    bool                    isHeadsetStand() const              { return KeyMap::isHeadsetStand(_model); }

    // Frame rate (all devices). Also updates the event timer in KbManager.
    static inline int               frameRate()                         { return _frameRate; }
    static void                     frameRate(int newFrameRate);
    // Layout
    inline KeyMap::Layout           layout()                            { return _layout; }
    void                            layout(KeyMap::Layout newLayout, bool stop);
    // Layout string as reported by the daemon
    QString hwlayout;
    // Whether dithering is used (all devices)
    static inline bool              dither()                            { return _dither; }
    static void                     dither(bool newDither);
    // OSX: mouse acceleration toggle (all devices)
    static inline bool              mouseAccel()                        { return _mouseAccel; }
    static void                     mouseAccel(bool newAccel);
    // OSX: scroll speed (-1 = use acceleration)
    static inline int               scrollSpeed()                       { return _scrollSpeed; }
    static void                     scrollSpeed(int newSpeed);

    // Profile saved to hardware
    inline KbProfile*   hwProfile() { return _hwProfile; }
    void                hwProfile(KbProfile* newHwProfile);
    // Required hardware modes
    int hwModeCount;
    const static int HWMODE_MAX = 3;

    // Perform a firmware update
    void fwUpdate(const QString& path);

    // Currently-selected profile
    inline KbProfile* currentProfile() { return _currentProfile; }
    // Profile list
    inline const QList<KbProfile*>& profiles() const                                { return _profiles; }
    void                            profiles(const QList<KbProfile*>& newProfiles)  { _needsSave = true; _profiles = newProfiles; }
    void                            appendProfile(KbProfile* newProfile)            { _needsSave = true; _profiles.append(newProfile); }
    inline int                      indexOf(KbProfile* profile)                     { return _profiles.indexOf(profile); }
    inline KbProfile*               find(const QUuid& id)                           { foreach(KbProfile* profile, _profiles) { if(profile->id().guid == id) return profile; } return nullptr; }

    // Currently-selected mode
    inline KbMode*  currentMode()   { return _currentMode; }
    inline KbLight* currentLight()  { return _currentMode ? _currentMode->light() : nullptr; }
    inline KbBind*  currentBind()   { return _currentMode ? _currentMode->bind() : nullptr; }
    inline KbPerf*  currentPerf()   { return _currentMode ? _currentMode->perf() : nullptr; }

    // Update selection
    void        setCurrentProfile(KbProfile* profile);
    void        setCurrentMode(KbMode* mode);

    // Create a new profile/mode. The newly-created object will NOT be inserted into the current profile/mode list.
    KbProfile*   newProfileWithBlankMode();
    inline KbProfile*   newProfile(KbProfile* other)                  { return new KbProfile(this, getKeyMap(), *other); }
    inline KbProfile*   newProfile(CkbExternalSettings* settings, QString guid) { return new KbProfile(this, getKeyMap(), *settings, guid); }
    inline KbMode*      newMode()                                     { return new KbMode(this, getKeyMap()); }
    inline KbMode*      newMode(KbMode* other)                        { return new KbMode(this, getKeyMap(), *other); }

    // Load/save stored settings
    void load();
    void save();
    bool needsSave() const;

    void hwSave();

    KeyMap::Layout getCurrentLayout();

    // Battery polling timer
    QTimer* batteryTimer;

    // Battery status icon
    BatteryStatusTrayIcon* batteryIcon;

    bool showBatteryIndicator;

    //////////
    /// For usage with macro definions, these two params must only be readable.
    /// So there are no setters.
    /// \brief getMacroNumber returns the macroNumber, which we have saved in the constructor.
    /// For usage with macro definions, this param must only be readable.
    /// So there is no setter.
    /// \return The Number is returned as int.
    ///
    inline int getMacroNumber () { return macroNumber; }

    ///
    /// \brief getMacroPath returns the macroPath (e.g. /dev/input/ckb1/notify),
    /// which we have saved in the constructor.
    /// For usage with macro definions, this param must only be readable.
    /// So there is no setter.
    /// \return The absolute path as String
    ///
    inline QString getMacroPath () { return macroPath; }

    inline ushort getMaxDpi () {return _maxDpi; }
    void setPollRate(const QString& poll);

    // The valid check is done because we don't start the timer on purpose in the constructor.
    // This is done so that when a new device is plugged in while the lights are off, it doesn't suddenly return a really low value and wake everything up.
    inline qint64 getDeviceIdleTime() const { return (deviceIdleTimer.isValid() ? deviceIdleTimer.elapsed() : std::numeric_limits<qint64>::max()); }

    ~Kb();

signals:
    // Layout/model updated
    void infoUpdated();

    // Profile/mode updates
    void profileAdded();
    void profileRenamed();
    void batteryChanged(uint batteryLevel, BatteryStatus batteryStatus);
    void batteryChangedLed(uint batteryLevel, BatteryStatus batteryStatus);

    void profileChanged();
    void profileAboutToChange();
    void modeChanged();

    // FW update status
    void fwUpdateProgress(int current, int total);
    void fwUpdateFinished(bool succeeded);

public slots:
    // Send lighting and settings to the driver
    void frameUpdate();

    // Auto-save every 15s (if settings have changed, and no other writes are in progress)
    void autoSave();

private slots:
    // Processes lines read from the notification node
    void readNotify(const QString& line);

    void deleteHw();
    void deletePrevious();
    void updateBattery();

private:
    // Following methods should only be used by KbManager
    friend class KbManager;

    // Creates a keyboard object with the given device path
    Kb(QObject *parent, const QString& path);

    inline bool isOpen() const { return cmd.isOpen(); }

    //////////
    /// \brief pathVars
    /// devpath is the device root path (e.g. /dev/device/ckb1),
    /// cmdpath leads to the daemon input pipe for daemon commands,
    /// notifyPath is the standard input monitor for general purpose,
    /// macroPath added for a second thread to read macro input.
    /// layoutPath is used to get the physical layout from the daemon
    QString devpath, cmdpath, notifyPath, macroPath;
    // Is this the keyboard at the given serial/path?
    inline bool matches(const QString& path, const QString& serial) { return path.trimmed() == devpath.trimmed() && usbSerial == serial.trimmed().toUpper(); }

private:
    // Following properties shouldn't be used by any other classes
    void updateLayout(bool stop);

    static int _frameRate, _scrollSpeed;
    static bool _dither, _mouseAccel;

    KbProfile*          _currentProfile;
    QList<KbProfile*>   _profiles;
    KbMode*             _currentMode;

    KeyMap::Model   _model;

    uint batteryLevel;
    BatteryStatus batteryStatus;

    // Indicator light state
    bool iState[KbPerf::HW_I_COUNT];

    // CkbSettings path
    QString prefsPath;

    // Current firmware update file
    QString fwUpdPath;

    KbProfile*  _hwProfile;
    // Previously-selected profile and mode
    KbProfile*  prevProfile;
    KbMode*     prevMode;
    // Used to write the profile info when switching
    void writeProfileHeader();

    // cmd and notify file handles
    QFile cmd;

    /// \brief notifyNumber is the trailing number in the device path.
    int notifyNumber;
    // Macro Numer to notify macro definition events
    int macroNumber;

    // Needs to be saved?
    bool _needsSave;

    KeyMap::Layout _layout;

    ushort _maxDpi;

    // Whether or not the hardware profile is being loaded
    // (0 = profile, 1...3 = modes)
    bool hwLoading[HWMODE_MAX + 1];

    // Key map for this keyboard
    KeyMap getKeyMap();

    // Notification reader, launches as a separate thread and reads from file.
    // (QFile doesn't have readyRead() so there's no other way to do this asynchronously)
    void run();

    QElapsedTimer deviceIdleTimer;
};

#endif // KB_H

#ifndef KB_H
#define KB_H

#include <QObject>
#include <QFile>
#include <QThread>
#include "kbprofile.h"

// Class for managing devices

class Kb : public QThread
{
    Q_OBJECT
public:
    // USB model and serial number
    QString usbModel, usbSerial;
    // Device information
    QString features, firmware, pollrate;
    bool monochrome;

    // Keyboard model
    inline KeyMap::Model    model() const                       { return _model; }
    bool                    isKeyboard() const                  { return KeyMap::isKeyboard(_model); }
    bool                    isMouse() const                     { return KeyMap::isMouse(_model); }

    // Frame rate (all devices). Also updates the event timer in KbManager.
    static inline int               frameRate()                         { return _frameRate; }
    static void                     frameRate(int newFrameRate);
    // Layout (all devices)
    static inline KeyMap::Layout    layout()                            { return _layout; }
    static void                     layout(KeyMap::Layout newLayout);
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
    inline KbProfile*               find(const QUuid& id)                           { foreach(KbProfile* profile, _profiles) { if(profile->id().guid == id) return profile; } return 0; }

    // Currently-selected mode
    inline KbMode*  currentMode()   { return _currentMode; }
    inline KbLight* currentLight()  { return !_currentMode ? 0 : _currentMode->light(); }
    inline KbBind*  currentBind()   { return !_currentMode ? 0 : _currentMode->bind(); }
    inline KbPerf*  currentPerf()   { return !_currentMode ? 0 : _currentMode->perf(); }

    // Update selection
    void        setCurrentProfile(KbProfile* profile, bool spontaneous = true);
    void        setCurrentMode(KbProfile* profile, KbMode* mode, bool spontaneous = true);
    inline void setCurrentMode(KbProfile* profile, int index, bool spontaneous = true)      { setCurrentMode(profile, profile->modes()[index], spontaneous); }
    inline void setCurrentMode(KbMode* mode, bool spontaneous = true)                       { setCurrentMode(_currentProfile, mode, spontaneous); }

    // Create a new profile/mode. The newly-created object will NOT be inserted into the current profile/mode list.
    inline KbProfile*   newProfile()                    { return new KbProfile(this, getKeyMap()); }
    inline KbProfile*   newProfile(KbProfile* other)    { return new KbProfile(this, getKeyMap(), *other); }
    inline KbMode*      newMode()                       { return new KbMode(this, getKeyMap()); }
    inline KbMode*      newMode(KbMode* other)          { return new KbMode(this, getKeyMap(), *other); }

    // Load/save stored settings
    void load();
    void save();
    bool needsSave() const;

    void hwSave();

    ~Kb();

signals:
    // Layout/model updated
    void infoUpdated();

    // Profile/mode updates
    void profileAdded();
    void profileRenamed();
    void modeRenamed();

    void profileChanged();
    void modeChanged(bool spontaneous);

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
    void readNotify(QString line);

    void deleteHw();
    void deletePrevious();

private:
    // Following methods should only be used by KbManager
    friend class KbManager;

    // Creates a keyboard object with the given device path
    Kb(QObject *parent, const QString& path);

    inline bool isOpen() const { return cmd.isOpen(); }

    // File paths
    QString devpath, cmdpath, notifyPath;
    // Is this the keyboard at the given serial/path?
    inline bool matches(const QString& path, const QString& serial) { return path.trimmed() == devpath.trimmed() && usbSerial == serial.trimmed().toUpper(); }

private:
    // Following properties shouldn't be used by any other classes
    static KeyMap::Layout _layout;
    void updateLayout();

    static int _frameRate, _scrollSpeed;
    static bool _dither, _mouseAccel;

    KbProfile*          _currentProfile;
    QList<KbProfile*>   _profiles;
    KbMode*             _currentMode;

    KeyMap::Model   _model;

    // Indicator light state
    bool iState[KbPerf::HW_I_COUNT];

    // CkbSettings path
    QString prefsPath;
    quint64 lastAutoSave;

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
    // Notification number
    int notifyNumber;

    // Needs to be saved?
    bool _needsSave;

    // Whether or not the hardware profile is being loaded
    // (0 = profile, 1...3 = modes)
    bool hwLoading[HWMODE_MAX + 1];

    // Key map for this keyboard
    KeyMap getKeyMap();

    // Notification reader, launches as a separate thread and reads from file.
    // (QFile doesn't have readyRead() so there's no other way to do this asynchronously)
    void run();
};

#endif // KB_H

#ifndef KB_H
#define KB_H

#include <QObject>
#include <QFile>
#include "kbprofile.h"

class Kb : public QObject
{
    Q_OBJECT
public:
    // Creates a keyboard object with the given device path
    Kb(QObject *parent, const QString& path);
    ~Kb();

    // File paths
    QString devpath, cmdpath, notifypath;
    // USB model and serial number
    QString usbModel, usbSerial;
    // Device information
    QString features, firmware, pollrate;

    // Keyboard model and layout
    inline KeyMap::Model model() const { return _model; }
    inline KeyMap::Layout layout() const { return _layout; }
    inline void layout(KeyMap::Layout newLayout) { layout(newLayout, true); }

    inline bool isOpen() const { return cmd.isOpen(); }

    // Profile saved to hardware
    KbProfile* hwProfile;
    // Required hardware modes
    int hwModeCount;

    // Currently-selected profile/mode
    KbProfile* currentProfile;
    QList<KbProfile*> profiles;
    inline int indexOf(KbProfile* profile) { return profiles.indexOf(profile); }
    inline KbProfile* find(const QUuid& id) { foreach(KbProfile* profile, profiles) { if(profile->id().guid == id) return profile; } return 0; }
    KbMode* currentMode;
    inline KbLight* currentLight() { return !currentMode ? 0 : currentMode->light(); }

    // Create a new profile/mode. The newly-created object will not be inserted into the current profile/mode list.
    inline KbProfile* newProfile() { return new KbProfile(this, getKeyMap()); }
    inline KbProfile* newProfile(KbProfile* other) { return new KbProfile(this, getKeyMap(), *other); }
    inline KbMode* newMode() { return new KbMode(this, getKeyMap()); }
    inline KbMode* newMode(KbMode* other) { return new KbMode(this, getKeyMap(), *other); }

    void setCurrentProfile(KbProfile* profile, bool spontaneous = true);
    void setCurrentMode(KbProfile* profile, KbMode* mode, bool spontaneous = true);
    inline void setCurrentMode(KbProfile* profile, int index, bool spontaneous = true) { setCurrentMode(profile, profile->modes[index], spontaneous); }
    inline void setCurrentMode(KbMode* mode, bool spontaneous = true) { setCurrentMode(currentProfile, mode, spontaneous); }

    void load(QSettings& settings);
    void save(QSettings& settings);
    void hwSave();

signals:
    void infoUpdated();

    void profileAdded();
    void profileRenamed();
    void modeRenamed();

    void profileChanged();
    void modeChanged(bool spontaneous);

public slots:
    void frameUpdate();

private slots:
    // Processes lines read from the notification node
    void readNotify(QString line);

    void deletePrevious();

private:
    KeyMap::Model _model;
    KeyMap::Layout _layout;
    void layout(KeyMap::Layout newLayout, bool write);

    // Previously-selected profile and mode
    KbProfile* prevProfile;
    KbMode* prevMode;
    // Used to write the profile info when switching
    void writeProfileHeader();

    // cmd and notify file handles
    QFile cmd, notify;
    // Notification number
    int notifyNumber;

    // Whether or not the hardware profile is being loaded
    bool hwLoading;

    // Key map for this keyboard
    KeyMap getKeyMap();

    // Launched in a separate thread, handles actually reading lines from file
    // (QFile doesn't have readyRead() so there's no other way to do this asynchronously)
    void _readNotify();
};

#endif // KB_H

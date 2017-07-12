#ifndef KBPROFILE_H
#define KBPROFILE_H

#include <QList>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QUuid>
#include "kbmode.h"

class KbProfile : public QObject
{
    Q_OBJECT
public:
    // Default constructor
    explicit KbProfile(Kb *parent, const KeyMap& keyMap, const KbProfile& other);
    // Construct empty profile with GUID/modification
    explicit KbProfile(Kb* parent, const KeyMap& keyMap, const QString& guid = "", const QString& modified = "");
    // Load profile from settings
    explicit KbProfile(Kb* parent, const KeyMap& keyMap, CkbSettings& settings, const QString& guid);

    // Save profile to settings
    void save(CkbSettings& settings);
    bool needsSave() const;
    inline void setNeedsSave()          { _needsSave = true; }

    // Profile properties
    inline QString  name() const                    { return _name; }
    inline void     name(const QString& newName)    { _needsSave = true; _name = newName.trimmed(); if(_name == "") _name = "Unnamed"; }
    inline UsbId&   id()                            { return _id; }
    inline void     id(const UsbId& newId)          { _needsSave = true; _id = newId; }

    // Creates a new ID for the profile and all of its modes
    void newId();

    // Profile key map
    inline const KeyMap&    keyMap() const                  { return _keyMap; }
    void                    keyMap(const KeyMap& newKeyMap);

    // Modes in this profile
    typedef QList<KbMode*> ModeList;
    inline const ModeList&  modes() const                           { return _modes; }
    inline void             modes(const QList<KbMode*>& newModes)   { setNeedsUpdate(); _modes = newModes; }
    inline void             append(KbMode* newMode)                 { setNeedsUpdate(); _modes.append(newMode); }
    inline void             insert(int index, KbMode* newMode)      { setNeedsUpdate(); _modes.insert(index, newMode); }
    inline void             removeAll(KbMode* mode)                 { setNeedsUpdate(); _modes.removeAll(mode); }
    inline void             move(int from, int to)                  { setNeedsUpdate(); _modes.move(from, to); }

    inline int              modeCount() const           { return _modes.count(); }
    inline int              indexOf(KbMode* mode) const { return _modes.indexOf(mode); }
    inline KbMode*          find(const QUuid& id)       { foreach(KbMode* mode, _modes) { if(mode->id().guid == id) return mode; } return 0; }

    // Currently-selected mode
    inline KbMode*  currentMode() const                 { return _currentMode; }
    inline void     currentMode(KbMode* newCurrentMode) { _needsSave = true; _currentMode = newCurrentMode; }

private:
    KbMode* _currentMode;
    QString _name;
    UsbId   _id;
    KeyMap  _keyMap;
    ModeList _modes;
    bool _needsSave;

    // Make note that all modes should be re-sent to the driver
    inline void setNeedsUpdate() { setNeedsSave(); foreach(KbMode* mode, _modes){ mode->setNeedsUpdate(); } }
};

#endif // KBPROFILE_H

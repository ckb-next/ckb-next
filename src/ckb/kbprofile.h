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
    explicit KbProfile(Kb* parent, const KeyMap& keyMap, QSettings& settings, const QString& guid);

    // Save profile to settings
    void save(QSettings& settings);

    // Profile properties
    inline QString name() const { return _name; }
    inline void name(const QString& newName) { _name = newName.trimmed(); if(_name == "") _name = "Unnamed"; }
    inline UsbId& id() { return _id; }
    inline void id(const UsbId& newId) { _id = newId; }

    // Creates a new ID for the profile and all of its modes
    void newId();

    // Profile key map
    inline const KeyMap& keyMap() const { return _keyMap; }
    void keyMap(const KeyMap& newKeyMap);

    // Modes in this profile
    QList<KbMode*> modes;
    inline int indexOf(KbMode* mode) { return modes.indexOf(mode); }
    inline KbMode* find(const QUuid& id) { foreach(KbMode* mode, modes) { if(mode->id().guid == id) return mode; } return 0; }
    // Currently-selected mode
    KbMode* currentMode;

private:
    QString _name;
    UsbId _id;
    KeyMap _keyMap;
};

#endif // KBPROFILE_H

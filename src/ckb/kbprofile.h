#ifndef KBPROFILE_H
#define KBPROFILE_H

#include <QList>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QUuid>
#include "kblight.h"

class KbProfile : public QObject
{
    Q_OBJECT
public:
    // Default constructor
    explicit KbProfile(QObject *parent, KbProfile& other);
    // Construct empty profile with GUID/modification
    explicit KbProfile(QObject* parent, const KeyMap& keyMap, const QString& guid, const QString& modified);
    // Load profile from settings
    explicit KbProfile(QObject* parent, const KeyMap& keyMap, QSettings& settings, const QString& guid);

    // Reloads lighting settings (does not reload profile name/ID or mode names/IDs)
    void reloadLight(QSettings& settings);
    // Save profile to settings
    void save(QSettings& settings);

    // Profile properties
    inline QString name() const { return _name; }
    inline void name(const QString& newName) { _name = newName; }
    inline QString guid() const { return id.guid.toString().toUpper(); }
    inline void guid(const QString& newGuid) { id.guid = newGuid; }
    inline QString modified() const { return QString::number(id.modified, 16); }
    inline void modified(const QString& newModified) { id.modified = newModified.toUInt(0, 16); }

    // Profile key map
    inline const KeyMap& keyMap() const { return _keyMap; }
    // Update key map (destroys RGB settings for all modes)
    void keyMap(const KeyMap& newKeyMap);
    // Update layout (RGB settings preserved)
    void keyLayout(KeyMap::Layout layout);

    // Profile modes
    inline int modeCount() const { return modeNames.count(); }
    inline int currentMode() const { return _currentMode; }
    inline void currentMode(int newCurrentMode) { _currentMode = newCurrentMode; }

    void duplicateMode(int mode);
    void deleteMode(int mode);
    void moveMode(int from, int to);

    QString modeName(int mode) const;
    void modeName(int mode, const QString& newName);
    QString modeGuid(int mode);
    void modeGuid(int mode, const QString& newGuid);
    QString modeModified(int mode);
    void modeModified(int mode, const QString& newModified);

    KbLight* modeLight(int mode);

private:
    struct UsbId {
        QUuid guid;
        quint32 modified;

        inline UsbId(const QString& _guid, quint32 _modified) : guid(_guid), modified(_modified) {}
        inline UsbId() : guid(QUuid::createUuid()),modified(0) {}
    };

    QString _name;
    QStringList modeNames;
    QList<KbLight*> modeLights;
    QList<UsbId> modeIds;
    UsbId id;
    KeyMap _keyMap;
    int _currentMode;
};

#endif // KBPROFILE_H

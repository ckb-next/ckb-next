#ifndef KBMODE_H
#define KBMODE_H

#include <QObject>
#include "kblight.h"
#include "kbbind.h"
#include "kbperf.h"

// ID structure for modes/profiles. Stores a GUID indentifying the item as well as a 32-bit number representing its last modification.

struct UsbId {
    QUuid   guid;
    quint32 modified;
    quint32 hwModified; // Last modification value saved to hardware

    inline  UsbId(const QString& _guid, quint32 _modified) : guid(_guid), modified(_modified), hwModified(_modified) {}
    inline  UsbId(const QString& _guid, const QString& _modified) : guid(_guid), modified(_modified.toUInt(0, 16)), hwModified(modified) {}
    inline  UsbId() : guid(QUuid::createUuid()),modified(0) {}

    QString guidString() const                          { return guid.toString().toUpper(); }
    void    guidString(const QString& newGuid)          { guid = newGuid; }
    QString modifiedString() const                      { return QString::number(modified, 16); }
    void    modifiedString(const QString& newModified)  { modified = newModified.toUInt(0, 16); }
    QString hwModifiedString() const                    { return QString::number(hwModified, 16); }
    void    hwModifiedString(const QString& newModified){ hwModified = newModified.toUInt(0, 16); }

    // Generate a new random ID
    void newGuid()                                      { guid = QUuid::createUuid(); }
    void newModified()                                  { quint32 newMod; do { newMod = qrand(); } while(newMod == modified); modified = newMod; }
};

class Kb;

// Profile mode

class KbMode : public QObject
{
    Q_OBJECT
public:
    // New mode with key map, and optionally ID
    KbMode(Kb* parent, const KeyMap& keyMap, const QString& guid = "", const QString& modified = "");
    // Mode from settings
    KbMode(Kb* parent, const KeyMap& keyMap, CkbSettings& settings);
    // Mode by copy
    KbMode(Kb* parent, const KeyMap& keyMap, const KbMode& other);

    // Mode properties
    inline const QString&   name() const                    { return _name; }
    inline void             name(const QString& newName)    { _needsSave = true; _name = newName.trimmed(); if(_name == "") _name = "Unnamed"; }
    inline UsbId&           id()                            { return _id; }
    inline void             id(const UsbId& newId)          { _needsSave = true; _id = newId; }
    void newId();

    // Device key map
    void keyMap(const KeyMap& keyMap);

    // Lighting and binding setup
    inline KbLight* light() { return _light; }
    inline KbBind*  bind() { return _bind; }
    inline KbPerf*  perf() { return _perf; }

    // Save settings
    void save(CkbSettings& settings);
    bool needsSave() const;
    inline void setNeedsSave()          { _needsSave = true; }
    inline void setNeedsUpdate()        { _bind->setNeedsUpdate(); _perf->setNeedsUpdate(); }

signals:
    void updated();

private:
    QString _name;
    UsbId _id;

    KbLight* _light;
    KbBind* _bind;
    KbPerf* _perf;

    bool _needsSave;

private slots:
    void doUpdate();
};

#endif // KBMODE_H

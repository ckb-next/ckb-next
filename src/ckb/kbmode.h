#ifndef KBMODE_H
#define KBMODE_H

#include <QObject>
#include "kblight.h"
#include "kbbind.h"

struct UsbId {
    QUuid guid;
    quint32 modified;

    inline UsbId(const QString& _guid, quint32 _modified) : guid(_guid), modified(_modified) {}
    inline UsbId(const QString& _guid, const QString& _modified) : guid(_guid), modified(_modified.toUInt(0, 16)) {}
    inline UsbId() : guid(QUuid::createUuid()),modified(0) {}

    QString guidString() const { return guid.toString().toUpper(); }
    void guidString(const QString& newGuid) { guid = newGuid; }
    QString modifiedString() const { return QString::number(modified, 16); }
    void modifiedString(const QString& newModified) { modified = newModified.toUInt(0, 16); }
};

class Kb;

class KbMode : public QObject
{
    Q_OBJECT
public:
    // New mode with key map, and optionally ID
    KbMode(Kb* parent, const KeyMap& keyMap, const QString& guid = "", const QString& modified = "");
    // Mode from settings
    KbMode(Kb* parent, const KeyMap& keyMap, QSettings& settings);
    // Mode by copy
    KbMode(Kb* parent, const KeyMap& keyMap, const KbMode& other);

    // Mode properties
    inline const QString& name() const { return _name; }
    inline void name(const QString& newName) { _name = newName.trimmed(); if(_name == "") _name = "Unnamed"; }
    inline UsbId& id() { return _id; }
    inline void id(const UsbId& newId) { _id = newId; }
    void newId();

    // Device key map
    void keyMap(const KeyMap& keyMap);

    // Lighting and binding setup
    inline KbLight* light() { return _light; }
    inline KbBind* bind() { return _bind; }

    // Save settings
    void save(QSettings& settings);

signals:
    void updated();

private:
    QString _name;
    UsbId _id;

    KbLight* _light;
    KbBind* _bind;

private slots:
    void doUpdate();
};

#endif // KBMODE_H

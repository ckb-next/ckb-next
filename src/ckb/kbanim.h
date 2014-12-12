#ifndef KBANIM_H
#define KBANIM_H

#include <QObject>
#include <QSettings>
#include "animscript.h"
#include "keymap.h"

class KbAnim : public QObject
{
    Q_OBJECT
    Q_ENUMS(Mode)
public:
    enum Mode {
        Normal,
        Add,
        Subtract,
        Multiply,
        Divide
    };

    KbAnim(QObject* parent, const KeyMap& map, const QUuid id, QSettings& settings);
    KbAnim(QObject* parent, const KeyMap& map, const QStringList& keys, const AnimScript* script);
    void save(QSettings& settings);

    inline const KeyMap& map() { return _map; }
    void map(const KeyMap& newMap);
    inline const QStringList& keys() { return _keys; }
    void keys(const QStringList& newKeys);

    void trigger();
    void blend(KeyMap& colorMap);
    void stop();

    inline const QUuid& guid() { return _guid; }
    inline const QString& name() { return _name; }
    inline void name(const QString& newName) { _name = newName; }
    inline float opacity() { return _opacity; }
    inline void opacity(float newOpacity) { _opacity = newOpacity; }
    inline Mode mode() { return _mode; }
    inline void mode(Mode newMode) { _mode = newMode; }

    const AnimScript* script() { return _script; }
    const QString& scriptName() { return _scriptName; }

private:
    AnimScript* _script;
    QUuid _scriptGuid;
    QString _scriptName;

    KeyMap _map;
    QStringList _keys;

    QUuid _guid;
    QString _name;
    float _opacity;
    Mode _mode;
};

#endif // KBANIM_H

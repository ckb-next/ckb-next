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
    // Blending modes
    enum Mode {
        Normal,
        Add,
        Subtract,
        Multiply,
        Divide
    };

    // Create a new animation
    KbAnim(QObject* parent, const KeyMap& map, const QStringList& keys, const AnimScript* script);
    // Load an animation from settings
    KbAnim(QObject* parent, const KeyMap& map, const QUuid id, QSettings& settings);
    // Save an animation to settings
    void save(QSettings& settings);

    // Key map
    inline const KeyMap& map() { return _map; }
    void map(const KeyMap& newMap);
    // Keys to animate
    inline const QStringList& keys() { return _keys; }
    void keys(const QStringList& newKeys);

    // Parameters (name -> value)
    QMap<QString, QVariant> parameters;
    // Re-initialize animation after changing parameters
    void reInit();

    // Begins or re-triggers the animation
    void trigger(quint64 timestamp);
    // Triggers a keypress in the animation
    void keypress(const QString& key, bool pressed, quint64 timestamp);
    // Stops the animation
    void stop();

    // Blends the animation into a color map, taking opacity and mode into account
    void blend(QHash<QString, QRgb>& animMap, quint64 timestamp);

    // Animation properties
    inline const QUuid& guid() { return _guid; }
    inline const QString& name() { return _name; }
    inline void name(const QString& newName) { _name = newName; }
    inline float opacity() { return _opacity; }
    inline void opacity(float newOpacity) { _opacity = newOpacity; }
    inline Mode mode() { return _mode; }
    inline void mode(Mode newMode) { _mode = newMode; }

    // Animation script properties
    const AnimScript* script() { return _script; }
    const QString& scriptName() { return _scriptName; }

private:
    AnimScript* _script;
    QUuid _scriptGuid;
    QString _scriptName;

    KeyMap _map;
    QStringList _keys;
    QString repeatKey;
    quint64 repeatTime, kpRepeatTime, stopTime, kpStopTime;
    int repeatMsec, kpRepeatMsec;

    QUuid _guid;
    QString _name;
    float _opacity;
    Mode _mode;
};

#endif // KBANIM_H

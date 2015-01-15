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

    // Gets a parameter value
    inline bool hasParameter(const QString& name) const { return _parameters.contains(name); }
    inline QVariant parameter(const QString& name) const { return _parameters.value(name); }
    // Sets a parameter value. Parameter changes are not permanent until commited.
    void parameter(const QString& name, const QVariant& value);
    // Commits unsaved parameters
    void commitParams();
    // Discards unsaved parameters
    void resetParams();
    // Re-initialize the animation. Useful for doing a hard restart.
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
    // Script (null if not loaded)
    AnimScript* _script;
    // GUID and name (duplicated here in case the load fails)
    QUuid _scriptGuid;
    QString _scriptName;

    KeyMap _map;
    QStringList _keys;
    // Committed parameters
    QMap<QString, QVariant> _parameters;
    // Uncommitted parameters
    QMap<QString, QVariant> _tempParameters;

    // Effective parameters (including unsaved)
    QMap<QString, QVariant> effectiveParams();
    // Updates parameters to animation if live params are enabled
    void updateParams();

    // Repeat/stop info (set from parameters)
    QString repeatKey;
    quint64 repeatTime, kpRepeatTime, stopTime, kpStopTime;
    int repeatMsec, kpRepeatMsec;

    QUuid _guid;
    QString _name;
    float _opacity;
    Mode _mode;
};

#endif // KBANIM_H

#ifndef KBANIM_H
#define KBANIM_H

#include <QObject>
#include "ckbsettings.h"
#include "animscript.h"
#include "keymap.h"

// Animation instance for a lighting mode.

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

    // Load an animation from settings
    KbAnim(QObject* parent, const KeyMap& map, const QUuid id, CkbSettings& settings);
    // Save an animation to settings
    void save(CkbSettings& settings);
    inline bool needsSave() const { return _needsSave; }

    // Create a new animation
    KbAnim(QObject* parent, const KeyMap& map, const QString& name, const QStringList& keys, const AnimScript* script);
    // Copy an existing animation
    KbAnim(QObject *parent, const KeyMap& map, const KbAnim& other);

    // Key map
    inline const KeyMap&        map()                               { return _map; }
    void                        map(const KeyMap& newMap);
    // Keys to animate
    inline const QStringList&   keys()                              { return _keys; }
    void                        keys(const QStringList& newKeys);

    // Gets a parameter value
    inline bool     hasParameter(const QString& name) const { return _parameters.contains(name); }
    inline QVariant parameter(const QString& name) const    { return _parameters.value(name); }
    // Sets a parameter value. Parameter changes are not permanent until commited.
    void parameter(const QString& name, const QVariant& value);
    // Commits unsaved parameters
    void commitParams();
    // Discards unsaved parameters
    void resetParams();
    // Re-initialize the animation. Useful for doing a hard restart.
    void reInit();

    // Begins or re-triggers the animation
    // Normally this will only start the animation if specified in the parameters. To ignore this and start it no matter what, use ignoreParameter.
    void trigger(quint64 timestamp, bool ignoreParameter = false);
    // Triggers a keypress in the animation
    void keypress(const QString& key, bool pressed, quint64 timestamp);
    // Stops the animation
    void stop();
    // Whether or not the animation is currently active
    bool isActive() const       { return _isActive || _isActiveKp; }
    // Whether or not the animation script is responding
    bool isRunning() const;

    // Blends the animation into a color map, taking opacity and mode into account
    void blend(ColorMap &animMap, quint64 timestamp);

    // Animation properties
    inline const QUuid&     guid() const                    { return _guid; }
    inline void             newId()                         { _needsSave = true; _guid = QUuid::createUuid(); }
    inline const QString&   name() const                    { return _name; }
    inline void             name(const QString& newName)    { _needsSave = true; _name = newName; }
    inline float            opacity() const                 { return _opacity; }
    inline void             opacity(float newOpacity)       { _needsSave = true; _opacity = newOpacity; }
    inline Mode             mode() const                    { return _mode; }
    inline void             mode(Mode newMode)              { _needsSave = true; _mode = newMode; }

    // Animation script properties
    const AnimScript*   script() const      { return _script; }
    const QString&      scriptName() const  { return _scriptName; }

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
    int     repeatMsec, kpRepeatMsec;
    // Catch up to the current timestamp, performing repeats/stops as necessary
    void catchUp(quint64 timestamp);

    QUuid _guid;
    QString _name;
    float _opacity;
    Mode _mode;
    bool _isActive, _isActiveKp;
    bool _needsSave;
};

#endif // KBANIM_H

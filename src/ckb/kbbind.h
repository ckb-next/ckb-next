#ifndef KBBIND_H
#define KBBIND_H

#include <QFile>
#include <QHash>
#include <QObject>
#include <QSettings>
#include "keymap.h"

class Kb;
class KbMode;

class KbBind : public QObject
{
    Q_OBJECT
public:
    // New binding setup
    explicit KbBind(KbMode* modeParent, Kb* devParent, const KeyMap& keyMap);
    // Copy a binding setup
    KbBind(KbMode* modeParent, Kb* devParent, const KeyMap& keyMap, const KbBind& other);

    // Load and save from stored settings
    void load(QSettings& settings);
    void save(QSettings& settings);

    // Key map
    inline const KeyMap& map() { return _map; }
    void map(const KeyMap& map);

    // Action for a given key. Blank means no action (unbound). Special actions start with '$'.
    QString action(const QString& key);
    // Default action for a key.
    static QString defaultAction(const QString& key);

    // Key identifiers
    static inline bool isUnbound(const QString& action) { return action.isEmpty(); }
    static inline bool isNormal(const QString& action) { return !action.isEmpty() && !isSpecial(action); }
    static inline bool isMedia(const QString& action) { return action == "mute" || action == "volup" || action == "voldn" || action == "stop" || action == "prev" || action == "play" || action == "next"; }
    static inline bool isSpecial(const QString& action) { return !action.isEmpty() && action[0] == '$'; }
    // Splits a special action into action and parameter.
    static inline QString specialInfo(const QString& action, int& parameter) { QStringList list = action.split(":"); if(list.length() < 2){ parameter = INT_MIN; return ""; } parameter = list[1].toInt(); return list[0].replace("$", ""); }

    // Friendly name for the action associated with a key
    QString friendlyName(const QString& key);

    // Resets a key to the default action.
    void resetAction(const QString &key);
    inline void resetAction(const QStringList& keys) { foreach(const QString& key, keys) resetAction(key); }
    // Unbinds a key.
    void noAction(const QString& key);
    inline void noAction(const QStringList& keys) { foreach(const QString& key, keys) noAction(key); }
    // Sets a key to a standard keypress. (key is the key to set, actionKey is what to set it to)
    void keyAction(const QString& key, const QString& actionKey);
    inline void keyAction(const QStringList& keys, const QString& actionKey) { foreach(const QString& key, keys) keyAction(key, actionKey); }
    // Sets a key to a mode-switch action.
    // 0 for first mode, 1 for second, etc. Constants below for movement options
    const static int MODE_PREV = -2, MODE_NEXT = -1;
    const static int MODE_PREV_WRAP = -4, MODE_NEXT_WRAP = -3;
    void modeAction(const QString& key, int mode);
    inline void modeAction(const QStringList& keys, int mode) { foreach(const QString& key, keys) modeAction(key, mode); }
    // Sets a key to control brightness.
    const static int LIGHT_UP = 0, LIGHT_DOWN = 1;
    const static int LIGHT_UP_WRAP = 2, LIGHT_DOWN_WRAP = 3;
    void lightAction(const QString& key, int type = LIGHT_UP_WRAP);
    inline void lightAction(const QStringList& keys, int type = LIGHT_UP_WRAP) { foreach(const QString& key, keys) lightAction(key, type); }
    // Sets a key to control win lock.
    const static int LOCK_TOGGLE = 0, LOCK_ON = 1, LOCK_OFF = 2;
    void lockAction(const QString& key, int type = LOCK_TOGGLE);
    inline void lockAction(const QStringList& keys, int type = LOCK_TOGGLE) { foreach(const QString& key, keys) lockAction(key, type); }

    // Current win lock state
    inline bool winLock() { return _winLock; }
    void winLock(bool newWinLock) { _winLock = newWinLock; _needsUpdate = true; }

    // Updates bindings to the driver. Write "mode %d" first.
    // By default, nothing will be written unless bindings have changed. Use force = true to overwrite.
    void update(QFile& cmd, bool force = false);

public slots:
    // Callback for a keypress event.
    void keyEvent(const QString& key, bool down);

signals:
    void didLoad();
    void updated();


private:
    Kb* _devParent;
    inline Kb* devParent() { return _devParent; }
    inline KbMode* modeParent() { return (KbMode*)parent(); }

    KeyMap _map;
    // Key -> action map (no entry = default action)
    QHash<QString, QString> _bind;

    bool _winLock;
    bool _needsUpdate;
    bool _osxSwap;
};

#endif // KBBIND_H

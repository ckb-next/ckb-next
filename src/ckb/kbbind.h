#ifndef KBBIND_H
#define KBBIND_H

#include <QFile>
#include <QHash>
#include <QObject>
#include <QProcess>
#include "ckbsettings.h"
#include "keymap.h"
#include "keyaction.h"

class Kb;
class KbMode;
class KbPerf;
class KbLight;

// Binding setup for a device mode
// See also KeyAction, a class for managing binding actions

class KbBind : public QObject
{
    Q_OBJECT
public:
    // New binding setup
    explicit KbBind(KbMode* modeParent, Kb* devParent, const KeyMap& keyMap);
    // Copy a binding setup
    KbBind(KbMode* modeParent, Kb* devParent, const KeyMap& keyMap, const KbBind& other);

    // Load and save from stored settings
    void        load(CkbSettings& settings);
    void        save(CkbSettings& settings);
    inline bool needsSave() const                   { return _needsSave; }

    // Key map
    inline const KeyMap&    map()                   { return _map; }
    void                    map(const KeyMap& map);
    inline bool             isISO()         const   { return _map.isISO(); }
    inline bool             isKeyboard()    const   { return _map.isKeyboard(); }
    inline bool             isMouse()       const   { return _map.isMouse(); }

    // Related objects
    KbPerf*     perf();
    KbLight*    light();

    // Global key remap (changes modifiers per Settings widget)
    // Use only when iterating the map manually; all KbBind functions already take this into account
    static QString  globalRemap(const QString& key);
    static void     setGlobalRemap(const QHash<QString, QString> keyToActual);
    static void     loadGlobalRemap();
    static void     saveGlobalRemap();

    // Action for a given key. Use KeyAction(action) to get info about it.
    QString         action(const QString& key);
    // Default action for a key.
    static QString  defaultAction(const QString& key);

    // Friendly name for a key, taking into account the global key remap
    QString friendlyName(const QString& key);
    // Friendly name for the action associated with a key
    QString friendlyActionName(const QString& key);

    // Changes a key's action
    void        setAction(const QString& key, const QString& action);
    inline void setAction(const QStringList& keys, const QString& action)   { foreach(const QString& key, keys) setAction(key, action); }
    // Resets a key to the default action.
    void        resetAction(const QString &key);
    inline void resetAction(const QStringList& keys)                        { foreach(const QString& key, keys) resetAction(key); }
    // Unbinds a key.
    void        noAction(const QString& key);
    inline void noAction(const QStringList& keys)                           { foreach(const QString& key, keys) noAction(key); }

    // Current win lock state
    inline bool winLock()                   { return _winLock; }
    void        winLock(bool newWinLock)    { _winLock = newWinLock; _needsUpdate = true; }

    // Updates bindings to the driver. Write "mode %d" first.
    // By default, nothing will be written unless bindings have changed. Use force = true or call setNeedsUpdate() to override.
    void        update(QFile& cmd, bool force = false);
    inline void setNeedsUpdate()                        { _needsUpdate = true; }

public slots:
    // Callback for a keypress event.
    void keyEvent(const QString& key, bool down);

signals:
    void didLoad();
    void layoutChanged();
    void updated();


private:
    Kb* _devParent;
    inline Kb*      devParent()     const   { return _devParent; }
    inline KbMode*  modeParent()    const   { return (KbMode*)parent(); }

    inline KeyAction* bindAction(const QString& key)    { if(!_bind.contains(key)) return _bind[key] = new KeyAction(KeyAction::defaultAction(key), this); return _bind[key]; }

    static QHash<QString, QString>  _globalRemap;
    static quint64                  globalRemapTime;
    quint64                         lastGlobalRemapTime;

    KeyMap _map;
    // Key -> action map (no entry = default action)
    QHash<QString, KeyAction*> _bind;

    bool _winLock;
    bool _needsUpdate;
    bool _needsSave;

    friend class KeyAction;
};

#endif // KBBIND_H

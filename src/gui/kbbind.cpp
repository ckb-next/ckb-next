#include "monotonicclock.h"
#include <QUrl>
#include "ckbsettings.h"
#include "kbbind.h"
#include "kbmode.h"
#include "kb.h"
#include "qdebug.h"
#include <typeinfo>

QHash<QString, QString> KbBind::_globalRemap;
qint64 KbBind::globalRemapTime = 0;

KbBind::KbBind(KbMode* modeParent, Kb* parentBoard, const KeyMap& keyMap) :
    QObject(modeParent), _devParent(parentBoard), lastGlobalRemapTime(globalRemapTime), _map(keyMap),
    _winLock(false), _catsLockMode(false), _catsLockActive(false), _needsUpdate(true), _needsSave(true) {
}

///
/// \brief KbBind::KbBind   // copy all existing Key bindings
/// \param modeParent
/// \param parentBoard
/// \param keyMap
/// \param other
///
KbBind::KbBind(KbMode* modeParent, Kb* parentBoard, const KeyMap& keyMap, const KbBind& other) :
    QObject(modeParent), _devParent(parentBoard), lastGlobalRemapTime(globalRemapTime), _bind(other._bind),
    _winLock(false), _catsLockMode(false), _catsLockActive(false), _needsUpdate(true), _needsSave(true) {
    map(keyMap);

    /// Create a new Hash table and copy all entries
    QHash<QString, KeyAction*> newBind;
    foreach(QString key, _bind.keys()) {
        KeyAction* act = _bind.value(key);
        if(act) {
            newBind[key] = new KeyAction(act->value(), this);
        }
    }

    /// clear the destination list (there are the original KeyActions as references, so do not delete them)
    _bind.clear();
    foreach(QString key, newBind.keys()) {
        KeyAction* act = newBind.value(key);
        if(act) {
            /// and move the KeyActions we just created
            _bind[key] = new KeyAction(act->value(), this);
        }
    }
    newBind.clear();      // here we *must not* delete the KeyActions, because they are referenced by _bind now
}

KbPerf* KbBind::perf(){
    return modeParent()->perf();
}

KbLight* KbBind::light(){
    return modeParent()->light();
}

void KbBind::load(CkbSettingsBase& settings){
    _needsSave = false;
    SGroup group(settings, "Binding");
    KeyMap currentMap = _map;
    _map = KeyMap::fromName(settings.value("KeyMap").toString());
    // Load key settings
    bool useReal = settings.value("UseRealNames").toBool();
    _bind.clear();
    {
        SGroup subgroup(settings, "Keys");
        foreach(QString key, settings.childKeys()){
            QString name = key.toLower();
            if(!useReal)
                name = _map.fromStorage(name);
            QString bind = settings.value(key).toString();
            _bind[name] = new KeyAction(bind, this);
        }
    }
    _winLock = settings.value("WinLock").toBool();
    _catsLockMode = settings.value("CatsLockMode").toBool();
    // _catsLockActive intentionally NOT loaded — always starts unlocked on profile load.
    emit didLoad();
    map(currentMap);
}

void KbBind::save(CkbSettingsBase& settings){
    if(typeid(settings) == typeid(CkbSettings))
        _needsSave = false;
    SGroup group(settings, "Binding");
    settings.setValue("KeyMap", _map.name());
    // Save key settings
    settings.setValue("UseRealNames", true);
    {
        SGroup subgroup(settings, "Keys");
        foreach(QString key, _bind.keys()){
            KeyAction* act = _bind.value(key);
            if(act && act->value() != KeyAction::defaultAction(key, devParent()->model()))
                settings.setValue(key, act->value());
        }
    }
    settings.setValue("WinLock", _winLock);
    settings.setValue("CatsLockMode", _catsLockMode);
}

QString KbBind::globalRemap(const QString& key){
    if(!_globalRemap.contains(key))
        return key;
    return _globalRemap.value(key);
}

void KbBind::setGlobalRemap(const QHash<QString, QString>& keyToActual){
    _globalRemap.clear();
    // Ignore any keys with the standard binding
    QHashIterator<QString, QString> i(keyToActual);
    while(i.hasNext()){
        i.next();
        if(i.key() != i.value())
            _globalRemap[i.key()] = i.value();
    }
    globalRemapTime = MonotonicClock::msecs();
}

void KbBind::loadGlobalRemap(){
    _globalRemap.clear();
    CkbSettings settings("Program/GlobalRemap");
    foreach(const QString& key, settings.childKeys())
        _globalRemap[key] = settings.value(key).toString();
    globalRemapTime = MonotonicClock::msecs();
}

void KbBind::saveGlobalRemap(){
    CkbSettings settings("Program/GlobalRemap", true);
    QHashIterator<QString, QString> i(_globalRemap);
    while(i.hasNext()){
        i.next();
        settings.setValue(i.key(), i.value());
    }
}

void KbBind::map(const KeyMap& map){
    _map = map;
    _needsUpdate = true;
    _needsSave = true;
    emit layoutChanged();
}

KeyAction* KbBind::bindAction(const QString& key) {
  if(!_bind.contains(key)) return _bind[key] = new KeyAction(KeyAction::defaultAction(key, devParent()->model()), this);
  return _bind[key];
}

QString KbBind::action(const QString& key){
    QString rKey = globalRemap(key);
    return bindAction(rKey)->value();
}

QString KbBind::defaultAction(const QString& key, KeyMap::Model model){
    QString rKey = globalRemap(key);
    return KeyAction::defaultAction(rKey, model);
}

QString KbBind::defaultAction(const QString& key){
  return defaultAction(key, devParent()->model());
}

QString KbBind::friendlyName(const QString& key){
    const Key& pos = _map.key(globalRemap(key));
    if(!pos)
        return "(Unknown)";
    return pos.friendlyName();
}

QString KbBind::friendlyActionName(const QString& key){
    QString act = action(key);
    return KeyAction(act).friendlyName(_map);
}

void KbBind::resetAction(const QString &key){
    QString rKey = globalRemap(key);
    // Clean up existing action (if any)
    KeyAction* action = _bind.value(rKey);
    delete action;
    _bind.remove(rKey);
    _needsUpdate = true;
    _needsSave = true;
}

void KbBind::noAction(const QString& key){
    resetAction(key);
    QString rKey = globalRemap(key);
    if(!_map.key(rKey))
        return;
    _bind[rKey] = new KeyAction(this);
}

void KbBind::setAction(const QString& key, const QString& action){
    resetAction(key);
    QString rKey = globalRemap(key);
    if(!_map.key(rKey))
        return;
    _bind[rKey] = new KeyAction(action, this);
}

void KbBind::update(QFile& cmd, int notify, bool force){
    if(!force && !_needsUpdate && lastGlobalRemapTime == globalRemapTime)
        return;
    lastGlobalRemapTime = globalRemapTime;
    emit updated();
    _needsUpdate = false;
    // Reset all keys and enable notifications for all
    cmd.write(QString("\n@%1 rebind all notify all").arg(notify).toLatin1());
    // Make sure modifier keys are included as they may be remapped globally
    QHash<QString, KeyAction*> bind(_bind);
    if(!_bind.contains("caps")) bind["caps"] = nullptr;
    if(!_bind.contains("lshift")) bind["lshift"] = nullptr;
    if(!_bind.contains("rshift")) bind["rshift"] = nullptr;
    if(!_bind.contains("lctrl")) bind["lctrl"] = nullptr;
    if(!_bind.contains("rctrl")) bind["rctrl"] = nullptr;
    if(!_bind.contains("lwin")) bind["lwin"] = nullptr;
    if(!_bind.contains("rwin")) bind["rwin"] = nullptr;
    if(!_bind.contains("lalt")) bind["lalt"] = nullptr;
    if(!_bind.contains("ralt")) bind["ralt"] = nullptr;
    if(!_bind.contains("fn")) bind["fn"] = nullptr;
    QHashIterator<QString, KeyAction*> i(bind);

    // Initialize String buffer for macro Key definitions (G-keys)
    // "macro clear" is neccessary, if an older definition is unbound.
    QString macros = "\nmacro clear\n";

    // Write out rebound keys and collect infos for macro definitions
    while(i.hasNext()){
        i.next();
        QString key = i.key();
        KeyAction* act = i.value();
        if(_globalRemap.contains(key))
            act = bindAction(_globalRemap.value(key));
        if(!act)
            continue;
        QString value = act->driverName();
        QByteArray keyLatin1 = key.toLatin1();
        if(value.isEmpty()){
            // If the key is unbound or is a special action, unbind it
            cmd.write(" unbind ");
            cmd.write(keyLatin1);
            // if a macro definiton for the key is given,
            // add the converted string to key-buffer "macro"
            if (act->isMacro()) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                QList<QStringView> macroContent = QStringView(act->value()).split(':');
#else // QT_VERSION < 6.0.0
                QVector<QStringRef> macroContent = act->value().splitRef(':');
#endif
                // Fields used: 1 - daemon string, 5 - repetition delay
                if (macroContent.at(1).length() > 0)
                    macros.append("macro " + keyLatin1 + ":" + macroContent.at(1).toLatin1());
                if (macroContent.length() > 5 && macroContent.at(5).length() > 0)
                    macros.append(":" + macroContent.at(5).toLatin1());
            }
        } else {
            // Otherwise, write the binding
            cmd.write(" bind ");
            cmd.write(keyLatin1);
            cmd.write(":");
            cmd.write(value.toLatin1());
        }
    }
    // If win lock is enabled, unbind windows keys
    if(_winLock)
        cmd.write(" unbind lwin rwin");

    // Cats Lock: when active, silence every key except the WinLock key itself.
    // Written after the normal binding loop so these override any custom binds.
    if(_catsLockMode && _catsLockActive) {
        static const char* const silenced[] = {
            // Function row
            "esc","f1","f2","f3","f4","f5","f6","f7","f8","f9","f10","f11","f12",
            // System cluster
            "prtscn","scroll","pause",
            // Navigation cluster
            "ins","home","pgup","del","end","pgdn",
            "up","down","left","right",
            // Number row
            "grave","1","2","3","4","5","6","7","8","9","0","minus","equal","bspace",
            // QWERTY rows
            "tab","q","w","e","r","t","y","u","i","o","p","lbrace","rbrace","bslash",
            "caps","a","s","d","f","g","h","j","k","l","semi","quote","enter",
            "lshift","z","x","c","v","b","n","m","comma","dot","slash","rshift",
            // Bottom row
            "lctrl","lwin","lalt","space","ralt","rwin","rmenu","rctrl",
            // Numpad
            "numlock","kp_slash","kp_asterisk","kp_minus",
            "kp7","kp8","kp9","kp_plus",
            "kp4","kp5","kp6",
            "kp1","kp2","kp3",
            "kp0","kp_dot","kp_enter",
            // Media / G-keys (present on K95 and similar)
            "mute","volup","voldn","stop","prev","play","next",
            "g1","g2","g3","g4","g5","g6","g7","g8","g9","g10",
            "g11","g12","g13","g14","g15","g16","g17","g18",
            // ISO extra key, light key, mode keys
            "bslash_iso","light","m1","m2","m3","mr",
            nullptr
        };
        for(const char* const* k = silenced; *k; ++k) {
            cmd.write(" unbind ");
            cmd.write(*k);
        }
    }

    // At last, send Macro definitions if available.
    // If no definitions are made, clear macro will be sent only to reset all macros,
    cmd.write(macros.toLatin1());
    cmd.write("\n");
    lastCmd = &cmd;
}

////////
/// \brief KbBind::getMacroNumber
/// \return number of notification channel. Use it in combination with notifyon/off-Statement
///
int KbBind::getMacroNumber() {
    return devParent()->getMacroNumber();
}

////////
/// \brief KbBind::getMacroPath
/// \return Filepath of macro notification pipe. If not set, returns initial value ""
///
QString KbBind::getMacroPath() {
    return devParent()->getMacroPath();
}

////////
/// \brief handleNotificationChannel sends commands to ckb-daemon for (de-) activating the notify channel.
/// Send a notify cmd to the keyboard to set or clear notification for reading macro definition.
/// The file handle for the cmd pipe is stored in lastCmd.
/// \param start If true, notification channel is opened for all keys, otherwise channel ist closed.
///
void KbBind::handleNotificationChannel(bool start) {
    if (getMacroNumber() > 0 && lastCmd) {
        if (start) {
            lastCmd->write (QString("\nnotifyon %1\n@%1 notify all:on\n").arg(getMacroNumber()).toLatin1());
        } else {
            lastCmd->write (QString("\n@%1 notify all:off\nnotifyoff %1\n").arg(getMacroNumber()).toLatin1());
        }
        lastCmd->flush();
    } else qDebug() << QString("No cmd or valid handle for notification found, macroNumber = %1, lastCmd = %2")
                       .arg(getMacroNumber()).arg(lastCmd? "set" : "unset");
}

void KbBind::keyEvent(const QString& key, bool down){
    // Cats Lock: intercept the WinLock key to toggle the lock state.
    // We handle it here (not via KeyAction) so the lock key is never truly
    // unbound — it always does something, and no changes to keyaction.cpp are needed.
    if(_catsLockMode && key == "lock") {
        if(down)
            setCatsLockActive(!_catsLockActive);
        return; // consume the event; don't pass to the normal action system
    }

    QString rKey = globalRemap(key);
    KeyAction* act = bindAction(rKey);
    if(act)
        act->keyEvent(this, down);
}

void KbBind::setCatsLockMode(bool enable) {
    if(_catsLockMode == enable)
        return;
    _catsLockMode = enable;
    if(!enable && _catsLockActive) {
        // Disabling the mode while locked: silently unlock first so the next
        // update() call restores full bindings.
        _catsLockActive = false;
        emit catsLockActiveChanged(false);
    }
    _needsUpdate = true;
    _needsSave   = true;
}

void KbBind::setCatsLockActive(bool active) {
    if(_catsLockActive == active)
        return;
    _catsLockActive = active;
    _needsUpdate    = true;
    emit catsLockActiveChanged(active);
}

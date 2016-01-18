#include <QDateTime>
#include <QUrl>
#include "ckbsettings.h"
#include "kbbind.h"
#include "kbmode.h"
#include "kb.h"

QHash<QString, QString> KbBind::_globalRemap;
quint64 KbBind::globalRemapTime = 0;

KbBind::KbBind(KbMode* modeParent, Kb* parentBoard, const KeyMap& keyMap) :
    QObject(modeParent), _devParent(parentBoard), lastGlobalRemapTime(globalRemapTime), _map(keyMap),
    _winLock(false), _needsUpdate(true), _needsSave(true) {
}

KbBind::KbBind(KbMode* modeParent, Kb* parentBoard, const KeyMap& keyMap, const KbBind& other) :
    QObject(modeParent), _devParent(parentBoard), lastGlobalRemapTime(globalRemapTime), _bind(other._bind),
    _winLock(false), _needsUpdate(true), _needsSave(true) {
    map(keyMap);
}

KbPerf* KbBind::perf(){
    return modeParent()->perf();
}

KbLight* KbBind::light(){
    return modeParent()->light();
}

void KbBind::load(CkbSettings& settings){
    _needsSave = false;
    SGroup group(settings, "Binding");
    KeyMap currentMap = _map;
    _map = KeyMap::fromName(settings.value("KeyMap").toString());
    // Load key settings
    bool useReal = settings.value("UseRealNames").toBool();
    _bind.clear();
    {
        SGroup group(settings, "Keys");
        foreach(QString key, settings.childKeys()){
            QString name = key.toLower();
            if(!useReal)
                name = _map.fromStorage(name);
            QString bind = settings.value(key).toString();
            _bind[name] = new KeyAction(bind, this);
        }
    }
    _winLock = settings.value("WinLock").toBool();
    emit didLoad();
    map(currentMap);
}

void KbBind::save(CkbSettings& settings){
    _needsSave = false;
    SGroup group(settings, "Binding");
    settings.setValue("KeyMap", _map.name());
    // Save key settings
    settings.setValue("UseRealNames", true);
    {
        SGroup group(settings, "Keys");
        foreach(QString key, _bind.keys()){
            KeyAction* act = _bind.value(key);
            if(act && act->value() != KeyAction::defaultAction(key))
                settings.setValue(key, act->value());
        }
    }
    settings.setValue("WinLock", _winLock);
}

QString KbBind::globalRemap(const QString& key){
    if(!_globalRemap.contains(key))
        return key;
    return _globalRemap.value(key);
}

void KbBind::setGlobalRemap(const QHash<QString, QString> keyToActual){
    _globalRemap.clear();
    // Ignore any keys with the standard binding
    QHashIterator<QString, QString> i(keyToActual);
    while(i.hasNext()){
        i.next();
        if(i.key() != i.value())
            _globalRemap[i.key()] = i.value();
    }
    globalRemapTime = QDateTime::currentMSecsSinceEpoch();
}

void KbBind::loadGlobalRemap(){
    _globalRemap.clear();
    CkbSettings settings("Program/GlobalRemap");
    foreach(const QString& key, settings.childKeys())
        _globalRemap[key] = settings.value(key).toString();
    globalRemapTime = QDateTime::currentMSecsSinceEpoch();
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

QString KbBind::action(const QString& key){
    QString rKey = globalRemap(key);
    return bindAction(rKey)->value();
}

QString KbBind::defaultAction(const QString& key){
    QString rKey = globalRemap(key);
    return KeyAction::defaultAction(rKey);
}

QString KbBind::friendlyName(const QString& key){
    const Key& pos = _map[globalRemap(key)];
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

void KbBind::update(QFile& cmd, bool force){
    if(!force && !_needsUpdate && lastGlobalRemapTime == globalRemapTime)
        return;
    lastGlobalRemapTime = globalRemapTime;
    emit updated();
    _needsUpdate = false;
    // Reset all keys and enable notifications for all
    cmd.write("rebind all notify all");
    // Make sure modifier keys are included as they may be remapped globally
    QHash<QString, KeyAction*> bind(_bind);
    if(!_bind.contains("caps")) bind["caps"] = 0;
    if(!_bind.contains("lshift")) bind["lshift"] = 0;
    if(!_bind.contains("rshift")) bind["rshift"] = 0;
    if(!_bind.contains("lctrl")) bind["lctrl"] = 0;
    if(!_bind.contains("rctrl")) bind["rctrl"] = 0;
    if(!_bind.contains("lwin")) bind["lwin"] = 0;
    if(!_bind.contains("rwin")) bind["rwin"] = 0;
    if(!_bind.contains("lalt")) bind["lalt"] = 0;
    if(!_bind.contains("ralt")) bind["ralt"] = 0;
    if(!_bind.contains("fn")) bind["fn"] = 0;
    QHashIterator<QString, KeyAction*> i(bind);
    // Write out rebound keys
    while(i.hasNext()){
        i.next();
        QString key = i.key();
        KeyAction* act = i.value();
        if(_globalRemap.contains(key))
            act = bindAction(_globalRemap.value(key));
        if(!act)
            continue;
        QString value = act->driverName();
        if(value.isEmpty()){
            // If the key is unbound or is a special action, unbind it
            cmd.write(" unbind ");
            cmd.write(key.toLatin1());
        } else {
            // Otherwise, write the binding
            cmd.write(" bind ");
            cmd.write(key.toLatin1());
            cmd.write(":");
            cmd.write(value.toLatin1());
        }
    }
    // If win lock is enabled, unbind windows keys
    if(_winLock)
        cmd.write(" unbind lwin rwin");
}

void KbBind::keyEvent(const QString& key, bool down){
    QString rKey = globalRemap(key);
    KeyAction* act = bindAction(rKey);
    if(act)
        act->keyEvent(this, down);
}

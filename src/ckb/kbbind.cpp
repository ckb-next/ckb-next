#include "kbbind.h"
#include "kbmode.h"
#include "kb.h"

KbBind::KbBind(KbMode* modeParent, Kb* parentBoard, const KeyMap& keyMap) :
    QObject(modeParent), _devParent(parentBoard), _map(keyMap),
    _winLock(false), _needsUpdate(true), _osxSwap(KeyPos::osxCmdSwap) {
}

KbBind::KbBind(KbMode* modeParent, Kb* parentBoard, const KeyMap& keyMap, const KbBind& other) :
    QObject(modeParent), _devParent(parentBoard), _bind(other._bind),
    _winLock(false), _needsUpdate(true), _osxSwap(KeyPos::osxCmdSwap) {
    map(keyMap);
}

void KbBind::load(QSettings& settings){
    settings.beginGroup("Binding");
    KeyMap currentMap = _map;
    _map = KeyMap::fromName(settings.value("KeyMap").toString());
    // Load key settings
    _bind.clear();
    settings.beginGroup("Keys");
    foreach(QString key, settings.childKeys()){
        QString bind = settings.value(key).toString();
        _bind[key] = bind;
    }
    settings.endGroup();
    settings.endGroup();
    emit didLoad();
    map(currentMap);
}

void KbBind::save(QSettings& settings){
    settings.beginGroup("Binding");
    settings.setValue("KeyMap", _map.name());
    // Save key settings
    settings.beginGroup("Keys");
    foreach(QString key, _bind.keys())
        settings.setValue(key, _bind.value(key));
    settings.endGroup();
    settings.endGroup();
}

void KbBind::map(const KeyMap& map){
    _map = map;
    // Remove any keys not present in the map
    QHashIterator<QString, QString> i(_bind);
    while(i.hasNext()){
        i.next();
        QString key = i.key();
        if(!map.key(key))
            _bind.remove(key);
    }
    _needsUpdate = true;
    emit updated();
}

QString KbBind::action(const QString& key){
    if(_bind.contains(key))
        return _bind.value(key);
    if(!_map.key(key))
        return "";
    // If the key isn't reassigned, return the default action
    return defaultAction(key);
}

QString KbBind::defaultAction(const QString& key){
    // G1-G18 are unbound by default
    if(key.length() >= 2 && key[0] == 'g' && key[1] >= '0' && key[1] <= '9')
        return "";
    // TODO: default action for MR
    if(key == "mr")
        return "";
    // M1-M3 switch modes
    if(key == "m1")
        return "$mode:0";
    if(key == "m2")
        return "$mode:1";
    if(key == "m3")
        return "$mode:2";
    // Brightness and Win Lock are their own functions
    if(key == "light")
        return "$light:2";
    if(key == "lock")
        return "$lock:0";
    // Everything else is a standard keypress
    return key;
}

QString KbBind::friendlyName(const QString& key){
    QString act = action(key);
    if(act.isEmpty())
        return "Unbound";
    QStringList parts = act.split(":");
    QString prefix = parts[0];
    if(parts.length() < 2){
        const KeyPos* key = _map.key(prefix);
        if(!key)
            return "(Unknown)";
        return key->friendlyName();
    }
    int suffix = parts[1].toInt();
    if(prefix == "$mode"){
        switch(suffix){
        case MODE_PREV:
        case MODE_PREV_WRAP:
            return "Switch to previous mode";
        case MODE_NEXT:
        case MODE_NEXT_WRAP:
            return "Switch to next mode";
        default:
            return tr("Switch to mode %1").arg(suffix + 1);
        }
    } else if(prefix == "$light"){
        switch(suffix){
        case LIGHT_UP:
        case LIGHT_UP_WRAP:
            return "Brightness up";
        case LIGHT_DOWN:
        case LIGHT_DOWN_WRAP:
            return "Brightness down";
        }
    } else if(prefix == "$lock"){
        switch(suffix){
        case LOCK_TOGGLE:
            return "Toggle Windows lock";
        case LOCK_ON:
            return "Windows lock on";
        case LOCK_OFF:
            return "Windows lock off";
        }
    }
    return "(Unknown)";
}

void KbBind::resetAction(const QString &key){
    _bind.remove(key);
    _needsUpdate = true;
    emit updated();
}

void KbBind::noAction(const QString& key){
    if(!_map.key(key))
        return;
    _bind[key] = "";
    _needsUpdate = true;
    emit updated();
}

void KbBind::keyAction(const QString& key, const QString& actionKey){
    if(!_map.key(key) || !_map.key(actionKey))
        return;
    _bind[key] = actionKey;
    _needsUpdate = true;
    emit updated();
}

void KbBind::modeAction(const QString& key, int mode){
    if(!_map.key(key))
        return;
    _bind[key] = QString("$mode:%1").arg(mode);
    _needsUpdate = true;
    emit updated();
}

void KbBind::lightAction(const QString& key, int type){
    if(!_map.key(key))
        return;
    _bind[key] = QString("$light:%1").arg(type);
    _needsUpdate = true;
    emit updated();
}

void KbBind::lockAction(const QString& key, int type){
    if(!_map.key(key))
        return;
    _bind[key] = QString("$lock:%1").arg(type);
    _needsUpdate = true;
    emit updated();
}

void KbBind::update(QFile& cmd, bool force){
    if(_osxSwap != KeyPos::osxCmdSwap){
        force = true;
        _osxSwap = KeyPos::osxCmdSwap;
    }
    if(!force && !_needsUpdate)
        return;
    _needsUpdate = false;
    // Reset all keys and enable notifications for all
    cmd.write("rebind all notify all");
#ifdef Q_OS_MACX
        if(_osxSwap)
            cmd.write(" bind lctrl:lwin rctrl:rwin lwin:lctrl rwin:rctrl");
#endif
    QHashIterator<QString, QString> i(_bind);
    // Write out rebound keys
    while(i.hasNext()){
        i.next();
        QString key = i.key();
        QString act = i.value();
#ifdef Q_OS_MACX
        if(_osxSwap){
            if(act == "lctrl") act = "lwin";
            else if(act == "rctrl") act = "rwin";
            else if(act == "lwin") act = "lctrl";
            else if(act == "rwin") act = "rctrl";
        }
#endif
        if(act.isEmpty() || isSpecial(act)){
            // If the key is unbound or is a special action, unbind it
            cmd.write(" unbind ");
            cmd.write(key.toLatin1());
        } else {
            // Otherwise, write the binding
            cmd.write(" bind ");
            cmd.write(key.toLatin1());
            cmd.write(":");
            cmd.write(act.toLatin1());
        }
    }
    // If win lock is enabled, unbind windows keys
    if(_winLock)
        cmd.write(" unbind lwin rwin");
}

void KbBind::keyEvent(const QString& key, bool down){
    QString act = action(key);
    // No need to respond to standard actions or keyups (all actions currently happen on keydown)
    if(!down || !isSpecial(act))
        return;
    QStringList parts = act.split(":");
    if(parts.length() < 2)
        return;
    QString prefix = parts[0];
    int suffix = parts[1].toInt();
    if(prefix == "$mode"){
        // Change mode
        Kb* device = devParent();
        KbProfile* currentProfile = device->currentProfile;
        int mode = currentProfile->modes.indexOf(currentProfile->currentMode);
        int modeCount = currentProfile->modes.count();
        switch(suffix){
        case MODE_PREV_WRAP:
            mode--;
            if(mode < 0)
                mode = modeCount - 1;
            break;
        case MODE_NEXT_WRAP:
            mode++;
            if(mode >= modeCount)
                mode = 0;
            break;
        case MODE_PREV:
            mode--;
            break;
        case MODE_NEXT:
            mode++;
            break;
        default:
            // Absolute
            mode = suffix;
            break;
        }
        if(mode < 0 || mode >= modeCount)
            return;
        device->setCurrentMode(currentProfile->modes.at(mode));
    } else if(prefix == "$light"){
        // Change brightness
        KbLight* light = modeParent()->light();
        int dim = light->dimming();
        switch(suffix){
        case LIGHT_UP:
            if(dim > 0)
                dim--;
            break;
        case LIGHT_DOWN:
            if(dim < KbLight::MAX_DIM)
                dim++;
            break;
        case LIGHT_UP_WRAP:
            dim--;
            if(dim < 0)
                dim = KbLight::MAX_DIM;
            break;
        case LIGHT_DOWN_WRAP:
            dim++;
            if(dim > KbLight::MAX_DIM)
                dim = 0;
            break;
        }
        light->dimming(dim);
    } else if(prefix == "$lock"){
        // Change win lock
        switch(suffix){
        case LOCK_TOGGLE:
            winLock(!winLock());
            break;
        case LOCK_ON:
            winLock(true);
            break;
        case LOCK_OFF:
            winLock(false);
            break;
        }
    }
}

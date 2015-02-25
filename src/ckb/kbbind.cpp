#include <QDateTime>
#include <QUrl>
#include "kbbind.h"
#include "kbmode.h"
#include "kb.h"

QHash<QString, QString> KbBind::_globalRemap;
quint64 KbBind::globalRemapTime = 0;

KbBind::KbBind(KbMode* modeParent, Kb* parentBoard, const KeyMap& keyMap) :
    QObject(modeParent), _devParent(parentBoard), lastGlobalRemapTime(globalRemapTime), _map(keyMap),
    _winLock(false), _needsUpdate(true) {
}

KbBind::KbBind(KbMode* modeParent, Kb* parentBoard, const KeyMap& keyMap, const KbBind& other) :
    QObject(modeParent), _devParent(parentBoard), lastGlobalRemapTime(globalRemapTime), _bind(other._bind),
    _winLock(false), _needsUpdate(true) {
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
    foreach(QString key, _bind.keys()){
        QString act = _bind.value(key);
        if(act != defaultAction(key))
            settings.setValue(key, act);
    }
    settings.endGroup();
    settings.endGroup();
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
    QSettings settings;
    settings.beginGroup("Program/GlobalRemap");
    foreach(const QString& key, settings.childKeys())
        _globalRemap[key] = settings.value(key).toString();
    globalRemapTime = QDateTime::currentMSecsSinceEpoch();
}

void KbBind::saveGlobalRemap(){
    QSettings settings;
    settings.remove("Program/GlobalRemap");
    settings.beginGroup("Program/GlobalRemap");
    QHashIterator<QString, QString> i(_globalRemap);
    while(i.hasNext()){
        i.next();
        settings.setValue(i.key(), i.value());
    }
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
    emit layoutChanged();
}

QString KbBind::action(const QString& key){
    QString rKey = globalRemap(key);
    if(_bind.contains(rKey))
        return _bind.value(rKey);
    if(!_map.key(rKey))
        return "";
    // If the key isn't reassigned, return the default action
    return defaultAction(key);
}

QString KbBind::defaultAction(const QString& key){
    QString rKey = globalRemap(key);
    // G1-G18 are unbound by default
    if(rKey.length() >= 2 && rKey[0] == 'g' && rKey[1] >= '0' && rKey[1] <= '9')
        return "";
    // TODO: default action for MR
    if(rKey == "mr")
        return "";
    // M1-M3 switch modes
    if(rKey == "m1")
        return "$mode:0";
    if(rKey == "m2")
        return "$mode:1";
    if(rKey == "m3")
        return "$mode:2";
    // Brightness and Win Lock are their own functions
    if(rKey == "light")
        return "$light:2";
    if(rKey == "lock")
        return "$lock:0";
    // Everything else is a standard keypress
    return rKey;
}

QString KbBind::friendlyName(const QString& key){
    const KeyPos* pos = map().key(globalRemap(key));
    if(!pos)
        return "(Unknown)";
    return pos->friendlyName();
}

QString KbBind::friendlyActionName(const QString& key){
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
    } else if(prefix == "$program"){
        return "Launch program";
    }
    return "(Unknown)";
}

void KbBind::resetAction(const QString &key){
    QString rKey = globalRemap(key);
    _bind.remove(rKey);
    _needsUpdate = true;
}

void KbBind::noAction(const QString& key){
    QString rKey = globalRemap(key);
    if(!_map.key(rKey))
        return;
    _bind[rKey] = "";
    _needsUpdate = true;
}

void KbBind::keyAction(const QString& key, const QString& actionKey){
    QString rKey = globalRemap(key);
    if(!_map.key(rKey) || !_map.key(actionKey))
        return;
    _bind[rKey] = actionKey;
    _needsUpdate = true;
}

void KbBind::modeAction(const QString& key, int mode){
    QString rKey = globalRemap(key);
    if(!_map.key(rKey))
        return;
    _bind[rKey] = QString("$mode:%1").arg(mode);
    _needsUpdate = true;
}

void KbBind::lightAction(const QString& key, int type){
    QString rKey = globalRemap(key);
    if(!_map.key(rKey))
        return;
    _bind[rKey] = QString("$light:%1").arg(type);
    _needsUpdate = true;
}

void KbBind::lockAction(const QString& key, int type){
    QString rKey = globalRemap(key);
    if(!_map.key(rKey))
        return;
    _bind[rKey] = QString("$lock:%1").arg(type);
    _needsUpdate = true;
}

void KbBind::programAction(const QString& key, const QString& onPress, const QString& onRelease, int stop){
    QString rKey = globalRemap(key);
    if(!_map.key(rKey))
        return;
    // URL-encode the commands and place them in the string (":" and "+" are both replaced, so they won't interfere)
    _bind[rKey] = "$program:" + QString::fromUtf8(QUrl::toPercentEncoding(onPress.trimmed())) + "+" + QString::fromUtf8(QUrl::toPercentEncoding(onRelease.trimmed())) + QString("+%1").arg(stop);
    _needsUpdate = true;
}

QString KbBind::specialInfo(const QString& action, int& parameter){
    QStringList list = action.split(":");
    if(list.length() < 2){
        parameter = INT_MIN;
        return "";
    }
    parameter = list[1].toInt();
    return list[0].replace("$", "");
}

int KbBind::programInfo(const QString& action, QString& onPress, QString& onRelease){
    if(!isProgram(action))
        return 0;
    QString param = action.mid(9);
    QStringList programs = param.split("+");
    if(programs.length() != 3)
        return 0;
    onPress = QUrl::fromPercentEncoding(programs[0].toUtf8());
    onRelease = QUrl::fromPercentEncoding(programs[1].toUtf8());
    return programs[2].toInt();
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
    QHash<QString, QString> bind(_bind);
    if(!_bind.contains("caps")) bind["caps"] = "caps";
    if(!_bind.contains("lshift")) bind["lshift"] = "lshift";
    if(!_bind.contains("rshift")) bind["rshift"] = "rshift";
    if(!_bind.contains("lctrl")) bind["lctrl"] = "lctrl";
    if(!_bind.contains("rctrl")) bind["rctrl"] = "rctrl";
    if(!_bind.contains("lwin")) bind["lwin"] = "lwin";
    if(!_bind.contains("rwin")) bind["rwin"] = "rwin";
    if(!_bind.contains("lalt")) bind["lalt"] = "lalt";
    if(!_bind.contains("ralt")) bind["ralt"] = "ralt";
    QHashIterator<QString, QString> i(bind);
    // Write out rebound keys
    while(i.hasNext()){
        i.next();
        QString key = i.key();
        QString act = i.value();
        if(_globalRemap.contains(key))
            act = action(key);
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
    QString rKey = globalRemap(key);
    QString act = action(key);
    // No need to respond to standard actions
    if(!isSpecial(act))
        return;
    QStringList parts = act.split(":");
    if(parts.length() < 2)
        return;
    QString prefix = parts[0];
    int suffix = parts[1].toInt();
    if(prefix == "$mode"){
        if(!down)
            return;
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
        if(!down)
            return;
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
        if(!down)
            return;
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
    } else if(prefix == "$program"){
        // Launch program
        QString onPress, onRelease;
        int stop = programInfo(act, onPress, onRelease);
        // Stop running programs based on setting
        QProcess* process = 0;
        if(down){
            if(stop & PROGRAM_PR_KPSTOP){
                process = kpPrograms.value(rKey);
                if(process)
                    process->kill();
                process = 0;
            }
            if(stop & PROGRAM_RE_KPSTOP)
                process = krPrograms.value(rKey);
        } else {
            if(stop & PROGRAM_PR_KRSTOP)
                process = kpPrograms.value(rKey);
        }
        if(process)
            process->kill();
        // Launch new process if requested
        QString& program = down ? onPress : onRelease;
        if(program.isEmpty())
            return;
        // Check if the program is running already. If so, don't start it again.
        QHash<QString, QProcess*>& running = down ? kpPrograms : krPrograms;
        process = running.value(rKey);
        if(process){
            if(process->state() == QProcess::NotRunning)
                delete process;
            else
                return;
        }
        // Start the program. Wrap it around sh to parse arguments.
        process = new QProcess(this);
        process->start("sh", QStringList() << "-c" << program);
        running[rKey] = process;
    }
}

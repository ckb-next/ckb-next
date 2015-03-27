#include <cmath>
#include <QDateTime>
#include "kblight.h"

KbLight::KbLight(QObject* parent, const KeyMap& keyMap) :
    QObject(parent), _map(), _dimming(0), _inactive(MAX_INACTIVE), _showMute(true), _start(false), _needsSave(true)
{
    map(keyMap);
}

KbLight::KbLight(QObject* parent, const KeyMap& keyMap, const KbLight& other) :
    QObject(parent), _map(other._map), _colorMap(other._colorMap), _dimming(other._dimming), _inactive(other._inactive), _showMute(other._showMute), _start(false), _needsSave(true)
{
    map(keyMap);
    // Duplicate animations
    foreach(KbAnim* animation, other._animList)
        _animList.append(new KbAnim(this, keyMap, *animation));
}

void KbLight::map(const KeyMap& map){
    uint newCount = map.count();
    uint oldCount = _map.count();
    QHash<QString, QRgb> newColorMap = _colorMap;
    // Translate key colors by position (if possible)
    for(uint i = 0; i < newCount; i++){
        const KeyPos* newPos = map.key(i);
        QString name = newPos->name;
        if(name != "enter"){
            bool found = false;
            for(uint j = 0; j < oldCount; j++){
                // Scan old map for matching positions
                const KeyPos* oldPos = _map.key(j);
                QString oldName = oldPos->name;
                if(oldPos->x == newPos->x && oldPos->y == newPos->y
                        && oldName != "enter" && _colorMap.contains(oldName)){
                    // If found, set color
                    found = true;
                    newColorMap[name] = _colorMap.value(oldName);
                    break;
                }
            }
            if(found)
                continue;
        }
        // If the map still doesn't contain the key, set it to white.
        if(!newColorMap.contains(name))
            newColorMap[name] = 0xFFFFFFFF;
    }
    // Set the new map
    _map = map;
    foreach(KbAnim* anim, _animList)
        anim->map(map);
    _colorMap = newColorMap;
    _needsSave = true;
    emit updated();
}

void KbLight::color(const QColor& newColor){
    QRgb newRgb = newColor.rgb();
    uint count = _map.count();
    for(uint i = 0; i < count; i++)
        _colorMap[_map.key(i)->name] = newRgb;
    _needsSave = true;
}

KbAnim* KbLight::addAnim(const AnimScript *base, const QStringList &keys){
    // Stop and restart all existing animations
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    foreach(KbAnim* anim, _animList){
        anim->stop();
        anim->trigger(timestamp);
    }
    // Add the new animation and start it
    KbAnim* anim = new KbAnim(this, _map, keys, base);
    _animList.append(anim);
    anim->trigger(timestamp);
    _start = true;
    _needsSave = true;
    return anim;
}

KbAnim* KbLight::duplicateAnim(KbAnim* oldAnim){
    // Stop and restart all existing animations
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    foreach(KbAnim* anim, _animList){
        anim->stop();
        anim->trigger(timestamp);
    }
    // Same as addAnim, just duplicate the existing one
    KbAnim* anim = new KbAnim(this, _map, *oldAnim);
    anim->newId();
    int index = _animList.indexOf(oldAnim);
    if(index < 0)
        _animList.append(anim);
    else
        _animList.insert(index + 1, anim);
    anim->trigger(timestamp);
    _start = true;
    _needsSave = true;
    return anim;
}

bool KbLight::isStarted(){
    if(!_start)
        return false;
    foreach(KbAnim* animation, _animList){
        if(!animation->isRunning())
            return false;
    }
    return true;
}

void KbLight::restartAnimation(){
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    foreach(KbAnim* anim, _animList){
        anim->stop();
        anim->trigger(timestamp);
    }
    _start = true;
}

void KbLight::animKeypress(const QString& key, bool down){
    foreach(KbAnim* anim, _animList){
        if(anim->keys().contains(key))
            anim->keypress(key, down, QDateTime::currentMSecsSinceEpoch());
    }
}

void KbLight::printRGB(QFile& cmd, const QHash<QString, QRgb>& animMap){
    cmd.write(" rgb on");
    QHashIterator<QString, QRgb> i(animMap);
    while(i.hasNext()){
        i.next();
        QString name = i.key();
        // Volume buttons don't have LEDs except on the K65
        if(_map.model() != KeyMap::K65 && (name == "volup" || name == "voldn"))
            continue;
        QRgb color = i.value();
        cmd.write(" ");
        cmd.write(name.toLatin1());
        char output[8];
        snprintf(output, sizeof(output), ":%02x%02x%02x", qRed(color), qGreen(color), qBlue(color));
        cmd.write(output);
    }
}

void KbLight::frameUpdate(QFile& cmd, int modeIndex, bool dimMute, bool dimLock){
    // Advance animations
    QHash<QString, QRgb> animMap = _colorMap;
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    foreach(KbAnim* anim, _animList)
        anim->blend(animMap, timestamp);

    cmd.write(QString().sprintf("mode %d switch", modeIndex + 1).toLatin1());
    if(_dimming == 3){
        // If brightness is at 0%, turn off lighting entirely
        cmd.write(" rgb off");
        return;
    }

    // Dim inactive keys
    float light = (3 - _dimming) / 3.f;
    QStringList inactiveList = QStringList();
    float inactiveLight = 1.f;
    if(_inactive >= 0){
        inactiveList << "mr" << "m1" << "m2" << "m3";
        if(dimLock)
            inactiveList << "lock";
        if(dimMute && _showMute)
            inactiveList << "mute";
        inactiveList.removeAll(QString("m%1").arg(modeIndex + 1));
        inactiveLight = (2 - _inactive) / 4.f;
    }

    // Set brightness
    if(light != 1.f || inactiveLight != 1.f){
        QMutableHashIterator<QString, QRgb> i(animMap);
        while(i.hasNext()){
            i.next();
            QRgb& rgb = i.value();
            float r = qRed(rgb);
            float g = qGreen(rgb);
            float b = qBlue(rgb);
            if(light != 1.f){
                r *= light;
                g *= light;
                b *= light;
            }
            if(inactiveLight != 1.f && inactiveList.contains(i.key())){
                r *= inactiveLight;
                g *= inactiveLight;
                b *= inactiveLight;
            }
            rgb = qRgb(round(r), round(g), round(b));
        }
    }

    // Apply light
    printRGB(cmd, animMap);
}

void KbLight::open(){
    if(_start)
        return;
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    foreach(KbAnim* anim, _animList)
        anim->trigger(timestamp);
    _start = true;
}

void KbLight::close(){
    foreach(KbAnim* anim, _animList)
        anim->stop();
    _start = false;
}

void KbLight::base(QFile &cmd, int modeIndex){
    close();
    if(_dimming == MAX_DIM){
        cmd.write(QString().sprintf("mode %d rgb off", modeIndex + 1).toLatin1());
        return;
    }
    // Set just the background color, ignoring any animation
    cmd.write(QString().sprintf("mode %d", modeIndex + 1).toLatin1());
    QHash<QString, QRgb> animMap = _colorMap;
    animMap["mr"] = qRgb(0, 0, 0);
    animMap["m1"] = qRgb(0, 0, 0);
    animMap["m2"] = qRgb(0, 0, 0);
    animMap["m3"] = qRgb(0, 0, 0);
    animMap["lock"] = qRgb(0, 0, 0);
    printRGB(cmd, animMap);
}

void KbLight::load(QSettings& settings){
    // Load light settings
    _needsSave = false;
    settings.beginGroup("Lighting");
    KeyMap currentMap = _map;
    _map = KeyMap::fromName(settings.value("KeyMap").toString());
    _dimming = settings.value("Brightness").toUInt();
    if(_dimming > MAX_DIM)
        _dimming = MAX_DIM;
    bool inOk = false;
    _inactive = settings.value("InactiveIndicators").toInt(&inOk);
    if(!inOk || _inactive > MAX_INACTIVE)
        _inactive = MAX_INACTIVE;
    _showMute = (settings.value("ShowMute").toInt(&inOk) != 0);
    if(!inOk)
        _showMute = true;
    // Load RGB settings
    settings.beginGroup("Keys");
    foreach(QString key, settings.childKeys()){
        QColor color = settings.value(key).toString();
        if(!color.isValid())
            color = QColor(255, 255, 255);
        _colorMap[key.toLower()] = color.rgb();
    }
    settings.endGroup();
    // Load animations
    foreach(KbAnim* anim, _animList)
        anim->deleteLater();
    _animList.clear();
    settings.beginGroup("Animations");
    foreach(QString anim, settings.value("List").toStringList()){
        QUuid id = anim;
        _animList.append(new KbAnim(this, _map, id, settings));
    }
    settings.endGroup();
    settings.endGroup();
    emit didLoad();
    map(currentMap);
}

void KbLight::save(QSettings& settings){
    _needsSave = false;
    settings.beginGroup("Lighting");
    settings.setValue("KeyMap", _map.name());
    settings.setValue("Brightness", _dimming);
    settings.setValue("InactiveIndicators", _inactive);
    settings.setValue("ShowMute", (int)_showMute);
    // Save RGB settings
    settings.beginGroup("Keys");
    foreach(QString key, _colorMap.keys())
        settings.setValue(key, QColor(_colorMap.value(key)).name());
    settings.endGroup();
    // Save animations
    settings.beginGroup("Animations");
    QStringList aList;
    foreach(KbAnim* anim, _animList){
        aList << anim->guid().toString().toUpper();
        anim->save(settings);
    }
    settings.setValue("List", aList);
    settings.endGroup();
    settings.endGroup();
}

bool KbLight::needsSave() const {
    if(_needsSave)
        return true;
    foreach(KbAnim* anim, _animList){
        if(anim->needsSave())
            return true;
    }
    return false;
}

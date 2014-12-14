#include <cmath>
#include "kblight.h"

#include "animscript.h"

KbLight::KbLight(QObject* parent, int modeIndex, const KeyMap& map) :
    QObject(parent), _map(map), _modeIndex(modeIndex + 1), _brightness(0), _inactive(MAX_INACTIVE), _winLock(false), _showMute(true)
{
}

void KbLight::map(const KeyMap& map){
    uint newCount = map.count();
    uint oldCount = _map.count();
    QHash<QString, QRgb> newColorMap = _colorMap;
    // Translate key colors by position (if possible)
    for(uint i = 0; i < newCount; i++){
        const KeyPos* newPos = map.key(i);
        QString name = newPos->name;
        if(name == "enter")
            // Leave the enter key alone
            continue;
        bool found = false;
        for(uint j = 0; j < oldCount; j++){
            // Scan old map for matching positions
            const KeyPos* oldPos = _map.key(j);
            QString oldName = oldPos->name;
            if(oldPos->x == newPos->x && oldPos->y == newPos->y
                    && _colorMap.contains(oldName)){
                // If found, set color
                found = true;
                newColorMap[name] = _colorMap.value(oldName);
                break;
            }
        }
        if(found)
            continue;
        // If the map still doesn't contain the key, set it to white.
        if(!newColorMap.contains(name))
            newColorMap[name] = 0xFFFFFFFF;
    }
    // Set the new map
    _map = map;
    foreach(KbAnim* anim, animList)
        anim->map(map);
    _colorMap = newColorMap;
}

void KbLight::color(const QColor& newColor){
    QRgb newRgb = newColor.rgb();
    uint count = _map.count();
    for(uint i = 0; i < count; i++)
        _colorMap[_map.key(i)->name] = newRgb;
}

KbAnim* KbLight::addAnim(const AnimScript *base, const QStringList &keys){
    KbAnim* anim = new KbAnim(this, _map, keys, base);
    animList.append(anim);
    return anim;
}

void KbLight::printRGB(QFile& cmd, const QHash<QString, QRgb>& animMap){
    cmd.write(" rgb on");
    QHashIterator<QString, QRgb> i(animMap);
    while(i.hasNext()){
        i.next();
        QRgb color = i.value();
        cmd.write(" ");
        cmd.write(i.key().toLatin1());
        char output[8];
        snprintf(output, 8, ":%02x%02x%02x", qRed(color), qGreen(color), qBlue(color));
        cmd.write(output);
    }
}

void KbLight::frameUpdate(QFile& cmd, bool dimMute){
    cmd.write(QString().sprintf("mode %d switch", _modeIndex).toLatin1());
    if(_brightness == 3){
        cmd.write(" rgb off");
        return;
    }
    QHash<QString, QRgb> animMap = _colorMap;
    float light = (3 - _brightness) / 3.f;
    QStringList inactiveList = QStringList();
    float inactiveLight = 1.f;
    if(_inactive >= 0){
        inactiveList << "mr" << "m1" << "m2" << "m3";
        if(!_winLock)
            inactiveList << "lock";
        if(dimMute && _showMute)
            inactiveList << "mute";
        inactiveList.removeAll(QString("m%1").arg(_modeIndex));
        inactiveLight = (2 - _inactive) / 4.f;
    }

    foreach(KbAnim* anim, animList)
        anim->blend(animMap);

    // Dim inactive keys
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

void KbLight::close(QFile &cmd){
    foreach(KbAnim* anim, animList)
        anim->stop();
    if(_brightness == MAX_BRIGHTNESS){
        cmd.write(QString().sprintf("mode %d rgb off", _modeIndex).toLatin1());
        return;
    }
    // Set just the background color, ignoring any animation
    cmd.write(QString().sprintf("mode %d", _modeIndex).toLatin1());
    QHash<QString, QRgb> animMap = _colorMap;
    animMap["mr"] = qRgb(0, 0, 0);
    animMap["m1"] = qRgb(0, 0, 0);
    animMap["m2"] = qRgb(0, 0, 0);
    animMap["m3"] = qRgb(0, 0, 0);
    animMap["lock"] = qRgb(0, 0, 0);
    printRGB(cmd, animMap);
}

void KbLight::winLock(QFile& cmd, bool lock){
    _winLock = lock;
    if(lock)
        cmd.write(QString().sprintf("mode %d unbind lwin rwin", _modeIndex).toLatin1());
    else
        cmd.write(QString().sprintf("mode %d rebind lwin rwin", _modeIndex).toLatin1());
}

void KbLight::load(QSettings& settings){
    // Load light settings
    settings.beginGroup("Lighting");
    _map = KeyMap::fromName(settings.value("KeyMap").toString());
    _brightness = settings.value("Brightness").toUInt();
    if(_brightness > MAX_BRIGHTNESS)
        _brightness = MAX_BRIGHTNESS;
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
    foreach(KbAnim* anim, animList)
        anim->deleteLater();
    animList.clear();
    settings.beginGroup("Animations");
    foreach(QString anim, settings.value("List").toStringList()){
        QUuid id = anim;
        animList.append(new KbAnim(this, _map, id, settings));
    }
    settings.endGroup();
    settings.endGroup();
    emit didLoad();
}

void KbLight::save(QSettings& settings){
    settings.beginGroup("Lighting");
    settings.setValue("KeyMap", _map.name());
    settings.setValue("Brightness", _brightness);
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
    foreach(KbAnim* anim, animList){
        aList << anim->guid().toString().toUpper();
        anim->save(settings);
    }
    settings.setValue("List", aList);
    settings.endGroup();
    settings.endGroup();
}

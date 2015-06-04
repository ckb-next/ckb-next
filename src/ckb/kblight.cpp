#include <cmath>
#include <QDateTime>
#include <QSet>
#include "kblight.h"
#include "kbmode.h"

static int _shareDimming = -1;
static QSet<KbLight*> activeLights;

KbLight::KbLight(KbMode* parent, const KeyMap& keyMap) :
    QObject(parent), _previewAnim(0), lastFrameSignal(0), _dimming(0), _inactive(MAX_INACTIVE), _showMute(true), _start(false), _needsSave(true)
{
    map(keyMap);
}

KbLight::KbLight(KbMode* parent, const KeyMap& keyMap, const KbLight& other) :
    QObject(parent), _previewAnim(0), _map(other._map), _colorMap(other._colorMap), lastFrameSignal(0), _dimming(other._dimming), _inactive(other._inactive), _showMute(other._showMute), _start(false), _needsSave(true)
{
    map(keyMap);
    // Duplicate animations
    foreach(KbAnim* animation, other._animList)
        _animList.append(new KbAnim(this, keyMap, *animation));
}

void KbLight::map(const KeyMap& map){
    // If any of the keys are missing from the color map, set them to white
    QHashIterator<QString, Key> i(map);
    while(i.hasNext()){
        i.next();
        const QString& key = i.key();
        if(!_colorMap.contains(key))
            _colorMap[key] = 0xFFFFFFFF;
    }
    // Set the new map
    _map = map;
    foreach(KbAnim* anim, _animList)
        anim->map(map);
    _needsSave = true;
    emit updated();
}

KbLight::~KbLight(){
    activeLights.remove(this);
}

void KbLight::color(const QColor& newColor){
    QRgb newRgb = newColor.rgb();
    QMutableHashIterator<QString, QRgb> i(_colorMap);
    while(i.hasNext()){
        i.next();
        i.value() = newRgb;
    }
    _needsSave = true;
}

int KbLight::shareDimming(){
    return _shareDimming;
}

void KbLight::shareDimming(int newShareDimming){
    if(_shareDimming == newShareDimming)
        return;
    _shareDimming = newShareDimming;
    if(newShareDimming != -1){
        foreach(KbLight* light, activeLights)
            light->dimming(newShareDimming);
    }
}

void KbLight::dimming(int newDimming){
    if(_shareDimming != -1)
        shareDimming(newDimming);
    _needsSave = true;
    _dimming = newDimming;
    emit updated();
}

KbAnim* KbLight::addAnim(const AnimScript *base, const QStringList &keys, const QString& name, const QMap<QString, QVariant>& preset){
    // Stop and restart all existing animations
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    foreach(KbAnim* anim, _animList){
        anim->stop();
        anim->trigger(timestamp);
    }
    // Load the new animation and set preset parameters
    KbAnim* anim = new KbAnim(this, _map, name, keys, base);
    QMapIterator<QString, QVariant> i(preset);
    while(i.hasNext()){
        i.next();
        anim->parameter(i.key(), i.value());
    }
    anim->commitParams();
    // Add the animation and start it
    _animList.append(anim);
    anim->trigger(timestamp);
    _start = true;
    _needsSave = true;
    return anim;
}

void KbLight::previewAnim(const AnimScript* base, const QStringList& keys, const QMap<QString, QVariant>& preset){
    if(_previewAnim)
        stopPreview();
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    // Load the new animation and set preset parameters
    KbAnim* anim = new KbAnim(this, _map, "", keys, base);
    QMapIterator<QString, QVariant> i(preset);
    while(i.hasNext()){
        i.next();
        anim->parameter(i.key(), i.value());
    }
    anim->commitParams();
    anim->reInit();
    // Add the animation and start it
    _previewAnim = anim;
    anim->trigger(timestamp);
    _start = true;
}

void KbLight::stopPreview(){
    delete _previewAnim;
    _previewAnim = 0;
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
    stopPreview();
    _start = true;
}

void KbLight::animKeypress(const QString& key, bool down){
    foreach(KbAnim* anim, _animList){
        if(anim->keys().contains(key))
            anim->keypress(key, down, QDateTime::currentMSecsSinceEpoch());
    }
    if(_previewAnim){
        if(_previewAnim->keys().contains(key))
            _previewAnim->keypress(key, down, QDateTime::currentMSecsSinceEpoch());
    }
}

void KbLight::open(){
    // Apply shared dimming if needed
    if(_shareDimming != -1 && _shareDimming != _dimming)
        dimming(_shareDimming);
    activeLights.insert(this);
    if(_start)
        return;
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    foreach(KbAnim* anim, _animList)
        anim->trigger(timestamp);
    if(_previewAnim)
        _previewAnim->trigger(timestamp);
    _start = true;
}

void KbLight::close(){
    activeLights.remove(this);
    foreach(KbAnim* anim, _animList)
        anim->stop();
    stopPreview();
    _start = false;
}

void KbLight::printRGB(QFile& cmd, const QHash<QString, QRgb>& animMap){
    QHashIterator<QString, QRgb> i(animMap);
    while(i.hasNext()){
        i.next();
        QString name = i.key();
        QRgb color = i.value();
        // Make sure the key is in the map before printing it
        const Key& key = _map[name];
        if(!key.hasLed)
            continue;
        cmd.write(" ");
        cmd.write(name.toLatin1());
        char output[8];
        snprintf(output, sizeof(output), ":%02x%02x%02x", qRed(color), qGreen(color), qBlue(color));
        cmd.write(output);
    }
}

void KbLight::frameUpdate(QFile& cmd, const QStringList& dimKeys){
    // Advance animations
    ColorMap animMap = _colorMap;
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    foreach(KbAnim* anim, _animList)
        anim->blend(animMap, timestamp);
    if(_previewAnim)
        _previewAnim->blend(animMap, timestamp);

    // Emit signals for the animation (only do this every 50ms - it can cause a lot of CPU usage)
    if(timestamp >= lastFrameSignal + 50){
        emit frameDisplayed(animMap);
        lastFrameSignal = timestamp;
    }

    cmd.write(QString().sprintf("rgb").toLatin1());
    // If brightness is at 0%, turn off lighting entirely
    if(_dimming == 3){
        cmd.write(" 000000");
        return;
    }

    // Set brightness
    float light = (3 - _dimming) / 3.f;
    float inactiveLight = (_inactive >= 0 ? (2 - _inactive) / 4.f : 1.f);
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
            if(inactiveLight != 1.f && dimKeys.contains(i.key())){
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

void KbLight::base(QFile &cmd, bool ignoreDim){
    close();
    if(_dimming == MAX_DIM && !ignoreDim){
        cmd.write(QString().sprintf("rgb 000000").toLatin1());
        return;
    }
    // Set just the background color, ignoring any animation
    cmd.write(QString().sprintf("rgb").toLatin1());
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
    QSGroup group(settings, "Lighting");
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
    bool useReal = settings.value("UseRealNames").toBool();
    {
        QSGroup group(settings, "Keys");
        foreach(QString key, settings.childKeys()){
            QString name = key.toLower();
            if(!useReal)
                name = _map.fromStorage(name);
            QColor color = settings.value(key).toString();
            if(!color.isValid())
                color = QColor(255, 255, 255);
            _colorMap[name] = color.rgb();
        }
    }
    // Load animations
    foreach(KbAnim* anim, _animList)
        anim->deleteLater();
    _animList.clear();
    {
        QSGroup group(settings, "Animations");
        foreach(QString anim, settings.value("List").toStringList()){
            QUuid id = anim;
            _animList.append(new KbAnim(this, _map, id, settings));
        }
    }
    emit didLoad();
    map(currentMap);
}

void KbLight::save(QSettings& settings){
    _needsSave = false;
    QSGroup group(settings, "Lighting");
    settings.setValue("KeyMap", _map.name());
    settings.setValue("Brightness", _dimming);
    settings.setValue("InactiveIndicators", _inactive);
    settings.setValue("ShowMute", (int)_showMute);
    settings.setValue("UseRealNames", true);
    {
        // Save RGB settings
        QSGroup group(settings, "Keys");
        foreach(QString key, _colorMap.keys())
            settings.setValue(key, QColor(_colorMap.value(key)).name());
    }
    {
        // Save animations
        QSGroup group(settings, "Animations");
        QStringList aList;
        foreach(KbAnim* anim, _animList){
            aList << anim->guid().toString().toUpper();
            anim->save(settings);
        }
        settings.setValue("List", aList);
    }
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

#include <cmath>
#include <QDateTime>
#include <QSet>
#include "kblight.h"
#include "kbmode.h"

static int _shareDimming = -1;
static QSet<KbLight*> activeLights;

KbLight::KbLight(KbMode* parent, const KeyMap& keyMap) :
    QObject(parent), _previewAnim(0), lastFrameSignal(0), _dimming(0), _start(false), _needsSave(true), _needsMapRefresh(true)
{
    map(keyMap);
}

KbLight::KbLight(KbMode* parent, const KeyMap& keyMap, const KbLight& other) :
    QObject(parent), _previewAnim(0), _map(other._map), _qColorMap(other._qColorMap), lastFrameSignal(0), _dimming(other._dimming), _start(false), _needsSave(true), _needsMapRefresh(true)
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
        if(!_qColorMap.contains(key))
            _qColorMap[key] = 0xFFFFFFFF;
    }
    // Set the new map
    _map = map;
    foreach(KbAnim* anim, _animList)
        anim->map(map);
    _colorMap.init(_map);
    _animMap.init(_map);
    _indicatorMap.init(_map);
    _needsSave = _needsMapRefresh = true;
    emit updated();
}

KbLight::~KbLight(){
    activeLights.remove(this);
}

void KbLight::color(const QString& key, const QColor& newColor){
    QRgb newRgb = newColor.rgb();
    _qColorMap[key] = newRgb;
    _needsSave = true;
    if(!_needsMapRefresh){
        // Update flat map if we're not scheduled to rebuild it
        QByteArray rawName = key.toLatin1();
        QRgb* rawRgb = _colorMap.colorForName(rawName.data());
        if(rawRgb)
            *rawRgb = newRgb;
    }
}

void KbLight::color(const QColor& newColor){
    QRgb newRgb = newColor.rgb();
    QMutableColorMapIterator i(_qColorMap);
    while(i.hasNext()){
        i.next();
        i.value() = newRgb;
    }
    _needsSave = true;
    // Reset flat map
    _needsMapRefresh = false;
    int mapCount = _colorMap.count();
    QRgb* flat = _colorMap.colors();
    for(int i = 0; i < mapCount; i++)
        flat[i] = mapCount;
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
    stopPreview();
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

void KbLight::printRGB(QFile& cmd, const ColorMap &animMap){
    int count = animMap.count();
    const char* const* names = animMap.keyNames();
    const QRgb* colors = animMap.colors();
    // Print each color and the corresponding RGB value
    for(int i = 0; i < count; i++){
        cmd.write(" ");
        cmd.write(names[i]);
        char output[8];
        QRgb color = colors[i];
        snprintf(output, sizeof(output), ":%02x%02x%02x", qRed(color), qGreen(color), qBlue(color));
        cmd.write(output);
    }
}

void KbLight::rebuildBaseMap(){
    if(!_needsMapRefresh)
        return;
    _needsMapRefresh = false;
    // Copy RGB values from QColorMap to ColorMap
    QColorMapIterator i(_qColorMap);
    while(i.hasNext()){
        i.next();
        QByteArray rawName = i.key().toLatin1();
        QRgb color = i.value();
        QRgb* rawColor = _colorMap.colorForName(rawName.data());
        if(rawColor)
            *rawColor = color;
    }
}

void KbLight::resetIndicators(){
    _indicatorMap.clear();
    _indicatorList.clear();
}

void KbLight::setIndicator(const char* name, QRgb argb){
    QRgb* dest = _indicatorMap.colorForName(name);
    if(dest){
        *dest = argb;
        _indicatorList.insert(name);
    }
}

// Colorspace conversion: linear <-> sRGB
// (sRGB: [0, 255], linear: [0, 1])

static float sToL(float srgb){
    srgb /= 255.f;
    if(srgb <= 0.04045f)
        return srgb / 12.92f;
    return pow((srgb + 0.055f) / 1.055f, 2.4f);
}

static float lToS(float linear){
    if(linear <= 0.0031308f)
        return 12.92f * linear * 255.f;
    return (1.055f * pow(linear, 1.f / 2.4f) - 0.055f) * 255.f;
}

// Convert RGB to monochrome
QRgb monoRgb(float r, float g, float b){
    // It's important to use a linear colorspace for this, otherwise the colors will appear inconsistent
    // Note that although we could use linear space for alpha blending or the animation blending functions, we don't.
    // The reason for this is that photo manipulation programs don't do it either, so even though the result would technically be more correct,
    // it would look wrong to most people.
    r = sToL(r);
    g = sToL(g);
    b = sToL(b);
    int value = round(lToS((r + g + b) / 3.f));
    return qRgb(value, value, value);
}

void KbLight::frameUpdate(QFile& cmd, bool monochrome){
    rebuildBaseMap();
    _animMap = _colorMap;
    // Advance animations
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    foreach(KbAnim* anim, _animList)
        anim->blend(_animMap, timestamp);
    if(_previewAnim)
        _previewAnim->blend(_animMap, timestamp);

    int count = _animMap.count();
    QRgb* colors = _animMap.colors();
    // Apply active indicators and/or perform monochrome conversion
    if(monochrome || !_indicatorList.isEmpty()){
        QRgb* indicators = _indicatorMap.colors();
        for(int i = 0; i < count; i++){
            QRgb& rgb = colors[i];
            float r = qRed(rgb);
            float g = qGreen(rgb);
            float b = qBlue(rgb);
            // Apply indicators
            QRgb rgb2 = indicators[i];
            if(qAlpha(rgb2) != 0){
                float r2 = qRed(rgb2);
                float g2 = qGreen(rgb2);
                float b2 = qBlue(rgb2);
                float a2 = qAlpha(rgb2) / 255.f;
                r = round(r2 * a2 + r * (1.f - a2));
                g = round(g2 * a2 + g * (1.f - a2));
                b = round(b2 * a2 + b * (1.f - a2));
            }
            // If monochrome mode is active, average the channels to get a grayscale image
            if(monochrome)
                rgb = monoRgb(r, g, b);
            else
                rgb = qRgb(r, g, b);
        }
    }

    // Emit signals for the animation (only do this every 50ms - it can cause a lot of CPU usage)
    if(timestamp >= lastFrameSignal + 50){
        emit frameDisplayed(_animMap, _indicatorList);
        lastFrameSignal = timestamp;
    }

    // If brightness is at 0%, turn off lighting entirely
    if(_dimming == 3){
        cmd.write("rgb 000000");
        return;
    }

    float light = (3 - _dimming) / 3.f;
    // Apply global dimming
    if(light != 1.f || monochrome){
        for(int i = 0; i < count; i++){
            QRgb& rgb = colors[i];
            // Like the monochrome conversion, this should be done in a linear colorspace
            float r = sToL(qRed(rgb));
            float g = sToL(qGreen(rgb));
            float b = sToL(qBlue(rgb));
            r *= light;
            g *= light;
            b *= light;
            r = round(lToS(r));
            g = round(lToS(g));
            b = round(lToS(b));
            rgb = qRgb(r, g, b);
        }
    }

    // Apply light
    cmd.write("rgb");
    printRGB(cmd, _animMap);
}

void KbLight::base(QFile &cmd, bool ignoreDim, bool monochrome){
    close();
    if(_dimming == MAX_DIM && !ignoreDim){
        cmd.write(QString().sprintf("rgb 000000").toLatin1());
        return;
    }
    // Set just the background color, ignoring any animation
    rebuildBaseMap();
    _animMap = _colorMap;
    // If monochrome is active, create grayscale
    if(monochrome){
        int count = _animMap.count();
        QRgb* colors = _animMap.colors();
        for(int i = 0; i < count; i++){
            QRgb& rgb = colors[i];
            rgb = monoRgb(qRed(rgb), qGreen(rgb), qBlue(rgb));
        }
    }
    // Set a few indicators to black as the hardware handles them differently
    QRgb* mr = _animMap.colorForName("mr"), *m1 = _animMap.colorForName("m1"), *m2 = _animMap.colorForName("m2"), *m3 = _animMap.colorForName("m3"), *lock = _animMap.colorForName("lock");
    if(mr) *mr = 0;
    if(m1) *m1 = 0;
    if(m2) *m2 = 0;
    if(m3) *m3 = 0;
    if(lock) *lock = 0;
    // Send to driver
    cmd.write("rgb");
    printRGB(cmd, _animMap);
}

void KbLight::load(CkbSettings& settings){
    // Load light settings
    _needsSave = false;
    SGroup group(settings, "Lighting");
    KeyMap currentMap = _map;
    _map = KeyMap::fromName(settings.value("KeyMap").toString());
    _dimming = settings.value("Brightness").toUInt();
    if(_dimming > MAX_DIM)
        _dimming = MAX_DIM;
    // Load RGB settings
    bool useReal = settings.value("UseRealNames").toBool();
    {
        SGroup group(settings, "Keys");
        foreach(QString key, settings.childKeys()){
            QString name = key.toLower();
            if(!useReal)
                name = _map.fromStorage(name);
            QColor color = settings.value(key).toString();
            if(!color.isValid())
                color = QColor(255, 255, 255);
            _qColorMap[name] = color.rgb();
        }
        _needsMapRefresh = true;
    }
    // Load animations
    foreach(KbAnim* anim, _animList)
        anim->deleteLater();
    _animList.clear();
    {
        SGroup group(settings, "Animations");
        foreach(QString anim, settings.value("List").toStringList()){
            QUuid id = anim;
            _animList.append(new KbAnim(this, _map, id, settings));
        }
    }
    emit didLoad();
    map(currentMap);
}

void KbLight::save(CkbSettings& settings){
    _needsSave = false;
    SGroup group(settings, "Lighting");
    settings.setValue("KeyMap", _map.name());
    settings.setValue("Brightness", _dimming);
    settings.setValue("UseRealNames", true);
    {
        // Save RGB settings
        SGroup group(settings, "Keys");
        QMutableColorMapIterator i(_qColorMap);
        while(i.hasNext()){
            i.next();
            settings.setValue(i.key(), QColor(i.value()).name());
        }
    }
    {
        // Save animations
        SGroup group(settings, "Animations");
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

#include <cmath>
#include <QDateTime>
#include <QMetaEnum>
#include <QDebug>
#include "ckbsettings.h"
#include "kbanim.h"

KbAnim::KbAnim(QObject *parent, const KeyMap& map, const QUuid id, CkbSettings& settings) :
    QObject(parent), _script(0), _map(map),
    repeatTime(0), kpRepeatTime(0), stopTime(0), kpStopTime(0), repeatMsec(0), kpRepeatMsec(0),
    _guid(id), _isActive(false), _isActiveKp(false), _needsSave(false)
{
    SGroup group(settings, _guid.toString().toUpper());
    _keys = settings.value("Keys").toStringList();
    // Convert key list from storage names if needed
    if(!settings.value("UseRealNames").toBool()){
        QMutableListIterator<QString> i(_keys);
        while(i.hasNext()){
            i.next();
            QString& key = i.value();
            key = _map.fromStorage(key);
        }
    }
    _name = settings.value("Name").toString().trimmed();
    _opacity = settings.value("Opacity").toString().toDouble();
    if(_opacity < 0.)
        _opacity = 0.;
    else if(_opacity > 1.)
        _opacity = 1.;
    bool modeOk = false;
    _mode = (Mode)metaObject()->enumerator(metaObject()->indexOfEnumerator("Mode")).keysToValue(settings.value("BlendMode").toString().toLatin1(), &modeOk);
    if(!modeOk)
        _mode = Normal;
    _scriptName = settings.value("ScriptName").toString().trimmed();
    _scriptGuid = settings.value("ScriptGuid").toString();
    {
        SGroup group(settings, "Parameters");
        foreach(const QString& param, settings.childKeys())
            _parameters[param.toLower()] = settings.value(param);
    }

    if(!_scriptGuid.isNull()){
        _script = AnimScript::copy(this, _scriptGuid);
        if(_script){
            // Remove nonexistant parameters
            foreach(const QString& name, _parameters.keys()){
                AnimScript::Param param = _script->param(name);
                if(param.type == AnimScript::Param::INVALID || param.type == AnimScript::Param::LABEL)
                    _parameters.remove(name);
            }
            // Add defaults for unset parameters
            QListIterator<AnimScript::Param> i = _script->paramIterator();
            while(i.hasNext()){
                AnimScript::Param param = i.next();
                if(!_parameters.contains(param.name) && param.type != AnimScript::Param::LABEL)
                    _parameters[param.name] = param.defaultValue;
            }
            _scriptName = _script->name();
            reInit();
        }
    }
}

void KbAnim::save(CkbSettings& settings){
    _needsSave = false;
    settings.beginGroup(_guid.toString().toUpper());
    settings.setValue("UseRealNames", true);
    settings.setValue("Keys", _keys);
    settings.setValue("Name", _name);
    settings.setValue("Opacity", QString::number(_opacity));
    settings.setValue("BlendMode", metaObject()->enumerator(metaObject()->indexOfEnumerator("Mode")).valueToKey(_mode));
    settings.setValue("ScriptName", _scriptName);
    settings.setValue("ScriptGuid", _scriptGuid.toString().toUpper());
    settings.beginGroup("Parameters");
    QMapIterator<QString, QVariant> i(_parameters);
    while(i.hasNext()){
        i.next();
        settings.setValue(i.key(), i.value());
    }
    settings.endGroup();
    settings.endGroup();
}

KbAnim::KbAnim(QObject* parent, const KeyMap& map, const QString& name, const QStringList& keys, const AnimScript* script) :
    QObject(parent),
    _script(AnimScript::copy(this, script->guid())), _map(map), _keys(keys),
    repeatTime(0), kpRepeatTime(0), stopTime(0), kpStopTime(0), repeatMsec(0), kpRepeatMsec(0),
    _guid(QUuid::createUuid()), _name(name), _opacity(1.), _mode(Normal), _isActive(false), _isActiveKp(false), _needsSave(true)
{
    if(_script){
        // Set default parameters
        QListIterator<AnimScript::Param> i = _script->paramIterator();
        while(i.hasNext()){
            AnimScript::Param param = i.next();
            if(param.type != AnimScript::Param::LABEL)
                _parameters[param.name] = param.defaultValue;
        }
        _scriptGuid = script->guid();
        _scriptName = script->name();
        reInit();
    }
}

KbAnim::KbAnim(QObject* parent, const KeyMap& map, const KbAnim& other) :
    QObject(parent),
    _script(AnimScript::copy(this, other.script()->guid())), _scriptGuid(_script->guid()), _scriptName(_script->name()),
    _map(map), _keys(other._keys), _parameters(other._parameters),
    repeatTime(0), kpRepeatTime(0), stopTime(0), kpStopTime(0), repeatMsec(0), kpRepeatMsec(0),
    _guid(other._guid), _name(other._name), _opacity(other._opacity), _mode(other._mode), _isActive(false), _isActiveKp(false), _needsSave(true)
{
    reInit();
}

void KbAnim::parameter(const QString& name, const QVariant& value){
    if(!_script->hasParam(name))
        return;
    _tempParameters[name] = value;
    updateParams();
}

void KbAnim::commitParams(){
    _needsSave = true;
    _parameters = effectiveParams();
    _tempParameters.clear();
}

void KbAnim::resetParams(){
    _tempParameters.clear();
    updateParams();
}

void KbAnim::updateParams(){
    if(_script)
        _script->parameters(effectiveParams());
    repeatKey = "";
}

QMap<QString, QVariant> KbAnim::effectiveParams(){
    QMap<QString, QVariant> res = _parameters;
    // Apply all uncommited parameters
    QMapIterator<QString, QVariant> i(_tempParameters);
    while(i.hasNext()){
        i.next();
        res[i.key()] = i .value();
    }
    return res;
}

void KbAnim::reInit(){
    if(_script)
        _script->init(_map, _keys, effectiveParams());
    repeatKey = "";
    _isActive = _isActiveKp = false;
}

void KbAnim::map(const KeyMap& newMap){
    _map = newMap;
    reInit();
}

void KbAnim::keys(const QStringList& newKeys){
    _keys = newKeys;
    reInit();
}

void KbAnim::catchUp(quint64 timestamp){
    QMap<QString, QVariant> parameters = effectiveParams();
    // Stop the animation if its time has run out
    if(stopTime != 0 && timestamp >= stopTime){
        repeatMsec = repeatTime = 0;
        if(!parameters.contains("repeat")){
            // If repeats aren't allowed, stop the animation entirely
            _script->end();
            _isActive = false;
            return;
        } else
            // Otherwise, simply stop repeating
            stopTime = 0;
    }
    if(kpStopTime != 0 && timestamp >= kpStopTime){
        kpRepeatMsec = kpRepeatTime = 0;
        if(!parameters.contains("kprepeat")){
            _script->end();
            _isActiveKp = false;
            return;
        } else
            kpStopTime = 0;
    }

    // Restart (or start, if there was a delay) the animation if its repeat time is up
    while(repeatTime > 0 && timestamp >= repeatTime){
        _script->retrigger(repeatTime);
        if(repeatMsec <= 0){
            repeatTime = 0;
            break;
        }
        repeatTime += repeatMsec;
    }
    while(kpRepeatTime > 0 && timestamp >= kpRepeatTime){
        _script->keypress(repeatKey, 1, kpRepeatTime);
        if(kpRepeatMsec <= 0){
            kpRepeatTime = 0;
            break;
        }
        kpRepeatTime += kpRepeatMsec;
    }
}

void KbAnim::trigger(quint64 timestamp, bool ignoreParameter){
    if(!_script)
        return;
    QMap<QString, QVariant> parameters = effectiveParams();

    catchUp(timestamp);
    if(parameters.value("trigger").toBool() || ignoreParameter){
        _isActive = true;
        int delay = round(parameters.value("delay").toDouble() * 1000.);
        if(delay > 0){
            // If delay is enabled, wait to trigger the event
            timestamp += delay;
            repeatTime = timestamp;
        } else
            _script->retrigger(timestamp, true);

        int repeat = round(parameters.value("repeat").toDouble() * 1000.);
        if(repeat <= 0){
            // If no repeat allowed, calculate stop time in seconds
            repeatMsec = -1;
            double stop = parameters.value("stop").toDouble();
            if(stop <= 0)
                stopTime = 0;
            else
                stopTime = timestamp + round(stop * 1000.);
        } else {
            // If repeat is allowed, calculate stop time in repetitions
            repeatMsec = repeat;
            if(delay <= 0)
                repeatTime = timestamp + repeat;
            int stop = parameters.value("stop").toInt();
            if(stop < 0)
                stopTime = 0;
            else
                stopTime = timestamp + repeat * (1 + stop);
        }
    }
    // Ask the script for a frame even if we didn't do anything here. This way ckb knows the script is responding.
    _script->frame(timestamp);
}

void KbAnim::keypress(const QString& key, bool pressed, quint64 timestamp){
    if(!_script)
        return;
    QMap<QString, QVariant> parameters = effectiveParams();
    if(pressed && parameters.value("kpmodestop").toBool()){
        // If stop on key press is enabled, stop mode-wide animation
        catchUp(timestamp);
        _script->stop(timestamp);
        stopTime = repeatTime = repeatMsec = 0;
        _isActive = false;
    } else {
        if(!parameters.value("kptrigger").toBool())
            return;
        catchUp(timestamp);
    }

    if(pressed){
        // Key pressed
        _isActiveKp = true;
        if(parameters.value("kpmode", 0).toInt() == 2 && isActive())
            // If mode is start once and a key has already been pressed, do nothing
            return;
        int delay = round(parameters.value("kpdelay").toDouble() * 1000.);
        if(delay > 0){
            // If delay is enabled, wait to trigger the event
            timestamp += delay;
            kpRepeatTime = timestamp;
        } else
            _script->keypress(key, pressed, timestamp);

        int repeat = round(parameters.value("kprepeat").toDouble() * 1000.);
        if(repeat <= 0){
            // If no repeat allowed, calculate stop time in seconds
            kpRepeatMsec = -1;
            double stop = parameters.value("kpstop").toDouble();
            if(stop <= 0.)
                kpStopTime = 0;
            else
                kpStopTime = timestamp + round(stop * 1000.);
        } else {
            // If repeat is allowed, calculate stop time in repetitions
            kpRepeatMsec = repeat;
            if(delay <= 0)
                kpRepeatTime = timestamp + repeat;
            int stop = parameters.value("kpstop").toInt();
            if(stop < 0)
                kpStopTime = 0;
            else
                kpStopTime = timestamp + repeat * (1 + stop);
        }
        repeatKey = key;
    } else {
        // Key released
        _isActiveKp = false;
        _script->keypress(key, pressed, timestamp);
        if(parameters.value("kprelease").toBool())
            // Stop repeating keypress if "Stop on key release" is enabled
            kpStopTime = timestamp;
    }
    _script->frame(timestamp);
}

void KbAnim::stop(){
    if(_script)
        _script->end();
    repeatTime = 0;
    kpRepeatTime = 0;
    repeatMsec = 0;
    kpRepeatMsec = 0;
    stopTime = 0;
    kpStopTime = 0;
    repeatKey = "";
    _isActive = _isActiveKp = false;
}

bool KbAnim::isRunning() const {
    if(!_script)
        return true;    // If the script wasn't loaded, pretend it's running anyway so it won't lock up
    return _script->hasFrame();
}

// Blending functions

static float blendNormal(float bg, float fg){
    return fg;
}

static float blendAdd(float bg, float fg){
    float res = bg + fg;
    if(res > 1.f)
        res = 1.f;
    return res;
}

static float blendSubtract(float bg, float fg){
    float res = bg - fg;
    if(res < 0.f)
        res = 0.f;
    return res;
}

static float blendMultiply(float bg, float fg){
    return bg * fg;
}

static float blendDivide(float bg, float fg){
    float res = bg / fg;
    if(res > 1.f)
        res = 1.f;
    return res;
}

typedef float (*blendFunc)(float,float);
static blendFunc functions[5] = { blendNormal, blendAdd, blendSubtract, blendMultiply, blendDivide };

void KbAnim::blend(ColorMap& animMap, quint64 timestamp){
    if(!_script)
        return;

    // Fetch the next frame from the script
    catchUp(timestamp);
    _script->frame(timestamp);

    // Blend the script's map with the current map
    int blendMode = (int)_mode;
    blendFunc blend = functions[blendMode];
    float fOpacity = _opacity / 255.f;  // save some math by pre-dividing the 255 for qAlpha

    const ColorMap& scriptMap = _script->colors();
    int count = animMap.count();
    if(scriptMap.count() != count){
        qDebug() << "Script map didn't match base map (" << count << " vs " << scriptMap.count() << "). This should never happen.";
        return;
    }
    QRgb* background = animMap.colors();
    const QRgb* foreground = scriptMap.colors();
    for(int i = 0; i < count; i++){
        // Mix the colors in with the color map according to blend mode and alpha
        QRgb& bg = background[i];
        QRgb fg = foreground[i];
        int alpha = qAlpha(fg);
        if(alpha == 0)
            continue;
        if(blendMode == 0){
            // Blend: normal
            // This is the most common use case and it requires much less arithmetic
            if(alpha == 255){
                bg = fg;
            } else {
                float r = qRed(bg), g = qGreen(bg), b = qBlue(bg);
                float a = alpha * fOpacity;
                r = r * (1.f - a) + qRed(fg) * a;
                g = g * (1.f - a) + qGreen(fg) * a;
                b = b * (1.f - a) + qBlue(fg) * a;
                bg = qRgb(round(r), round(g), round(b));
            }
        } else {
            // Use blend function
            float r = qRed(bg) / 255.f, g = qGreen(bg) / 255.f, b = qBlue(bg) / 255.f;
            float a = alpha * fOpacity;
            r = r * (1.f - a) + blend(r, qRed(fg) / 255.f) * a;
            g = g * (1.f - a) + blend(g, qGreen(fg) / 255.f) * a;
            b = b * (1.f - a) + blend(b, qBlue(fg) / 255.f) * a;
            bg = qRgb(round(r * 255.f), round(g * 255.f), round(b * 255.f));
        }
    }
}

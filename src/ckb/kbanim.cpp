#include <QMetaEnum>
#include "kbanim.h"

KbAnim::KbAnim(QObject *parent, const KeyMap& map, const QUuid id, QSettings& settings) :
    QObject(parent), _script(0), _map(map), _guid(id)
{
    settings.beginGroup(_guid.toString().toUpper());
    _keys = settings.value("Keys").toStringList();
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
    settings.endGroup();

    if(!_scriptGuid.isNull()){
        _script = AnimScript::copy(this, _scriptGuid);
        if(_script){
            _scriptName = _script->name();
            _script->init(_map, _keys, 2.0);
        }
    }
}

void KbAnim::save(QSettings& settings){
    settings.beginGroup(_guid.toString().toUpper());
    settings.setValue("Keys", _keys);
    settings.setValue("Name", _name);
    settings.setValue("Opacity", QString::number(_opacity));
    settings.setValue("BlendMode", metaObject()->enumerator(metaObject()->indexOfEnumerator("Mode")).valueToKey(_mode));
    settings.setValue("ScriptName", _scriptName);
    settings.setValue("ScriptGuid", _scriptGuid.toString().toUpper());
    settings.endGroup();
}

KbAnim::KbAnim(QObject* parent, const KeyMap& map, const QStringList& keys, const AnimScript* script) :
    QObject(parent),
    _script(AnimScript::copy(this, script->guid())), _map(map), _keys(keys),
    _guid(QUuid::createUuid()), _name(_script ? _script->name() : ""), _opacity(1.), _mode(Normal)
{
    if(_script){
        _script->init(_map, _keys, 2.0);
        _scriptGuid = script->guid();
        _scriptName = script->name();
    }
}

void KbAnim::map(const KeyMap& newMap){
    _map = newMap;
    if(_script)
        _script->init(newMap, _keys, 2.0);
}

void KbAnim::keys(const QStringList& newKeys){
    _keys = newKeys;
    if(_script)
        _script->init(_map, newKeys, 2.0);
}

void KbAnim::trigger(){
    if(_script)
        _script->retrigger();
}

void KbAnim::stop(){
    if(_script){
        _script->stop();
        _script->resetPosition();
    }
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

void KbAnim::blend(KeyMap& colorMap){
    if(!_script || _opacity == 0.f)
        return;
    // Fetch the next frame from the script
    _script->frame();
    QHash<QString, QRgb> mix = _script->colors();
    foreach(QString key, mix.keys()){
        // Mix the colors in with the color map according to blend mode and alpha
        QColor bg = colorMap.color(key);
        if(!bg.isValid())
            continue;
        QColor fg = QColor::fromRgba(mix.value(key));
        blendFunc f = functions[(int)_mode];
        float r = bg.redF(), g = bg.greenF(), b = bg.blueF();
        float a = fg.alphaF() * _opacity;
        r = r * (1.f - a) + f(r, fg.redF()) * a;
        g = g * (1.f - a) + f(g, fg.greenF()) * a;
        b = b * (1.f - a) + f(b, fg.blueF()) * a;
        colorMap.color(key, QColor::fromRgbF(r, g, b));
    }
}

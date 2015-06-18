#include "kbmode.h"
#include "kb.h"

KbMode::KbMode(Kb* parent, const KeyMap& keyMap, const QString &guid, const QString& modified) :
    QObject(parent),
    _name("Unnamed"), _id(guid, modified),
    _light(new KbLight(this, keyMap)), _bind(new KbBind(this, parent, keyMap)), _perf(new KbPerf(this)),
    _needsSave(true)
{
    connect(_light, SIGNAL(updated()), this, SLOT(doUpdate()));
    if(_id.guid.isNull())
        _id.guid = QUuid::createUuid();
}

KbMode::KbMode(Kb* parent, const KeyMap& keyMap, const KbMode& other) :
    QObject(parent),
    _name(other._name), _id(other._id),
    _light(new KbLight(this, keyMap, *other._light)), _bind(new KbBind(this, parent, keyMap, *other._bind)), _perf(new KbPerf(this, *other._perf)),
    _needsSave(true)
{
    connect(_light, SIGNAL(updated()), this, SLOT(doUpdate()));
}

KbMode::KbMode(Kb* parent, const KeyMap& keyMap, CkbSettings& settings) :
    QObject(parent),
    _name(settings.value("Name").toString().trimmed()),
    _id(settings.value("GUID").toString().trimmed(), settings.value("Modified").toString().trimmed()),
    _light(new KbLight(this, keyMap)), _bind(new KbBind(this, parent, keyMap)), _perf(new KbPerf(this)),
    _needsSave(false)
{
    if(settings.contains("HwModified"))
        _id.hwModifiedString(settings.value("HwModified").toString());
    else
        _id.hwModified = _id.modified;

    connect(_light, SIGNAL(updated()), this, SLOT(doUpdate()));
    if(_id.guid.isNull())
        _id.guid = QUuid::createUuid();
    if(_name == "")
        _name = "Unnamed";
    _light->load(settings);
    _bind->load(settings);
    _perf->load(settings);
}

void KbMode::newId(){
    _needsSave = true;
    _id = UsbId();
    // Create new IDs for animations
    foreach(KbAnim* anim, _light->animList())
        anim->newId();
}

void KbMode::keyMap(const KeyMap &keyMap){
    _needsSave = true;
    _light->map(keyMap);
    _bind->map(keyMap);
}

void KbMode::save(CkbSettings& settings){
    _needsSave = false;
    _id.newModified();
    settings.setValue("GUID", _id.guidString());
    settings.setValue("Modified", _id.modifiedString());
    settings.setValue("HwModified", _id.hwModifiedString());
    settings.setValue("Name", _name);
    _light->save(settings);
    _bind->save(settings);
    _perf->save(settings);
}

bool KbMode::needsSave() const {
    return _needsSave || _light->needsSave() || _bind->needsSave() || _perf->needsSave();
}

void KbMode::doUpdate(){
    emit updated();
}

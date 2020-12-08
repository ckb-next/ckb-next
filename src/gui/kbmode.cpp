#include "kbmode.h"
#include "kb.h"
#include <typeinfo>

KbMode::KbMode(Kb* parent, const KeyMap& keyMap, const QString &guid, const QString& modified) :
    QObject(parent),
    _name("Unnamed"), _usbId(guid, modified),
    _light(new KbLight(this, keyMap)), _bind(new KbBind(this, parent, keyMap)), _perf(new KbPerf(this)),
    _winInfo(new KbWindowInfo(this)),
    _needsSave(true)
{
    connect(_light, SIGNAL(updated()), this, SLOT(doUpdate()));
    if(_usbId.guid.isNull())
        _usbId.guid = QUuid::createUuid();
}

///
/// \brief KbMode::KbMode
/// \param parent   Kb as parent (append to the Keyboard list
/// \param keyMap   Map to copy from
/// \param other    Mode to copy from
/// Constructor to copy an existing Keyboard-Mode KbMode &other
KbMode::KbMode(Kb* parent, const KeyMap& keyMap, const KbMode& other) :
    QObject(parent),
    _name(other._name), _usbId(other._usbId),
    _light(new KbLight(this, keyMap, *other._light)), _bind(new KbBind(this, parent, keyMap, *other._bind)), _perf(new KbPerf(this, *other._perf)),
    _winInfo(new KbWindowInfo(this, *other._winInfo)),
    _needsSave(true)
{
    connect(_light, SIGNAL(updated()), this, SLOT(doUpdate()));
}

KbMode::KbMode(Kb* parent, const KeyMap& keyMap, CkbSettingsBase& settings) :
    QObject(parent),
    _name(settings.value("Name").toString().trimmed()),
    _usbId(settings.value("GUID").toString().trimmed(), settings.value("Modified").toString().trimmed()),
    _light(new KbLight(this, keyMap)), _bind(new KbBind(this, parent, keyMap)), _perf(new KbPerf(this)),
    _winInfo(new KbWindowInfo(this)),
    _needsSave(false)
{
    if(settings.contains("HwModified"))
        _usbId.hwModifiedString(settings.value("HwModified").toString());
    else
        _usbId.hwModified = _usbId.modified;

    connect(_light, SIGNAL(updated()), this, SLOT(doUpdate()));
    if(_usbId.guid.isNull())
        _usbId.guid = QUuid::createUuid();
    if(_name == "")
        _name = "Unnamed";
    _light->load(settings);
    _bind->load(settings);
    _perf->load(settings);
    _winInfo->load(settings);
}

void KbMode::newId(){
    _needsSave = true;
    _usbId = UsbId();
    // Create new IDs for animations
    foreach(KbAnim* anim, _light->animList())
        anim->newId();
}

void KbMode::keyMap(const KeyMap &keyMap){
    _needsSave = true;
    _light->map(keyMap);
    _bind->map(keyMap);
}

void KbMode::save(CkbSettingsBase& settings){
    if(typeid(settings) == typeid(CkbSettings)){
        _needsSave = false;
        _usbId.newModified();
    }
    settings.setValue("GUID", _usbId.guidString());
    settings.setValue("Modified", _usbId.modifiedString());
    settings.setValue("HwModified", _usbId.hwModifiedString());
    settings.setValue("Name", _name);
    _light->save(settings);
    _bind->save(settings);
    _perf->save(settings);
    _winInfo->save(settings);
}

bool KbMode::needsSave() const {
    return _needsSave || _light->needsSave() || _bind->needsSave() || _perf->needsSave() || _winInfo->needsSave();
}

void KbMode::doUpdate(){
    emit updated();
}

///
/// \brief KbMode::~KbMode
/// Destructor may be used for Debugging (issue #38 with SIGSEGV). Insert qDebug-statement
KbMode::~KbMode() {
}

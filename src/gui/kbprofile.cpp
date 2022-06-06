#include "kbprofile.h"
#include "kb.h"
#include <typeinfo>

KbProfile::KbProfile(Kb* parent, const KeyMap& keyMap, const KbProfile& other) :
    QObject(parent), _currentMode(nullptr), _name(other._name), _usbId(other._usbId), _keyMap(keyMap), _needsSave(true)
{
    foreach(KbMode* mode, other.modes()){
        KbMode* newMode = new KbMode(parent, keyMap, *mode);
        if(!_currentMode || mode == other._currentMode)
            _currentMode = newMode;
        _modes.append(newMode);
    }
}

KbProfile::KbProfile(Kb* parent, const KeyMap& keyMap, const QString& guid, const QString& modified) :
    QObject(parent), _currentMode(nullptr), _usbId(guid, modified.toUInt(nullptr, 16)), _keyMap(keyMap), _needsSave(true)
{
    if(_usbId.guid.isNull())
        _usbId.guid = QUuid::createUuid();
}

KbProfile::KbProfile(Kb* parent, const KeyMap& keyMap, CkbSettingsBase& settings, const QString& guid) :
    QObject(parent), _currentMode(nullptr), _usbId(guid, 0), _keyMap(keyMap), _needsSave(false)
{
    // Load data from preferences
    // If we're importing external profiles, then always read the GUID from the first group.
    // This is done because we can import profiles with existing GUIDs, but assign new GUIDs without replacing them.
    const QString importGuid = (typeid(settings) == typeid(CkbExternalSettings) ? settings.childGroups().first() : guid);
    SGroup group(settings, importGuid);
    _name = settings.value("Name").toString().trimmed();
    if(_name == "")
        _name = "Unnamed";
    _usbId.modifiedString(settings.value("Modified").toString());
    if(settings.contains("HwModified"))
        _usbId.hwModifiedString(settings.value("HwModified").toString());
    else
        _usbId.hwModified = _usbId.modified;
    QUuid current = settings.value("CurrentMode").toString().trimmed();
    // Load modes
    uint count = settings.value("ModeCount").toUInt();
    for(uint i = 0; i < count; i++){
        SGroup subGroup(settings, QString::number(i));
        KbMode* mode = new KbMode(parent, _keyMap, settings);
        _modes.append(mode);
        // Set currentMode to the mode matching the current GUID, or the first mode in case it's not found
        if(current == mode->id().guid || !_currentMode)
            _currentMode = mode;
    }
}

void KbProfile::save(CkbSettingsBase& settings){
    if(typeid(settings) == typeid(CkbSettings)){
        _needsSave = false;
        _usbId.newModified();
    }
    // Save data to preferences
    SGroup group(settings, id().guidString());
    settings.setValue("Name", name());
    settings.setValue("Modified", _usbId.modifiedString());
    settings.setValue("HwModified", _usbId.hwModifiedString());
    if(_currentMode)
        settings.setValue("CurrentMode", _currentMode->id().guidString());
    // Save modes
    uint count = modeCount();
    settings.setValue("ModeCount", count);
    for(uint i = 0; i < count; i++){
        SGroup subGroup(settings, QString::number(i));
        KbMode* mode = _modes.at(i);
        mode->save(settings);
    }
}

bool KbProfile::needsSave() const {
    if(_needsSave)
        return true;
    foreach(KbMode* mode, _modes){
        if(mode->needsSave())
            return true;
    }
    return false;
}

void KbProfile::newId(){
    _needsSave = true;
    _usbId = UsbId();
    foreach(KbMode* mode, _modes)
        mode->newId();
}

void KbProfile::keyMap(const KeyMap& newKeyMap){
    _keyMap = newKeyMap;
    foreach(KbMode* mode, _modes)
        mode->keyMap(newKeyMap);
    setNeedsUpdate();
}

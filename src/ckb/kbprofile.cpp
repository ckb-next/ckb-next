#include "kbprofile.h"
#include "kb.h"

KbProfile::KbProfile(Kb* parent, const KeyMap& keyMap, const KbProfile& other) :
    QObject(parent), currentMode(0), _name(other._name), _id(other._id), _keyMap(keyMap)
{
    foreach(KbMode* mode, other.modes){
        KbMode* newMode = new KbMode(parent, keyMap, *mode);
        if(!currentMode || mode == other.currentMode)
            currentMode = newMode;
        modes.append(newMode);
    }
}

KbProfile::KbProfile(Kb* parent, const KeyMap& keyMap, const QString& guid, const QString& modified) :
    QObject(parent), currentMode(0), _id(guid, modified.toUInt(0, 16)), _keyMap(keyMap)
{
    if(_id.guid.isNull())
        _id.guid = QUuid::createUuid();
}

KbProfile::KbProfile(Kb* parent, const KeyMap& keyMap, QSettings& settings, const QString& guid) :
    QObject(parent), currentMode(0), _id(guid, 0), _keyMap(keyMap)
{
    // Load data from preferences
    settings.beginGroup(guid);
    _name = settings.value("Name").toString().trimmed();
    if(_name == "")
        _name = "Unnamed";
    _id.modifiedString(settings.value("Modified").toString());
    QUuid current = settings.value("CurrentMode").toString().trimmed();
    // Load modes
    uint count = settings.value("ModeCount").toUInt();
    for(uint i = 0; i < count; i++){
        settings.beginGroup(QString::number(i));
        KbMode* mode = new KbMode(parent, _keyMap, settings);
        modes.append(mode);
        // Set currentMode to the mode matching the current GUID, or the first mode in case it's not found
        if(current == mode->id().guid || !currentMode)
            currentMode = mode;
        settings.endGroup();
    }
    settings.endGroup();
}

void KbProfile::save(QSettings& settings){
    // Save data to preferences
    settings.beginGroup(id().guidString());
    settings.setValue("Name", name());
    settings.setValue("Modified", _id.modifiedString());
    if(currentMode)
        settings.setValue("CurrentMode", currentMode->id().guidString());
    // Save modes
    uint count = modes.count();
    settings.setValue("ModeCount", count);
    for(uint i = 0; i < count; i++){
        settings.beginGroup(QString::number(i));
        KbMode* mode = modes[i];
        mode->save(settings);
        settings.endGroup();
    }
    settings.endGroup();
}

void KbProfile::newId(){
    _id = UsbId();
    foreach(KbMode* mode, modes)
        mode->newId();
}

void KbProfile::keyMap(const KeyMap& newKeyMap){
    _keyMap = newKeyMap;
    foreach(KbMode* mode, modes)
        mode->keyMap(newKeyMap);
}

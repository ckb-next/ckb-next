#include "kbprofile.h"

KbProfile::KbProfile(QObject* parent, const KeyMap& keyMap, const QString& guid, const QString& modified) :
    QObject(parent), id(guid, modified.toUInt(0, 16)), _keyMap(keyMap), _currentMode(0), _hwAssigned(true)
{
}

KbProfile::KbProfile(QObject* parent, const KeyMap& keyMap, QSettings& settings, const QString& guid) :
    QObject(parent), id(guid, 0), _keyMap(keyMap), _hwAssigned(true)
{
    // Load data from preferences
    settings.beginGroup(guid);
    _name = settings.value("Name").toString().trimmed();
    if(_name == "")
        _name = "Unnamed";
    modified(settings.value("Modified").toString());
    _currentMode = settings.value("CurrentMode").toUInt();
    // Load modes
    uint count = settings.value("ModeCount").toUInt();
    for(uint i = 0; i < count; i++){
        settings.beginGroup(QString::number(i));
        QString guid = settings.value("GUID").toString().trimmed();
        QString modified = settings.value("Modified").toString().trimmed();
        QString name = settings.value("Name").toString().trimmed();
        if(guid != "" && modified != "" && name != ""){
            modeNames.append(name);
            modeIds.append(UsbId(guid, modified.toUInt(0, 16)));
            KbLight* light = new KbLight(this, modeLights.length(), _keyMap);
            modeLights.append(light);
            // Load lighting
            light->load(settings);
        }
        settings.endGroup();
    }
    if(_currentMode >= modeCount())
        _currentMode = modeCount() - 1;
    settings.endGroup();
}

void KbProfile::save(QSettings& settings){
    // Save data to preferences
    settings.beginGroup(guid());
    settings.setValue("Name", name());
    settings.setValue("Modified", modified());
    settings.setValue("CurrentMode", currentMode());
    // Save modes
    uint count = modeCount();
    settings.setValue("ModeCount", count);
    for(uint i = 0; i < count; i++){
        settings.beginGroup(QString::number(i));
        settings.setValue("GUID", modeGuid(i));
        settings.setValue("Modified", modeModified(i));
        settings.setValue("Name", modeName(i));
        modeLight(i)->save(settings);
        settings.endGroup();
    }
    settings.endGroup();
}

KbProfile::KbProfile(QObject *parent, KbProfile& other) :
    QObject(parent), _name(other._name), id(other.id), _keyMap(other._keyMap), _currentMode(other._currentMode)
{
    int count = other.modeCount();
    for(int i = 0; i < count; i++){
        modeNames.append(other.modeName(i));
        modeLights.append(new KbLight(this, modeLights.length(), other.modeLight(i)->map()));
        modeIds.append(UsbId(other.modeGuid(i), other.modeModified(i).toUInt(0, 16)));
    }
}

void KbProfile::keyMap(const KeyMap& newKeyMap){
    _keyMap = newKeyMap;
    int count = modeCount();
    for(int i = 0; i < count; i++)
        modeLight(i)->map(newKeyMap);
}

QString KbProfile::modeName(int mode) const {
    if(modeCount() <= mode)
        return "";
    return modeNames[mode];
}

void KbProfile::modeName(int mode, const QString& newName){
    while(modeCount() <= mode)
        modeNames.append("");
    modeNames[mode] = newName;
}

KbLight* KbProfile::modeLight(int mode){
    while(modeLights.length() <= mode)
        modeLights.append(new KbLight(this, modeLights.length(), _keyMap));
    return modeLights[mode];
}

QString KbProfile::modeGuid(int mode) {
    while(modeIds.length() <= mode)
        modeIds.append(UsbId());
    return modeIds[mode].guid.toString().toUpper();
}

void KbProfile::modeGuid(int mode, const QString& newGuid){
    while(modeIds.length() <= mode)
        modeIds.append(UsbId());
    modeIds[mode].guid = newGuid;
}

QString KbProfile::modeModified(int mode) {
    while(modeIds.length() <= mode)
        modeIds.append(UsbId());
    return QString::number(modeIds[mode].modified, 16);
}

void KbProfile::modeModified(int mode, const QString& newModified){
    while(modeIds.length() <= mode)
        modeIds.append(UsbId());
    modeIds[mode].modified = newModified.toUInt(0, 16);
}

void KbProfile::moveMode(int from, int to){
    if(from == to)
        return;
    QString name = modeName(from);
    KbLight* light = modeLight(from);
    modeNames.removeAt(from);
    modeLights.removeAt(from);
    while(modeIds.length() <= from)
        modeIds.append(UsbId());
    UsbId id = modeIds.takeAt(from);
    modeNames.insert(to, name);
    modeLights.insert(to, light);
    modeIds.insert(to, id);
    // Recompute mode indices
    int i = 0;
    foreach(KbLight* light, modeLights)
        light->modeIndex(i++);
}

void KbProfile::duplicateMode(int mode){
    QString name = modeName(mode) + " copy";
    KbLight* light = new KbLight(this, mode + 1, modeLight(mode)->map());
    modeNames.insert(mode + 1, name);
    modeLights.insert(mode + 1, light);
    modeIds.insert(mode + 1, UsbId());
    // Recompute mode indices
    int i = 0;
    foreach(KbLight* light, modeLights)
        light->modeIndex(i++);
}

void KbProfile::deleteMode(int mode){
    modeNames.removeAt(mode);
    modeLights.takeAt(mode)->deleteLater();
    modeIds.removeAt(mode);
    if(currentMode() >= modeCount())
        currentMode(currentMode() - 1);
    // Recompute mode indices
    int i = 0;
    foreach(KbLight* light, modeLights)
        light->modeIndex(i++);
}

#include "ckbsettings.h"

CkbSettings::CkbSettings(){
}

CkbSettings::CkbSettings(const QString& basePath, bool eraseExisting){
    if(eraseExisting)
        remove(basePath);
    beginGroup(basePath);
}

CkbSettings::~CkbSettings(){
}

QVariant CkbSettings::get(const QString& key, const QVariant& defaultValue){
    CkbSettings settings;
    return settings.value(key, defaultValue);
}

void CkbSettings::set(const QString& key, const QVariant& value){
    CkbSettings settings;
    settings.setValue(key, value);
}

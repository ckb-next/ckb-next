#include "ckbsettings.h"
#include "kbwindowinfo.h"
#include "kbmode.h"

KbWindowInfo::KbWindowInfo(KbMode* parent) :
    QObject(parent), items(), enabled(false)
{
    _needsSave = true;
}

KbWindowInfo::KbWindowInfo(KbMode* parent, const KbWindowInfo &other) :
    QObject(parent), items(other.items), enabled(other.enabled)
{
    _needsSave = true;
}

void KbWindowInfo::load(CkbSettingsBase& settings){
    if(typeid(settings) == typeid(CkbSettings))
        _needsSave = false;

    SGroup group(settings, "WindowInfo");
    enabled = settings.value("enabled", false).toBool();
    SGroup rulegroup(settings, "Rules");
    QStringList children = settings.childGroups();
    for(int i = 0; i < children.count(); i++){
        SGroup currentrule(settings, QString::number(i));
        MatchPair mp;
        mp.type = static_cast<MatchType>(settings.value("type").toInt());
        mp.item = settings.value("item").toString();
        mp.flags = MatchFlags(settings.value("flags").toInt());
        mp.op = static_cast<MatchOperator>(settings.value("op").toInt());
        items.append(mp);
    }
}

void KbWindowInfo::save(CkbSettingsBase& settings){
    if(typeid(settings) == typeid(CkbSettings))
        _needsSave = false;

    SGroup group(settings, "WindowInfo");
    settings.setValue("enabled", enabled);
    SGroup rulegroup(settings, "Rules");
    for(int i = 0; i < items.count(); i++){
        SGroup currentrule(settings, QString::number(i));
        const MatchPair& mp = items.at(i);
        settings.setValue("type", mp.type);
        settings.setValue("item", mp.item);
        settings.setValue("flags", static_cast<int>(mp.flags));
        settings.setValue("op", static_cast<int>(mp.op));
    }
}

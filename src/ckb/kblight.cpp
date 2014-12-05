#include <cmath>
#include "kblight.h"

KbLight::KbLight(QObject* parent, int modeIndex, const KeyMap& map) :
    QObject(parent), _fgColor(255, 255, 255), _map(map),
    _modeIndex(modeIndex + 1), _brightness(0), _inactive(MAX_INACTIVE), _animation(0), _winLock(false),
    animPos(-36.f)
{
}

extern int framerate;

void KbLight::animWave(const QStringList& keys, KeyMap& colorMap){
    float fgr = _fgColor.redF();
    float fgg = _fgColor.greenF();
    float fgb = _fgColor.blueF();
    float size = colorMap.width() + 36.f;
    foreach(QString key, keys){
        const KeyPos& pos = *colorMap.key(key);
        QColor bg = colorMap.color(key);
        float r = bg.redF();
        float g = bg.greenF();
        float b = bg.blueF();
        float distance = fabs(pos.x - animPos);
        if(distance <= 36.f){
            r = r * distance / 36.f + fgr * (1.f - distance / 36.f);
            g = g * distance / 36.f + fgg * (1.f - distance / 36.f);
            b = b * distance / 36.f + fgb * (1.f - distance / 36.f);
        }
        colorMap.color(key, QColor::fromRgbF(r, g, b));
    }
    animPos += (size + 36.f) / 2.f / (float)framerate;
    if(animPos >= size)
        animPos = -36.f;
}

void KbLight::animRipple(const QStringList& keys, KeyMap& colorMap){
    float fgr = _fgColor.redF();
    float fgg = _fgColor.greenF();
    float fgb = _fgColor.blueF();
    float size = sqrt(((double)_map.width())*_map.width()/2. + ((double)_map.height())*_map.height()/2.);
    float cx = _map.width() / 2.f, cy = _map.height() / 2.f;
    foreach(QString key, keys){
        const KeyPos& pos = *colorMap.key(key);
        QColor bg = colorMap.color(key);
        float r = bg.redF();
        float g = bg.greenF();
        float b = bg.blueF();
        float distance = fabs(sqrt(pow(pos.x - cx, 2.) + pow(pos.y - cy, 2.)) - animPos);
        if(distance <= 36.f){
            r = r * distance / 36.f + fgr * (1.f - distance / 36.f);
            g = g * distance / 36.f + fgg * (1.f - distance / 36.f);
            b = b * distance / 36.f + fgb * (1.f - distance / 36.f);
        }
        colorMap.color(key, QColor::fromRgbF(r, g, b));
    }
    animPos += (size + 36.f) / (float)framerate;
    if(animPos >= size)
        animPos = -36.f;
}

void KbLight::printRGB(QFile& cmd, KeyMap& colorMap){
    cmd.write(" rgb on");
    uint count = colorMap.count();
    for(uint i = 0; i < count; i++){
        const KeyPos& pos = *colorMap.key(i);
        QColor color = colorMap.color(i);
        cmd.write(QString().sprintf(" %s:%02x%02x%02x", pos.name, color.red(), color.green(), color.blue()).toLatin1());
    }
}

void KbLight::frameUpdate(QFile& cmd, bool dimMute){
    cmd.write(QString().sprintf("mode %d switch", _modeIndex).toLatin1());
    cmd.write(" notify mr m1 m2 m3 light lock");
    if(_brightness == 3){
        cmd.write(" rgb off\n");
        return;
    }
    KeyMap colorMap = _map;
    float light = (3 - _brightness) / 3.f;
    QStringList inactiveList = QStringList();
    float inactiveLight = 1.f;
    if(_inactive >= 0){
        inactiveList << "mr" << "m1" << "m2" << "m3";
        if(!_winLock)
            inactiveList << "lock";
        if(dimMute)
            inactiveList << "mute";
        inactiveList.removeAll(QString("m%1").arg(_modeIndex));
        inactiveLight = (2 - _inactive) / 4.f;
    }

    switch(_animation){
    case 1:
        // Wave
        animWave(_animated, colorMap);
        break;
    case 2:
        // Ripple
        animRipple(_animated, colorMap);
        break;
    default:;
    }
    // Dim inactive keys
    if(light != 1.f || inactiveLight != 1.f){
        uint count = colorMap.count();
        for(uint i = 0; i < count; i++){
            const KeyPos& pos = *colorMap.key(i);
            QColor bg = colorMap.color(i);
            float r = bg.redF();
            float g = bg.greenF();
            float b = bg.blueF();
            r *= light;
            g *= light;
            b *= light;
            if(inactiveList.contains(pos.name)){
                r *= inactiveLight;
                g *= inactiveLight;
                b *= inactiveLight;
            }
            colorMap.color(i, QColor::fromRgbF(r, g, b));
        }
    }
    // Apply light
    printRGB(cmd, colorMap);
    cmd.write("\n");
}

void KbLight::close(QFile &cmd){
    animPos = -36.f;
    if(_brightness == MAX_BRIGHTNESS){
        cmd.write(QString().sprintf("mode %d rgb off", _modeIndex).toLatin1());
        return;
    }
    // Set just the background color, ignoring any animation
    cmd.write(QString().sprintf("mode %d", _modeIndex).toLatin1());
    KeyMap colorMap = _map;
    colorMap.color("mr", QColor(0, 0, 0));
    colorMap.color("m1", QColor(0, 0, 0));
    colorMap.color("m2", QColor(0, 0, 0));
    colorMap.color("m3", QColor(0, 0, 0));
    colorMap.color("lock", QColor(0, 0, 0));
    printRGB(cmd, colorMap);
}

void KbLight::winLock(QFile& cmd, bool lock){
    _winLock = lock;
    if(lock)
        cmd.write(QString().sprintf("mode %d unbind lwin rwin", _modeIndex).toLatin1());
    else
        cmd.write(QString().sprintf("mode %d rebind lwin rwin", _modeIndex).toLatin1());
}

void KbLight::load(QSettings& settings){
    settings.beginGroup("Lighting");
    _brightness = settings.value("Brightness").toUInt();
    if(_brightness > MAX_BRIGHTNESS)
        _brightness = MAX_BRIGHTNESS;
    bool inOk = false;
    _inactive = settings.value("InactiveIndicators").toInt(&inOk);
    if(!inOk || _inactive > MAX_INACTIVE)
        _inactive = MAX_INACTIVE;
    _animation = settings.value("Animation").toUInt();
    QString clr = settings.value("Foreground").toString();
    if(clr != "")
        _fgColor = QColor(clr);
    // Load RGB settings
    settings.beginGroup("Keys");
    uint count = _map.count();
    for(uint i = 0; i < count; i++){
        QColor color = settings.value(QString(_map.key(i)->name).toUpper()).toString();
        if(!color.isValid())
            color = QColor(0, 0, 0);
        _map.color(i, color);
    }
    settings.endGroup();
    _animated = settings.value("AnimatedKeys").toStringList();
    settings.endGroup();
}

void KbLight::save(QSettings& settings){
    settings.beginGroup("Lighting");
    settings.setValue("Brightness", _brightness);
    settings.setValue("InactiveIndicators", _inactive);
    settings.setValue("Animation", _animation);
    settings.setValue("Foreground", fgColor().name());
    // Save RGB settings
    settings.beginGroup("Keys");
    uint count = _map.count();
    for(uint i = 0; i < count; i++)
        settings.setValue(QString(_map.key(i)->name).toUpper(), _map.color(i).name());
    settings.endGroup();
    settings.setValue("AnimatedKeys", _animated);
    settings.endGroup();
}

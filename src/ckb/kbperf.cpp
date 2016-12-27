#include "kbperf.h"
#include "kbmode.h"
#include "kb.h"
#include "media.h"
#include <cmath>

#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
#define map_last(map) ((map).last())
#else
#define map_last(map) ((map).value((map).keys().last()))
#endif

KbPerf::KbPerf(KbMode* parent) :
    QObject(parent), runningPushIdx(1),
    _iOpacity(1.f), _dpiIndicator(true), _liftHeight(MEDIUM), _angleSnap(false),
    _needsUpdate(true), _needsSave(true) {
    // Default DPI settings
    dpiX[0] = dpiY[0] = 400;
    dpiX[1] = dpiY[1] = 450;
    dpiX[2] = dpiY[2] = 800;
    dpiX[3] = dpiY[3] = 1500;
    dpiX[4] = dpiY[4] = 3000;
    dpiX[5] = dpiY[5] = 6000;
    dpiClr[0] = QColor(255, 0, 0);
    dpiClr[1] = QColor(255, 192, 0);
    dpiClr[2] = QColor(255, 255, 0);
    dpiClr[3] = QColor(0, 255, 0);
    dpiClr[4] = QColor(0, 255, 255);
    dpiClr[5] = QColor(255, 255, 255);
    dpiClr[6] = QColor(192, 192, 192);
    for(int i = 0; i < DPI_COUNT; i++)
        dpiOn[i] = true;
    dpiLastIdx = dpiCurIdx = 3;
    dpiCurX = dpiX[dpiCurIdx];
    dpiCurY = dpiY[dpiCurIdx];
    // Default indicators
    iColor[NUM][0] = iColor[CAPS][0] = iColor[SCROLL][0] = QColor(0, 255, 0);   // Lock lights: on = green, off = black
    iColor[NUM][1] = iColor[CAPS][1] = iColor[SCROLL][1] = QColor(0, 0, 0);
    iColor[MACRO][0] = QColor(255, 0, 0);                                       // Macro: on = red, off = black
    iColor[LOCK][0] = QColor(255, 255, 255);                                    // Win lock: on = white, off = black
    iColor[MODE][0] = iColor[MUTE][0] = QColor(255, 255, 255, 0);               // Mode, mute: on = transparent, off = black
    iColor[MODE][1] = iColor[MACRO][1] = iColor[LOCK][1] = iColor[MUTE][1] = muteNAColor = QColor(0, 0, 0);
    iColor[LIGHT][0] = QColor(255, 0, 0);       // Brightness: red, yellow, white
    iColor[LIGHT][1] = QColor(255, 255, 0);
    light100Color = QColor(255, 255, 255);
    for(int i = 0; i < HW_I_COUNT; i++){
        // Set all lock lights to HW mode
        iEnable[i] = false;
        hwIType[i] = NORMAL;
    }
    for(int i = HW_I_COUNT; i < I_COUNT; i++){
        // Turn all other indicators on
        iEnable[i] = true;
    }
}

KbPerf::KbPerf(KbMode* parent, const KbPerf& other) :
    QObject(parent), dpiCurX(other.dpiCurX), dpiCurY(other.dpiCurY), dpiCurIdx(other.dpiCurIdx), dpiLastIdx(other.dpiLastIdx), runningPushIdx(1),
    _iOpacity(other._iOpacity), light100Color(other.light100Color), muteNAColor(other.muteNAColor), _dpiIndicator(other._dpiIndicator),
    _liftHeight(other._liftHeight), _angleSnap(other._angleSnap),
    _needsUpdate(true), _needsSave(true) {
    memcpy(dpiX, other.dpiX, sizeof(dpiX));
    memcpy(dpiY, other.dpiY, sizeof(dpiY));
    for(int i = 0; i < DPI_COUNT + 1; i++)
        dpiClr[i] = other.dpiClr[i];
    memcpy(dpiOn, other.dpiOn, sizeof(dpiOn));
    for(int i = 0; i < I_COUNT; i++){
        iColor[i][0] = other.iColor[i][0];
        iColor[i][1] = other.iColor[i][1];
    }
    memcpy(iEnable, other.iEnable, sizeof(iEnable));
    memcpy(hwIType, other.hwIType, sizeof(hwIType));
    // Don't copy pushed DPI states. If the other mode has any, restore the original DPI
    if(!other.pushedDpis.isEmpty())
        curDpi(other.pushedDpis[0]);
}

const KbPerf& KbPerf::operator= (const KbPerf& other){
    dpiCurX = other.dpiCurX; dpiCurY = other.dpiCurY; dpiCurIdx = other.dpiCurIdx; dpiLastIdx = other.dpiLastIdx; runningPushIdx = 1;
    _iOpacity = other._iOpacity; light100Color = other.light100Color; muteNAColor = other.muteNAColor; _dpiIndicator = other._dpiIndicator;
    _liftHeight = other._liftHeight; _angleSnap = other._angleSnap;
    _needsUpdate = true; _needsSave = true;
    memcpy(dpiX, other.dpiX, sizeof(dpiX));
    memcpy(dpiY, other.dpiY, sizeof(dpiY));
    for(int i = 0; i < DPI_COUNT + 1; i++)
        dpiClr[i] = other.dpiClr[i];
    memcpy(dpiOn, other.dpiOn, sizeof(dpiOn));
    for(int i = 0; i < I_COUNT; i++){
        iColor[i][0] = other.iColor[i][0];
        iColor[i][1] = other.iColor[i][1];
    }
    memcpy(iEnable, other.iEnable, sizeof(iEnable));
    memcpy(hwIType, other.hwIType, sizeof(hwIType));
    // Don't copy pushed DPI states. If the other mode has any, restore the original DPI
    if(!other.pushedDpis.isEmpty())
        curDpi(other.pushedDpis[0]);
    return other;
}

KbBind* KbPerf::bind() const {
    return modeParent()->bind();
}

KbLight* KbPerf::light() const {
    return modeParent()->light();
}

void KbPerf::load(CkbSettings& settings){
    pushedDpis.clear();
    runningPushIdx = 1;
    _needsSave = false;
    bool readIndicators = true;
    if(!settings.containsGroup("Performance/Indicators")){
        // Read old indicator settings from the lighting group, if present
        // (ckb <= v0.2.0)
        SGroup group(settings, "Lighting");
        if(settings.contains("InactiveIndicators")){
            bool inOk = false;
            int inactive = settings.value("InactiveIndicators").toInt(&inOk);
            if(!inOk || inactive > 2)
                inactive = 2;
            if(inactive == 1)
                _iOpacity = 0.75f;
            else if(inactive == 0)
                _iOpacity = 0.5f;
            else if(inactive < 0){
                // Indicators disabled
                iEnable[MODE] = iEnable[MACRO] = iEnable[LIGHT] = iEnable[LOCK]  = iEnable[MUTE] = false;
            }
            bool showMute = (settings.value("ShowMute").toInt(&inOk) != 0);
            if(inOk && !showMute)
                iEnable[MUTE] = false;
            readIndicators = false;
        }
    }
    SGroup group(settings, "Performance");
    // Read DPI settings
    {
        SGroup group(settings, "DPI");
        for(int i = 0; i < DPI_COUNT; i++){
            QString iStr = QString::number(i);
            QPoint value = settings.value(iStr).toPoint();
            if(value.isNull())
                continue;
            dpiX[i] = value.x(); dpiY[i] = value.y();
            QColor color = settings.value(iStr + "RGB").toString();
            if(color.isValid())
                dpiClr[i] = color;
            if(i != 0)
                dpiOn[i] = !settings.value(iStr + "Disabled").toBool();
        }
        QColor color = settings.value("6RGB").toString();
        if(color.isValid())
            dpiClr[OTHER] = color;
        if(settings.contains("LastIdx")){
            dpiLastIdx = settings.value("LastIdx").toInt();
            if(dpiLastIdx >= DPI_COUNT || dpiLastIdx < 0)
                dpiLastIdx = 1;
        }
        QPoint value = settings.value("Current").toPoint();
        if(!value.isNull())
            curDpi(value);
    }
    // Read misc. mouse settings
    _liftHeight = (height)settings.value("LiftHeight").toInt();
    if(_liftHeight < LOW || _liftHeight > HIGH)
        _liftHeight = MEDIUM;
    _angleSnap = settings.value("AngleSnap").toBool();
    if(settings.contains("NoIndicator")){
        // ckb <= v0.2.0
        _dpiIndicator = !settings.value("NoIndicator").toBool();
    } else {
        _dpiIndicator = settings.value("Indicators/DPI", true).toBool();
    }
    // Read indicator settings
    if(readIndicators){
        SGroup group(settings, "Indicators");
        _iOpacity = settings.value("Opacity", 100).toInt() / 100.f;
        for(int i = 0; i < I_COUNT; i++){
            SGroup group(settings, QString::number(i));
            QColor color = settings.value("RGB0").toString();
            if(color.isValid())
                iColor[i][0] = color;
            color = settings.value("RGB1").toString();
            if(color.isValid())
                iColor[i][1] = color;
            if(i == LIGHT){
                color = settings.value("RGB2").toString();
                if(color.isValid())
                    light100Color = color;
            } else if(i == MUTE){
                color = settings.value("RGB2").toString();
                if(color.isValid())
                    muteNAColor = color;
            }
            if(i <= HW_IMAX){
                iEnable[i] = settings.value("Enable", false).toBool();
                hwIType[i] = (i_hw)settings.value("Hardware", (int)NORMAL).toInt();
            } else {
                iEnable[i] = settings.value("Enable", true).toBool();
            }
        }
    }
    emit didLoad();
}

void KbPerf::save(CkbSettings& settings){
    _needsSave = false;
    SGroup group(settings, "Performance");
    {
        SGroup group(settings, "DPI");
        for(int i = 0; i < DPI_COUNT; i++){
            QString iStr = QString::number(i);
            settings.setValue(iStr, QPoint(dpiX[i], dpiY[i]));
            settings.setValue(iStr + "RGB", dpiClr[i].name(QColor::HexArgb));
            if(i != 0)
                settings.setValue(iStr + "Disabled", !dpiOn[i]);
        }
        settings.setValue("6RGB", dpiClr[OTHER].name(QColor::HexArgb));
        settings.setValue("LastIdx", dpiLastIdx);
        int curX = dpiCurX, curY = dpiCurY;
        // Ignore any pushed modes
        if(pushedDpis.count() > 0){
            curX = pushedDpis.value(0).x();
            curY = pushedDpis.value(0).y();
        }
        settings.setValue("Current", QPoint(curX, curY));
    }
    settings.setValue("LiftHeight", _liftHeight);
    settings.setValue("AngleSnap", _angleSnap);
    {
        SGroup group(settings, "Indicators");
        settings.setValue("DPI", _dpiIndicator);
        for(int i = 0; i < I_COUNT; i++){
            SGroup group(settings, QString::number(i));
            settings.setValue("RGB0", iColor[i][0].name(QColor::HexArgb));
            settings.setValue("RGB1", iColor[i][1].name(QColor::HexArgb));
            if(i == LIGHT)
                settings.setValue("RGB2", light100Color.name(QColor::HexArgb));
            else if(i == MUTE)
                settings.setValue("RGB2", muteNAColor.name(QColor::HexArgb));
            settings.setValue("Enable", iEnable[i]);
            if(i <= HW_IMAX)
                settings.setValue("Hardware", (int)hwIType[i]);
        }
    }
}

void KbPerf::dpi(int index, const QPoint& newValue){
    if(index < 0 || index >= DPI_COUNT)
        return;
    dpiX[index] = newValue.x();
    dpiY[index] = newValue.y();
    // Update current DPI if needed
    if(dpiCurIdx == index){
        dpiCurX = newValue.x();
        dpiCurY = newValue.y();
    }
    _needsUpdate = _needsSave = true;
}

void KbPerf::_curDpi(const QPoint& newDpi){
    dpiCurX = newDpi.x();
    dpiCurY = newDpi.y();
    // Set current/last index
    dpiCurIdx = -1;
    for(int i = 0; i < DPI_COUNT; i++){
        if(dpiCurX == dpiX[i] && dpiCurY == dpiY[i]){
            dpiCurIdx = i;
            if(i != 0)
                dpiLastIdx = i;
            break;
        }
    }
    _needsUpdate = _needsSave = true;
}

void KbPerf::curDpi(const QPoint& newDpi){
    while(pushedDpis.count() > 0)
        popDpi(pushedDpis.keys().last());
    _curDpi(newDpi);
}

quint64 KbPerf::pushDpi(const QPoint& newDpi){
    if(pushedDpis.isEmpty())
        // Push original DPI
        pushedDpis[0] = curDpi();
    quint64 index = runningPushIdx++;
    if(runningPushIdx == 0)
        runningPushIdx = 1;
    pushedDpis[index] = newDpi;
    _curDpi(newDpi);
    return index;
}

void KbPerf::popDpi(quint64 pushIdx){
    if(pushIdx == 0 || !pushedDpis.contains(pushIdx))
        return;
    pushedDpis.remove(pushIdx);
    // Set the DPI to the last-pushed value still on the stack
    _curDpi(map_last(pushedDpis));
    // If all values have been popped, remove the original DPI
    if(pushedDpis.count() == 1)
        pushedDpis.clear();
    _needsUpdate = _needsSave = true;
}

void KbPerf::dpiUp(){
    // Scroll past disabled DPIs and choose the next one up
    int idx = curDpiIdx();
    do {
        idx++;
        if(idx >= DPI_COUNT)
            return;
    } while(!dpiOn[idx]);
    curDpiIdx(idx);
}

void KbPerf::dpiDown(){
    int idx = curDpiIdx();
    do {
        idx--;
        if(idx <= SNIPER)
            return;
    } while(!dpiOn[idx]);
    curDpiIdx(idx);
}

void KbPerf::getIndicator(indicator index, QColor& color1, QColor& color2, QColor& color3, bool& software_enable, i_hw& hardware_enable){
    if(index < 0 || index >= I_COUNT)
        return;
    color1 = iColor[index][0];
    color2 = iColor[index][1];
    if(index == LIGHT)
        color3 = light100Color;
    else if(index == MUTE)
        color3 = muteNAColor;
    else
        color3 = QColor();
    software_enable = iEnable[index];
    hardware_enable = (index <= HW_IMAX ? hwIType[index] : NONE);
}

void KbPerf::setIndicator(indicator index, const QColor& color1, const QColor& color2, const QColor& color3, bool software_enable, i_hw hardware_enable){
    if(index < 0 || index >= I_COUNT)
        return;
    iColor[index][0] = color1;
    iColor[index][1] = color2;
    if(index == LIGHT)
        light100Color = color3;
    else if(index == MUTE)
        muteNAColor = color3;
    iEnable[index] = software_enable;
    if(hardware_enable == NONE)
        hardware_enable = NORMAL;
    if(index <= HW_IMAX)
        hwIType[index] = hardware_enable;
    _needsUpdate = _needsSave = true;
}

void KbPerf::liftHeight(height newHeight){
    if(newHeight < LOW || newHeight > HIGH)
        return;
    _liftHeight = newHeight;
    _needsUpdate = _needsSave = true;
}

void KbPerf::angleSnap(bool newAngleSnap){
    _angleSnap = newAngleSnap;
    _needsUpdate = _needsSave = true;
}

void KbPerf::update(QFile& cmd, bool force, bool saveCustomDpi){
    if(!force && !_needsUpdate)
        return;
    emit settingsUpdated();
    _needsUpdate = false;
    // Save DPI stage 0 (sniper)
    cmd.write(QString("dpi 0:%1,%2").arg(dpiX[0]).arg(dpiY[0]).toLatin1());
    // If the mouse is set to a custom DPI, save it in stage 1
    int stage = dpiCurIdx;
    if(stage < 0 && saveCustomDpi){
        stage = 1;
        cmd.write(QString(" 1:%1,%2").arg(dpiCurX).arg(dpiCurY).toLatin1());
    } else {
        // Otherwise, save stage 1 normally
        if(!dpiOn[1] && stage != 1)
            cmd.write(" 1:off");
        else
            cmd.write(QString(" 1:%1,%2").arg(dpiX[1]).arg(dpiY[1]).toLatin1());
    }
    // Save stages 1 - 5
    for(int i = 2; i < DPI_COUNT; i++){
        if(!dpiOn[i] && stage != i)
            cmd.write(QString(" %1:off").arg(i).toLatin1());
        else
        cmd.write(QString(" %1:%2,%3").arg(i).arg(dpiX[i]).arg(dpiY[i]).toLatin1());
    }
    // Save stage selection, lift height, and angle snap
    cmd.write(QString(" dpisel %1 lift %2 snap %3").arg(stage).arg(_liftHeight).arg(_angleSnap ? "on" : "off").toLatin1());
    // Save DPI colors
    cmd.write(" rgb");
    for(int i = 0; i < DPI_COUNT; i++){
        QColor color = dpiColor(i);
        cmd.write(" dpi");
        char output[9];
        snprintf(output, sizeof(output), "%1d:%02x%02x%02x", i, color.red(), color.green(), color.blue());
        cmd.write(output);
    }
    // Enable indicator notifications
    cmd.write(" inotify all");
    // Set indicator state
    const char* iNames[HW_I_COUNT] = { "num", "caps", "scroll" };
    for(int i = 0; i < HW_I_COUNT; i++){
        if(hwIType[i] == ON)
            cmd.write(" ion ");
        else if(hwIType[i] == OFF)
            cmd.write(" ioff ");
        else
            cmd.write(" iauto ");
        cmd.write(iNames[i]);
    }
}

void KbPerf::lightIndicator(const char* name, QRgb rgba){
    int a = round(qAlpha(rgba) * _iOpacity);
    if(a <= 0)
        return;
    light()->setIndicator(name, qRgba(qRed(rgba), qGreen(rgba), qBlue(rgba), a));
}

void KbPerf::applyIndicators(int modeIndex, const bool indicatorState[]){
    light()->resetIndicators();
    if(_iOpacity <= 0.f)
        return;
    if(_dpiIndicator){
        // Set DPI indicator according to index
        int index = curDpiIdx();
        if(index == -1 || index > OTHER)
            index = OTHER;
        lightIndicator("dpi", dpiClr[index].rgba());
    }
    // KB indicators
    if(iEnable[MODE]){
        for(int i = 0; i < Kb::HWMODE_MAX; i++){
            char name[4];
            snprintf(name, sizeof(name), "m%d", i + 1);
            if(modeIndex == i)
                lightIndicator(name, iColor[MODE][0].rgba());
            else
                lightIndicator(name, iColor[MODE][1].rgba());
        }
    }
    if(iEnable[MACRO])
        lightIndicator("mr", iColor[MUTE][1].rgba());
    if(iEnable[LIGHT]){
        switch(light()->dimming()){
        case 0: // 100%
            lightIndicator("light", light100Color.rgba());
            break;
        case 1: // 67%
            lightIndicator("light", iColor[LIGHT][1].rgba());
            break;
        case 2: // 33%
        case 3: // light off
            lightIndicator("light", iColor[LIGHT][0].rgba());
            break;
        }
    }
    if(iEnable[LOCK]){
        if(bind()->winLock())
            lightIndicator("lock", iColor[LOCK][0].rgba());
        else
            lightIndicator("lock", iColor[LOCK][1].rgba());
    }
    if(iEnable[MUTE]){
        switch(getMuteState()){
        case MUTED:
            lightIndicator("mute", iColor[MUTE][0].rgba());
            break;
        case UNMUTED:
            lightIndicator("mute", iColor[MUTE][1].rgba());
            break;
        default:
            lightIndicator("mute", muteNAColor.rgba());
            break;
        }
    }
    // Lock lights
    if(iEnable[NUM])
        lightIndicator("numlock", indicatorState[0] ? iColor[NUM][0].rgba() : iColor[NUM][1].rgba());
    if(iEnable[CAPS])
        lightIndicator("caps", indicatorState[1] ? iColor[CAPS][0].rgba() : iColor[CAPS][1].rgba());
    if(iEnable[SCROLL])
        lightIndicator("scroll", indicatorState[2] ? iColor[SCROLL][0].rgba() : iColor[SCROLL][1].rgba());
}


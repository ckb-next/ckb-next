#include "kbperf.h"
#include "kbmode.h"

#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
#define map_last(map) ((map).last())
#else
#define map_last(map) ((map).value((map).keys().last()))
#endif

KbPerf::KbPerf(KbMode* parent) :
    QObject(parent), _liftHeight(MEDIUM), _angleSnap(false), _enableIndicator(true),
    _needsUpdate(true), _needsSave(true), runningPushIdx(1) {
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
}

KbPerf::KbPerf(KbMode* parent, const KbPerf& other) :
    QObject(parent), dpiCurX(other.dpiCurX), dpiCurY(other.dpiCurY), dpiCurIdx(other.dpiCurIdx), dpiLastIdx(other.dpiLastIdx),
    _liftHeight(other._liftHeight), _angleSnap(other._angleSnap), _enableIndicator(other._enableIndicator),
    _needsUpdate(true), _needsSave(true), runningPushIdx(1) {
    memcpy(dpiX, other.dpiX, sizeof(dpiX));
    memcpy(dpiY, other.dpiY, sizeof(dpiY));
    for(int i = 0; i < DPI_COUNT + 1; i++)
        dpiClr[i] = other.dpiClr[i];
    memcpy(dpiOn, other.dpiOn, sizeof(dpiOn));
    // Don't copy pushed DPI states. If the other mode has any, restore the original DPI
    if(!other.pushedDpis.isEmpty())
        curDpi(other.pushedDpis[0]);
}

void KbPerf::load(CkbSettings& settings){
    pushedDpis.clear();
    runningPushIdx = 1;
    _needsSave = false;
    SGroup group(settings, "Performance");
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
    _liftHeight = (height)settings.value("LiftHeight").toInt();
    if(_liftHeight < LOW || _liftHeight > HIGH)
        _liftHeight = MEDIUM;
    _angleSnap = settings.value("AngleSnap").toBool();
    _enableIndicator = !settings.value("NoIndicator").toBool();
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
            settings.setValue(iStr + "RGB", dpiClr[i].name());
            if(i != 0)
                settings.setValue(iStr + "Disabled", !dpiOn[i]);
        }
        settings.setValue("6RGB", dpiClr[OTHER].name());
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
    settings.setValue("NoIndicator", !_enableIndicator);
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
}

QHash<QString, QRgb> KbPerf::indicatorLights() const {
    QHash<QString, QRgb> res;
    if(_enableIndicator){
        // Set DPI indicator according to index
        int index = curDpiIdx();
        if(index == -1 || index > OTHER)
            index = OTHER;
        res["dpi"] = dpiClr[index].rgb();
    }
    return res;
}


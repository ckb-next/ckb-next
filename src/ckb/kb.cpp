#include <fcntl.h>
#include <QSet>
#include <QUrl>
#include <QMutex>
#include <QDateTime>
#include "kb.h"
#include "kbmanager.h"

// All active devices
static QSet<Kb*> activeDevices;
// Active notification node paths
static QSet<QString> notifyPaths;
static QMutex notifyPathMutex;

int Kb::_frameRate = 30, Kb::_scrollSpeed = 0;
KeyMap::Layout Kb::_layout = KeyMap::NO_LAYOUT;
bool Kb::_dither = false, Kb::_mouseAccel = true;

Kb::Kb(QObject *parent, const QString& path) :
    QThread(parent), features("N/A"), firmware("N/A"), pollrate("N/A"), monochrome(false),
    devpath(path), cmdpath(path + "/cmd"), notifyPath(path + "/notify1"),
    _currentProfile(0), _currentMode(0), _model(KeyMap::NO_MODEL),
    lastAutoSave(QDateTime::currentMSecsSinceEpoch()),
    _hwProfile(0), prevProfile(0), prevMode(0),
    cmd(cmdpath), notifyNumber(1), _needsSave(false)
{
    memset(iState, 0, sizeof(iState));
    memset(hwLoading, 0, sizeof(hwLoading));

    // Get the features, model, serial number, FW version (if available), and poll rate (if available) from /dev nodes
    QFile ftpath(path + "/features"), mpath(path + "/model"), spath(path + "/serial"), fwpath(path + "/fwversion"), ppath(path + "/pollrate");
    if(ftpath.open(QIODevice::ReadOnly)){
        features = ftpath.read(1000);
        features = features.trimmed();
        ftpath.close();
        // Read model from features (first word: vendor, second word: product)
        QStringList list = features.split(" ");
        if(list.length() < 2)
            return;
        _model = KeyMap::getModel(list[1]);
        if(_model == KeyMap::NO_MODEL)
            return;
    } else
        // Bail if features aren't readable
        return;
    if(features.contains("monochrome"))
        monochrome = true;
    if(mpath.open(QIODevice::ReadOnly)){
        usbModel = mpath.read(100);
        usbModel = usbModel.remove("Corsair").remove("Gaming").remove("Keyboard").remove("Mouse").remove("Bootloader").trimmed();
        mpath.close();
    }
    if(usbModel == "")
        usbModel = "Keyboard";
    if(spath.open(QIODevice::ReadOnly)){
        usbSerial = spath.read(100);
        usbSerial = usbSerial.trimmed().toUpper();
        spath.close();
    }
    if(usbSerial == "")
        usbSerial = "Unknown-" + usbModel;
    if(features.contains("fwversion") && fwpath.open(QIODevice::ReadOnly)){
        firmware = fwpath.read(100);
        firmware = QString::number(firmware.trimmed().toInt() / 100., 'f', 2);
        fwpath.close();
    }
    if(features.contains("pollrate") && ppath.open(QIODevice::ReadOnly)){
        pollrate = ppath.read(100);
        pollrate = pollrate.trimmed();
        ppath.close();
    }

    prefsPath = "Devices/" + usbSerial;

    hwModeCount = (_model == KeyMap::K95) ? 3 : 1;
    // Open cmd in non-blocking mode so that it doesn't lock up if nothing is reading
    // (e.g. if the daemon crashed and didn't clean up the node)
    int fd = open(cmdpath.toLatin1().constData(), O_WRONLY | O_NONBLOCK);
    if(!cmd.open(fd, QIODevice::WriteOnly, QFileDevice::AutoCloseHandle))
        return;

    // Find an available notification node (if none is found, take notify1)
    {
        QMutexLocker locker(&notifyPathMutex);
        for(int i = 1; i < 10; i++){
            QString notify = QString(path + "/notify%1").arg(i);
            if(!QFile::exists(notify) && !notifyPaths.contains(notify)){
                notifyNumber = i;
                notifyPath = notify;
                break;
            }
        }
        notifyPaths.insert(notifyPath);
    }
    cmd.write(QString("notifyon %1\n").arg(notifyNumber).toLatin1());
    cmd.flush();
    // Activate device, apply settings, and ask for hardware profile
    cmd.write(QString("fps %1\n").arg(_frameRate).toLatin1());
    cmd.write(QString("dither %1\n").arg(static_cast<int>(_dither)).toLatin1());
#ifdef Q_OS_MACX
    // Write ANSI/ISO flag to daemon (OSX only)
    cmd.write("layout ");
    cmd.write(KeyMap::isISO(_layout) ? "iso" : "ansi");
    // Also OSX only: scroll speed and mouse acceleration
    cmd.write(QString("accel %1\n").arg(QString(_mouseAccel ? "on" : "off")).toLatin1());
    cmd.write(QString("scrollspeed %1\n").arg(_scrollSpeed).toLatin1());
#endif
    cmd.write(QString("\nactive\n@%1 get :hwprofileid").arg(notifyNumber).toLatin1());
    hwLoading[0] = true;
    for(int i = 0; i < hwModeCount; i++){
        cmd.write(QString(" mode %1 get :hwid").arg(i + 1).toLatin1());
        hwLoading[i + 1] = true;
    }
    // Ask for current indicator and key state
    cmd.write(" get :i :keys\n");
    cmd.flush();

    emit infoUpdated();
    activeDevices.insert(this);

    // Start a separate thread to read from the notification node
    start();
}

Kb::~Kb(){
    // Save settings first
    save();
    // Kill notification thread and remove node
    activeDevices.remove(this);
    if(!isOpen()){
        terminate();
        wait(1000);
        return;
    }
    if(notifyNumber > 0)
        cmd.write(QString("idle\nnotifyoff %1\n").arg(notifyNumber).toLatin1());
    cmd.flush();
    terminate();
    wait(1000);
    cmd.close();
}

void Kb::frameRate(int newFrameRate){
    KbManager::fps(newFrameRate);
    // If the rate has changed, send to all devices
    if(newFrameRate == _frameRate)
        return;
    _frameRate = newFrameRate;
    foreach(Kb* kb, activeDevices){
        kb->cmd.write(QString("fps %1\n").arg(newFrameRate).toLatin1());
        kb->cmd.flush();
    }
}

void Kb::layout(KeyMap::Layout newLayout){
    if(newLayout == KeyMap::NO_LAYOUT || newLayout == _layout)
        return;
    _layout = newLayout;
    // Update all devices
    foreach(Kb* kb, activeDevices)
        kb->updateLayout();
}

void Kb::updateLayout(){
#ifdef Q_OS_MACX
    // Write ANSI/ISO flag to daemon (OSX only)
    cmd.write("layout ");
    cmd.write(KeyMap::isISO(_layout) ? "iso" : "ansi");
    cmd.write("\n");
    cmd.flush();
#endif
    foreach(KbProfile* profile, _profiles)
        profile->keyMap(getKeyMap());
    if(_hwProfile && !_profiles.contains(_hwProfile))
        _hwProfile->keyMap(getKeyMap());
    // Stop all animations as they'll need to be restarted
    foreach(KbMode* mode, _currentProfile->modes())
        mode->light()->close();
    emit infoUpdated();
}

void Kb::dither(bool newDither){
    if(newDither == _dither)
        return;
    _dither = newDither;
    // Update all devices
    foreach(Kb* kb, activeDevices){
        kb->cmd.write(QString("dither %1\n").arg(static_cast<int>(newDither)).toLatin1());
        kb->cmd.flush();
    }
}

void Kb::mouseAccel(bool newAccel){
    if(newAccel == _mouseAccel)
        return;
    _mouseAccel = newAccel;
#ifdef Q_OS_MACX
    // Update all devices
    foreach(Kb* kb, activeDevices){
        kb->cmd.write(QString("accel %1\n").arg(QString(newAccel ? "on" : "off")).toLatin1());
        kb->cmd.flush();
    }
#endif
}

void Kb::scrollSpeed(int newSpeed){
    if(newSpeed == _scrollSpeed)
        return;
    _scrollSpeed = newSpeed;
#ifdef Q_OS_MACX
    // Update all devices
    foreach(Kb* kb, activeDevices){
        kb->cmd.write(QString("scrollspeed %1\n").arg(newSpeed).toLatin1());
        kb->cmd.flush();
    }
#endif
}

void Kb::load(){
    if(prefsPath.isEmpty())
        return;
    _needsSave = false;
    CkbSettings settings(prefsPath);
    // Read profiles
    KbProfile* newCurrentProfile = 0;
    QString current = settings.value("CurrentProfile").toString().trimmed().toUpper();
    foreach(QString guid, settings.value("Profiles").toString().split(" ")){
        guid = guid.trimmed().toUpper();
        if(guid != ""){
            KbProfile* profile = new KbProfile(this, getKeyMap(), settings, guid);
            _profiles.append(profile);
            if(guid == current || !newCurrentProfile)
                newCurrentProfile = profile;
        }
    }
    if(newCurrentProfile)
        setCurrentProfile(newCurrentProfile);
    else {
        // If nothing was loaded, load the demo profile
        QSettings demoSettings(":/txt/demoprofile.conf", QSettings::IniFormat, this);
        CkbSettings cSettings(demoSettings);
        KbProfile* demo = new KbProfile(this, getKeyMap(), cSettings, "{BA7FC152-2D51-4C26-A7A6-A036CC93D924}");
        _profiles.append(demo);
        setCurrentProfile(demo);
    }

    emit infoUpdated();
    emit profileAdded();
}

void Kb::save(){
    if(prefsPath.isEmpty())
        return;
    _needsSave = false;
    CkbSettings settings(prefsPath, true);
    QString guids, currentGuid;
    foreach(KbProfile* profile, _profiles){
        guids.append(" " + profile->id().guidString());
        if(profile == _currentProfile)
            currentGuid = profile->id().guidString();
        profile->save(settings);
    }
    settings.setValue("CurrentProfile", currentGuid);
    settings.setValue("Profiles", guids.trimmed());
}

void Kb::autoSave(){
    quint64 now = QDateTime::currentMSecsSinceEpoch();
    if(needsSave() && now >= lastAutoSave + 15 * 1000 && !CkbSettings::isBusy()){
        save();
        lastAutoSave = now;
    }
}

void Kb::hwSave(){
    if(!_currentProfile)
        return;
    // Close active lighting (if any)
    if(prevMode){
        prevMode->light()->close();
        deletePrevious();
    }
    hwProfile(_currentProfile);
    _hwProfile->id().hwModified = _hwProfile->id().modified;
    _hwProfile->setNeedsSave();
    // Re-send the current profile from scratch to ensure consistency
    writeProfileHeader();
    // Make sure there are enough modes
    while(_currentProfile->modeCount() < hwModeCount)
        _currentProfile->append(new KbMode(this, getKeyMap()));
    // Write only the base colors of each mode, no animations
    for(int i = 0; i < hwModeCount; i++){
        KbMode* mode = _currentProfile->modes()[i];
        cmd.write(QString("\nmode %1").arg(i + 1).toLatin1());
        KbLight* light = mode->light();
        KbPerf* perf = mode->perf();
        if(mode == _currentMode)
            cmd.write(" switch");
        // Write the mode name and ID
        cmd.write(" name ");
        cmd.write(QUrl::toPercentEncoding(mode->name()));
        cmd.write(" id ");
        cmd.write(mode->id().guidString().toLatin1());
        cmd.write(" ");
        cmd.write(mode->id().modifiedString().toLatin1());
        cmd.write(" ");
        // Write lighting and performance
        light->base(cmd, true, monochrome);
        cmd.write(" ");
        perf->update(cmd, true, false);
        // Update mode ID
        mode->id().hwModified = mode->id().modified;
        mode->setNeedsSave();
    }
    cmd.write("\n");

    // Save the profile to memory
    cmd.write("hwsave\n");
    cmd.flush();
}

bool Kb::needsSave() const {
    if(_needsSave)
        return true;
    foreach(const KbProfile* profile, _profiles){
        if(profile->needsSave())
            return true;
    }
    return false;
}

void Kb::writeProfileHeader(){
    cmd.write("eraseprofile");
    // Write the profile name and ID
    cmd.write(" profilename ");
    cmd.write(QUrl::toPercentEncoding(_currentProfile->name()));
    cmd.write(" profileid ");
    cmd.write(_currentProfile->id().guidString().toLatin1());
    cmd.write(" ");
    cmd.write(_currentProfile->id().modifiedString().toLatin1());
}

void Kb::fwUpdate(const QString& path){
    fwUpdPath = path;
    // Write the active command to ensure it's not ignored
    cmd.write("active");
    cmd.write(QString(" @%1 ").arg(notifyNumber).toLatin1());
    cmd.write("fwupdate ");
    cmd.write(path.toLatin1());
    cmd.write("\n");
}

void Kb::frameUpdate(){
    // Advance animation frame
    if(!_currentMode)
        return;
    KbLight* light = _currentMode->light();
    KbBind* bind = _currentMode->bind();
    KbPerf* perf = _currentMode->perf();
    if(!light->isStarted()){
        // Don't do anything until the animations are started
        light->open();
        return;
    }

    // Stop animations on the previously active mode (if any)
    bool changed = false;
    if(prevMode != _currentMode){
        if(prevMode){
            prevMode->light()->close();
            disconnect(prevMode, SIGNAL(destroyed()), this, SLOT(deletePrevious()));
        }
        prevMode = _currentMode;
        connect(prevMode, SIGNAL(destroyed()), this, SLOT(deletePrevious()));
        changed = true;
    }

    // If the profile has changed, update it
    if(prevProfile != _currentProfile){
        writeProfileHeader();
        cmd.write(" ");
        prevProfile = _currentProfile;
    }

    // Update current mode
    int index = _currentProfile->indexOf(_currentMode);
    // ckb-daemon only has 6 modes: 3 hardware, 3 non-hardware. Beyond mode six, switch back to four.
    // e.g. 1, 2, 3, 4, 5, 6, 4, 5, 6, 4, 5, 6 ...
    if(index >= 6)
        index = 3 + index % 3;

    // Send lighting/binding to driver
    cmd.write(QString("mode %1 switch ").arg(index + 1).toLatin1());
    perf->applyIndicators(index, iState);
    light->frameUpdate(cmd, monochrome);
    cmd.write(QString("\n@%1 ").arg(notifyNumber).toLatin1());
    bind->update(cmd, changed);
    cmd.write(" ");
    perf->update(cmd, changed);
    cmd.write("\n");
    cmd.flush();
}

void Kb::deletePrevious(){
    disconnect(prevMode, SIGNAL(destroyed()), this, SLOT(deletePrevious()));
    prevMode = 0;
}

void Kb::hwProfile(KbProfile* newHwProfile){
    if(_hwProfile == newHwProfile)
        return;
    if(_hwProfile)
        disconnect(_hwProfile, SIGNAL(destroyed()), this, SLOT(deleteHw()));
    _hwProfile = newHwProfile;
    if(_hwProfile)
        connect(_hwProfile, SIGNAL(destroyed()), this, SLOT(deleteHw()));
}

void Kb::deleteHw(){
    disconnect(_hwProfile, SIGNAL(destroyed()), this, SLOT(deleteHw()));
    _hwProfile = 0;
}

void Kb::run(){
    QFile notify(notifyPath);
    // Wait a small amount of time for the node to open (100ms)
    QThread::usleep(100000);
    if(!notify.open(QIODevice::ReadOnly)){
        // If it's still not open, try again before giving up (1s at a time, 10s total)
        QThread::usleep(900000);
        for(int i = 1; i < 10; i++){
            if(notify.open(QIODevice::ReadOnly))
                break;
            QThread::sleep(1);
        }
        if(!notify.isOpen())
            return;
    }
    // Read data from notification node
    QByteArray line;
    while(notify.isOpen() && (line = notify.readLine()).length() > 0){
        QString text = QString::fromUtf8(line);
        metaObject()->invokeMethod(this, "readNotify", Qt::QueuedConnection, Q_ARG(QString, text));
    }
    QMutexLocker locker(&notifyPathMutex);
    notifyPaths.remove(notifyPath);
}

void Kb::readNotify(QString line){
    QStringList components = line.trimmed().split(" ");
    if(components.count() < 2)
        return;
    if(components[0] == "key"){
        // Key event
        QString key = components[1];
        if(key.length() < 2)
            return;
        QString keyName = key.mid(1);
        bool keyPressed = (key[0] == '+');
        KbMode* mode = _currentMode;
        if(mode){
            mode->light()->animKeypress(keyName, keyPressed);
            mode->bind()->keyEvent(keyName, keyPressed);
        }
    } else if(components[0] == "i"){
        // Indicator event
        QString i = components[1];
        if(i.length() < 2)
            return;
        QString iName = i.mid(1);
        bool on = (i[0] == '+');
        if(iName == "num")
            iState[0] = on;
        else if(iName == "caps")
            iState[1] = on;
        else if(iName == "scroll")
            iState[2] = on;
    } else if(components[0] == "hwprofileid"){
        // Hardware profile ID
        if(components.count() < 3)
            return;
        // Find the hardware profile in the list of profiles
        QString guid = components[1];
        QString modified = components[2];
        KbProfile* newProfile = 0;
        foreach(KbProfile* profile, _profiles){
            if(profile->id().guid == guid){
                newProfile = profile;
                break;
            }
        }
        // If it wasn't found, create it
        if(!newProfile){
            newProfile = new KbProfile(this, getKeyMap(), guid, modified);
            hwLoading[0] = true;
            cmd.write(QString("@%1 get :hwprofilename\n").arg(notifyNumber).toLatin1());
            cmd.flush();
        } else {
            // If it's been updated, fetch its name
            if(newProfile->id().hwModifiedString() != modified){
                newProfile->id().modifiedString(modified);
                newProfile->id().hwModifiedString(modified);
                newProfile->setNeedsSave();
                if(hwLoading[0]){
                    cmd.write(QString("@%1 get :hwprofilename\n").arg(notifyNumber).toLatin1());
                    cmd.flush();
                }
            } else {
                hwLoading[0] = false;
            }
        }
        hwProfile(newProfile);
        emit profileAdded();
        if(_hwProfile == _currentProfile)
            emit profileChanged();
    } else if(components[0] == "hwprofilename"){
        // Hardware profile name
        QString name = QUrl::fromPercentEncoding(components[1].toUtf8());
        if(!_hwProfile || !hwLoading[0])
            return;
        QString oldName = _hwProfile->name();
        if(!(oldName.length() >= name.length() && oldName.left(name.length()) == name)){
            // Don't change the name if it's a truncated version of what we already have
            _hwProfile->name(name);
            emit profileRenamed();
        }
    } else if(components[0] == "mode"){
        // Mode-specific data
        if(components.count() < 4)
            return;
        int mode = components[1].toInt() - 1;
        if(components[2] == "hwid"){
            if(components.count() < 5 || mode >= HWMODE_MAX || !_hwProfile)
                return;
            // Hardware mode ID
            QString guid = components[3];
            QString modified = components[4];
            // Look for this mode in the hardware profile
            KbMode* hwMode = 0;
            bool isUpdated = false;
            foreach(KbMode* kbMode, _hwProfile->modes()){
                if(kbMode->id().guid == guid){
                    hwMode = kbMode;
                    if(kbMode->id().hwModifiedString() != modified){
                        // Update modification time
                        hwMode->id().modifiedString(modified);
                        hwMode->id().hwModifiedString(modified);
                        hwMode->setNeedsSave();
                        isUpdated = true;
                    } else {
                        hwLoading[mode + 1] = false;
                    }
                    break;
                }
            }
            // If it wasn't found, add it
            if(!hwMode){
                isUpdated = true;
                hwMode = new KbMode(this, getKeyMap(), guid, modified);
                _hwProfile->append(hwMode);
                // If the hardware profile now contains enough modes to be added to the list, do so
                if(!_profiles.contains(_hwProfile) && _hwProfile->modeCount() >= hwModeCount){
                    _profiles.append(_hwProfile);
                    _needsSave = true;
                    emit profileAdded();
                    if(!_currentProfile)
                        setCurrentProfile(_hwProfile);
                }
            }
            if(hwLoading[mode + 1] && isUpdated){
                // If the mode isn't in the right place, move it
                int index = _hwProfile->indexOf(hwMode);
                if(mode < _hwProfile->modeCount() && index != mode)
                    _hwProfile->move(index, mode);
                // Fetch the updated data
                cmd.write(QString("@%1 mode %2 get :hwname :hwrgb").arg(notifyNumber).arg(mode + 1).toLatin1());
                if(isMouse())
                    cmd.write(" :hwdpi :hwdpisel :hwlift :hwsnap");
                cmd.write("\n");
                cmd.flush();
            }
        } else if(components[2] == "hwname"){
            // Mode name - update list
            if(!_hwProfile || _hwProfile->modeCount() <= mode || mode >= HWMODE_MAX || !hwLoading[mode + 1])
                return;
            KbMode* hwMode = _hwProfile->modes()[mode];
            QString name = QUrl::fromPercentEncoding(components[3].toUtf8());
            QString oldName = hwMode->name();
            if(!(oldName.length() >= name.length() && oldName.left(name.length()) == name)){
                // Don't change the name if it's a truncated version of what we already have
                hwMode->name(name);
                if(_hwProfile == _currentProfile)
                    emit modeRenamed();
            }
        } else if(components[2] == "hwrgb"){
            // RGB - set mode lighting
            if(!_hwProfile || _hwProfile->modeCount() <= mode || mode >= HWMODE_MAX || !hwLoading[mode + 1])
                return;
            KbMode* kbmode = _hwProfile->modes()[mode];
            KbLight* light = kbmode->light();
            // Scan the input for colors
            QColor lightColor = QColor();
            for(int i = 3; i < components.count(); i++){
                QString comp = components[i];
                if(comp.indexOf(":") < 0){
                    // No ":" - single hex constant
                    bool ok;
                    int rgb = comp.toInt(&ok, 16);
                    if(ok)
                        light->color(QColor::fromRgb((QRgb)rgb));
                } else {
                    // List of keys ("a,b:xxxxxx"). Parse color first
                    QStringList set = comp.split(":");
                    bool ok;
                    int rgb = set[1].toInt(&ok, 16);
                    if(ok){
                        QColor color = QColor::fromRgb((QRgb)rgb);
                        // Parse keys
                        QStringList keys = set[0].split(",");
                        foreach(QString key, keys){
                            if(key == "light")
                                // Extrapolate the Light key to the M-keys and Lock key, since those will be set to black on hwsave
                                lightColor = color;
                            if(key.startsWith("dpi") && key.length() > 3){
                                // DPI levels go to the KbPerf object instead of KbLight
                                bool ok = false;
                                int index = key.mid(3).toInt(&ok);
                                if(ok)
                                    kbmode->perf()->dpiColor(index, color);
                                continue;
                            }
                            light->color(key, color);
                        }
                    }
                }
            }
            if(lightColor.isValid()){
                light->color("mr", lightColor);
                light->color("m1", lightColor);
                light->color("m2", lightColor);
                light->color("m3", lightColor);
                light->color("lock", lightColor);
            }
        } else if(components[2] == "hwdpi"){
            // DPI settings
            if(!_hwProfile || _hwProfile->modeCount() <= mode || mode >= HWMODE_MAX || !hwLoading[mode + 1])
                return;
            KbPerf* perf = _hwProfile->modes()[mode]->perf();
            // Read the rest of the line as stage:x,y
            foreach(QString comp, components.mid(3)){
                QStringList dpi = comp.split(':');
                if(dpi.length() != 2)
                    continue;
                QStringList xy = dpi[1].split(',');
                int x, y;
                bool off = false;
                if(xy.length() < 2){
                    // If the right side only has one parameter, set both X and Y
                    if(xy[0] == "off")
                        off = true;
                    else
                        x = y = xy[0].toInt();
                } else {
                    x = xy[0].toInt();
                    y = xy[1].toInt();
                }
                // Set DPI for this stage
                int index = dpi[0].toInt();
                if(off){
                    perf->dpiEnabled(index, false);
                    // If all DPIs have been disabled, turn them back on
                    bool allOff = true;
                    for(int i = 1; i < KbPerf::DPI_COUNT; i++){
                        if(perf->dpiEnabled(i)){
                            allOff = false;
                            break;
                        }
                    }
                    if(allOff){
                        for(int i = 1; i < KbPerf::DPI_COUNT; i++)
                            perf->dpiEnabled(i, true);
                    }
                } else {
                    perf->dpiEnabled(index, true);
                    perf->dpi(index, QPoint(x, y));
                }
            }
        } else if(components[2] == "hwdpisel"){
            // Hardware DPI selection (0...5)
            if(!_hwProfile || _hwProfile->modeCount() <= mode || mode >= HWMODE_MAX || !hwLoading[mode + 1])
                return;
            KbPerf* perf = _hwProfile->modes()[mode]->perf();
            int idx = components[3].toInt();
            if(idx < 1)
                idx = 1;
            if(idx >= KbPerf::DPI_COUNT)
                idx = KbPerf::DPI_COUNT - 1;
            perf->curDpiIdx(idx);
        } else if(components[2] == "hwlift"){
            // Mouse lift height (1...5)
            if(!_hwProfile || _hwProfile->modeCount() <= mode || mode >= HWMODE_MAX || !hwLoading[mode + 1])
                return;
            KbPerf* perf = _hwProfile->modes()[mode]->perf();
            perf->liftHeight((KbPerf::height)components[3].toInt());
        } else if(components[3] == "hwsnap"){
            // Mouse angle snapping ("on" or "off")
            if(!_hwProfile || _hwProfile->modeCount() <= mode || mode >= HWMODE_MAX || !hwLoading[mode + 1])
                return;
            KbPerf* perf = _hwProfile->modes()[mode]->perf();
            perf->angleSnap(components[3] == "on");
        }
    } else if(components[0] == "fwupdate"){
        // Firmware update progress
        if(components.count() < 3)
            return;
        // Make sure path is the same
        if(components[1] != fwUpdPath)
            return;
        QString res = components[2];
        if(res == "invalid" || res == "fail")
            emit fwUpdateFinished(false);
        else if(res == "ok")
            emit fwUpdateFinished(true);
        else {
            // "xx/yy" indicates progress
            if(!res.contains("/"))
                return;
            QStringList numbers = res.split("/");
            emit fwUpdateProgress(numbers[0].toInt(), numbers[1].toInt());
        }
    }
}

KeyMap Kb::getKeyMap(){
    return KeyMap(_model, _layout);
}

void Kb::setCurrentProfile(KbProfile *profile, bool spontaneous){
    while(profile->modeCount() < hwModeCount)
        profile->append(new KbMode(this, getKeyMap()));
    KbMode* mode = profile->currentMode();
    if(!mode)
        profile->currentMode(mode = profile->modes().first());
    setCurrentMode(profile, mode, spontaneous);
}

void Kb::setCurrentMode(KbProfile* profile, KbMode* mode, bool spontaneous){
    if(_currentProfile != profile){
        _currentProfile = profile;
        _needsSave = true;
        emit profileChanged();
    }
    if(_currentMode != mode || _currentProfile->currentMode() != mode){
        _currentProfile->currentMode(_currentMode = mode);
        _needsSave = true;
        emit modeChanged(spontaneous);
    }
}


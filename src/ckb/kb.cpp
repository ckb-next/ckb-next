#include <fcntl.h>
#include <QtConcurrent/QtConcurrent>
#include <QThreadPool>
#include "kb.h"
#include "media.h"

Kb::Kb(QObject *parent, const QString& path) :
    QThread(parent), devpath(path), cmdpath(path + "/cmd"),
    features("N/A"), firmware("N/A"), pollrate("N/A"),
    currentProfile(0), currentMode(0), _model(KeyMap::NO_MODEL), _layout(KeyMap::NO_LAYOUT),
    _hwProfile(0), prevProfile(0), prevMode(0),
    cmd(cmdpath), notifyNumber(1), hwLoading(true)
{
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
    if(mpath.open(QIODevice::ReadOnly)){
        usbModel = mpath.read(100);
        usbModel = usbModel.remove("Corsair").remove("Gaming Keyboard").remove("Keyboard").remove("Bootloader").trimmed();
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

    hwModeCount = (_model == KeyMap::K95) ? 3 : 1;
    // Open cmd in non-blocking mode so that it doesn't lock up if nothing is reading
    // (e.g. if the daemon crashed and didn't clean up the node)
    int fd = open(cmdpath.toLatin1().constData(), O_WRONLY | O_NONBLOCK);
    if(!cmd.open(fd, QIODevice::WriteOnly, QFileDevice::AutoCloseHandle))
        return;

    // Find an available notification node (if none is found, take notify1)
    for(int i = 1; i < 10; i++){
        QString notify = QString(path + "/notify%1").arg(i);
        if(!QFile::exists(notify)){
            notifyNumber = i;
            notifyPath = notify;
            break;
        }
    }
    cmd.write(QString("notifyon %1\n").arg(notifyNumber).toLatin1());
    cmd.flush();
    cmd.write(QString("active\n@%1 get :hwprofileid").arg(notifyNumber).toLatin1());
    for(int i = 0; i < hwModeCount; i++)
        cmd.write(QString(" mode %1 get :hwid").arg(i + 1).toLatin1());
    cmd.write("\n");
    cmd.flush();

    emit infoUpdated();

    // Start a separate thread to read from the notification node
    start();
}

Kb::~Kb(){
    if(!isOpen())
        return;
    // Kill notification thread and remove node
    if(notifyNumber > 0)
        cmd.write(QString("idle\nnotifyoff %1\n").arg(notifyNumber).toLatin1());
    cmd.flush();
    terminate();
    wait(1000);
    // Reset to hardware profile
    if(_hwProfile){
        currentProfile = _hwProfile;
        hwSave();
    }
    cmd.close();
}

void Kb::load(QSettings &settings){
    // Read layout
    _layout = KeyMap::getLayout(settings.value("Layout").toString());
    if(_layout == KeyMap::NO_LAYOUT){
        // If the layout couldn't be loaded, fetch it from the driver
        cmd.write(QString("@%1 get :layout\n").arg(notifyNumber).toLatin1());
    } else {
        cmd.write("layout ");
        cmd.write(KeyMap::getLayoutHw(_layout).toLatin1());
        cmd.write("\n");
        emit infoUpdated();
    }
    cmd.flush();

    // Read profiles
    KbProfile* newCurrentProfile = 0;
    QString current = settings.value("CurrentProfile").toString().trimmed().toUpper();
    foreach(QString guid, settings.value("Profiles").toString().split(" ")){
        guid = guid.trimmed().toUpper();
        if(guid != ""){
            KbProfile* profile = new KbProfile(this, getKeyMap(), settings, guid);
            profiles.append(profile);
            if(guid == current || !newCurrentProfile)
                newCurrentProfile = profile;
        }
    }
    if(newCurrentProfile)
        setCurrentProfile(newCurrentProfile);
    else {
        // If nothing was loaded, load the demo profile
        QSettings demoSettings(":/txt/demoprofile.conf", QSettings::IniFormat, this);
        KbProfile* demo = new KbProfile(this, getKeyMap(), demoSettings, "{BA7FC152-2D51-4C26-A7A6-A036CC93D924}");
        profiles.append(demo);
        setCurrentProfile(demo);
    }

    emit profileAdded();
}

void Kb::save(QSettings& settings){
    QString guids, currentGuid;
    foreach(KbProfile* profile, profiles){
        guids.append(" " + profile->id().guidString());
        if(profile == currentProfile)
            currentGuid = profile->id().guidString();
        profile->save(settings);
    }
    settings.setValue("CurrentProfile", currentGuid);
    settings.setValue("Profiles", guids.trimmed());
    settings.setValue("Layout", KeyMap::getLayout(_layout));
}

void Kb::hwSave(){
    if(!currentProfile)
        return;
    // Close active lighting (if any)
    if(prevMode){
        prevMode->light()->close();
        deletePrevious();
    }
    hwProfile(currentProfile);
    hwLoading = false;
    // Rewrite the current profile from scratch to ensure consistency
    writeProfileHeader();
    // Write only the base colors of each mode, no animations
    for(int i = 0; i < hwModeCount; i++){
        KbMode* mode = currentProfile->modes.at(i);
        cmd.write("\n");
        KbLight* light = mode->light();
        light->base(cmd, i);
        if(mode == currentMode)
            cmd.write(" switch");
        // Write the mode name and ID
        cmd.write(" name ");
        cmd.write(QUrl::toPercentEncoding(mode->name()));
        cmd.write(" id ");
        cmd.write(mode->id().guidString().toLatin1());
        cmd.write(" ");
        cmd.write(mode->id().modifiedString().toLatin1());
    }
    cmd.write("\n");
    cmd.flush();

    // Save the profile to memory
    cmd.write("hwsave\n");
    // Load the new modification times for the profile
    cmd.write(QString("@%1 get :hwprofileid").arg(notifyNumber).toLatin1());
    for(int i = 0; i < hwModeCount; i++)
        cmd.write(QString(" mode %1 get:hwid").arg(i + 1).toLatin1());
    cmd.write("\n");
    cmd.flush();
}

void Kb::writeProfileHeader(){
    cmd.write("eraseprofile");
    // Write the profile name and ID
    cmd.write(" profilename ");
    cmd.write(QUrl::toPercentEncoding(currentProfile->name()));
    cmd.write(" profileid ");
    cmd.write(currentProfile->id().guidString().toLatin1());
    cmd.write(" ");
    cmd.write(currentProfile->id().modifiedString().toLatin1());
}

void Kb::layout(KeyMap::Layout newLayout, bool write){
    if(newLayout == KeyMap::NO_LAYOUT)
        return;
    _layout = newLayout;
    if(write){
        cmd.write("layout ");
        cmd.write(KeyMap::getLayoutHw(newLayout).toLatin1());
        cmd.write("\n");
        cmd.flush();
    }
    foreach(KbProfile* profile, profiles)
        profile->keyMap(getKeyMap());
    if(_hwProfile && !profiles.contains(_hwProfile))
        _hwProfile->keyMap(getKeyMap());
    // Stop all animations as they'll need to be restarted
    foreach(KbMode* mode, currentProfile->modes)
        mode->light()->close();
    emit infoUpdated();
}

void Kb::frameUpdate(){
    // Get system mute state
    muteState mute = getMuteState();
    if(mute == UNKNOWN)
        mute = MUTED;

    // Advance animation frame
    if(!currentMode)
        return;
    KbLight* light = currentMode->light();
    KbBind* bind = currentMode->bind();
    if(!light->isStarted()){
        // Don't do anything until the animations are started
        light->open();
        return;
    }

    // Stop animations on the previously active mode (if any)
    bool changed = false;
    if(prevMode != currentMode){
        if(prevMode){
            prevMode->light()->close();
            disconnect(prevMode, SIGNAL(destroyed()), this, SLOT(deletePrevious()));
        }
        prevMode = currentMode;
        connect(prevMode, SIGNAL(destroyed()), this, SLOT(deletePrevious()));
        changed = true;
    }

    // If the profile has changed, update it
    if(prevProfile != currentProfile){
        writeProfileHeader();
        cmd.write(" ");
        prevProfile = currentProfile;
    }
    // Update current mode
    int index = currentProfile->indexOf(currentMode);
    light->frameUpdate(cmd, index, mute != MUTED, !bind->winLock());
    cmd.write(QString(" @%1 ").arg(notifyNumber).toLatin1());
    bind->update(cmd, changed);
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
        // If it's still not open, try again before giving up (1s total)
        QThread::usleep(900000);
        if(!notify.open(QIODevice::ReadOnly))
            return;
    }
    // Read data from notification node
    QByteArray line;
    while(1){
        if((line = notify.readLine()).length() > 0){
            QString text = QString::fromUtf8(line);
            metaObject()->invokeMethod(this, "readNotify", Qt::QueuedConnection, Q_ARG(QString, text));
        }
    }
}

void Kb::readNotify(QString line){
    QStringList components = line.trimmed().split(" ");
    if(components.count() < 2)
        return;
    if(components[0] == "layout"){
        // Layout change - set new layout
        KeyMap::Layout newLayout = KeyMap::getLayout(components[1]);
        layout(newLayout, false);
    } else if(components[0] == "key"){
        // Key event
        QString key = components[1];
        if(key.length() < 2)
            return;
        QString keyName = key.mid(1);
        bool keyPressed = (key[0] == '+');
        KbMode* mode = currentMode;
        if(mode){
            mode->light()->animKeypress(keyName, keyPressed);
            mode->bind()->keyEvent(keyName, keyPressed);
        }
    } else if(components[0] == "hwprofileid"){
        // Hardware profile ID
        if(components.count() < 3)
            return;
        // Find the hardware profile in the list of profiles
        QString guid = components[1];
        QString modified = components[2];
        KbProfile* newProfile = 0;
        foreach(KbProfile* profile, profiles){
            if(profile->id().guid == guid){
                newProfile = profile;
                break;
            }
        }
        // If it wasn't found, create it
        if(!newProfile){
            newProfile = new KbProfile(this, getKeyMap(), guid, modified);
            hwLoading = true;
            cmd.write(QString("@%1 get :hwprofilename\n").arg(notifyNumber).toLatin1());
            cmd.flush();
        } else {
            // If it's been updated (and we're loading hardware data), fetch its name
            if(newProfile->id().modifiedString() != modified){
                newProfile->id().modifiedString(modified);
                if(hwLoading){
                    cmd.write(QString("@%1 get :hwprofilename\n").arg(notifyNumber).toLatin1());
                    cmd.flush();
                }
            }
        }
        hwProfile(newProfile);
        emit profileAdded();
        if(_hwProfile == currentProfile)
            emit profileChanged();
    } else if(components[0] == "hwprofilename"){
        // Hardware profile name
        QString name = QUrl::fromPercentEncoding(components[1].toUtf8());
        if(!_hwProfile)
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
            if(components.count() < 5 || !_hwProfile)
                return;
            // Hardware mode ID
            QString guid = components[3];
            QString modified = components[4];
            // Look for this mode in the hardware profile
            KbMode* hwMode = 0;
            bool isNew = false;
            foreach(KbMode* mode, _hwProfile->modes){
                if(mode->id().guid == guid){
                    hwMode = mode;
                    if(mode->id().modifiedString() != modified){
                        // Update modification time
                        hwMode->id().modifiedString(modified);
                        isNew = true;
                    }
                    break;
                }
            }
            // If it wasn't found, add it
            if(!hwMode){
                isNew = true;
                hwMode = new KbMode(this, getKeyMap(), guid, modified);
                _hwProfile->modes.append(hwMode);
                // If the hardware profile now contains enough modes to be added to the list, do so
                if(!profiles.contains(_hwProfile) && _hwProfile->modes.count() >= hwModeCount){
                    profiles.append(_hwProfile);
                    emit profileAdded();
                    if(!currentProfile)
                        setCurrentProfile(_hwProfile);
                }
            }
            // If the mode isn't in the right place, move it
            int index = _hwProfile->modes.indexOf(hwMode);
            if(isNew && mode < _hwProfile->modes.count() && index != mode)
                _hwProfile->modes.move(index, mode);
            // If loading hardware profile, fetch the updated data
            if(isNew && hwLoading){
                cmd.write(QString("@%1 mode %2 get :hwname :hwrgb\n").arg(notifyNumber).arg(mode + 1).toLatin1());
                cmd.flush();
            }
        } else if(components[2] == "hwname"){
            // Mode name - update list
            if(!_hwProfile || _hwProfile->modes.count() <= mode)
                return;
            KbMode* hwMode = _hwProfile->modes[mode];
            QString name = QUrl::fromPercentEncoding(components[3].toUtf8());
            QString oldName = hwMode->name();
            if(!(oldName.length() >= name.length() && oldName.left(name.length()) == name)){
                // Don't change the name if it's a truncated version of what we already have
                hwMode->name(name);
                if(_hwProfile == currentProfile)
                    emit modeRenamed();
            }
        } else if(components[2] == "hwrgb"){
            // RGB - set mode lighting
            if(!_hwProfile || _hwProfile->modes.count() <= mode)
                return;
            KbLight* light = _hwProfile->modes[mode]->light();
            // If it's a color command, scan the input
            QColor lightColor = QColor();
            for(int i = 3; i < components.count(); i++){
                QString comp = components[i];
                if(comp.indexOf(":") < 0){
                    // Single hex constant?
                    bool ok;
                    int rgb = comp.toInt(&ok, 16);
                    if(ok)
                        light->color(QColor::fromRgb((QRgb)rgb));
                } else {
                    // List of keys. Parse color first
                    QStringList set = comp.split(":");
                    bool ok;
                    int rgb = set[1].toInt(&ok, 16);
                    if(ok){
                        QColor color = QColor::fromRgb((QRgb)rgb);
                        // Parse keys
                        QStringList keys = set[0].split(",");
                        foreach(QString key, keys){
                            light->color(key, color);
                            if(key == "light")
                                // Extrapolate the Light key to the M-keys and Lock key, since those will be set to black on exit
                                lightColor = color;
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
        }
    }
}

KeyMap Kb::getKeyMap(){
    return KeyMap::standard(_model, _layout);
}

void Kb::setCurrentProfile(KbProfile *profile, bool spontaneous){
    while(profile->modes.count() < hwModeCount)
        profile->modes.append(new KbMode(this, getKeyMap()));
    KbMode* mode = profile->currentMode;
    if(!mode)
        mode = profile->currentMode = profile->modes.first();
    setCurrentMode(profile, mode, spontaneous);
}

void Kb::setCurrentMode(KbProfile* profile, KbMode* mode, bool spontaneous){
    if(currentProfile != profile){
        currentProfile = profile;
        emit profileChanged();
    }
    if(currentMode != mode || currentProfile->currentMode != mode){
        currentMode = currentProfile->currentMode = mode;
        emit modeChanged(spontaneous);
    }
}


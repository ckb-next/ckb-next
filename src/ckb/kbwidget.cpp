#include <cmath>
#include <fcntl.h>
#include <unistd.h>
#include <QMenu>
#include <QSettings>
#include <QUrl>
#include "kbwidget.h"
#include "kblightwidget.h"
#include "media.h"
#include "ui_kbwidget.h"
#include "ui_kblightwidget.h"

KbWidget::KbWidget(QWidget *parent, const QString &path, const QString &prefsBase) :
    QWidget(parent), devpath(path), cmdpath(path + "/cmd"), disconnect(false), notifyNumber(0), layoutLoaded(false), hwLoading(true), queueProfileSwitch(true), hwProfile(0), currentProfile(0), prevLight(0),
    ui(new Ui::KbWidget)
{
    ui->setupUi(this);

    // Get the model, serial number, and firmware version
    QFile mpath(path + "/model"), spath(path + "/serial"), fpath(path + "/fwversion");
    if(mpath.open(QIODevice::ReadOnly)){
        model = mpath.read(100);
        model = model.remove("Corsair").remove("Gaming Keyboard").remove("Keyboard").remove("Bootloader").trimmed();
        mpath.close();
    }
    if(spath.open(QIODevice::ReadOnly)){
        serial = spath.read(100);
        serial = serial.trimmed().toUpper();
        spath.close();
        ui->serialLabel->setText(serial);
    }
    if(fpath.open(QIODevice::ReadOnly)){
        firmware = fpath.read(100);
        firmware = QString::number(firmware.trimmed().toInt() / 100., 'f', 2);
        fpath.close();
        ui->fwLabel->setText(firmware);
    }

    prefsPath = prefsBase + "/" + serial;
    hwModeCount = (model.indexOf("K95") >= 0) ? 3 : 1;

    // Load profiles from stored settings
    QSettings settings;
    settings.beginGroup(prefsPath);
    KbProfile* newCurrentProfile = 0;
    QString current = settings.value("CurrentProfile").toString().trimmed().toUpper();
    foreach(QString guid, settings.value("Profiles").toString().split(" ")){
        guid = guid.trimmed().toUpper();
        if(guid != ""){
            addProfile(new KbProfile(this, getKeyMap(), settings, guid));
            if(guid == current || !newCurrentProfile)
                newCurrentProfile = profiles.last();
        }
    }
    if(newCurrentProfile)
        setCurrentProfile(newCurrentProfile);

    // Find an available notification node
    for(int i = 1; i < 10; i++){
        QString notify = QString(path + "/notify%1").arg(i);
        if(!QFile::exists(notify)){
            notifyNumber = i;
            notifypath = notify;
            break;
        }
    }
    QFile cmd;
    getCmd(cmd);
    if(!cmd.isOpen())
        return;
    if(notifyNumber > 0){
        cmd.write(QString("notifyon %1\n").arg(notifyNumber).toLatin1());
        cmd.flush();
    } else
        // If there were none, take notify0
        notifypath = path + "/notify0";
    cmd.write("get :hwprofileid :layout\n");
    for(int i = 0; i < hwModeCount; i++)
        cmd.write(QString("mode %1 get :hwid\n").arg(i + 1).toLatin1());
    cmd.close();
    // Wait a while for the node to open
    usleep(100000);
}

void KbWidget::save(){
    QSettings settings;
    settings.remove(prefsPath);
    settings.beginGroup(prefsPath);
    QString guids, currentGuid;
    foreach(KbProfile* profile, profiles){
        guids.append(" " + profile->guid());
        if(profile == currentProfile)
            currentGuid = profile->guid();
        profile->save(settings);
    }
    settings.setValue("CurrentProfile", currentGuid);
    settings.setValue("Profiles", guids.trimmed());
}

KbWidget::~KbWidget(){
    QFile cmd;
    getCmd(cmd);
    if(cmd.isOpen()){
        // Save settings
        save();
        // Reset to hardware profile and save it to the device
        if(hwProfile){
            currentProfile = hwProfile;
            on_hwSaveButton_clicked();
        }
        if(notifyNumber > 0)
            cmd.write(QString("notifyoff %1\n").arg(notifyNumber).toLatin1());
        cmd.close();
    }
    delete ui;
}

void KbWidget::setProfileName(KbProfile* profile, const QString& name){
    profile->name(name);
    int index = profiles.indexOf(profile);
    ui->profileBox->setItemText(index, name);
    ui->profilesList->item(index)->setText(name);
}

void KbWidget::setModeName(KbProfile* profile, int mode, const QString& name){
    profile->modeName(mode, name);
    if(profile == currentProfile){
        ui->modeBox->setItemText(mode, name);
        ui->modesList->item(mode)->setText(name);
    }
}

void KbWidget::updateUI(){
    if(ui->modesList->count() != ui->modeBox->count() || ui->modesList->count() != currentProfile->modeCount()
            || ui->profilesList->count() != ui->profileBox->count() || ui->profilesList->count() != profiles.count())
        return;

    ui->lightWidget->setLight(currentLight());

    if(currentMode() == 0)
        ui->modeUpButton->setEnabled(false);
    else
        ui->modeUpButton->setEnabled(true);
    if(currentMode() == currentProfile->modeCount() - 1)
        ui->modeDownButton->setEnabled(false);
    else
        ui->modeDownButton->setEnabled(true);

    if(currentProfile == profiles.first())
        ui->profileUpButton->setEnabled(false);
    else
        ui->profileUpButton->setEnabled(true);
    if(currentProfile == profiles.last())
        ui->profileDownButton->setEnabled(false);
    else
        ui->profileDownButton->setEnabled(true);

    ui->inactiveSwitchCheck->setChecked(currentLight()->inactive() >= 0);
    ui->inactiveSwitchBox->setCurrentIndex(currentLight()->inactive() >= 0 ? currentLight()->inactive() : KbLight::MAX_INACTIVE);
    ui->muteCheck->setChecked(currentLight()->showMute());

    int i = 0;
    ui->modeBox->setIconSize(QSize(24, 18));
    ui->profileBox->setIconSize(QSize(24, 18));
    ui->modesList->setIconSize(QSize(24, 18));
    ui->profilesList->setIconSize(QSize(24, 18));
    foreach(KbProfile* profile, profiles){
        QIcon profileIcon = (profile == hwProfile) ? QIcon(":/img/icon_profile_hardware.png") : QIcon(":/img/icon_profile.png");
        ui->profileBox->setItemIcon(i, profileIcon);
        ui->profilesList->item(i)->setIcon(profileIcon);
        if(profile == currentProfile){
            for(int j = 0; j < profile->modeCount(); j++){
                QIcon modeIcon;
                if(j >= hwModeCount)
                    modeIcon = QIcon(":/img/icon_mode.png");
                else
                    modeIcon = QIcon(QString(profile == hwProfile ? ":/img/icon_mode%1_hardware.png" : ":/img/icon_mode%1.png").arg(j + 1));
                ui->modesList->item(j)->setIcon(modeIcon);
                ui->modeBox->setItemIcon(j, modeIcon);
            }
        }
        i++;
    }
}

void KbWidget::addProfile(KbProfile* profile){
    profiles.append(profile);
    ui->profilesList->addItem(profile->name());
    ui->profileBox->addItem(profile->name());
    QListWidgetItem* item = ui->profilesList->item(ui->profilesList->count() - 1);
    item->setFlags(item->flags() | Qt::ItemIsEditable);

    if(!currentProfile)
        currentProfile = profiles.last();
    // Add hardware modes to the new profile
    while(profile->modeCount() < hwModeCount)
        addMode(profile);
}

void KbWidget::addMode(KbProfile* profile){
    profile->modeName(profile->modeCount(), "Unnamed");
    if(profile == currentProfile){
        ui->modesList->addItem("Unnamed");
        ui->modeBox->addItem("Unnamed");
        QListWidgetItem* item = ui->modesList->item(ui->modesList->count() - 1);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        updateUI();
    }
}

void KbWidget::duplicateMode(KbProfile *profile, int mode){
    profile->duplicateMode(mode);
    if(profile == currentProfile){
        QListWidgetItem* item = ui->modesList->item(mode)->clone();
        item->setText(profile->modeName(mode + 1));
        ui->modesList->insertItem(mode + 1, item);
        ui->modeBox->insertItem(mode + 1, profile->modeName(mode + 1));
        setCurrentMode(currentProfile, mode + 1);
        // Reset daemon profile
        queueProfileSwitch = true;
    }
}

void KbWidget::duplicateProfile(KbProfile* profile){
    KbProfile* newProfile = new KbProfile(this, *profile);
    newProfile->name(newProfile->name() + " copy");
    int index = profiles.indexOf(profile);
    profiles.insert(index + 1, newProfile);

    QListWidgetItem* item = ui->profilesList->item(index)->clone();
    item->setText(newProfile->name());
    ui->profilesList->insertItem(index + 1, item);
    ui->profileBox->insertItem(index + 1, newProfile->name());
    setCurrentProfile(newProfile);
    // Reset daemon profile
    queueProfileSwitch = true;
}

void KbWidget::deleteMode(KbProfile *profile, int mode){
    profile->deleteMode(mode);
    if(profile == currentProfile){
        ui->modesList->takeItem(mode);
        ui->modeBox->removeItem(mode);
        setCurrentMode(currentProfile, mode >= currentProfile->modeCount() ? currentProfile->modeCount() - 1 : mode);
        // Reset daemon profile
        queueProfileSwitch = true;
    }
}

void KbWidget::deleteProfile(KbProfile *profile){
    int index = profiles.indexOf(profile);
    profiles.removeAt(index);
    if(profile == currentProfile){
        ui->profilesList->takeItem(index);
        ui->profileBox->removeItem(index);
        setCurrentProfile(profiles[index >= profiles.count() ? profiles.count() - 1 : index]);
        // Reset daemon profile
        queueProfileSwitch = true;
    }
}

void KbWidget::setCurrentProfile(KbProfile* profile){
    if(profile == currentProfile)
        return;
    setCurrentMode(profile, profile->currentMode());
}

void KbWidget::setCurrentMode(KbProfile* profile, int mode){
    if(profile != currentProfile){
        currentProfile = profile;
        int index = profiles.indexOf(profile);
        ui->profileBox->setCurrentIndex(index);
        ui->profilesList->setCurrentRow(index);
        ui->modesList->clear();
        ui->modeBox->clear();
        int count = profile->modeCount();
        for(int i = 0; i < count; i++){
            ui->modesList->addItem(profile->modeName(i));
            QListWidgetItem* item = ui->modesList->item(ui->modesList->count() - 1);
            item->setFlags(item->flags() | Qt::ItemIsEditable);
            ui->modeBox->addItem(profile->modeName(i));
        }
    }
    profile->currentMode(mode);
    ui->modesList->setCurrentRow(mode);
    ui->modeBox->setCurrentIndex(mode);
    updateUI();
}

KeyMap KbWidget::getKeyMap(){
    return KeyMap::standard(model.indexOf("K95") >= 0 ? KeyMap::K95 : KeyMap::K70, (KeyMap::Layout)ui->layoutBox->currentIndex());
}

void KbWidget::getCmd(QFile& file){
    int fd = open(cmdpath.toLatin1().constData(), O_WRONLY | O_NONBLOCK);
    if(!file.open(fd, QIODevice::WriteOnly, QFileDevice::AutoCloseHandle))
        cmdpath = notifypath = devpath = "";
}

void KbWidget::on_layoutBox_currentIndexChanged(int index){
    QFile cmd;
    getCmd(cmd);
    if(!cmd.isOpen())
        return;
    if(index == 1)
        cmd.write("layout uk");
    else
        cmd.write("layout us");
    cmd.write("\n");
    cmd.close();
}

void KbWidget::on_profileBox_currentIndexChanged(int index){
    if(index < 0)
        return;
    if(index >= profiles.count())
        index = profiles.count() - 1;
    setCurrentProfile(profiles[index]);
}

void KbWidget::on_profilesList_currentRowChanged(int currentRow){
    if(currentRow < 0)
        return;
    if(currentRow >= profiles.count())
        currentRow = profiles.count() - 1;
    setCurrentProfile(profiles[currentRow]);
}

void KbWidget::on_modeBox_currentIndexChanged(int index){
    if(index < 0)
        return;
    if(index >= currentProfile->modeCount())
        index = currentProfile->modeCount() - 1;
    currentProfile->currentMode(index);
    ui->modesList->setCurrentRow(index);
    updateUI();
}

void KbWidget::on_modesList_currentRowChanged(int currentRow){
    if(currentRow < 0)
        return;
    if(currentRow >= currentProfile->modeCount())
        currentRow = currentProfile->modeCount() - 1;
    currentProfile->currentMode(currentRow);
    ui->modeBox->setCurrentIndex(currentRow);
    updateUI();
}

void KbWidget::on_modesList_itemChanged(QListWidgetItem *item){
    // Mode name changed?
    int count = currentProfile->modeCount();
    if(count == 0 || ui->modesList->count() != count)
        return;
    int index = 0;
    // Find the mode's index
    for(; index < count; index++){
        if(ui->modesList->item(index) == item)
            break;
    }
    QString newName = item->text().trimmed(), oldName = currentProfile->modeName(index);
    if(newName != oldName){
        // Don't allow a blank name
        if(newName == ""){
            item->setText(oldName);
            return;
        }
        currentProfile->modeName(index, newName);
        ui->modeBox->setItemText(index, newName);
        // Write the name to the daemon
        QFile cmd;
        getCmd(cmd);
        if(!cmd.isOpen())
            return;
        cmd.write(QString("mode %1 name ").arg(index + 1).toLatin1());
        cmd.write(QUrl::toPercentEncoding(newName));
        cmd.write("\n");
        cmd.close();
    }
}

void KbWidget::on_profilesList_itemChanged(QListWidgetItem *item){
    // Profile name changed?
    int count = profiles.count();
    if(count == 0 || ui->profilesList->count() != count)
        return;
    int index = 0;
    // Find the profiles's index
    for(; index < count; index++){
        if(ui->profilesList->item(index) == item)
            break;
    }
    KbProfile* profile = profiles[index];
    QString newName = item->text().trimmed(), oldName = profile->name();
    if(newName != oldName){
        // Don't allow a blank name
        if(newName == ""){
            item->setText(oldName);
            return;
        }
        currentProfile->name(newName);
        ui->profileBox->setItemText(profiles.indexOf(profile), newName);
        // Write the name to the daemon
        QFile cmd;
        getCmd(cmd);
        if(!cmd.isOpen())
            return;
        cmd.write("profilename ");
        cmd.write(QUrl::toPercentEncoding(newName));
        cmd.write("\n");
        cmd.close();
    }
}

void KbWidget::on_hwSaveButton_clicked(){
    QFile cmd;
    getCmd(cmd);
    if(!cmd.isOpen())
        return;
    if(currentProfile != hwProfile)
        save();
    switchProfile(cmd);
    cmd.write("hwsave\n");
    // Load the new modification times for the profile
    hwLoading = false;
    cmd.write("get :hwprofileid\n");
    for(int i = 0; i < hwModeCount; i++)
        cmd.write(QString("mode %1 get :hwid\n").arg(i + 1).toLatin1());
    cmd.close();
    hwProfile = currentProfile;
    updateUI();
}

void KbWidget::on_tabWidget_currentChanged(int index){
    if(index == 0)
        ui->lightWidget->ui->rgbWidget->clearSelection();
}

void KbWidget::frameUpdate(){
    QFile cmd;
    getCmd(cmd);
    if(!cmd.isOpen())
        return;

    if(queueProfileSwitch){
        switchProfile(cmd);
        queueProfileSwitch = false;
    }

    // Read from the notification node
    readInput(cmd);
    // Get system mute state
    muteState mute = getMuteState();
    if(mute == UNKNOWN)
        mute = UNMUTED;
    // If the mode changed, close the old one first
    KbLight* light = currentLight();
    if(!light){
        cmd.close();
        return;
    }
    if(prevLight && prevLight != light){
        prevLight->close(cmd);
        cmd.write("\n");
    }

    // Output the current mode/animation
    light->frameUpdate(cmd, mute != MUTED);
    cmd.close();
    prevLight = light;
}

void KbWidget::readInput(QFile& cmd){
    // Read from the notification node
    QFile notify;
    int fd = open(notifypath.toLatin1().constData(), O_RDONLY | O_NONBLOCK);
    if(notify.open(fd, QIODevice::ReadOnly, QFileDevice::AutoCloseHandle)){
        QString line;
        while((line = notify.readLine()) != ""){
            QStringList components = line.trimmed().split(" ");
            if(components[0] == "layout"){
                // Layout change - set new layout
                if(components[1] == "uk")
                    ui->layoutBox->setCurrentIndex(1);
                else if(components[1] == "us")
                    ui->layoutBox->setCurrentIndex(0);

                if(!layoutLoaded){
                    // If the layout wasn't loaded previously, the RGB settings need to be fetched now
                    QSettings settings;
                    settings.beginGroup(prefsPath);
                    // Set all light setups to the new layout
                    foreach(KbProfile* profile, profiles){
                        profile->keyLayout((KeyMap::Layout)ui->layoutBox->currentIndex());
                        profile->reloadLight(settings);
                    }
                    layoutLoaded = true;
                } else {
                    foreach(KbProfile* profile, profiles)
                        profile->keyLayout((KeyMap::Layout)ui->layoutBox->currentIndex());
                }
                queueProfileSwitch = true;
                updateUI();
            } else if(components[0] == "key"){
                // Key event
                if(components[1] == "+light"){
                    KbLight* light = currentLight();
                    int brightness = light->brightness() - 1;
                    if(brightness < 0)
                        brightness = KbLight::MAX_BRIGHTNESS;
                    ui->lightWidget->ui->brightnessBox->setCurrentIndex(brightness);
                } else if(components[1] == "+m1"){
                    setCurrentMode(currentProfile, 0);
                } else if(components[1] == "+m2"){
                    setCurrentMode(currentProfile, 1);
                } else if(components[1] == "+m3"){
                    setCurrentMode(currentProfile, 2);
                } else if(components[1] == "+lock"){
#ifndef __APPLE__
                    KbLight* light = currentLight();
                    light->winLock(cmd, !light->winLock());
                    cmd.write("\n");
#endif
                }
            } else if(components[0] == "hwprofileid"){
                // Find the hardware profile in the list of profiles
                QString guid = components[1];
                QString modified = components[2];
                foreach(KbProfile* profile, profiles){
                    if(profile->guid() == guid){
                        hwProfile = profile;
                        break;
                    }
                }
                // If it wasn't found, create it
                if(!hwProfile){
                    addProfile(new KbProfile(this, getKeyMap(), guid, modified));
                    hwProfile = profiles.last();
                    hwProfile->hwAssigned(false);
                    hwLoading = true;
                    cmd.write("get :hwprofilename\n");
                } else {
                    hwProfile->hwAssigned(true);
                    if(hwProfile->modified() != modified){
                        // If it's been updated (and we're loading hardware profiles), fetch its name
                        hwProfile->modified(modified);
                        if(hwLoading)
                            cmd.write("get :hwprofilename\n");
                    }
                }
            } else if(components[0] == "hwprofilename"){
                // Profile name
                QString name = QUrl::fromPercentEncoding(components[1].toUtf8());
                QString oldName = hwProfile->name();
                if(!(oldName.length() >= name.length() && oldName.left(name.length()) == name))
                    // Don't change the name if it's a truncated version of what we already have
                    setProfileName(hwProfile, name);
            } else if(components[0] == "mode"){
                if(components.count() < 3)
                    continue;
                int mode = components[1].toInt() - 1;
                if(components[2] == "hwid"){
                    // Mode ID
                    QString guid = components[3];
                    QString modified = components[4];
                    // See if the profile has the same mode in the same place (or a blank mode here)
                    hwModes[mode] = -1;
                    if(!hwProfile->hwAssigned() || hwProfile->modeGuid(mode) == guid){
                        hwProfile->modeGuid(mode, guid);
                        hwModes[mode] = mode;
                    } else {
                        // Otherwise, look for it
                        int count = hwProfile->modeCount();
                        for(int i = 0; i < count; i++){
                            if(hwProfile->modeGuid(i) == guid){
                                hwModes[mode] = i;
                                break;
                            }
                        }
                        if(hwModes[mode] == -1){
                            // If it still wasn't found, add it
                            addMode(hwProfile);
                            hwModes[mode] = hwProfile->modeCount() - 1;
                            hwProfile->modeGuid(hwModes[mode], guid);
                        }
                    }
                    // Update modification time
                    if(hwProfile->modeModified(hwModes[mode]) != modified){
                        hwProfile->modeModified(hwModes[mode], modified);
                        // If loading hardware profile, fetch the updated data
                        if(hwLoading)
                            cmd.write(QString("mode %1 get :hwname :hwrgb\n").arg(mode + 1).toLatin1());
                    }
                } else if(components[2] == "hwname"){
                    // Mode name - update list
                    QString name = QUrl::fromPercentEncoding(components[3].toUtf8());
                    QString oldName = hwProfile->modeName(hwModes[mode]);
                    if(!(oldName.length() >= name.length() && oldName.left(name.length()) == name))
                        // Don't change the name if it's a truncated version of what we already have
                        setModeName(hwProfile, hwModes[mode], name);
                } else if(components[2] == "hwrgb"){
                    // RGB - set mode lighting
                    KbLight* light = hwProfile->modeLight(hwModes[mode]);
                    // If it's a color command, scan the input
                    QColor lightColor = QColor();
                    for(int i = 3; i < components.count(); i++){
                        QString comp = components[i];
                        if(comp.indexOf(":") < 0){
                            // Single hex constant?
                            bool ok;
                            int rgb = comp.toInt(&ok, 16);
                            if(ok)
                                light->colorAll(QColor::fromRgb((QRgb)rgb));
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
                    updateUI();
                }
            }
        }
        notify.close();
    }
}

void KbWidget::switchProfile(QFile& cmd){
    prevLight = 0;
    if(!cmd.isOpen() || !currentProfile)
        return;
    cmd.write("eraseprofile");
    // Write the profile name and ID
    cmd.write(" profilename ");
    cmd.write(QUrl::toPercentEncoding(currentProfile->name()));
    cmd.write(" profileid ");
    cmd.write(currentProfile->guid().toLatin1());
    cmd.write(" ");
    cmd.write(currentProfile->modified().toLatin1());
    // Close every mode so that only the base colors are written, not the animations
    int count = currentProfile->modeCount();
    for(int mode = 0; mode < count; mode++){
        cmd.write("\n");
        KbLight* light = currentProfile->modeLight(mode);
        light->close(cmd);
#ifndef __APPLE__
        // Bindings will be reset, so re-enable Win Lock if it's turned on
        if(light->winLock()){
            cmd.write(" ");
            light->winLock(cmd, true);
        }
#endif
        if(mode == currentMode())
            cmd.write(" switch");
        // Write the mode name and ID
        cmd.write(" name ");
        cmd.write(QUrl::toPercentEncoding(currentProfile->modeName(mode)));
        cmd.write(" id ");
        cmd.write(currentProfile->modeGuid(mode).toLatin1());
        cmd.write(" ");
        cmd.write(currentProfile->modeModified(mode).toLatin1());
    }
    cmd.write("\n");
}

void KbWidget::on_modesList_customContextMenuRequested(const QPoint &pos){
    QListWidgetItem* item = ui->modesList->itemAt(pos);
    if(!item)
        return;
    // Find the mode's index
    int count = currentProfile->modeCount();
    int index = 0;
    for(; index < count; index++){
        if(ui->modesList->item(index) == item)
            break;
    }
    setCurrentMode(currentProfile, index);

    QMenu menu(this);
    QAction* rename = new QAction("Rename...", this);
    QAction* duplicate = new QAction("Duplicate", this);
    QAction* del = new QAction("Delete", this);
    if(currentProfile->modeCount() <= hwModeCount)
        del->setEnabled(false);
    QAction* moveup = new QAction("Move Up", this);
    if(index == 0)
        moveup->setEnabled(false);
    QAction* movedown = new QAction("Move Down", this);
    if(index == currentProfile->modeCount() - 1)
        movedown->setEnabled(false);
    menu.addAction(rename);
    menu.addAction(duplicate);
    menu.addAction(del);
    menu.addSeparator();
    menu.addAction(moveup);
    menu.addAction(movedown);
    QAction* result = menu.exec(QCursor::pos());
    if(result == rename){
        ui->modesList->editItem(item);
    } else if(result == duplicate){
        duplicateMode(currentProfile, index);
    } else if(result == del){
        if(currentProfile->modeCount() <= hwModeCount)
            return;
        deleteMode(currentProfile, index);
    } else if(result == moveup){
        on_modeUpButton_clicked();
    } else if(result == movedown){
        on_modeDownButton_clicked();
    }
}

void KbWidget::on_profilesList_customContextMenuRequested(const QPoint &pos){
    QListWidgetItem* item = ui->profilesList->itemAt(pos);
    if(!item)
        return;
    // Find the mode's index
    int count = profiles.count();
    int index = 0;
    for(; index < count; index++){
        if(ui->profilesList->item(index) == item)
            break;
    }
    setCurrentProfile(profiles[index]);

    QMenu menu(this);
    QAction* rename = new QAction("Rename...", this);
    QAction* duplicate = new QAction("Duplicate", this);
    QAction* del = new QAction("Delete", this);
    if(profiles.count() == 1)
        del->setEnabled(false);
    QAction* moveup = new QAction("Move Up", this);
    if(index == 0)
        moveup->setEnabled(false);
    QAction* movedown = new QAction("Move Down", this);
    if(index == profiles.count() - 1)
        movedown->setEnabled(false);
    menu.addAction(rename);
    menu.addAction(duplicate);
    menu.addAction(del);
    menu.addSeparator();
    menu.addAction(moveup);
    menu.addAction(movedown);
    QAction* result = menu.exec(QCursor::pos());
    if(result == rename){
        ui->profilesList->editItem(item);
    } else if(result == duplicate){
        duplicateProfile(currentProfile);
    } else if(result == del){
        if(profiles.count() == 1)
            return;
        deleteProfile(currentProfile);
    } else if(result == moveup){
        on_profileUpButton_clicked();
    } else if(result == movedown){
        on_profileDownButton_clicked();
    }
}

void KbWidget::on_modeUpButton_clicked(){
    int mode = currentMode();
    if(mode == 0)
        return;
    currentProfile->moveMode(mode, mode - 1);
    QListWidgetItem* item = ui->modesList->takeItem(mode);
    ui->modesList->insertItem(mode - 1, item);
    ui->modeBox->removeItem(mode);
    ui->modeBox->insertItem(mode - 1, currentProfile->modeName(mode - 1));
    setCurrentMode(currentProfile, mode - 1);
    // Reset the profile on the daemon so that the modes get moved properly
    queueProfileSwitch = true;
}

void KbWidget::on_modeDownButton_clicked(){
    int mode = currentMode();
    if(mode == currentProfile->modeCount() - 1)
        return;
    currentProfile->moveMode(mode, mode + 1);
    QListWidgetItem* item = ui->modesList->takeItem(mode);
    ui->modesList->insertItem(mode + 1, item);
    setCurrentMode(currentProfile, mode + 1);
    ui->modeBox->removeItem(mode);
    ui->modeBox->insertItem(mode + 1, currentProfile->modeName(mode + 1));
    setCurrentMode(currentProfile, mode + 1);
    // Reset the profile on the daemon so that the modes get moved properly
    queueProfileSwitch = true;
}

void KbWidget::on_newModeButton_clicked(){
    addMode(currentProfile);
    QListWidgetItem* item = ui->modesList->item(currentProfile->modeCount() - 1);
    ui->modesList->setCurrentItem(item);
    ui->modesList->editItem(item);
}

void KbWidget::on_profileUpButton_clicked(){
    int profile = profiles.indexOf(currentProfile);
    if(profile == 0)
        return;
    profiles.move(profile, profile - 1);
    QListWidgetItem* item = ui->profilesList->takeItem(profile);
    ui->profilesList->insertItem(profile - 1, item);
    ui->profileBox->removeItem(profile);
    ui->profileBox->insertItem(profile - 1, currentProfile->name());
    setCurrentProfile(currentProfile);
    ui->profileBox->setCurrentIndex(profile - 1);
    ui->profilesList->setCurrentRow(profile - 1);
}

void KbWidget::on_profileDownButton_clicked(){
    int profile = profiles.indexOf(currentProfile);
    if(profile == profiles.count() - 1)
        return;
    profiles.move(profile, profile + 1);
    QListWidgetItem* item = ui->profilesList->takeItem(profile);
    ui->profilesList->insertItem(profile + 1, item);
    ui->profileBox->removeItem(profile);
    ui->profileBox->insertItem(profile + 1, currentProfile->name());
    setCurrentProfile(currentProfile);
    ui->profileBox->setCurrentIndex(profile + 1);
    ui->profilesList->setCurrentRow(profile + 1);
}

void KbWidget::on_newProfileButton_clicked(){
    addProfile();
    QListWidgetItem* item = ui->profilesList->item(profiles.count() - 1);
    ui->profilesList->setCurrentItem(item);
    ui->profilesList->editItem(item);
    // Switch to the new profile
    queueProfileSwitch = true;
}

void KbWidget::on_showModes_toggled(bool checked){
    ui->showModes_2->setChecked(checked);
    ui->showProfiles->setChecked(!checked);
    ui->showProfiles_2->setChecked(!checked);
    ui->profileModeSwitch->setCurrentIndex(checked ? 0 : 1);
}

void KbWidget::on_showProfiles_toggled(bool checked){
    ui->showModes->setChecked(!checked);
    ui->showModes_2->setChecked(!checked);
    ui->showProfiles_2->setChecked(checked);
    ui->profileModeSwitch->setCurrentIndex(checked ? 1 : 0);
}

void KbWidget::on_showModes_2_toggled(bool checked){
    ui->showModes->setChecked(checked);
    ui->showProfiles->setChecked(!checked);
    ui->showProfiles_2->setChecked(!checked);
    ui->profileModeSwitch->setCurrentIndex(checked ? 0 : 1);
}

void KbWidget::on_showProfiles_2_toggled(bool checked){
    ui->showModes->setChecked(!checked);
    ui->showModes_2->setChecked(!checked);
    ui->showProfiles->setChecked(checked);
    ui->profileModeSwitch->setCurrentIndex(checked ? 1 : 0);
}

void KbWidget::on_inactiveSwitchBox_activated(int index){
    if(ui->inactiveSwitchCheck->isChecked())
        currentLight()->inactive(index);
}

void KbWidget::on_inactiveSwitchCheck_clicked(bool checked){
    if(checked){
        currentLight()->inactive(ui->inactiveSwitchBox->currentIndex());
        currentLight()->showMute(ui->muteCheck->isChecked());
        ui->inactiveSwitchBox->setEnabled(true);
        ui->muteCheck->setEnabled(true);
    } else {
        currentLight()->inactive(-1);
        ui->inactiveSwitchBox->setEnabled(false);
        ui->muteCheck->setEnabled(false);
    }
}

void KbWidget::on_muteCheck_clicked(bool checked){
    if(ui->inactiveSwitchCheck->isCheckable())
        currentLight()->showMute(checked);
}

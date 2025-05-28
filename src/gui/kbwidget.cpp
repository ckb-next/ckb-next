#include <cmath>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QUrl>
#include <QTimer>
#include "ckbsettings.h"
#include "fwupgradedialog.h"
#include "kbfirmware.h"
#include "kbwidget.h"
#include "kblightwidget.h"
#include "kbprofiledialog.h"
#include "ui_kbwidget.h"
#include "ui_kblightwidget.h"
#include "kbmodeeventmgr.h"
#include "mainwindow.h"
#include <QItemSelectionModel>
#include "modelisttablemodel.h"
#include "ckbmainbackgroundcolour.h"

KbWidget::KbWidget(QWidget *parent, Kb *_device, XWindowDetector* windowDetector) :
    QWidget(parent),
    device(_device), hasShownNewFW(false),
    ui(new Ui::KbWidget), currentMode(nullptr),
    prevmode(nullptr)
{
    CkbMainBackgroundColour::init(parent);
    ui->setupUi(this);
    Q_ASSERT(ui->pollRateBox->count() == Kb::POLLRATE_COUNT);
    ui->modesList->setDevice(device);
    connect(device, &Kb::profileRenamed, this, &KbWidget::updateProfileList);
    connect(device, &Kb::profileAdded, this, &KbWidget::updateProfileList);
    connect(device, &Kb::modeChanged, this, &KbWidget::modeChanged);
    connect(device, &Kb::infoUpdated, this, &KbWidget::devUpdate);
    connect(ui->batteryTrayBox, &QCheckBox::stateChanged, this, &KbWidget::batteryTrayBox_stateChanged);
    connect(MainWindow::mainWindow, &MainWindow::switchToProfileCLI, this, &KbWidget::switchToProfile);
    connect(MainWindow::mainWindow, &MainWindow::switchToModeCLI, this, &KbWidget::switchToMode);
    connect(ui->modesList->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &KbWidget::currentSelectionChanged);

#ifdef USE_XCB_EWMH
    if(windowDetector)
        connect(windowDetector, &XWindowDetector::activeWindowChanged, this, &KbWidget::switchToModeByFocus);
#endif

    // Remove the Legacy Lighting tab on anything other than the K95L
    if(device->model() != KeyMap::K95L)
        ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->legacyLightTab));

    // Remove the Lighting and Performance tabs from non-RGB keyboards
    if(!device->features.contains("rgb")){
        if(device->model() != KeyMap::M95){
            ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->mPerfTab));
            ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->lightTab));
        }
        ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->kPerfTab));
    } else {
        // Remove mouse Performance tab from non-mice
        if(!device->isMouse())
            ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->mPerfTab));
        // Remove keyboard Performance tab from non-keyboards
        if(!device->isKeyboard())
            ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->kPerfTab));
    }

    // If we have an M95, set the performance and lighting tabs as such
    if(device->model() == KeyMap::M95){
        ui->mPerfWidget->setLegacyM95();
        ui->lightWidget->setLegacyM95();
    }

    // If we have a DARK CORE or Dark Core SE, then change the performance tab accordingly
    if(device->model() == KeyMap::DARKCORE)
        ui->mPerfWidget->setDarkCore();

    // If the device is supports it, show the battery
    if(device->features.contains("battery")){
        connect(device, &Kb::batteryChanged, this, &KbWidget::updateBattery);
    } else {
        ui->batteryLabel->hide();
        ui->batteryStatusLabel->hide();
        ui->batteryTrayBox->hide();
    }

    // Hide poll rate and FW update as appropriate
    if(device->pollrate == Kb::POLLRATE_UNKNOWN){
        ui->pollRateBox->hide();
        ui->pollLabel2->hide();
        ui->horizontalLayout_2->removeItem(ui->horizontalSpacer_4);
        delete ui->horizontalSpacer_4;
        ui->horizontalSpacer_4 = nullptr;
    }
    if(!device->features.contains("fwupdate")){
        ui->fwUpdButton->hide();
        ui->fwUpdLabel->hide();
        ui->fwUpdLayout->removeItem(ui->fwUpdLayout->itemAt(1));
    }
    // Remove unsupported pollrates
    // Block signals so that the pollrate doesn't change
    bool block = ui->pollRateBox->blockSignals(true);
    if(device->maxpollrate != Kb::POLLRATE_UNKNOWN){
        for(int i = 0; i < (Kb::POLLRATE_COUNT - 1) - device->maxpollrate; i++)
            ui->pollRateBox->removeItem(0);
    }
    ui->pollRateBox->blockSignals(block);

    // Remove binding tab if the device doesn't support it
    if(!device->features.contains("bind")){
        ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->bindTab));
    }
    // Set monochrome mode according to hardware
    if(device->monochrome)
        ui->lightWidget->setMonochrome();
    // Disable Save to hardware button for unsupported devices
    if(!device->hwload){
        ui->hwSaveButton->setDisabled(true);
        ui->hwSaveButton->setToolTip(QString(tr("Saving to hardware is not supported on this device.")));
    }
    // Read device layout
    if(device->features.contains("bind")){
        // Clear the "Default" value
        ui->layoutBox->clear();

        // Load the current device's layout from the settings
        QString layoutSettingsPath("Devices/%1");
        CkbSettings settings(layoutSettingsPath.arg(device->usbSerial));

        QList<QPair<int, QString>> layoutnames = KeyMap::layoutNames(device->hwlayout);

        // Enable the ComboBox only if there is more than one supported layout
        if(layoutnames.count() > 1)
            ui->layoutBox->setEnabled(true);

        for(int i = 0; i < layoutnames.count(); i++)
            ui->layoutBox->addItem(layoutnames[i].second, layoutnames[i].first);

        KeyMap::Layout layout = KeyMap::getLayout(settings.value("hwLayout").toString());
        if(layout == KeyMap::NO_LAYOUT){
            // If the layout hasn't been set yet, first check if one was set globally from a previous version
            // If not, try to pick an appropriate one that's supported by the hardware
            KeyMap::Layout oldLayout = KeyMap::getLayout(CkbSettings::get("Program/KbdLayout").toString());
            if(oldLayout == KeyMap::NO_LAYOUT){
                layout = KeyMap::locale(&layoutnames);
            } else {
                CkbSettings::set("Program/KbdLayout", "");
                layout = oldLayout;
            }
        }
        // Find the position of the layout in the QComboBox and set it
        int layoutpos = -1;
        if(layout != KeyMap::NO_LAYOUT){
            for(int i = 0; i < layoutnames.count(); i++){
                if(layoutnames.at(i).first == (int)layout){
                    layoutpos = i;
                    break;
                }
            }
        }
        // If no layout was found, pick the first one from the list
        if(layoutpos == -1){
            layout = (KeyMap::Layout)layoutnames.at(0).first;
            layoutpos = 0;
        }

        ui->layoutBox->setCurrentIndex(layoutpos);

        // Set the layout and save it
        device->layout(layout, false);
    }
    else
        device->layout(KeyMap::GB, false);

    // Set max DPI for mice
    if(device->isMouse())
        ui->mPerfWidget->setMaxDpi(device->getMaxDpi());

    if(!device->adjrate){
        ui->pollRateBox->setEnabled(false);
        ui->pollRateBox->setToolTip(tr("This device does not support setting the poll rate through software."));
    }
}

KbWidget::~KbWidget(){
    delete ui;
}

void KbWidget::showDeviceTab(){
    ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->devTab));
}

void KbWidget::updateProfileList(){
    // Clear profile list and rebuild
    KbProfile* hwProfile = device->hwProfile(), *currentProfile = device->currentProfile();
    ui->profileBox->clear();
    int i = 0;
    foreach(KbProfile* profile, device->profiles()){
        ui->profileBox->addItem((profile == hwProfile) ? QIcon(":/img/icon_profile_hardware.png") : QIcon(":/img/icon_profile.png"),
                                profile->name());
        if(profile == currentProfile)
            ui->profileBox->setCurrentIndex(i);
        i++;
    }
    ui->profileBox->addItem(QIcon(":/img/icon_blank.png"), tr("Manage profiles..."));
    QFont font = ui->profileBox->font();
    font.setItalic(true);
    ui->profileBox->setItemData(ui->profileBox->count() - 1, font, Qt::FontRole);
}

void KbWidget::on_profileBox_activated(int index){
    if(index < 0)
        return;
    if(index >= device->profiles().count()){
        // "Manage profiles" option
        KbProfileDialog dialog(this);
        dialog.exec();
        updateProfileList();
        return;
    }
    device->setCurrentProfile(device->profiles()[index]);
    // Focus the mode list to highlight the whole row properly
    ui->modesList->setFocus();
    // Device will emit profileChanged() and modeChanged() signals to update UI
}

void KbWidget::modeChanged(){
    int index = device->currentProfile()->indexOf(device->currentMode());
    if(index < 0)
        return;
    // Update tabs
    ui->lightWidget->setLight(device->currentLight());
    ui->bindWidget->setBind(device->currentBind(), device->currentProfile());
    ui->kPerfWidget->setPerf(device->currentPerf(), device->currentProfile());
    ui->mPerfWidget->setPerf(device->currentPerf(), device->currentProfile());
    // Update selection
    ui->modesList->setCurrentIndex(ui->modesList->model()->index(index, 0));
    currentMode = device->currentMode();
}

void KbWidget::currentSelectionChanged(const QModelIndex& current, const QModelIndex& previous){
    if(current.row() > device->currentProfile()->modeCount() - 1){
        const int row = dynamic_cast<ModeListTableModel*>(ui->modesList->model())->addNewMode();
        ui->modesList->edit(ui->modesList->model()->index(row, ModeListTableModel::COL_MODE_NAME));
        return;
    }
    KbMode* mode = device->currentProfile()->at(current.row());
    device->setCurrentMode(mode);
}

void KbWidget::batteryTrayBox_stateChanged(int state){
    if(!device->features.contains("battery"))
        return;
    device->showBatteryIndicator = state > 0;
    device->needsSave();
    if(state){
        device->batteryIcon->show();
    } else {
        device->batteryIcon->hide();
    }
}

void KbWidget::on_modesList_customContextMenuRequested(const QPoint& pos){
    QModelIndex idx = ui->modesList->indexAt(pos);
    KbProfile* currentProfile = device->currentProfile();
    if(!idx.isValid() || !currentMode || idx.row() > currentProfile->modeCount() - 1)
        return;

    const int row = idx.row();
    if(currentProfile->modes().at(row) != currentMode){
        return;
    }

    QMenu menu(this);
    QAction* rename = new QAction(tr("Rename..."), this);
    QAction* duplicate = new QAction(tr("Duplicate"), this);
    QAction* del = new QAction(tr("Delete"), this);
    bool canDelete = (device->currentProfile()->modeCount() > device->hwModeCount);
    if(!canDelete)
        // Can't delete modes if they're required by hardware
        del->setEnabled(false);
    QAction* moveup = new QAction(tr("Move Up"), this);
#ifdef USE_XCB_EWMH
    QAction* focusevts = new QAction(tr("Manage Events"), this);
#endif
    if(row == 0)
        moveup->setEnabled(false);
    QAction* movedown = new QAction(tr("Move Down"), this);
    if(row >= currentProfile->modeCount() - 1)
        movedown->setEnabled(false);
    menu.addAction(rename);
    menu.addAction(duplicate);
    menu.addAction(del);
#ifdef USE_XCB_EWMH
    menu.addSeparator();
    menu.addAction(focusevts);
    menu.addSeparator();
#endif
    menu.addAction(moveup);
    menu.addAction(movedown);
    ui->modesList->setIgnoreFocusLoss(true);
    QAction* result = menu.exec(QCursor::pos());
    ui->modesList->setIgnoreFocusLoss(false);
    if(result == rename){
        ui->modesList->edit(ui->modesList->model()->index(idx.row(), ModeListTableModel::COL_MODE_NAME, idx.parent()));
    } else if(result == duplicate){
        KbMode* newMode = device->newMode(currentMode);
        newMode->newId();
        profileAboutToChange();
        currentProfile->insert(row + 1, newMode);
        // Update UI
        profileChanged();
        device->setCurrentMode(newMode);
    } else if(result == del){
        if(!canDelete)
            return;
        if(QMessageBox::question(this, tr("Delete mode"), tr("Are you sure you want to delete this mode?")) != QMessageBox::Yes)
            return;
        profileAboutToChange();
        currentProfile->removeAll(currentMode);
        currentMode->deleteLater();
        currentMode = nullptr;
        // Select next mode
        profileChanged();
        if(row < currentProfile->modeCount())
            device->setCurrentMode(currentProfile->modes()[row]);
        else
            device->setCurrentMode(currentProfile->modes().last());
    } else if(result == moveup){
        profileAboutToChange();
        currentProfile->removeAll(currentMode);
        currentProfile->insert(row - 1, currentMode);
        // Update UI
        profileChanged();
        modeChanged();
    } else if(result == movedown){
        profileAboutToChange();
        currentProfile->removeAll(currentMode);
        currentProfile->insert(row + 1, currentMode);
        // Update UI
        profileChanged();
        modeChanged();
    }
#ifdef USE_XCB_EWMH
     else if(result == focusevts) {
        openEventMgr(currentProfile->currentMode());
    }
#endif
}

void KbWidget::devUpdate(){
    // Update device tab
    ui->devLabel->setText(device->usbModel);
    ui->serialLabel->setText(device->usbSerial);
    ui->fwLabel->setText(device->firmware.app.toString());
    ui->bldValLabel->setText(device->firmware.bld.toString());
    // Not all WL devices have a radio BLD so these must be kept separate
    if(device->firmware.radioapp.isNull()){
        ui->wlLabel->setVisible(false);
        ui->wlValLabel->setVisible(false);
    } else {
        ui->wlValLabel->setText(device->firmware.radioapp.toString());
    }
    if(device->firmware.radiobld.isNull()){
        ui->wlBldLabel->setVisible(false);
        ui->wlBldValLabel->setVisible(false);
    } else {
        ui->wlBldValLabel->setText(device->firmware.radiobld.toString());
    }
    // This is needed so that the currentIndexChanged event doesn't fire
    // If it does, we'll end up with an always greyed out box when pollrate != 1
    bool block = ui->pollRateBox->blockSignals(true);
    ui->pollRateBox->setCurrentIndex(getPollRateBoxIdx(device->pollrate));
    ui->pollRateBox->blockSignals(block);
    ui->batteryTrayBox->setChecked(device->showBatteryIndicator);
}

void KbWidget::updateBattery(uint battery, BatteryStatus charging){
    QString label = QString("%1 (%2%), %3")
                    .arg(BatteryStatusTrayIcon::BATTERY_VALUES[BatteryStatusTrayIcon::getBatteryString(battery)],
                    QString::number(battery),
                    BatteryStatusTrayIcon::BATTERY_STATUS_VALUES[charging]);
    ui->batteryStatusLabel->setText(label);
}

void KbWidget::on_hwSaveButton_clicked(){
    profileAboutToChange();
    device->save();
    device->hwSave();
    updateProfileList();
    profileChanged();
}

void KbWidget::on_tabWidget_currentChanged(int index){
    if(!device)
        return;
    if(index == ui->tabWidget->count() - 1){
        // Device tab
        updateFwButton();
    }
}

void KbWidget::updateFwButton(){
    if(!MainWindow::mainWindow->kbfw->hasDownloaded())
        ui->fwUpdButton->setText(tr("Check for updates"));
    else {
        CkbVersionNumber newVersion = MainWindow::mainWindow->kbfw->versionForBoard(device->productID);
        const CkbVersionNumber& oldVersion = device->firmware.app;
        if(newVersion.isNull() || newVersion <= oldVersion)
            ui->fwUpdButton->setText(tr("Up to date"));
        else
            ui->fwUpdButton->setText(tr("Upgrade to v%1").arg(newVersion.toString()));
    }
}

void KbWidget::setTabBarEnabled(const bool e){
    ui->tabWidget->tabBar()->setEnabled(e);
    ui->profileBox->setEnabled(e);
    ui->modesList->setEnabled(e);
    ui->hwSaveButton->setEnabled(e);
    ui->bindWidget->setControlsEnabled(e);
}

void KbWidget::on_fwUpdButton_clicked(){
    // If alt is pressed, ignore upgrades and go straight to the manual prompt
    if(!(qApp->keyboardModifiers() & Qt::AltModifier)){
        // Check version numbers
        if(!MainWindow::mainWindow->kbfw->hasDownloaded()){
            ui->fwUpdButton->setText(tr("Checking..."));
            ui->fwUpdButton->setEnabled(false);
        }
        const CkbVersionNumber newVersion = MainWindow::mainWindow->kbfw->versionForBoard(device->productID, true);
        const CkbVersionNumber& oldVersion = device->firmware.app;
        ui->fwUpdButton->setEnabled(true);
        updateFwButton();
        if(newVersion.isNull()){
            if(QMessageBox::question(this, tr("Firmware update"), tr("<center>There was a problem getting the status for this device.<br />Would you like to select a file manually?</center>")) != QMessageBox::Yes)
                return;
            // "Yes" -> fall through to browse file
        } else if(newVersion.CkbTooOld()){
            QMessageBox::information(this, tr("Firmware update"), tr("<center>There is a new firmware available for this device (v%1).<br />However, it requires a newer version of ckb-next.<br />Please upgrade ckb-next and try again.</center>").arg(newVersion.toString()));
            return;
        } else if(newVersion <= oldVersion){
            if(QMessageBox::question(this, tr("Firmware update"), tr("<center>Your firmware is already up to date.<br />Would you like to select a file manually?</center>")) != QMessageBox::Yes)
                return;
            // "Yes" -> fall through to browse file
        } else {
            // Automatic upgrade. Fetch file from web.
            // FwUpgradeDialog can't be parented to KbWidget because KbWidget may be deleted before the dialog exits
            FwUpgradeDialog dialog(parentWidget(), newVersion, QByteArray(), device);
            dialog.exec();
            return;
        }
    }
    // Browse for file
    QString path = QFileDialog::getOpenFileName(this, tr("Select firmware file"), QStandardPaths::writableLocation(QStandardPaths::DownloadLocation), tr("Firmware blobs (*.bin)"));
    if(path.isEmpty())
        return;
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)){
        QMessageBox::warning(parentWidget(), tr("Error"), tr("<center>File could not be read.</center>"));
        return;
    }
    QByteArray blob = file.readAll();
    FwUpgradeDialog dialog(parentWidget(), CkbVersionNumber(), blob, device);
    dialog.exec();
}

void KbWidget::on_layoutBox_activated(int index){
    // Can't use currentIndexChanged as it fires when the GUI is first drawn
    // before the layout has been initialised
    int idxLayout = ui->layoutBox->itemData(index).toInt();
    KeyMap::Layout layout = (KeyMap::Layout)idxLayout;
    // Only set the layout if it was changed
    if(layout == device->getCurrentLayout())
        return;
    QString layoutSettingsPath("Devices/%1/hwLayout");
    CkbSettings::set(layoutSettingsPath.arg(device->usbSerial), KeyMap::getLayout(layout));
    device->layout(layout, true);
}

void KbWidget::switchToProfile(const QString& profile){
    int len = device->profiles().length();
    for(int i = 0; i < len; i++){
        KbProfile* loopProfile = device->profiles().at(i);
        if(loopProfile->name() != profile)
            continue;

        qDebug() << "Switching" << this->name() << "to" << profile;
        device->setCurrentProfile(loopProfile);

        // Also update the dropdown
        ui->profileBox->setCurrentIndex(i);
        return;
    }
}

void KbWidget::switchToMode(const QString& mode){
    KbProfile* currentProfile = device->currentProfile();
    int len = currentProfile->modes().length();

    for(int i = 0; i < len; i++){
        KbMode* loopMode = currentProfile->modes().at(i);
        if(loopMode->name() != mode)
            continue;

        qDebug() << "Switching" << this->name() << "to mode" << mode << "in" << currentProfile->name();
        device->setCurrentMode(loopMode);

        return;
    }
}

void KbWidget::on_pollRateBox_currentIndexChanged(int arg1) {
    if(arg1 == -1)
        return;
    const QString str = ui->pollRateBox->itemText(arg1);
    ui->pollRateBox->setEnabled(false);
    device->setPollRate(str.left(str.indexOf(QLatin1String(" ms"))));
}

// Returns true if a match is found
static inline bool checkForWinInfoMatch(KbWindowInfo* kbinfo, XWindowInfo* wininfo) {
    if(kbinfo->isEmpty() || !kbinfo->isEnabled())
        return false;

    QVector<KbWindowInfo::MatchPair>& rules = kbinfo->items;
    bool result = false;
    for(int i = 0; i < rules.length(); i++){
        const KbWindowInfo::MatchPair& mp = rules.at(i);

        Qt::CaseSensitivity sensitivity = Qt::CaseSensitive;
        if(mp.flags.testFlag(KbWindowInfo::MATCH_CASE_INSENSITIVE))
            sensitivity = Qt::CaseInsensitive;

        const QString* target = nullptr;

        switch(mp.type){
        case KbWindowInfo::MATCH_TYPE_WINDOW_TITLE:
            target = &wininfo->windowTitle;
            break;
        case KbWindowInfo::MATCH_TYPE_PROGRAM_PATH:
            target = &wininfo->program;
            break;
        case KbWindowInfo::MATCH_TYPE_WM_CLASS_NAME:
            target = &wininfo->wm_class_name;
            break;
        case KbWindowInfo::MATCH_TYPE_WM_INSTANCE_NAME:
            target = &wininfo->wm_instance_name;
            break;
        default:
            qDebug() << "Invalid match type" << mp.type;
            return false;
        }

        // Do the comparison
        if(mp.flags.testFlag(KbWindowInfo::MATCH_SUBSTRING))
            result = target->contains(mp.item, sensitivity);
        else if(mp.flags.testFlag(KbWindowInfo::MATCH_STARTS_WITH))
            result = target->startsWith(mp.item, sensitivity);
        else if(mp.flags.testFlag(KbWindowInfo::MATCH_ENDS_WITH))
            result = target->endsWith(mp.item, sensitivity);
        else
            result = !target->compare(mp.item, sensitivity);

        // If it's an OR and we found a match, return immediately
        // If it's an AND and we haven't found a match, also return immediately
        if(mp.op == KbWindowInfo::MATCH_OP_OR && result)
            break;
        else if(mp.op == KbWindowInfo::MATCH_OP_AND && !result)
            break;
    }
    return result;
}

void KbWidget::switchToModeByFocus(XWindowInfo win) {
    if(win.isEmpty())
        return;

    KbProfile* currentProfile = device->currentProfile();
    int len = currentProfile->modes().length();
    for(int i = 0; i < len; i++)
    {
        KbMode* loopMode = currentProfile->modes().at(i);
        if(!checkForWinInfoMatch(loopMode->winInfo(), &win))
            continue;

        if(!prevmode)
            prevmode = currentMode;

        // Set the new mode
        device->setCurrentMode(loopMode);
        return;
    }
    // If we got here, we found no match
    const int mode = currentProfile->indexOf(prevmode);
    if(prevmode && mode != -1)
        device->setCurrentMode(prevmode);

    prevmode = nullptr;
}

void KbWidget::openEventMgr(KbMode* mode) {
    KbModeEventMgr* mgr = new KbModeEventMgr(this, mode);
    // We set this attribute so that we don't have to free it
    mgr->setAttribute(Qt::WA_DeleteOnClose);
    mgr->show();
}

void KbWidget::on_modesList_doubleClicked(const QModelIndex& index) {
    if(index.column() != ModeListTableModel::COL_EVENT_ICON)
        return;
    // If the current state is "enabled", the previous one was "disabled", which means the user
    // most likely just wanted to edit, and not disable it, so re-enable it.
    if(!currentMode->winInfo()->isEnabled())
        currentMode->winInfo()->setEnabled(true);
    openEventMgr(currentMode);
}

void KbWidget::on_modesList_clicked(const QModelIndex& index) {
    if(index.column() != ModeListTableModel::COL_EVENT_ICON || index.row() > device->currentProfile()->modeCount() - 1)
        return;
    currentMode->winInfo()->setEnabled(!currentMode->winInfo()->isEnabled());
    if(currentMode->winInfo()->isEmpty())
        openEventMgr(currentMode);
}

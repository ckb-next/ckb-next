#include <cmath>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QUrl>
#include "ckbsettings.h"
#include "fwupgradedialog.h"
#include "kbfirmware.h"
#include "kbwidget.h"
#include "kblightwidget.h"
#include "kbprofiledialog.h"
#include "ui_kbwidget.h"
#include "ui_kblightwidget.h"

KbWidget::KbWidget(QWidget *parent, Kb *_device) :
    QWidget(parent),
    device(_device), hasShownNewFW(false),
    ui(new Ui::KbWidget),
    currentMode(0)
{
    ui->setupUi(this);
    connect(ui->modesList, SIGNAL(orderChanged()), this, SLOT(modesList_reordered()));

    connect(device, SIGNAL(infoUpdated()), this, SLOT(devUpdate()));
    connect(device, SIGNAL(profileAdded()), this, SLOT(updateProfileList()));
    connect(device, SIGNAL(profileChanged()), this, SLOT(profileChanged()));
    connect(device, SIGNAL(profileRenamed()), this, SLOT(updateProfileList()));
    connect(device, SIGNAL(modeRenamed()), this, SLOT(profileChanged()));
    connect(device, SIGNAL(modeRenamed()), this, SLOT(modeChanged()));
    connect(device, SIGNAL(modeChanged(bool)), this, SLOT(modeChanged(bool)));

    // Remove the Lighting and Performance tabs from non-RGB keyboards
    if(!device->features.contains("rgb")){
        ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->lightTab));
        ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->kPerfTab));
        ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->mPerfTab));
    } else {
        // Remove mouse Performance tab from non-mice
        if(!device->isMouse())
            ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->mPerfTab));
        // Remove keyboard Performance tab from non-keyboards
        if(!device->isKeyboard())
            ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->kPerfTab));
    }
    // Hide poll rate and FW update as appropriate
    if(!device->features.contains("pollrate")){
        ui->pollLabel->hide();
        ui->pollLabel2->hide();
    }
    if(!device->features.contains("fwupdate")){
        ui->fwUpdButton->hide();
        ui->fwUpdLabel->hide();
        ui->fwUpdLayout->removeItem(ui->fwUpdLayout->itemAt(1));
    }
    // Set monochrome mode according to hardware
    if(device->monochrome)
        ui->lightWidget->setMonochrome();
}

KbWidget::~KbWidget(){
    delete ui;
}

void KbWidget::showFirstTab(){
    ui->tabWidget->setCurrentIndex(0);
}

void KbWidget::showLastTab(){
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count() - 1);
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
    ui->profileBox->addItem(QIcon(":/img/icon_blank.png"), "Manage profiles...");
    QFont font = ui->profileBox->font();
    font.setItalic(true);
    ui->profileBox->setItemData(ui->profileBox->count() - 1, font, Qt::FontRole);
}

void KbWidget::profileChanged(){
    // Rebuild mode list
    ui->modesList->clear();
    int i = 0;
    QListWidgetItem* current = 0;
    foreach(KbMode* mode, device->currentProfile()->modes()){
        QListWidgetItem* item = new QListWidgetItem(modeIcon(i), mode->name(), ui->modesList);
        item->setData(GUID, mode->id().guid);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        if(mode == currentMode){
            item->setSelected(true);
            current = item;
        }
        ui->modesList->addItem(item);
        i++;
    }
    if(current)
        ui->modesList->setCurrentItem(current);
    addNewModeItem();
    // Wait for modeChanged() to refresh the rest of the UI
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
    // Device will emit profileChanged() and modeChanged() signals to update UI
}

QIcon KbWidget::modeIcon(int i){
    KbProfile* currentProfile = device->currentProfile(), *hwProfile = device->hwProfile();
    int hwModeCount = device->hwModeCount;
    if(i >= hwModeCount)
        return QIcon(":/img/icon_mode.png");
    else
        return QIcon(QString(currentProfile == hwProfile ? ":/img/icon_mode%1_hardware.png" : ":/img/icon_mode%1.png").arg(i + 1));
}

void KbWidget::addNewModeItem(){
    // Add an item for creating a new mode. Make it editable but not dragable.
    QListWidgetItem* item = new QListWidgetItem("New mode...", ui->modesList);
    item->setFlags((item->flags() | Qt::ItemIsEditable) & ~Qt::ItemIsDragEnabled & ~Qt::ItemIsDropEnabled);
    item->setData(NEW_FLAG, 1);
    QFont font = item->font();
    font.setItalic(true);
    item->setFont(font);
    item->setIcon(QIcon(":/img/icon_plus.png"));
    ui->modesList->addItem(item);
}

void KbWidget::modeChanged(bool spontaneous){
    int index = device->currentProfile()->indexOf(device->currentMode());
    if(index < 0)
        return;
    // Update tabs
    ui->lightWidget->setLight(device->currentLight());
    ui->bindWidget->setBind(device->currentBind(), device->currentProfile());
    ui->kPerfWidget->setPerf(device->currentPerf(), device->currentProfile());
    ui->mPerfWidget->setPerf(device->currentPerf(), device->currentProfile());
    // Update selection
    if(spontaneous)
        ui->modesList->setCurrentRow(index);
    // Connect signals
    if(currentMode)
        disconnect(currentMode, SIGNAL(updated()), this, SLOT(modeUpdate()));
    currentMode = device->currentMode();
    connect(currentMode, SIGNAL(updated()), this, SLOT(modeUpdate()));
    modeUpdate();
}

void KbWidget::on_modesList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous){
    if(!current)
        return;
    KbMode* mode = device->currentProfile()->find(current->data(GUID).toUuid());
    if(!mode)
        return;
    device->setCurrentMode(mode, false);
}

void KbWidget::modesList_reordered(){
    KbProfile* currentProfile = device->currentProfile();
    // Rebuild mode list from items
    QList<KbMode*> newModes;
    int count = ui->modesList->count();
    for(int i = 0; i < count; i++){
        QListWidgetItem* item = ui->modesList->item(i);
        KbMode* mode = currentProfile->find(item->data(GUID).toUuid());
        if(mode && !newModes.contains(mode))
            newModes.append(mode);
        if(item->data(NEW_FLAG).toInt() != 1)
            item->setIcon(modeIcon(i));
        item->setFlags(item->flags() | Qt::ItemIsEditable);
    }
    // Add any missing modes at the end of the list
    foreach(KbMode* mode, currentProfile->modes()){
        if(!newModes.contains(mode))
            newModes.append(mode);
    }
    currentProfile->modes(newModes);
}

void KbWidget::on_modesList_itemChanged(QListWidgetItem *item){
    if(!item || !currentMode || item->data(GUID).toUuid() != currentMode->id().guid)
        return;
    currentMode->name(item->text());
    // Set the text to the actual name (trimmed, "" replaced with "Unnamed")
    item->setText(currentMode->name());
}

void KbWidget::on_modesList_itemClicked(QListWidgetItem* item){
    QUuid guid = item->data(GUID).toUuid();
    if(guid.isNull() && item->data(NEW_FLAG).toInt() == 1){
        // "New mode" item. Clear text and start editing
        item->setText("");
        ui->modesList->editItem(item);
        item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
        QFont font = item->font();
        font.setItalic(false);
        item->setFont(font);
        item->setIcon(QIcon(":/img/icon_mode.png"));
        // Add the new mode and assign it to this item
        KbMode* newMode = device->newMode();
        device->currentProfile()->append(newMode);
        item->setData(GUID, newMode->id().guid);
        item->setData(NEW_FLAG, 0);
        device->setCurrentMode(newMode, false);
        // Create another "new mode" item to replace this one
        addNewModeItem();
    }
}

void KbWidget::on_modesList_customContextMenuRequested(const QPoint &pos){
    QListWidgetItem* item = ui->modesList->itemAt(pos);
    if(!item || !currentMode || item->data(GUID).toUuid() != currentMode->id().guid)
        return;
    KbProfile* currentProfile = device->currentProfile();
    int index = currentProfile->indexOf(currentMode);

    QMenu menu(this);
    QAction* rename = new QAction("Rename...", this);
    QAction* duplicate = new QAction("Duplicate", this);
    QAction* del = new QAction("Delete", this);
    bool canDelete = (device->currentProfile()->modeCount() > device->hwModeCount);
    if(!canDelete)
        // Can't delete modes if they're required by hardware
        del->setEnabled(false);
    QAction* moveup = new QAction("Move Up", this);
    if(index == 0)
        moveup->setEnabled(false);
    QAction* movedown = new QAction("Move Down", this);
    if(index >= currentProfile->modeCount() - 1)
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
        KbMode* newMode = device->newMode(currentMode);
        newMode->newId();
        currentProfile->insert(index + 1, newMode);
        // Update UI
        profileChanged();
        device->setCurrentMode(newMode);
    } else if(result == del){
        if(!canDelete)
            return;
        currentProfile->removeAll(currentMode);
        currentMode->deleteLater();
        currentMode = 0;
        // Select next mode
        profileChanged();
        if(index < currentProfile->modeCount())
            device->setCurrentMode(currentProfile->modes()[index]);
        else
            device->setCurrentMode(currentProfile->modes().last());
    } else if(result == moveup){
        currentProfile->removeAll(currentMode);
        currentProfile->insert(index - 1, currentMode);
        // Update UI
        profileChanged();
        modeChanged(true);
    } else if(result == movedown){
        currentProfile->removeAll(currentMode);
        currentProfile->insert(index + 1, currentMode);
        // Update UI
        profileChanged();
        modeChanged(true);
    }
}

void KbWidget::devUpdate(){
    // Update device tab
    ui->serialLabel->setText(device->usbSerial);
    ui->fwLabel->setText(device->firmware);
    ui->pollLabel->setText(device->pollrate);
}

void KbWidget::modeUpdate(){
}

void KbWidget::on_hwSaveButton_clicked(){
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
    if(!KbFirmware::hasDownloaded())
        ui->fwUpdButton->setText("Check for updates");
    else {
        float newVersion = KbFirmware::versionForBoard(device->features);
        float oldVersion = device->firmware.toFloat();
        if(newVersion <= 0.f || newVersion <= oldVersion)
            ui->fwUpdButton->setText("Up to date");
        else
            ui->fwUpdButton->setText(tr("Upgrade to v%1").arg(QString::number(newVersion, 'f', 2)));
    }
}

void KbWidget::on_fwUpdButton_clicked(){
    // If alt is pressed, ignore upgrades and go straight to the manual prompt
    if(!(qApp->keyboardModifiers() & Qt::AltModifier)){
        // Check version numbers
        if(!KbFirmware::hasDownloaded()){
            ui->fwUpdButton->setText("Checking...");
            ui->fwUpdButton->setEnabled(false);
        }
        float newVersion = KbFirmware::versionForBoard(device->features, true);
        float oldVersion = device->firmware.toFloat();
        ui->fwUpdButton->setEnabled(true);
        updateFwButton();
        if(newVersion == -1.f){
            QMessageBox::information(this, "Firmware update", "<center>There is a new firmware available for this device.<br />However, it requires a newer version of ckb.<br />Please upgrade ckb and try again.</center>");
            return;
        } else if(newVersion == 0.f){
            if(QMessageBox::question(this, "Firmware update", "<center>There was a problem getting the status for this device.<br />Would you like to select a file manually?</center>") != QMessageBox::Yes)
                return;
            // "Yes" -> fall through to browse file
        } else if(newVersion <= oldVersion){
            if(QMessageBox::question(this, "Firmware update", "<center>Your firmware is already up to date.<br />Would you like to select a file manually?</center>") != QMessageBox::Yes)
                return;
            // "Yes" -> fall through to browse file
        } else {
            // Automatic upgrade. Fetch file from web.
            // FwUpgradeDialog can't be parented to KbWidget because KbWidget may be deleted before the dialog exits
            FwUpgradeDialog dialog(parentWidget(), newVersion, "", device);
            dialog.exec();
            return;
        }
    }
    // Browse for file
    QString path = QFileDialog::getOpenFileName(this, "Select firmware file", QStandardPaths::writableLocation(QStandardPaths::DownloadLocation), "Firmware blobs (*.bin)");
    if(path.isEmpty())
        return;
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)){
        QMessageBox::warning(parentWidget(), "Error", "<center>File could not be read.</center>");
        return;
    }
    QByteArray blob = file.readAll();
    FwUpgradeDialog dialog(parentWidget(), 0.f, blob, device);
    dialog.exec();
}

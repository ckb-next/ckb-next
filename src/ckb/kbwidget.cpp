#include <cmath>
#include <QMenu>
#include <QSettings>
#include <QUrl>
#include "kbwidget.h"
#include "kblightwidget.h"
#include "kbprofiledialog.h"
#include "ui_kbwidget.h"
#include "ui_kblightwidget.h"

KbWidget::KbWidget(QWidget *parent, const QString &path, const QString &prefsBase) :
    QWidget(parent),
    device(new Kb(this, path)),
    ui(new Ui::KbWidget), _active(true),
    currentMode(0)
{
    ui->setupUi(this);
    connect(ui->modesList, SIGNAL(orderChanged()), this, SLOT(modesList_reordered()));

    if(!device->isOpen() || device->usbSerial == ""){
        _active = false;
        delete device;
        device = 0;
        return;
    }
    prefsPath = prefsBase + "/" + device->usbSerial;

    connect(device, SIGNAL(infoUpdated()), this, SLOT(devUpdate()));
    connect(device, SIGNAL(profileAdded()), this, SLOT(updateProfileList()));
    connect(device, SIGNAL(profileChanged()), this, SLOT(profileChanged()));
    connect(device, SIGNAL(profileRenamed()), this, SLOT(updateProfileList()));
    connect(device, SIGNAL(modeRenamed()), this, SLOT(profileChanged()));
    connect(device, SIGNAL(modeRenamed()), this, SLOT(modeChanged()));
    connect(device, SIGNAL(modeChanged(bool)), this, SLOT(modeChanged(bool)));

    // Load profiles from stored settings
    QSettings settings;
    settings.beginGroup(prefsPath);
    device->load(settings);
}

KbWidget::~KbWidget(){
    // Save settings
    QSettings settings;
    settings.remove(prefsPath);
    settings.beginGroup(prefsPath);
    device->save(settings);
    settings.endGroup();

    delete ui;
}

void KbWidget::updateProfileList(){
    // Clear profile list and rebuild
    KbProfile* hwProfile = device->hwProfile, *currentProfile = device->currentProfile;
    ui->profileBox->clear();
    int i = 0;
    foreach(KbProfile* profile, device->profiles){
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
    foreach(KbMode* mode, device->currentProfile->modes){
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
    if(index >= device->profiles.count()){
        // "Manage profiles" option
        KbProfileDialog dialog(this);
        dialog.exec();
        updateProfileList();
        return;
    }
    device->setCurrentProfile(device->profiles[index]);
    // Device will emit profileChanged() and modeChanged() signals to update UI
}

QIcon KbWidget::modeIcon(int i){
    KbProfile* currentProfile = device->currentProfile, *hwProfile = device->hwProfile;
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
    int index = device->currentProfile->modes.indexOf(device->currentMode);
    if(index < 0)
        return;
    // Update lighting tab
    ui->lightWidget->setLight(device->currentLight());
    // Update selection
    if(spontaneous)
        ui->modesList->setCurrentRow(index);
    // Connect signals
    if(currentMode)
        disconnect(currentMode, SIGNAL(updated()), this, SLOT(modeUpdate()));
    currentMode = device->currentMode;
    connect(currentMode, SIGNAL(updated()), this, SLOT(modeUpdate()));
    modeUpdate();
}

void KbWidget::on_modesList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous){
    if(!current)
        return;
    KbMode* mode = device->currentProfile->find(current->data(GUID).toUuid());
    if(!mode)
        return;
    device->setCurrentMode(mode, false);
}

void KbWidget::modesList_reordered(){
    KbProfile* currentProfile = device->currentProfile;
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
    foreach(KbMode* mode, currentProfile->modes){
        if(!newModes.contains(mode))
            newModes.append(mode);
    }
    currentProfile->modes = newModes;
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
        device->currentProfile->modes.append(newMode);
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
    KbProfile* currentProfile = device->currentProfile;
    int index = currentProfile->modes.indexOf(currentMode);

    QMenu menu(this);
    QAction* rename = new QAction("Rename...", this);
    QAction* duplicate = new QAction("Duplicate", this);
    QAction* del = new QAction("Delete", this);
    bool canDelete = (device->currentProfile->modes.count() > device->hwModeCount);
    if(!canDelete)
        // Can't delete modes if they're required by hardware
        del->setEnabled(false);
    QAction* moveup = new QAction("Move Up", this);
    if(index == 0)
        moveup->setEnabled(false);
    QAction* movedown = new QAction("Move Down", this);
    if(index >= currentProfile->modes.count() - 1)
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
        currentProfile->modes.insert(index + 1, newMode);
        // Update UI
        profileChanged();
        device->setCurrentMode(newMode);
    } else if(result == del){
        if(!canDelete)
            return;
        currentProfile->modes.removeAll(currentMode);
        currentMode->deleteLater();
        currentMode = 0;
        // Select next mode
        profileChanged();
        if(index < currentProfile->modes.count())
            device->setCurrentMode(currentProfile->modes.at(index));
        else
            device->setCurrentMode(currentProfile->modes.last());
    } else if(result == moveup){
        currentProfile->modes.removeAll(currentMode);
        currentProfile->modes.insert(index - 1, currentMode);
        // Update UI
        profileChanged();
        modeChanged(true);
    } else if(result == movedown){
        currentProfile->modes.removeAll(currentMode);
        currentProfile->modes.insert(index + 1, currentMode);
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
    ui->layoutBox->setCurrentIndex(device->layout());
}

void KbWidget::modeUpdate(){
    KbLight* currentLight = currentMode->light();
    bool inactiveCheck = (currentLight->inactive() >= 0);
    ui->inactiveSwitchCheck->setChecked(inactiveCheck);
    if(inactiveCheck){
        ui->inactiveSwitchBox->setEnabled(true);
        ui->muteCheck->setEnabled(true);
    } else {
        ui->inactiveSwitchBox->setEnabled(false);
        ui->muteCheck->setEnabled(false);
    }
    ui->inactiveSwitchBox->setCurrentIndex(currentLight->inactive() >= 0 ? currentLight->inactive() : KbLight::MAX_INACTIVE);
    ui->muteCheck->setChecked(currentLight->showMute());
}

void KbWidget::on_hwSaveButton_clicked(){
    device->hwSave();
    updateProfileList();
    profileChanged();
}

void KbWidget::on_inactiveSwitchCheck_clicked(bool checked){
    KbLight* currentLight = currentMode->light();
    if(checked){
        currentLight->inactive(ui->inactiveSwitchBox->currentIndex());
        currentLight->showMute(ui->muteCheck->isChecked());
        ui->inactiveSwitchBox->setEnabled(true);
        ui->muteCheck->setEnabled(true);
    } else {
        currentLight->inactive(-1);
        ui->inactiveSwitchBox->setEnabled(false);
        ui->muteCheck->setEnabled(false);
    }
}

void KbWidget::on_inactiveSwitchBox_activated(int index){
    if(ui->inactiveSwitchCheck->isChecked())
        currentMode->light()->inactive(index);
}

void KbWidget::on_muteCheck_clicked(bool checked){
    if(ui->inactiveSwitchCheck->isCheckable())
        currentMode->light()->showMute(checked);
}

void KbWidget::on_layoutBox_activated(int index){
    device->layout((KeyMap::Layout)index);
}

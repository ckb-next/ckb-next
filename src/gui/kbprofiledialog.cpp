#include <QMenu>
#include "kbprofiledialog.h"
#include "ui_kbprofiledialog.h"

KbProfileDialog::KbProfileDialog(KbWidget *parent) :
    QDialog(parent),
    ui(new Ui::KbProfileDialog), device(parent->device)
{
    ui->setupUi(this);
    connect(ui->profileList, SIGNAL(orderChanged()), this, SLOT(profileList_reordered()));

    // Populate profile list
    repopulate();
}

KbProfileDialog::~KbProfileDialog(){
    delete ui;
}

void KbProfileDialog::profileList_reordered(){
    // Rebuild profile list from items
    QList<KbProfile*> newProfiles;
    int count = ui->profileList->count();
    for(int i = 0; i < count; i++){
        QListWidgetItem* item = ui->profileList->item(i);
        KbProfile* profile = device->find(item->data(GUID).toUuid());
        if(profile && !newProfiles.contains(profile))
            newProfiles.append(profile);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
    }
    // Add any missing profiles at the end of the list
    foreach(KbProfile* profile, device->profiles()){
        if(!newProfiles.contains(profile))
            newProfiles.append(profile);
    }
    device->profiles(newProfiles);
}

void KbProfileDialog::on_profileList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous){
    if(!current)
        return;
    KbProfile* profile = device->find(current->data(GUID).toUuid());
    if(!profile)
        return;
    device->setCurrentProfile(profile);
}

void KbProfileDialog::repopulate(){
    ui->profileList->clear();
    QListWidgetItem* current = 0;
    foreach(KbProfile* profile, device->profiles()){
        QListWidgetItem* item = new QListWidgetItem(QIcon((profile == device->hwProfile()) ? ":/img/icon_profile_hardware.png" : ":/img/icon_profile.png"), profile->name(), ui->profileList);
        item->setData(GUID, profile->id().guid);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        if(profile == device->currentProfile()){
            item->setSelected(true);
            current = item;
        }
        ui->profileList->addItem(item);
    }
    if(current)
        ui->profileList->setCurrentItem(current);
    addNewProfileItem();
}

void KbProfileDialog::addNewProfileItem(){
    // Add an item for creating a new profile. Make it editable but not dragable.
    QListWidgetItem* item = new QListWidgetItem("New profile...", ui->profileList);
    item->setFlags((item->flags() | Qt::ItemIsEditable) & ~Qt::ItemIsDragEnabled & ~Qt::ItemIsDropEnabled);
    item->setData(NEW_FLAG, 1);
    QFont font = item->font();
    font.setItalic(true);
    item->setFont(font);
    item->setIcon(QIcon(":/img/icon_plus.png"));
    ui->profileList->addItem(item);
}

void KbProfileDialog::on_profileList_itemClicked(QListWidgetItem *item){
    QUuid guid = item->data(GUID).toUuid();
    if(guid.isNull() && item->data(NEW_FLAG).toInt() == 1){
        // New profile
        item->setText("");
        ui->profileList->editItem(item);
        item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
        QFont font = item->font();
        font.setItalic(false);
        item->setFont(font);
        item->setIcon(QIcon(":/img/icon_profile.png"));
        // Add the new profile and assign it to this item
        KbProfile* newProfile = device->newProfile();
        device->appendProfile(newProfile);
        item->setData(GUID, newProfile->id().guid);
        item->setData(NEW_FLAG, 0);
        device->setCurrentProfile(newProfile);
        // Create another "new profile" item to replace this one
        addNewProfileItem();
    }
}

void KbProfileDialog::on_profileList_itemChanged(QListWidgetItem *item){
    KbProfile* currentProfile = device->currentProfile();
    if(!item || !currentProfile || item->data(GUID).toUuid() != currentProfile->id().guid)
        return;
    currentProfile->name(item->text());
    // Set the text to the actual name (trimmed, "" replaced with "Unnamed")
    item->setText(currentProfile->name());
}

void KbProfileDialog::on_profileList_customContextMenuRequested(const QPoint &pos){
    QListWidgetItem* item = ui->profileList->itemAt(pos);
    KbProfile* currentProfile = device->currentProfile();
    if(!item || !currentProfile || item->data(GUID).toUuid() != currentProfile->id().guid)
        return;
    int index = device->indexOf(currentProfile);
    QList<KbProfile*> profiles = device->profiles();

    QMenu menu(this);
    QAction* rename = new QAction("Rename...", this);
    QAction* duplicate = new QAction("Duplicate", this);
    QAction* del = new QAction("Delete", this);
    bool canDelete = (profiles.count() > 1);
    if(!canDelete)
        // Can't delete the last profile on the device
        del->setEnabled(false);
    QAction* hwsave = new QAction("Save to Hardware", this);
    QAction* moveup = new QAction("Move Up", this);
    if(index == 0)
        moveup->setEnabled(false);
    QAction* movedown = new QAction("Move Down", this);
    if(index >= profiles.count() - 1)
        movedown->setEnabled(false);
    menu.addAction(rename);
    menu.addAction(duplicate);
    menu.addAction(del);
    menu.addSeparator();
    menu.addAction(hwsave);
    menu.addSeparator();
    menu.addAction(moveup);
    menu.addAction(movedown);
    QAction* result = menu.exec(QCursor::pos());
    if(result == rename){
        ui->profileList->editItem(item);
    } else if(result == duplicate){
        KbProfile* newProfile = device->newProfile(currentProfile);
        newProfile->newId();
        profiles.insert(index + 1, newProfile);
        device->profiles(profiles);
        device->setCurrentProfile(newProfile);
    } else if(result == del){
        if(!canDelete)
            return;
        profiles.removeAll(currentProfile);
        device->profiles(profiles);
        currentProfile->deleteLater();
        // Select next profile
        if(index < profiles.count())
            device->setCurrentProfile(profiles.at(index));
        else
            device->setCurrentProfile(profiles.last());
    } else if(result == hwsave){
        device->hwSave();
        // Refresh item icons
        int count = ui->profileList->count();
        for(int i = 0; i < count; i++){
            QListWidgetItem* item = ui->profileList->item(i);
            if(item->data(NEW_FLAG).toUInt() == 1)
                continue;
            KbProfile* profile = device->find(item->data(GUID).toUuid());
            item->setIcon(QIcon((profile == device->hwProfile()) ? ":/img/icon_profile_hardware.png" : ":/img/icon_profile.png"));
        }
    } else if(result == moveup){
        profiles.removeAll(currentProfile);
        profiles.insert(index - 1, currentProfile);
        device->profiles(profiles);
    } else if(result == movedown){
        profiles.removeAll(currentProfile);
        profiles.insert(index + 1, currentProfile);
        device->profiles(profiles);
    }
    repopulate();
}

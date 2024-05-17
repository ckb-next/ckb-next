#include <QMenu>
#include "kbprofiledialog.h"
#include "ui_kbprofiledialog.h"
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <JlCompress.h>
#include <QCryptographicHash>
#include <ckbnextconfig.h>

KbProfileDialog::KbProfileDialog(KbWidget *parent) :
    QDialog(parent),
    ui(new Ui::KbProfileDialog), device(parent->device)
{
    ui->setupUi(this);
    connect(ui->profileList, SIGNAL(orderChanged()), this, SLOT(profileList_reordered()));

    // Populate profile list
    repopulate();
    //activeProfile = device->newProfile(device->currentProfile());
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

void KbProfileDialog::repopulate(){
    ui->profileList->clear();
    QListWidgetItem* current = nullptr;
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
    QListWidgetItem* item = new QListWidgetItem(tr("New profile..."), ui->profileList);
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
        KbProfile* newProfile = device->newProfileWithBlankMode();
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
    QAction* rename = new QAction(tr("Rename"), this);
    QAction* duplicate = new QAction(tr("Duplicate"), this);
    QAction* del = new QAction(tr("Delete"), this);
    bool canDelete = (profiles.count() > 1);
    if(!canDelete)
        // Can't delete the last profile on the device
        del->setEnabled(false);
    QAction* hwsave = new QAction(tr("Save to Hardware"), this);
    // Disable Save to hardware button for unsupported devices
    if(!device->hwload){
        hwsave->setDisabled(true);
        hwsave->setToolTip(tr("Saving to hardware is not supported on this device."));
    }
    QAction* moveup = new QAction(tr("Move Up"), this);
    if(index == 0)
        moveup->setEnabled(false);
    QAction* movedown = new QAction(tr("Move Down"), this);
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
        // We need to return otherwise repopulate() gets called and the item is destroyed
        return;
    } else if(result == duplicate){
        KbProfile* newProfile = device->newProfile(currentProfile);
        newProfile->newId();
        profiles.insert(index + 1, newProfile);
        device->profiles(profiles);
        device->setCurrentProfile(newProfile);
    } else if(result == del){
        if(!canDelete)
            return;
        if(QMessageBox::question(this, tr("Delete profile"), tr("Are you sure you want to delete this profile?")) != QMessageBox::Yes)
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
            QListWidgetItem* hwitem = ui->profileList->item(i);
            if(hwitem->data(NEW_FLAG).toUInt() == 1)
                continue;
            KbProfile* profile = device->find(hwitem->data(GUID).toUuid());
            hwitem->setIcon(QIcon((profile == device->hwProfile()) ? ":/img/icon_profile_hardware.png" : ":/img/icon_profile.png"));
        }
        // No need to repopulate here either
        return;
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

void KbProfileDialog::extractedFileCleanup(const QStringList& extracted){
    for(int i = 0; i < extracted.count(); i++){
        // Delete extracted files
        QFile fdel(extracted.at(i));
        fdel.remove();
    }
}

void KbProfileDialog::on_exportButton_clicked(){
    const quint16 profileVer = CKB_NEXT_PROFILE_VER;

    // Selected items
    QList<QListWidgetItem*> selectedItems = ui->profileList->selectedItems();

    // Set up the file dialog
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("ckb-next profiles (*.ckb)"));
    dialog.setViewMode(QFileDialog::List);
    dialog.setAcceptMode(QFileDialog::AcceptSave);

    // Generate a default filename
    QString filename;
    int itemcount = selectedItems.count();
    for(int i = 0; i < itemcount; i++){
        QString pname(selectedItems.at(i)->text());
        pname.replace(QRegularExpression("[^a-zA-Z0-9_]"), QString());
        if(pname.isEmpty())
            continue;
        filename.append(pname);
        if(i < itemcount - 1)
            filename.append("_");
    }

    if(!filename.isEmpty())
        dialog.selectFile(filename);

    if(!dialog.exec())
        return;

    QStringList filenames = dialog.selectedFiles();

    if(filenames.empty())
        return;

    // Pick only the first filename
    filename = filenames.at(0);
    if(!filename.endsWith(".ckb"))
        filename.append(".ckb");

    QStringList tmpExported;

    {
        // Generate package metadata in /tmp/
        QString metadatapath = "/tmp/ckbpkg.metadata";
        QSettings metadata(metadatapath, QSettings::IniFormat);

        metadata.beginGroup("metadata");
        metadata.setValue("version", profileVer);
        metadata.setValue("isKeyboard", device->isKeyboard());
        metadata.setValue("isMouse", device->isMouse());
        metadata.setValue("isMousepad", device->isMousepad());
        metadata.setValue("isHeadsetStand", device->isHeadsetStand());
        metadata.setValue("model", KeyMap::getModel(device->model()));
        metadata.setValue("layout", device->getCurrentLayout());
        metadata.setValue("monochrome", device->monochrome);
        metadata.endGroup();

        metadata.sync();

        if(metadata.status() == QSettings::NoError)
            tmpExported.append(metadatapath);
    }

    // Create a QSettings ini in /tmp/ for each selected profile
    for(int p = 0; p < selectedItems.count(); p++){
        // Get the profile's pointer
        QListWidgetItem* item = selectedItems.at(p);
        KbProfile* prof = device->find(item->data(GUID).toUuid());

        QString tmp("/tmp/ckbprofile");
        tmp.append(QString::number(p));
        tmp.append(".ini");

        // Used to make sure QSettings is destroyed before trying to hash the ini
        {
            // The version passed here isn't used anywhere
            CkbExternalSettings exportitem(tmp, QSettings::IniFormat, CKB_NEXT_PROFILE_VER);
            exportitem.clear();
            prof->save(exportitem);
            exportitem.sync();

            if(exportitem.status() == QSettings::NoError)
                tmpExported.append(tmp);
        }
    }

    int fileCount = tmpExported.count();

    for(int p = 0; p < fileCount; p++){
        // Create a hash file for each generated file
        QString tmp(tmpExported.at(p));
        QFile tmpHashFile(tmp);
        if(tmpHashFile.open(QFile::ReadOnly)){
            QCryptographicHash tmpHash(QCryptographicHash::Sha256);
            if(tmpHash.addData(&tmpHashFile)){
                tmp.append(".s256");
                QFile tmpHashFileW(tmp);
                if(tmpHashFileW.open(QFile::WriteOnly)){
                    tmpExported.append(tmp);
                    QTextStream hashStream(&tmpHashFileW);
                    hashStream << tmpHash.result().toHex() << "\n";
                }
            }
        }
    }

    bool compress = JlCompress::compressFiles(filename, tmpExported);

    if(!compress)
        QMessageBox::warning(this, tr("Error"), tr("An error occurred while exporting the selected profiles."), QMessageBox::Ok);

    extractedFileCleanup(tmpExported);

    if(compress)
        QMessageBox::information(this, tr("Export Successful"), tr("Selected profiles have been exported successfully."), QMessageBox::Ok);
}

bool KbProfileDialog::verifyHash(const QString& file){
    QFile iniFile(file);
    QFile hashFile(file + ".s256");
    if(iniFile.open(QFile::ReadOnly) && hashFile.open(QFile::ReadOnly)){
        // Max sha256 string length is 64
        QByteArray pkgHash = QByteArray::fromHex(hashFile.read(64).data());

        QCryptographicHash iniHash(QCryptographicHash::Sha256);
        if(iniHash.addData(&iniFile)){
            hashFile.close();
            iniFile.close();
            if(iniHash.result() == pkgHash)
                return true;
        }
    }
    return false;
}

void KbProfileDialog::importCleanup(const QStringList& extracted, const QList<QPair<CkbExternalSettings*, QString>>& profileptrs){
    // Destroy the open CkbExternalSettings objects
    for(int i = 0; i < profileptrs.count(); i++)
        delete profileptrs.at(i).first;

    extractedFileCleanup(extracted);

    repopulate();
}

void KbProfileDialog::on_importButton_clicked(){
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("ckb-next profiles (*.ckb)"));

    if(!dialog.exec())
        return;

    QStringList filenames = dialog.selectedFiles();

    // Pick only the first filename
    const QString& filename = filenames.at(0);
    if(!filename.endsWith(".ckb")){
        QMessageBox::warning(this, tr("Error"), tr("Selected file is not a valid profile."), QMessageBox::Ok);
        return;
    }

    // Extract the ckb package
    QStringList extracted = JlCompress::extractDir(filename, QString("/tmp/"));

    if(extracted.empty()){
        QMessageBox::warning(this, tr("Error"), tr("Could not read %1.").arg(filename), QMessageBox::Ok);
        return;
    }

    // Check and read the metadata
    if(!verifyHash("/tmp/ckbpkg.metadata")){
        QMessageBox::warning(this, tr("Error"), tr("Package contains invalid metadata."), QMessageBox::Ok);
        extractedFileCleanup(extracted);
        return;
    }
    quint16 currentProfileVer = 0;
    {
        QSettings metadata("/tmp/ckbpkg.metadata", QSettings::IniFormat);
        metadata.beginGroup("metadata");

        // Pkg version check
        currentProfileVer = metadata.value("version").toInt();
        if(CKB_NEXT_PROFILE_VER < currentProfileVer){
            QMessageBox::warning(this, tr("Error"), tr("Unsupported package selected. Please update ckb-next to import it."), QMessageBox::Ok);
            extractedFileCleanup(extracted);
            return;
        }

        // Don't allow importing profiles of devices that aren't the same type
        if(device->isKeyboard() != metadata.value("isKeyboard").toBool() ||
           device->isMouse() != metadata.value("isMouse").toBool() ||
           device->isMousepad() != metadata.value("isMousepad").toBool() ||
           device->isHeadsetStand() != metadata.value("isHeadsetStand").toBool()){
            QMessageBox::warning(this, tr("Error"), tr("This profile was not created for the current device type."), QMessageBox::Ok);
            extractedFileCleanup(extracted);
            return;
        }

        QString metadataDevModel = metadata.value("model").toString();
        QString currentDevModel = KeyMap::getModel(device->model());

        int currentDevLayout = (int)device->getCurrentLayout();
        int metadataDevLayout = metadata.value("layout").toInt();

        if(currentDevModel != metadataDevModel ||
            currentDevLayout != metadataDevLayout){

            currentDevModel[0] = currentDevModel[0].toUpper();
            metadataDevModel[0] = metadataDevModel[0].toUpper();

            QString metadataDevLayoutStr = KeyMap::getLayout((KeyMap::Layout)metadataDevLayout).replace("_", " ").toUpper();
            QString currentDevLayoutStr = KeyMap::getLayout((KeyMap::Layout)currentDevLayout).replace("_", " ").toUpper();

            // Only identical devices are supported for now, but let the user override this if they want
            int ret = QMessageBox::question(this, tr("Profile Import"),
                                                  tr("This profile was created for a %1 (%2)\nbut it is going to be imported to a %3 (%4).\n\n"
                                                       "You may need to manually add some keys to the appropriate animations.\n\n"
                                                       "Import Anyway?").arg(metadataDevModel, metadataDevLayoutStr, currentDevModel, currentDevLayoutStr),
                                            QMessageBox::Yes,
                                            QMessageBox::No);

            if(ret == QMessageBox::No){
                extractedFileCleanup(extracted);
                return;
            }
        }

        metadata.endGroup();
    }

    QStringList profilestr;
    // <Pointer>, <GUID>
    QList<QPair<CkbExternalSettings*, QString>> profileptrs;
    int pkgProfileCount = 0;
    int pkgVerifiedCount = 0;

    for(int i = 0; i < extracted.count(); i++){
        const QString& tmpFile = extracted.at(i);
        if(tmpFile.endsWith("ini")){
            pkgProfileCount++;
            if(verifyHash(tmpFile)){
                CkbExternalSettings* sptr = new CkbExternalSettings(extracted.at(i), QSettings::IniFormat, currentProfileVer);
                QString guid(sptr->childGroups().first());

                profileptrs.append(QPair<CkbExternalSettings*, QString>(sptr, guid));
                profilestr.append(sptr->value(guid + "/Name").toString());
                pkgVerifiedCount++;
            }
        }
    }

    if(pkgVerifiedCount == 0){
        QMessageBox::warning(this, tr("Error"), tr("Selected package contains no valid profiles."), QMessageBox::Ok);
        extractedFileCleanup(extracted);
        return;
    }

    // TODO use a proper widget with a profile picker
    // For now, import everything included
    if(pkgVerifiedCount != pkgProfileCount)
        QMessageBox::warning(this, tr("Error"), tr("Selected package contains some corrupt profiles.\n\nAn attempt will be made to import as many as possible."), QMessageBox::Ok);

    QString profilestrf(profilestr.join("\n• "));
    int msgret = QMessageBox::information(this, tr("Profile Import"),
                                   tr("The following profiles will be imported.\n\n• ").append(profilestrf),
                                   QMessageBox::Ok,
                                   QMessageBox::Cancel);
    if(msgret == QMessageBox::Cancel){
        importCleanup(extracted, profileptrs);
        return;
    }

    QList<KbProfile*> profiles = device->profiles();

    for(int i = 0; i < profileptrs.count(); i++){
        int ret = 0;
        CkbExternalSettings* sptr = profileptrs.at(i).first;
        //QString guid = profileptrs.at(i).second;
        //QUuid current = guid.trimmed();
        QUuid guid(sptr->childGroups().first().trimmed());
        // Messy, shhhh
        const QString& profname = profilestr.at(i);
        KbProfile* profilematch = nullptr;
        foreach(KbProfile* profile, device->profiles()){

            if(profile->id().guid == guid){
                ret = QMessageBox::question(this, tr("Profile Import"),
                                            profname + tr(" already exists. Overwrite it?"),
                                            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                            QMessageBox::Cancel);
                profilematch = profile;
                break;
            }
        }

        if(ret == QMessageBox::Cancel)
            continue;

        if(ret == QMessageBox::Yes)
            profiles.removeAll(profilematch);

        QString profileGuid = sptr->childGroups().first();

        if(ret == QMessageBox::No)
            profileGuid = QUuid::createUuid().toString();

        qDebug() << "Importing" << profname;
        KbProfile* newProfile = device->newProfile(sptr, profileGuid);
        profiles.append(newProfile);
    }
    device->profiles(profiles);

    QMessageBox::information(this, tr("Import Successful"), tr("Profiles have been imported successfully."), QMessageBox::Ok);

    // Clean up
    importCleanup(extracted, profileptrs);
}

void KbProfileDialog::on_profileList_itemSelectionChanged(){
    // Only change the profile if a single item is selected
    if(ui->profileList->selectedItems().count() > 1)
        return;

    QList<QListWidgetItem*> itemlist = ui->profileList->selectedItems();
    if(itemlist.empty())
        return;

    KbProfile* profile = device->find(itemlist.first()->data(GUID).toUuid());
    if(!profile)
        return;

    device->setCurrentProfile(profile);
}

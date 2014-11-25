#include <cmath>
#include <fcntl.h>
#include <unistd.h>
#include <QUrl>
#include "kbwidget.h"
#include "kblightwidget.h"
#include "ui_kbwidget.h"
#include "ui_kblightwidget.h"

void KbWidget::frameUpdate(){
    // Read from the notification node
    readInput();

    QFile cmd;
    getCmd(cmd);
    // If the mode changed, close the old one first
    static int prevMode = -1;
    if(prevMode >= 0 && prevMode != currentMode){
        KbLightWidget* light = (KbLightWidget*)ui->lightWidgets->widget(prevMode);
        light->close(cmd, prevMode + 1);
    }

    // Output the current mode/animation
    KbLightWidget* light = (KbLightWidget*)ui->lightWidgets->widget(currentMode);
    if(prevMode != currentMode)
        light->forceLight = true;
    light->frameUpdate(cmd, currentMode + 1);
    cmd.close();
    prevMode = currentMode;
}

void KbWidget::readInput(){
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

                // Set all light setups to the new layout
                int modecount = (model.indexOf("K95") >= 0) ? 3 : 1;
                for(int mode = 0; mode < modecount; mode++){
                    KbLightWidget* light = (KbLightWidget*)ui->lightWidgets->widget(mode);
                    light->rgbWidget->map(getKeyMap());
                }
            } else if(components[0] == "key"){
                // Key event
                if(components[1] == "+light"){
                    KbLightWidget* light = (KbLightWidget*)ui->lightWidgets->widget(currentMode);
                    int index = light->ui->brightnessBox->currentIndex() - 1;
                    if(index < 0)
                        index = light->ui->brightnessBox->count() - 1;
                    light->ui->brightnessBox->setCurrentIndex(index);
                } else if(components[1] == "+m1"){
                    currentMode = 0;
                    ui->modeList->setCurrentRow(0);
                    ui->lightWidgets->setCurrentIndex(0);
                } else if(components[1] == "+m2"){
                    currentMode = 1;
                    ui->modeList->setCurrentRow(1);
                    ui->lightWidgets->setCurrentIndex(1);
                } else if(components[1] == "+m3"){
                    currentMode = 2;
                    ui->modeList->setCurrentRow(2);
                    ui->lightWidgets->setCurrentIndex(2);
                }
            } else if(components[0] == "profilename"){
                // Profile name - update list
                QString name = QUrl::fromPercentEncoding(components[1].toUtf8());
                lastProfileName = name;
                ui->profileText->setText(name);
            } else if(components[0] == "mode"){
                if(components.count() < 3)
                    continue;
                int mode = components[1].toInt() - 1;
                if(components[2] == "switch"){
                    // Mode switch - activate specified mode
                    if(mode < ui->modeList->children().count()){
                        currentMode = mode;
                        ui->modeList->setCurrentRow(mode);
                        ui->lightWidgets->setCurrentIndex(mode);
                    }
                } else if(components[2] == "name"){
                    // Mode name - update list
                    QString name = QUrl::fromPercentEncoding(components[3].toUtf8());
                    lastModeNames[mode] = name;
                    ui->modeList->item(mode)->setText(name);
                } else if(components[2] == "rgb"){
                    // RGB - set mode lighting
                    KbLightWidget* light = (KbLightWidget*)ui->lightWidgets->widget(mode);
                    light->forceLight = true;
                    // If it's an on or off command, just set the brightness
                    if(components[3] == "on"){
                        if(light->ui->brightnessBox->currentIndex() == 3)
                            light->ui->brightnessBox->setCurrentIndex(0);
                        continue;
                    } else if(components[3] == "off"){
                        light->ui->brightnessBox->setCurrentIndex(3);
                        continue;
                    }
                    // If it's a color command, scan the input
                    for(int i = 3; i < components.count(); i++){
                        QString comp = components[i];
                        if(comp.indexOf(":") < 0){
                            // Single hex constant?
                            bool ok;
                            int rgb = comp.toInt(&ok, 16);
                            if(ok)
                                light->rgbWidget->set(QColor::fromRgb((QRgb)rgb));
                        } else {
                            // List of keys. Parse color first
                            QStringList set = comp.split(":");
                            bool ok;
                            int rgb = set[1].toInt(&ok, 16);
                            if(ok){
                                QColor color = QColor::fromRgb((QRgb)rgb);
                                // Parse keys
                                QStringList keys = set[0].split(",");
                                foreach(QString key, keys)
                                    light->rgbWidget->set(key, color);
                            }
                        }
                    }
                }
            }
        }
        notify.close();
    }
}

KbWidget::KbWidget(QWidget *parent, const QString &path) :
    QWidget(parent), devpath(path), cmdpath(path + "/cmd"), disconnect(false), notifyNumber(0), currentMode(0),
    ui(new Ui::KbWidget)
{
    ui->setupUi(this);

    // Get the model, serial number, and firmware version
    QFile mpath(path + "/model"), spath(path + "/serial"), fpath(path + "/fwversion");
    if(mpath.open(QIODevice::ReadOnly)){
        model = mpath.read(100);
        model = model.remove("Corsair").remove("Gaming Keyboard").remove("Keyboard").trimmed();
        mpath.close();
    }
    if(spath.open(QIODevice::ReadOnly)){
        serial = spath.read(100);
        serial = serial.trimmed();
        spath.close();
        ui->serialLabel->setText(serial);
    }
    if(fpath.open(QIODevice::ReadOnly)){
        firmware = fpath.read(100);
        firmware = QString::number(firmware.trimmed().toInt() / 100., 'f', 2);
        fpath.close();
        ui->fwLabel->setText(firmware);
    }

    // Create modes
    int modecount = (model.indexOf("K95") >= 0) ? 3 : 1;
    for(int i = 0; i < modecount; i++)
        addMode();
    ui->modeList->setCurrentRow(0);
    ui->lightWidgets->setCurrentIndex(0);

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
    if(notifyNumber > 0){
        cmd.write(QString("notifyon %1\n").arg(notifyNumber).toLatin1());
        cmd.flush();
    } else
        // If there were none, take notify0
        notifypath = path + "/notify0";
    cmd.write("get :profilename :layout :mode\n");
    for(int i = 0; i < modecount; i++)
        cmd.write(QString("mode %1 get :name :rgbon :rgb\n").arg(i + 1).toLatin1());
    cmd.close();
    usleep(100000);
}

KbWidget::~KbWidget(){
    if(notifyNumber > 0){
        QFile cmd;
        getCmd(cmd);
        // Close all modes
        int modecount = ui->modeList->count() ;
        for(int i = 0; i < modecount; i++)
            ((KbLightWidget*)ui->lightWidgets->widget(i))->close(cmd, i + 1);
        cmd.write(QString("notifyoff %1\n").arg(notifyNumber).toLatin1());
        cmd.close();
    }
    delete ui;
}

void KbWidget::addMode(){
    lastModeNames.append("");
    ui->modeList->addItem("");
    QListWidgetItem* item = ui->modeList->item(ui->modeList->count() - 1);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    ui->lightWidgets->addWidget(new KbLightWidget(this));
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
    if(index == 1)
        cmd.write("layout uk");
    else
        cmd.write("layout us");
    // Ask for all the RGB settings again
    // No need to switch the modes' layouts here, we'll see a notification for that shortly
    int modecount = (model.indexOf("K95") >= 0) ? 3 : 1;
    for(int mode = 0; mode < modecount; mode++)
        cmd.write(QString(" mode %1 get :rgbon :rgb").arg(mode + 1).toLatin1());
    cmd.write("\n");
    cmd.close();
}

void KbWidget::on_modeList_currentRowChanged(int currentRow){
    currentMode = currentRow;
    ui->lightWidgets->setCurrentIndex(currentRow);
}

void KbWidget::on_profileText_editingFinished(){
    // Profile name changed?
    QString newName = ui->profileText->text();
    if(newName != lastProfileName){
        if(newName == ""){
            // Don't allow a blank name
            ui->profileText->setText(lastProfileName);
            return;
        }
        // Write the name to the daemon
        QFile cmd;
        getCmd(cmd);
        cmd.write("profilename ");
        cmd.write(QUrl::toPercentEncoding(newName));
        cmd.write("\n");
        cmd.close();
    }
}

void KbWidget::on_modeList_itemChanged(QListWidgetItem *item){
    // Mode name changed?
    int index = 0;
    int modecount = (model.indexOf("K95") >= 0) ? 3 : 1;
    // Find the mode's index
    for(; index < modecount; index++){
        if(ui->modeList->item(index) == item)
            break;
    }
    QString newName = item->text(), oldName = lastModeNames[index];
    if(newName != oldName){
        // Don't allow a blank name
        if(newName == ""){
            item->setText(oldName);
            return;
        }
        // Write the name to the daemon
        QFile cmd;
        getCmd(cmd);
        cmd.write(QString("mode %1 name ").arg(index + 1).toLatin1());
        cmd.write(QUrl::toPercentEncoding(newName));
        cmd.write("\n");
        cmd.close();
    }
}

void KbWidget::on_pushButton_clicked(){
    QFile cmd;
    getCmd(cmd);
    // Close every mode so that only the base colors are written, not the animations
    int modecount = (model.indexOf("K95") >= 0) ? 3 : 1;
    for(int mode = 0; mode < modecount; mode++){
        KbLightWidget* light = (KbLightWidget*)ui->lightWidgets->widget(mode);
        light->close(cmd, mode + 1);
    }
    cmd.write("hwsave\n");
    cmd.close();
}

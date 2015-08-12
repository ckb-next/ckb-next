#include <QDateTime>
#include <QMenu>
#include "animsettingdialog.h"
#include "kbanimwidget.h"
#include "ui_kbanimwidget.h"

KbAnimWidget::KbAnimWidget(QWidget* parent) :
    QWidget(parent), light(0), current(0), noReorder(false),
    ui(new Ui::KbAnimWidget)
{
    ui->setupUi(this);
    ui->animList->setVisible(false);
    setCurrent(0);
    connect(ui->animList, SIGNAL(orderChanged()), this, SLOT(reorderAnims()));
}

KbAnimWidget::~KbAnimWidget(){
    delete ui;
}

void KbAnimWidget::setLight(KbLight* newLight){
    if(light != newLight){
        if(light)
            disconnect(light, SIGNAL(didLoad()), this, SLOT(refreshList()));
        if(newLight)
            connect(newLight, SIGNAL(didLoad()), this, SLOT(refreshList()));
        light = newLight;
    }
    refreshList();
}

void KbAnimWidget::refreshList(){
    noReorder = true;
    setCurrent(0);
    ui->animList->clear();
    animations.clear();
    // Add the animations from the new lighting mode
    if(!light){
        ui->animList->setVisible(false);
        ui->noAnimLabel->setVisible(true);
        return;
    }
    QList<KbAnim*> newAnimations = light->animList();
    if(newAnimations.count() == 0){
        ui->animList->setVisible(false);
        ui->noAnimLabel->setVisible(true);
        return;
    }
    ui->animList->setVisible(true);
    foreach(KbAnim* anim, newAnimations){
        QListWidgetItem* item = new QListWidgetItem(anim->name(), ui->animList);
        item->setData(Qt::UserRole, anim->guid());
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        animations[anim->guid()] = anim;
        ui->animList->addItem(item);
    }
    ui->noAnimLabel->setVisible(false);
    noReorder = false;
}

void KbAnimWidget::reorderAnims(){
    if(light && !noReorder){
        // Clear and rebuild the list of animations in case the animation moved
        int count = ui->animList->count();
        QList<KbAnim*> animList;
        for(int i = 0; i < count; i++){
            QListWidgetItem* item = ui->animList->item(i);
            KbAnim* anim = animations[item->data(Qt::UserRole).toUuid()];
            if(anim && !animList.contains(anim))
                animList.append(anim);
            item->setFlags(item->flags() | Qt::ItemIsEditable);
        }
        light->animList(animList);
    }
}

void KbAnimWidget::clearSelection(){
    ui->animList->setCurrentItem(0);
    setCurrent(0);
}

void KbAnimWidget::addAnim(const AnimScript* base, const QStringList& keyList, const QString& name, const QMap<QString, QVariant>& preset){
    if(!light)
        return;
    noReorder = true;
    KbAnim* animation = light->addAnim(base, keyList, name, preset);
    QListWidgetItem* item = new QListWidgetItem(animation->name(), ui->animList);
    item->setData(Qt::UserRole, animation->guid());
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    animations[animation->guid()] = animation;
    ui->animList->addItem(item);
    ui->animList->setCurrentItem(item);
    ui->animList->setVisible(true);
    ui->noAnimLabel->setVisible(false);

    setCurrent(animation);
    noReorder = false;

    // Activate settings dialog
    on_propertyButton_clicked();
}

void KbAnimWidget::duplicateAnim(KbAnim* old){
    if(!light)
        return;
    noReorder = true;
    KbAnim* animation = light->duplicateAnim(old);
    // Refresh the list. insertItem doesn't seem to place the item in the correct position on its own...
    refreshList();
    ui->animList->setCurrentRow(light->animList().indexOf(animation));
    ui->animList->setVisible(true);
    ui->noAnimLabel->setVisible(false);

    setCurrent(animation);
    noReorder = false;
}

void KbAnimWidget::setCurrent(KbAnim* newCurrent){
    if(newCurrent != current)
        emit animChanged(current = newCurrent);
    if(!current){
        selectedKeys.clear();
        ui->selectionStack->setCurrentIndex(0);
        return;
    }
    selectedKeys = current->keys();
    const AnimScript* script = current->script();
    if(!script){
        ui->selectionStack->setCurrentIndex(2);
        ui->aMissingLabel->setText("The \"" + current->scriptName() + "\" script could not be loaded. Please check your animation directory.");
        return;
    }
    ui->selectionStack->setCurrentIndex(1);
    ui->aNameLabel->setText(script->name());
    ui->aVerLabel->setText("v" + script->version());
    ui->aCopyLabel->setText(script->copyright());

    ui->nameBox->setText(current->name());
    ui->opacityBox->setValue(current->opacity() * 100.);
    ui->blendBox->setCurrentIndex((int)current->mode());
}

void KbAnimWidget::setSelectedKeys(const QStringList& keys){
    selectedKeys = keys;
    if(keys.count() == 0)
        ui->keyButton->setVisible(false);
    else
        ui->keyButton->setVisible(true);
}

void KbAnimWidget::on_animList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous){
    if(!current)
        setCurrent(0);
    else
        setCurrent(animations[current->data(Qt::UserRole).toUuid()]);
}

void KbAnimWidget::on_animList_itemChanged(QListWidgetItem *item){
    if(item){
        KbAnim* anim = animations[item->data(Qt::UserRole).toUuid()];
        if(anim){
            anim->name(item->text().trimmed());
            if(anim == current && !noReorder)
                ui->nameBox->setText(anim->name());
        }
    }
}

void KbAnimWidget::on_animList_customContextMenuRequested(const QPoint &pos){
    QListWidgetItem* item = ui->animList->itemAt(pos);
    if(!item)
        return;
    setCurrent(animations[item->data(Qt::UserRole).toUuid()]);

    QMenu menu(this);
    QAction* rename = new QAction("Rename...", this);
    QAction* duplicate = new QAction("Duplicate", this);
    QAction* del = new QAction("Delete", this);
    menu.addAction(rename);
    menu.addAction(duplicate);
    menu.addAction(del);
    QAction* result = menu.exec(QCursor::pos());
    if(result == rename)
        ui->animList->editItem(item);
    else if(result == duplicate)
        duplicateAnim(current);
    else if(result == del)
        on_deleteButton_clicked();
}

void KbAnimWidget::on_nameBox_textEdited(const QString &arg1){
    if(current){
        noReorder = true;
        current->name(arg1.trimmed());
        ui->animList->currentItem()->setText(current->name());
        noReorder = false;
    }
}

void KbAnimWidget::on_opacityBox_valueChanged(double arg1){
    if(current)
        current->opacity(arg1 / 100.);
}

void KbAnimWidget::on_blendBox_activated(int index){
    if(current)
        current->mode((KbAnim::Mode)index);
}

void KbAnimWidget::on_keyButton_clicked(){
    if(current){
        QStringList keys = selectedKeys;
        // If any keys were selected previously that aren't in the keymap now, leave them in
        // This is important for layout compatibility - e.g. being able to select both bslash_iso and bslash, because no single layout contains both
        const KeyMap& map = light->map();
        foreach(const QString& key, current->keys()){
            if(!map.contains(key))
                keys << key;
        }
        current->keys(keys);
        emit didUpdateSelection(selectedKeys);
    }
    light->restartAnimation();
}

void KbAnimWidget::on_deleteButton_clicked(){
    if(current){
        animations.remove(current->guid());
        QList<KbAnim*> animList = light->animList();
        animList.removeAll(current);
        light->animList(animList);
        current->deleteLater();
        setCurrent(0);
        delete ui->animList->currentItem();
        if(animations.count() == 0){
            ui->animList->setVisible(false);
            ui->noAnimLabel->setVisible(true);
        }
    }
    light->restartAnimation();
}

void KbAnimWidget::on_propertyButton_clicked(){
    if(!current)
        return;
    // Present animation property popup
    AnimSettingDialog dialog(this, current);
    dialog.exec();
    if(dialog.result() != QDialog::Accepted){
        current->resetParams();
        return;
    }
    // Apply settings and restart all animations
    current->commitParams();
    current->reInit();
    light->restartAnimation();
    // Update name
    ui->nameBox->setText(dialog.name());
    on_nameBox_textEdited(dialog.name());
}

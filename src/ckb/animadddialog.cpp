#include "animadddialog.h"
#include "ui_animadddialog.h"

AnimAddDialog::AnimAddDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AnimAddDialog)
{
    ui->setupUi(this);
    scripts = AnimScript::list();
    foreach(const AnimScript* script, scripts)
        ui->animBox->addItem(script->name());
    on_animBox_activated(0);
}

AnimAddDialog::~AnimAddDialog(){
    delete ui;
}

const AnimScript* AnimAddDialog::chosenScript(){
    return scripts[ui->animBox->currentIndex()];
}

int AnimAddDialog::chosenPreset(){
    return ui->presetBox->currentIndex();
}

void AnimAddDialog::on_animBox_activated(int index){
    // Update preset list
    ui->presetBox->clear();
    foreach(const QString& preset, chosenScript()->presets())
        ui->presetBox->addItem(preset);
    ui->presetBox->setCurrentIndex(0);
}

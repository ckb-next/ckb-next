#include "animadddialog.h"
#include "ui_animadddialog.h"

AnimAddDialog::AnimAddDialog(QWidget *parent, KbLight *light, const QStringList &keys) :
    QDialog(parent),
    ui(new Ui::AnimAddDialog), _light(light), _keys(keys), showPreview(true)
{
    ui->setupUi(this);
    scripts = AnimScript::list();
    foreach(const AnimScript* script, scripts)
        ui->animBox->addItem(script->name());
    on_animBox_activated(0);
}

AnimAddDialog::~AnimAddDialog(){
    delete ui;
    _light->stopPreview();
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
    on_presetBox_activated(0);
}

void AnimAddDialog::on_presetBox_activated(int index){
    // Update preview
    if(showPreview)
        _light->previewAnim(chosenScript(), _keys, chosenScript()->preset(index));
}

void AnimAddDialog::on_previewBox_clicked(bool checked){
    showPreview = checked;
    if(showPreview)
        on_presetBox_activated(ui->presetBox->currentIndex());
    else
        _light->stopPreview();
}

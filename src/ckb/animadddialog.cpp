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
}

AnimAddDialog::~AnimAddDialog(){
    delete ui;
}

const AnimScript* AnimAddDialog::chosenScript(){
    return scripts[ui->animBox->currentIndex()];
}

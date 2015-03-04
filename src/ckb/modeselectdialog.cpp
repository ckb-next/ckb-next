#include "modeselectdialog.h"
#include "ui_modeselectdialog.h"

ModeSelectDialog::ModeSelectDialog(QWidget* parent, KbMode* currentMode, QList<KbMode*> modeList, const QString& textLabel) :
    QDialog(parent), ui(new Ui::ModeSelectDialog),
    _modeList(modeList)
{
    ui->setupUi(this);
    _modeList.removeAll(currentMode);
    ui->label->setText(textLabel);
    // Populate list
    foreach(KbMode* mode, _modeList)
        ui->modeList->addItem(mode->name());
}

ModeSelectDialog::~ModeSelectDialog(){
    delete ui;
}

QList<KbMode*> ModeSelectDialog::selection(){
    QList<KbMode*> res;
    int count = ui->modeList->count();
    for(int i = 0; i < count; i++){
        if(ui->modeList->item(i)->isSelected())
            res.append(_modeList[i]);
    }
    return res;
}

void ModeSelectDialog::on_selAllButton_clicked(){
    int count = ui->modeList->count();
    for(int i = 0; i < count; i++)
        ui->modeList->item(i)->setSelected(true);
}

void ModeSelectDialog::on_selNoneButton_clicked(){
    int count = ui->modeList->count();
    for(int i = 0; i < count; i++)
        ui->modeList->item(i)->setSelected(false);
}

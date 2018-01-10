#include "layoutdialog.h"
#include "kb.h"
#include "ui_layoutdialog.h"

LayoutDialog::LayoutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LayoutDialog) {
    ui->setupUi(this);
    // Populate layout list
    ui->comboBox->addItems(KeyMap::layoutNames());
    ui->comboBox->setCurrentIndex((int)Kb::layout());
}

LayoutDialog::~LayoutDialog(){
    delete ui;
}

KeyMap::Layout LayoutDialog::selected() const {
    return (KeyMap::Layout)ui->comboBox->currentIndex();
}

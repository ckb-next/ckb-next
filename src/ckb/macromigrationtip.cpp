#include "macromigrationtip.h"
#include "ui_macromigrationtip.h"

macroMigrationTip::macroMigrationTip(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::macroMigrationTip)
{
    ui->setupUi(this);
}

macroMigrationTip::~macroMigrationTip()
{
    delete ui;
}

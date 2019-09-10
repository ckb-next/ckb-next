#include "animdetailsdialog.h"
#include "ui_animdetailsdialog.h"
#include <QTextDocumentFragment>

AnimDetailsDialog::AnimDetailsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AnimDetailsDialog)
{
    ui->setupUi(this);
    foreach(const AnimScript* script, AnimScript::list()){
        int row = ui->animTable->rowCount();
        ui->animTable->insertRow(row);
        ui->animTable->setItem(row, 0, new QTableWidgetItem(script->name()));
        ui->animTable->setItem(row, 1, new QTableWidgetItem(script->path()));
        ui->animTable->setItem(row, 2, new QTableWidgetItem(script->version()));
        ui->animTable->setItem(row, 3, new QTableWidgetItem(script->author()));
        ui->animTable->setItem(row, 4, new QTableWidgetItem(script->year()));
        ui->animTable->setItem(row, 5, new QTableWidgetItem(script->license()));
        ui->animTable->setItem(row, 6, new QTableWidgetItem(QTextDocumentFragment::fromHtml(script->description()).toPlainText()));
    }
    ui->animTable->resizeColumnsToContents();
}

AnimDetailsDialog::~AnimDetailsDialog()
{
    delete ui;
}

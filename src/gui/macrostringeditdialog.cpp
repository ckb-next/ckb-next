#include "macrostringeditdialog.h"
#include "ui_macrostringeditdialog.h"
#include <QDebug>
#include <QMessageBox>

MacroStringEditDialog::MacroStringEditDialog(MacroTableModel* mt, QWidget* parent) :
    QDialog(parent),
    ui(new Ui::MacroStringEditDialog), m(mt){
    ui->setupUi(this);
    ui->plainTextEdit->insertPlainText(mt->toString(true));
}

MacroStringEditDialog::~MacroStringEditDialog(){
    delete ui;
}

void MacroStringEditDialog::on_buttonBox_accepted(){
    QString macro = ui->plainTextEdit->toPlainText();
    QString err = m->fromString(macro, true);
    if(err.length()){
        QMessageBox::critical(this, tr("Macro string parse error"), tr("An error occured while parsing at column %1<br><br>Please correct it and try again.").arg(err));
        return;
    }
    accept();
}

#include "daemondialog.h"
#include "ui_daemondialog.h"

#include <QDebug>

DaemonDialog::DaemonDialog(QWidget *parent, QString cmdMsgStart, QString cmdMsgEnable) :
    QDialog(parent),
    ui(new Ui::DaemonDialog)
{
    ui->setupUi(this);

    ui->cmdLabelStart->setText(cmdMsgStart);
    ui->cmdLabelEnable->setText(cmdMsgEnable);
}

DaemonDialog::~DaemonDialog(){
    delete ui;
}

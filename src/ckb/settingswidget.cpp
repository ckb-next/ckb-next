#include "animscript.h"
#include "mainwindow.h"
#include "settingswidget.h"
#include "ui_settingswidget.h"
#include <unistd.h>

int framerate = 30;
extern QString devpath;
extern QTimer* eventTimer;

int fpsTable[] = { 60, 50, 30, 25, 15 };

SettingsWidget::SettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsWidget)
{
    ui->setupUi(this);
    ui->versionLabel->setText("ckb " CKB_VERSION_STR);

    QSettings settings;
    settings.beginGroup("Program");
#ifdef Q_OS_MACX
    KeyPos::osxCmdSwap = settings.value("osxCmdSwap").toBool();
    ui->osxSwapBox->setChecked(KeyPos::osxCmdSwap);
#else
    ui->osxSwapBox->hide();
#endif

    // Read frame rate from settings and send to ckb-daemon
    int rate = settings.value("framerate").toInt();
    if(rate <= 0)
        rate = framerate;
    // Pick the closest rate at or below this one
    for(uint i = 0; i < sizeof(fpsTable)/sizeof(int); i++){
        if(rate >= fpsTable[i]){
            ui->fpsBox->setCurrentIndex(i);
            framerate = fpsTable[i];
            break;
        }
    }
    QFile cmd(devpath.arg(0) + "/cmd");
    if(cmd.open(QIODevice::WriteOnly)){
        cmd.write(QString("fps %1\n").arg(framerate).toLatin1());
        cmd.close();
    }

    ui->animPathLabel->setText(AnimScript::path());
    on_animScanButton_clicked();
}

SettingsWidget::~SettingsWidget(){
    delete ui;
}

void SettingsWidget::on_pushButton_clicked(){
    qApp->quit();
}

void SettingsWidget::on_fpsBox_activated(const QString &arg1){
    if(!eventTimer)
        return;
    // Set FPS
    framerate = arg1.split(" ")[0].toInt();
    eventTimer->setInterval(1000 / framerate);
    QSettings settings;
    settings.beginGroup("Program");
    settings.setValue("framerate", framerate);
    // Send FPS message to ckb-daemon
    QFile cmd(devpath.arg(0) + "/cmd");
    if(cmd.open(QIODevice::WriteOnly)){
        cmd.write(QString("fps %1\n").arg(framerate).toLatin1());
        cmd.close();
    }
}

void SettingsWidget::on_animScanButton_clicked(){
    AnimScript::scan();
    int count = AnimScript::count();
    if(count == 0)
        ui->animCountLabel->setText("No animations found");
    else if(count == 1)
        ui->animCountLabel->setText("1 animation found");
    else
        ui->animCountLabel->setText(QString("%1 animations found").arg(count));
}

void SettingsWidget::on_osxSwapBox_clicked(bool checked){
    KeyPos::osxCmdSwap = checked;
    QSettings settings;
    settings.beginGroup("Program");
    settings.setValue("osxCmdSwap", checked);
}

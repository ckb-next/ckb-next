#include "animscript.h"
#include "mainwindow.h"
#include "settingswidget.h"
#include "ui_settingswidget.h"
#include <unistd.h>

int framerate = 60;
extern QString devpath;
extern QTimer* eventTimer;

int fpsTable[] = { 60, 50, 30, 25, 15 };

SettingsWidget::SettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsWidget)
{
    ui->setupUi(this);

    // Try to get frame rate from ckb-daemon
    QFile cmd(devpath.arg(0) + "/cmd");
    if(cmd.open(QIODevice::WriteOnly)){
        // Find an available notification node
        int notifyNumber = -1;
        for(int i = 1; i < 10; i++){
            QString notify = QString(devpath.arg(0) + "/notify%1").arg(i);
            if(!QFile::exists(notify)){
                notifyNumber = i;
                break;
            }
        }
        if(notifyNumber > 0){
            // Create notification node and ask for FPS
            cmd.write(QString("notifyon %1\n@%1 get :fps\n").arg(notifyNumber).toLatin1());
            cmd.flush();
            // Open the node and read it
            usleep(100000);
            QFile notify(devpath.arg(0) + QString("/notify%1").arg(notifyNumber));
            if(notify.open(QIODevice::ReadOnly)){
                QString line;
                while((line = notify.readLine()) != ""){
                    QStringList words = line.trimmed().split(" ");
                    if(words.length() == 2 && words[0] == "fps"){
                        // Pick the closest rate at or below this one
                        int rate = words[1].toInt();
                        for(uint i = 0; i < sizeof(fpsTable)/sizeof(int); i++){
                            if(rate >= fpsTable[i]){
                                ui->fpsBox->setCurrentIndex(i);
                                framerate = fpsTable[i];
                                // If it didn't matche exactly, write the FPS back to the daemon
                                if(framerate != rate)
                                    cmd.write(QString("fps %1\n").arg(framerate).toLatin1());
                                break;
                            }
                        }
                        break;
                    }
                }
                notify.close();
            }
            // Close the notification node
            cmd.write(QString("notifyoff %1\n").arg(notifyNumber).toLatin1());
        }
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

void SettingsWidget::on_fpsBox_currentIndexChanged(const QString &arg1){
    if(!eventTimer)
        return;
    // Set FPS
    framerate = arg1.split(" ")[0].toInt();
    eventTimer->setInterval(1000 / framerate);
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

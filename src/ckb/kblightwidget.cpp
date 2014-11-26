#include "kblightwidget.h"
#include "ui_kblightwidget.h"
#include <cmath>

extern int framerate;

void KbLightWidget::animSolid(QFile& cmd, float light, QStringList inactive, float inactiveLevel){
    cmd.write(" rgb on");
    const KeyMap& map = rgbWidget->map();
    uint count = map.count();
    for(uint i = 0; i < count; i++){
        QColor bg = map.color(i);
        float r = bg.redF() * 255.f;
        float g = bg.greenF() * 255.f;
        float b = bg.blueF() * 255.f;
        r *= light;
        g *= light;
        b *= light;
        const KeyPos& pos = *map.key(i);
        foreach(QString name, inactive){
            if(!strcmp(pos.name, name.toLatin1().constData())){
                r *= inactiveLevel;
                g *= inactiveLevel;
                b *= inactiveLevel;
                break;
            }
        }
        cmd.write(QString().sprintf(" %s:%02x%02x%02x", pos.name, (int)r, (int)g, (int)b).toLatin1());
    }
}

void KbLightWidget::animWave(QFile& cmd, float light, QStringList inactive, float inactiveLevel){
    const KeyMap& map = rgbWidget->map();
    int fg = fgColor.rgb() & 0xFFFFFF;
    float size = map.width() + 36.f;
    static float wavepos = -36.f;
    if(forceLight)
        wavepos = -36.f;
    cmd.write(" rgb on");
    uint count = map.count();
    for(uint i = 0; i < count; i++){
        const KeyPos& pos = *map.key(i);
        QColor bg = map.color(i);
        float r = bg.redF() * 255.f;
        float g = bg.greenF() * 255.f;
        float b = bg.blueF() * 255.f;
        float distance = fabs(pos.x - wavepos);
        if(distance <= 36.f){
            r = r * distance / 36.f + ((fg >> 16) & 0xFF) * (1.f - distance / 36.f);
            g = g * distance / 36.f + ((fg >> 8) & 0xFF) * (1.f - distance / 36.f);
            b = b * distance / 36.f + (fg & 0xFF) * (1.f - distance / 36.f);
        }
        r *= light;
        g *= light;
        b *= light;
        foreach(QString name, inactive){
            if(!strcmp(pos.name, name.toLatin1().constData())){
                r *= inactiveLevel;
                g *= inactiveLevel;
                b *= inactiveLevel;
                break;
            }
        }
        cmd.write(QString().sprintf(" %s:%02x%02x%02x", pos.name, (int)r, (int)g, (int)b).toLatin1());
    }
    wavepos += (size + 36.f) / 2.f / (float)framerate;
    if(wavepos >= size)
        wavepos = -36.f;
}

void KbLightWidget::animRipple(QFile& cmd, float light, QStringList inactive, float inactiveLevel){
    const KeyMap& map = rgbWidget->map();
    int fg = fgColor.rgb() & 0xFFFFFF;
    float size = sqrt(((double)map.width())*map.width()/2. + ((double)map.height())*map.height()/2.);
    float cx = map.width() / 2.f, cy = map.height() / 2.f;
    static float ringpos = -36.f;
    if(forceLight)
        ringpos = -36.f;
    cmd.write(" rgb on");
    uint count = map.count();
    for(uint i = 0; i < count; i++){
        const KeyPos& pos = *map.key(i);
        QColor bg = map.color(i);
        float r = bg.redF() * 255.f;
        float g = bg.greenF() * 255.f;
        float b = bg.blueF() * 255.f;
        float distance = fabs(sqrt(pow(pos.x - cx, 2.) + pow(pos.y - cy, 2.)) - ringpos);
        if(distance <= 36.f){
            r = r * distance / 36.f + ((fg >> 16) & 0xFF) * (1.f - distance / 36.f);
            g = g * distance / 36.f + ((fg >> 8) & 0xFF) * (1.f - distance / 36.f);
            b = b * distance / 36.f + (fg & 0xFF) * (1.f - distance / 36.f);
        }
        r *= light;
        g *= light;
        b *= light;
        foreach(QString name, inactive){
            if(!strcmp(pos.name, name.toLatin1().constData())){
                r *= inactiveLevel;
                g *= inactiveLevel;
                b *= inactiveLevel;
                break;
            }
        }
        cmd.write(QString().sprintf(" %s:%02x%02x%02x", pos.name, (int)r, (int)g, (int)b).toLatin1());
    }
    ringpos += (size + 36.f) / (float)framerate;
    if(ringpos >= size)
        ringpos = -36.f;
}

void KbLightWidget::frameUpdate(QFile& cmd, int modenumber, bool dimMute){
    cmd.write(QString().sprintf("mode %d switch", modenumber).toLatin1());
    if(forceLight)
        cmd.write(" notify mr m1 m2 m3 light lock");
    if(ui->brightnessBox->currentIndex() == 3){
        if(forceLight)
            cmd.write(" rgb off\n");
        forceLight = false;
        return;
    }
    float light = (3 - ui->brightnessBox->currentIndex()) / 3.f;
    QStringList inactiveList = QStringList();
    float inactiveLight = 1.f;
    if(ui->inactiveCheck->isChecked()){
        inactiveList << "mr" << "m1" << "m2" << "m3";
        if(!winLock)
            inactiveList << "lock";
        if(dimMute)
            inactiveList << "mute";
        inactiveList.removeAll(QString("m%1").arg(modenumber));
        inactiveLight = (2 - ui->inactiveLevelBox->currentIndex()) / 4.f;
    }

    int anim = ui->animBox->currentIndex();
    switch(anim){
    case 0:
        // No animation
        animSolid(cmd, light, inactiveList, inactiveLight);
        break;
    case 1: {
        // Wave
        animWave(cmd, light, inactiveList, inactiveLight);
        break;
    }
    case 2: {
        // Ripple
        animRipple(cmd, light, inactiveList, inactiveLight);
        break;
    }
    }
    cmd.write("\n");
    forceLight = false;
}

void KbLightWidget::close(QFile &cmd, int modenumber){
    forceLight = true;
    if(ui->brightnessBox->currentIndex() == 3){
        cmd.write(QString().sprintf("mode %d rgb off\n", modenumber).toLatin1());
        return;
    }
    // Set just the background color, ignoring any animation
    cmd.write(QString().sprintf("mode %d", modenumber).toLatin1());
    animSolid(cmd, 1.f, QStringList() << "mr" << "m1" << "m2" << "m3" << "lock", 0.f);
    cmd.write("\n");
}

void KbLightWidget::setWinLock(QFile& cmd, int modenumber, bool lock){
    winLock = lock;
    if(lock)
        cmd.write(QString().sprintf("mode %d unbind lwin rwin\n", modenumber).toLatin1());
    else
        cmd.write(QString().sprintf("mode %d rebind lwin rwin\n", modenumber).toLatin1());
}

void KbLightWidget::newSelection(QColor selectedColor, int selectedCount){
    ui->bgButton->color = selectedColor;
    ui->bgButton->updateImage();
    if(selectedCount == 0){
        ui->selLabel->setText("No keys selected");
        ui->bgButton->setVisible(false);
        return;
    } else if(selectedCount == 1)
        ui->selLabel->setText("1 key selected");
    else
        ui->selLabel->setText(QString("%1 keys selected").arg(selectedCount));
    ui->bgButton->setVisible(true);
}

void KbLightWidget::changeBG(QColor newColor){
    rgbWidget->setSelected(newColor);
    forceLight = true;
}

void KbLightWidget::changeFG(QColor newColor){
    fgColor = newColor;
    forceLight = true;
}

void KbLightWidget::on_brightnessBox_currentIndexChanged(int index){
    forceLight = true;
}

void KbLightWidget::on_animBox_currentIndexChanged(int index){
    forceLight = true;
}

KbLightWidget::KbLightWidget(QWidget *parent) :
    QWidget(parent), active(false), forceLight(true), winLock(false),
    ui(new Ui::KbLightWidget)
{
    ui->setupUi(this);

    fgColor = QColor(255, 255, 255);
    ui->fgButton->color = fgColor;
    ui->fgButton->updateImage();
    connect(ui->fgButton, SIGNAL(colorChanged(QColor)), this, SLOT(changeFG(QColor)));
    connect(ui->bgButton, SIGNAL(colorChanged(QColor)), this, SLOT(changeBG(QColor)));

    connect(ui->rgbWidget, SIGNAL(selectionChanged(QColor,int)), this, SLOT(newSelection(QColor,int)));
    ui->bgButton->setVisible(false);

    rgbWidget = ui->rgbWidget;
}

KbLightWidget::~KbLightWidget(){
    delete ui;
}

void KbLightWidget::on_inactiveCheck_stateChanged(int arg1){
    forceLight = true;
}

void KbLightWidget::on_inactiveLevelBox_currentIndexChanged(int index){
    forceLight = true;
}

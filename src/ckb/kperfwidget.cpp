#include "kperfwidget.h"
#include "ui_kperfwidget.h"
#include "modeselectdialog.h"
#include <cmath>

KPerfWidget::KPerfWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::KPerfWidget)
{
    ui->setupUi(this);
    // Set up indicators
    indicators[0].enable = 0; indicators[0].hwEnable = ui->numBox; indicators[0].color1 = ui->numColorOn; indicators[0].color2 = ui->numColorOff; indicators[0].color3 = 0;
    indicators[1].enable = 0; indicators[1].hwEnable = ui->capsBox; indicators[1].color1 = ui->capsColorOn; indicators[1].color2 = ui->capsColorOff; indicators[1].color3 = 0;
    indicators[2].enable = 0; indicators[2].hwEnable = ui->scrollBox; indicators[2].color1 = ui->scrollColorOn; indicators[2].color2 = ui->scrollColorOff; indicators[2].color3 = 0;
    indicators[3].enable = ui->modeBox; indicators[3].hwEnable = 0; indicators[3].color1 = ui->modeColorOn; indicators[3].color2 = ui->modeColorOff; indicators[3].color3 = 0;
    indicators[4].enable = ui->macroBox; indicators[4].hwEnable = 0; indicators[4].color1 = ui->macroColorOn; indicators[4].color2 = ui->macroColorOff; indicators[4].color3 = 0;
    indicators[5].enable = ui->lightBox; indicators[5].hwEnable = 0; indicators[5].color1 = ui->lightColor1; indicators[5].color2 = ui->lightColor2; indicators[5].color3 = ui->lightColor3;
    indicators[6].enable = ui->lockBox; indicators[6].hwEnable = 0; indicators[6].color1 = ui->lockColorOn; indicators[6].color2 = ui->lockColorOff; indicators[6].color3 = 0;
    indicators[7].enable = ui->muteBox; indicators[7].hwEnable = 0; indicators[7].color1 = ui->muteColorOn; indicators[7].color2 = ui->muteColorOff; indicators[7].color3 = ui->muteColorNA;
    for(int i = 0; i < I_COUNT; i++){
        indicators[i].color1->setLabel(false);
        indicators[i].color1->bigIcons(true);
        indicators[i].color1->allowAlpha(true);
        indicators[i].color2->setLabel(false);
        indicators[i].color2->bigIcons(true);
        indicators[i].color2->allowAlpha(true);
        if(indicators[i].color3){
            indicators[i].color3->setLabel(false);
            indicators[i].color3->bigIcons(true);
            indicators[i].color3->allowAlpha(true);
        }
        // Map signals
        if(indicators[i].enable){
            connect(indicators[i].enable, SIGNAL(clicked(bool)), &updateMapper, SLOT(map()));
            updateMapper.setMapping(indicators[i].enable, i);
        }
        if(indicators[i].hwEnable){
            connect(indicators[i].hwEnable, SIGNAL(activated(int)), &updateMapper, SLOT(map()));
            updateMapper.setMapping(indicators[i].hwEnable, i);
        }
        connect(indicators[i].color1, SIGNAL(colorChanged(QColor)), &updateMapper, SLOT(map()));
        updateMapper.setMapping(indicators[i].color1, i);
        connect(indicators[i].color2, SIGNAL(colorChanged(QColor)), &updateMapper, SLOT(map()));
        updateMapper.setMapping(indicators[i].color2, i);
        if(indicators[i].color3){
            connect(indicators[i].color3, SIGNAL(colorChanged(QColor)), &updateMapper, SLOT(map()));
            updateMapper.setMapping(indicators[i].color3, i);
        }
    }
    connect(&updateMapper, SIGNAL(mapped(int)), this, SLOT(uiUpdated(int)));
    k95Widgets << ui->modeBox << ui->modeColorOn << ui->modeColorOff << ui->macroBox << ui->macroColorOn << ui->macroColorOff << ui->k95Label1 << ui->k95Label2 << ui->k95Label3 << ui->k95Label4 << ui->k95Label5 << ui->k95Label6 << ui->k95Line << ui->k95Spacer;
}

KPerfWidget::~KPerfWidget(){
    delete ui;
}

KPerfWidget::HwMode KPerfWidget::raw2Mode(bool sw_enable, i_hw hw_enable){
    if(sw_enable){
        if(hw_enable == KbPerf::NORMAL)
            return BOTH;
        return RGB;
    } else {
        if(hw_enable == KbPerf::ON)
            return ALWAYS_ON;
        else if(hw_enable == KbPerf::OFF)
            return ALWAYS_OFF;
        return NORMAL;
    }
}

void KPerfWidget::mode2Raw(HwMode mode, bool& sw_enable, i_hw& hw_enable){
    switch(mode){
    case NORMAL:
        sw_enable = false;
        hw_enable = KbPerf::NORMAL;
        break;
    case ALWAYS_ON:
        sw_enable = false;
        hw_enable = KbPerf::ON;
        break;
    case ALWAYS_OFF:
        sw_enable = false;
        hw_enable = KbPerf::OFF;
        break;
    case RGB:
        sw_enable = true;
        hw_enable = KbPerf::OFF;
        break;
    case BOTH:
        sw_enable = true;
        hw_enable = KbPerf::NORMAL;
        break;
    }
}

void KPerfWidget::setPerf(KbPerf* newPerf, KbProfile* newProfile){
    perf = newPerf;
    profile = newProfile;
    // Set intensity
    ui->intensityBox->setValue(round(perf->iOpacity() * 100.f));
    // Set hardware indicator values
    for(int i = 0; i < HW_I_COUNT; i++){
        QColor c1, c2, c3;
        bool software;
        i_hw hardware;
        perf->getIndicator((indicator)i, c1, c2, c3, software, hardware);
        HwMode mode = raw2Mode(software, hardware);
        indicators[i].hwEnable->setCurrentIndex((int)mode);
        indicators[i].color1->color(c1);
        indicators[i].color2->color(c2);
        if(mode == RGB || mode == BOTH){
            indicators[i].color1->setEnabled(true);
            indicators[i].color2->setEnabled(true);
        } else {
            indicators[i].color1->setEnabled(false);
            indicators[i].color2->setEnabled(false);
        }
    }
    // Set software indicators
    for(int i = HW_I_COUNT; i < I_COUNT; i++){
        QColor c1, c2, c3;
        bool software;
        i_hw hardware;
        perf->getIndicator((indicator)i, c1, c2, c3, software, hardware);
        indicators[i].enable->setChecked(software);
        indicators[i].color1->color(c1);
        indicators[i].color2->color(c2);
        if(indicators[i].color3) indicators[i].color3->color(c3);
        if(software){
            indicators[i].color1->setEnabled(true);
            indicators[i].color2->setEnabled(true);
            if(indicators[i].color3) indicators[i].color3->setEnabled(true);
        } else {
            indicators[i].color1->setEnabled(false);
            indicators[i].color2->setEnabled(false);
            if(indicators[i].color3) indicators[i].color3->setEnabled(false);
        }
    }
    // Hide K95 indicators on non-K95s
    if(profile->keyMap().model() == KeyMap::K95){
        foreach(QWidget* w, k95Widgets)
            w->show();
    } else {
        foreach(QWidget* w, k95Widgets)
            w->hide();
        if(profile->keyMap().model() == KeyMap::STRAFE) { // hide indicators unsupported by STRAFE
            ui->muteBox->hide();
            ui->muteColorOn->hide();
            ui->muteColorOff->hide();
            ui->muteColorNA->hide();
            ui->label_12->hide();
            ui->label_14->hide();
            ui->label_17->hide();
       }
    }
}

void KPerfWidget::uiUpdated(int index){
    if(!perf)
        return;
    // Read HW/SW enable state
    bool software;
    i_hw hardware;
    if(indicators[index].enable){
        software = indicators[index].enable->isChecked();
        hardware = KbPerf::NONE;
    } else {
        mode2Raw((HwMode)indicators[index].hwEnable->currentIndex(), software, hardware);
    }
    // Enable/disable color buttons based on whether or not the indicator is turned on
    if(software){
        indicators[index].color1->setEnabled(true);
        indicators[index].color2->setEnabled(true);
        if(indicators[index].color3) indicators[index].color3->setEnabled(true);
    } else {
        indicators[index].color1->setEnabled(false);
        indicators[index].color2->setEnabled(false);
        if(indicators[index].color3) indicators[index].color3->setEnabled(false);
    }
    QColor c1 = indicators[index].color1->color();
    QColor c2 = indicators[index].color2->color();
    QColor c3 = indicators[index].color3 ? indicators[index].color3->color() : QColor();
    perf->setIndicator((indicator)index, c1, c2, c3, software, hardware);
}

void KPerfWidget::on_intensityBox_valueChanged(int arg1){
    if(!perf)
        return;
    perf->iOpacity(arg1 / 100.f);
}

void KPerfWidget::on_copyButton_clicked(){
    ModeSelectDialog dialog(this, profile->currentMode(), profile->modes(), "Copy performance settings to:");
    if(dialog.exec() != QDialog::Accepted)
        return;
    QList<KbMode*> selectedModes = dialog.selection();
    foreach(KbMode* mode, selectedModes){
        *mode->perf() = *perf;
    }
}

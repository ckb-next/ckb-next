#include "mperfwidget.h"
#include "ui_mperfwidget.h"
#include <cmath>

const static QString xyLinkPath = "UI/DPI/UnlinkXY";

MPerfWidget::MPerfWidget(QWidget *parent) :
    QWidget(parent), ui(new Ui::MPerfWidget), _xyLink(!CkbSettings::get(xyLinkPath).toBool()), isSetting(false) {
    ui->setupUi(this);
    ui->xyBox->setChecked(!_xyLink);
    // Set up DPI stages
    stages[0].indicator = ui->iButton0; stages[0].xSlider = ui->xSlider0; stages[0].ySlider = ui->ySlider0; stages[0].xBox = ui->xBox0; stages[0].yBox = ui->yBox0; stages[0].enableCheck = 0;
    stages[1].indicator = ui->iButton1; stages[1].xSlider = ui->xSlider1; stages[1].ySlider = ui->ySlider1; stages[1].xBox = ui->xBox1; stages[1].yBox = ui->yBox1; stages[1].enableCheck = ui->eBox1;
    stages[2].indicator = ui->iButton2; stages[2].xSlider = ui->xSlider2; stages[2].ySlider = ui->ySlider2; stages[2].xBox = ui->xBox2; stages[2].yBox = ui->yBox2; stages[2].enableCheck = ui->eBox2;
    stages[3].indicator = ui->iButton3; stages[3].xSlider = ui->xSlider3; stages[3].ySlider = ui->ySlider3; stages[3].xBox = ui->xBox3; stages[3].yBox = ui->yBox3; stages[3].enableCheck = ui->eBox3;
    stages[4].indicator = ui->iButton4; stages[4].xSlider = ui->xSlider4; stages[4].ySlider = ui->ySlider4; stages[4].xBox = ui->xBox4; stages[4].yBox = ui->yBox4; stages[4].enableCheck = ui->eBox4;
    stages[5].indicator = ui->iButton5; stages[5].xSlider = ui->xSlider5; stages[5].ySlider = ui->ySlider5; stages[5].xBox = ui->xBox5; stages[5].yBox = ui->yBox5; stages[5].enableCheck = ui->eBox5;
    ui->iButtonO->setLabel(false);
    ui->iButtonO->bigIcons(true);
    ui->iButtonO->allowAlpha(true);
    for(int i = 0; i < DPI_COUNT; i++){
        stages[i].indicator->setLabel(false);
        stages[i].indicator->bigIcons(true);
        stages[i].indicator->allowAlpha(true);
        // Map signals
        connect(stages[i].indicator, SIGNAL(colorChanged(QColor)), &buttonMapper, SLOT(map()));
        connect(stages[i].xSlider, SIGNAL(valueChanged(int)), &sliderXMapper, SLOT(map()));
        connect(stages[i].ySlider, SIGNAL(valueChanged(int)), &sliderYMapper, SLOT(map()));
        connect(stages[i].xBox, SIGNAL(valueChanged(int)), &boxXMapper, SLOT(map()));
        connect(stages[i].yBox, SIGNAL(valueChanged(int)), &boxYMapper, SLOT(map()));
        if(stages[i].enableCheck)
            // Sniper has no enable
            connect(stages[i].enableCheck, SIGNAL(stateChanged(int)), &enableMapper, SLOT(map()));
        // Set names
        buttonMapper.setMapping(stages[i].indicator, i);
        sliderXMapper.setMapping(stages[i].xSlider, i);
        sliderYMapper.setMapping(stages[i].ySlider, i);
        boxXMapper.setMapping(stages[i].xBox, i);
        boxYMapper.setMapping(stages[i].yBox, i);
        if(stages[i].enableCheck)
            enableMapper.setMapping(stages[i].enableCheck, i);
    }
    // Connect to slots
    connect(&buttonMapper, SIGNAL(mapped(int)), this, SLOT(colorChanged(int)));
    connect(&sliderXMapper, SIGNAL(mapped(int)), this, SLOT(sliderXMoved(int)));
    connect(&sliderYMapper, SIGNAL(mapped(int)), this, SLOT(sliderYMoved(int)));
    connect(&boxXMapper, SIGNAL(mapped(int)), this, SLOT(boxXChanged(int)));
    connect(&boxYMapper, SIGNAL(mapped(int)), this, SLOT(boxYChanged(int)));
    connect(&enableMapper, SIGNAL(mapped(int)), this, SLOT(enableChanged(int)));
}

MPerfWidget::~MPerfWidget(){
    delete ui;
}

void MPerfWidget::setPerf(KbPerf *newPerf){
    perf = newPerf;
    for(int i = 0; i < DPI_COUNT; i++){
        stages[i].indicator->color(perf->dpiColor(i));
        bool oldLink = _xyLink;
        // Don't force X/Y to the same value if they were set differently in the past
        _xyLink = false;
        stages[i].xBox->setValue(perf->dpi(i).x());
        stages[i].yBox->setValue(perf->dpi(i).y());
        _xyLink = oldLink;
        if(stages[i].enableCheck)
            stages[i].enableCheck->setChecked(perf->dpiEnabled(i));
    }
    ui->iButtonO->color(perf->dpiColor(KbPerf::OTHER));
    ui->aSnapBox->setChecked(perf->angleSnap());
    ui->lHeightBox->setCurrentIndex(perf->liftHeight() - 1);
    ui->indicBox->setChecked(perf->enableIndicator());
}

void MPerfWidget::colorChanged(int index){
    perf->dpiColor(index, stages[index].indicator->color());
}

inline int dpiRound(int value){
    if(value > 1000)
        return round(value / 100.) * 100;
    else
        return round(value / 50.) * 50;
}

// Potential recursion in sliderMoved/boxMoved due to rounding
#define SET_START   if(isSetting) return; isSetting = true
#define SET_END     isSetting = false

void MPerfWidget::sliderXMoved(int index){
    SET_START;
    QSlider* slider = stages[index].xSlider;
    int value = dpiRound(slider->value());
    stages[index].xBox->setValue(value);
    SET_END;
    if(_xyLink)
        stages[index].ySlider->setValue(slider->value());
}

void MPerfWidget::sliderYMoved(int index){
    SET_START;
    QSlider* slider = stages[index].ySlider;
    int value = dpiRound(slider->value());
    stages[index].yBox->setValue(value);
    SET_END;
    if(!ui->xyBox->isChecked())
        // X/Y linked?
        stages[index].xSlider->setValue(slider->value());
}

void MPerfWidget::boxXChanged(int index){
    SET_START;
    QSpinBox* box = stages[index].xBox;
    QSlider* slider = stages[index].xSlider;
    int value = box->value();
    slider->setValue(value);
    SET_END;
    if(_xyLink)
        stages[index].yBox->setValue(value);
}

void MPerfWidget::boxYChanged(int index){
    SET_START;
    QSpinBox* box = stages[index].yBox;
    QSlider* slider = stages[index].ySlider;
    int value = box->value();
    slider->setValue(value);
    SET_END;
    if(_xyLink)
        stages[index].xBox->setValue(value);
}

void MPerfWidget::enableChanged(int index){
    perf->dpiEnabled(index, stages[index].enableCheck->isChecked());
}

void MPerfWidget::on_xyBox_clicked(bool checked){
    _xyLink = !checked;
    CkbSettings::set(xyLinkPath, checked);
    // FIXME: update other MPerfWidgets, since this is a global setting
}

void MPerfWidget::on_indicBox_clicked(bool checked){
    perf->enableIndicator(checked);
}

void MPerfWidget::on_aSnapBox_clicked(bool checked){
    perf->angleSnap(checked);
}

void MPerfWidget::on_lHeightBox_activated(int index){
    perf->liftHeight((KbPerf::height)(index + 1));
}

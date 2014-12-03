#include "kblightwidget.h"
#include "ui_kblightwidget.h"

KbLightWidget::KbLightWidget(QWidget *parent) :
    QWidget(parent), light(0),
    ui(new Ui::KbLightWidget)
{
    ui->setupUi(this);

    ui->fgButton->color(QColor(255, 255, 255));
    connect(ui->fgButton, SIGNAL(colorChanged(QColor)), this, SLOT(changeFG(QColor)));
    connect(ui->bgButton, SIGNAL(colorChanged(QColor)), this, SLOT(changeBG(QColor)));

    connect(ui->rgbWidget, SIGNAL(selectionChanged(QColor,int)), this, SLOT(newSelection(QColor,int)));
    ui->bgButton->setVisible(false);

    rgbWidget = ui->rgbWidget;
}

KbLightWidget::~KbLightWidget(){
    delete ui;
}

void KbLightWidget::setLight(KbLight* newLight){
    if(light == newLight)
        return;
    light = newLight;
    rgbWidget->clearSelection();
    ui->inactiveCheck->setChecked(newLight->inactive() >= 0);
    ui->inactiveLevelBox->setCurrentIndex(newLight->inactive() >= 0 ? newLight->inactive() : KbLight::MAX_INACTIVE);
    ui->fgButton->color(newLight->fgColor());
    ui->animBox->setCurrentIndex(newLight->animation());
    ui->brightnessBox->setCurrentIndex(newLight->brightness());
}

void KbLightWidget::newSelection(QColor selectedColor, int selectedCount){
    ui->bgButton->color(selectedColor);
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
    if(light)
        light->map(rgbWidget->map());
}

void KbLightWidget::changeFG(QColor newColor){
    if(light)
        light->fgColor(newColor);
}

void KbLightWidget::on_inactiveCheck_stateChanged(int arg1){
    if(!light)
        return;
    if(arg1)
        light->inactive(ui->inactiveLevelBox->currentIndex());
    else
        light->inactive(-1);
}

void KbLightWidget::on_inactiveLevelBox_currentIndexChanged(int index){
    if(light && ui->inactiveCheck->isChecked())
        light->inactive(index);
}

void KbLightWidget::on_brightnessBox_currentIndexChanged(int index){
    if(light)
        light->brightness(index);
}

void KbLightWidget::on_animBox_currentIndexChanged(int index){
    if(light)
        light->animation(index);
}

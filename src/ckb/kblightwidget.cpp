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

    connect(ui->rgbWidget, SIGNAL(selectionChanged(QColor,QStringList)), this, SLOT(newSelection(QColor,QStringList)));
    ui->bgButton->setVisible(false);
}

KbLightWidget::~KbLightWidget(){
    delete ui;
}

void KbLightWidget::setLight(KbLight* newLight){
    if(light == newLight)
        return;
    light = newLight;
    ui->rgbWidget->clearSelection();
    ui->rgbWidget->map(newLight->map());
    ui->inactiveCheck->setChecked(newLight->inactive() >= 0);
    ui->inactiveLevelBox->setCurrentIndex(newLight->inactive() >= 0 ? newLight->inactive() : KbLight::MAX_INACTIVE);
    ui->fgButton->color(newLight->fgColor());
    ui->animBox->setCurrentIndex(newLight->animation());
    ui->brightnessBox->setCurrentIndex(newLight->brightness());
    if(light->animation() != 0)
        ui->rgbWidget->setAnimation(light->animated());
    else
        ui->rgbWidget->setAnimation(QStringList());
}

void KbLightWidget::newSelection(QColor selectedColor, QStringList selection){
    currentSelection = selection;
    ui->bgButton->color(selectedColor);
    int count = selection.count();
    if(count == 0){
        ui->selLabel->setText("No keys selected");
        ui->bgButton->setVisible(false);
        return;
    } else if(count == 1)
        ui->selLabel->setText("1 key selected");
    else
        ui->selLabel->setText(QString("%1 keys selected").arg(count));
    ui->bgButton->setVisible(true);
}

void KbLightWidget::changeBG(QColor newColor){
    ui->rgbWidget->setSelected(newColor);
    if(light)
        light->map(ui->rgbWidget->map());
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
    if(light){
        light->animation(index);
        if(index == 0)
            ui->rgbWidget->setAnimation(QStringList());
        else
            ui->rgbWidget->setAnimation(light->animated());
    }
}

void KbLightWidget::on_animButton_clicked(){
    light->animated(currentSelection);
    ui->rgbWidget->setAnimationToSelection();
}

#include "animadddialog.h"
#include "kblightwidget.h"
#include "ui_kblightwidget.h"

KbLightWidget::KbLightWidget(QWidget *parent) :
    QWidget(parent), light(0),
    ui(new Ui::KbLightWidget)
{
    ui->setupUi(this);
    ui->bgButton->setVisible(false);
    ui->animButton->setVisible(false);

    connect(ui->bgButton, SIGNAL(colorChanged(QColor)), this, SLOT(changeColor(QColor)));
    connect(ui->rgbWidget, SIGNAL(selectionChanged(QColor,QStringList)), this, SLOT(newSelection(QColor,QStringList)));
    connect(ui->animWidget, SIGNAL(animChanged(KbAnim*)), this, SLOT(changeAnim(KbAnim*)));
    connect(ui->animWidget, SIGNAL(didUpdateSelection(QStringList)), this, SLOT(changeAnimKeys(QStringList)));
}

KbLightWidget::~KbLightWidget(){
    delete ui;
}

void KbLightWidget::setLight(KbLight* newLight){
    ui->rgbWidget->map(newLight->map());
    ui->rgbWidget->colorMap(newLight->colorMap());
    ui->rgbWidget->clearSelection();
    ui->rgbWidget->clearAnimation();
    ui->animWidget->clearSelection();
    if(light == newLight)
        return;
    light = newLight;
    ui->brightnessBox->setCurrentIndex(newLight->brightness());
    ui->rgbWidget->setAnimation(QStringList());
    ui->animWidget->setLight(newLight);
}

void KbLightWidget::newSelection(QColor selectedColor, QStringList selection){
    currentSelection = selection;
    ui->animWidget->setSelectedKeys(selection);
    ui->bgButton->color(selectedColor);
    int count = selection.count();
    if(count == 0){
        ui->selLabel->setText("Click to select keys");
        ui->bgButton->setVisible(false);
        ui->animButton->setVisible(false);
        return;
    } else if(count == 1)
        ui->selLabel->setText("1 key selected");
    else
        ui->selLabel->setText(QString("%1 keys selected").arg(count));
    ui->bgButton->setVisible(true);
    if(AnimScript::count() > 0)
        ui->animButton->setVisible(true);
}

void KbLightWidget::changeColor(QColor newColor){
    if(light){
        foreach(QString key, currentSelection)
            light->color(key, newColor);
        ui->rgbWidget->colorMap(light->colorMap());
    }
}

void KbLightWidget::changeAnim(KbAnim *newAnim){
    if(newAnim)
        ui->rgbWidget->setSelection(newAnim->keys());
    else
        ui->rgbWidget->clearSelection();
    ui->rgbWidget->setAnimationToSelection();
}

void KbLightWidget::changeAnimKeys(QStringList keys){
    ui->rgbWidget->setAnimation(keys);
}

void KbLightWidget::on_brightnessBox_currentIndexChanged(int index){
    if(light)
        light->brightness(index);
}

void KbLightWidget::on_animButton_clicked(){
    if(currentSelection.count() == 0 || AnimScript::count() == 0)
        return;
    AnimAddDialog dialog(this);
    dialog.exec();
    if(dialog.result() != QDialog::Accepted)
        return;
    ui->animWidget->addAnim(dialog.chosenScript(), currentSelection);
}

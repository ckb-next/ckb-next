#include "animadddialog.h"
#include "kblightwidget.h"
#include "ui_kblightwidget.h"

KbLightWidget::KbLightWidget(QWidget *parent) :
    QWidget(parent), light(0),
    ui(new Ui::KbLightWidget)
{
    ui->setupUi(this);
    if(AnimScript::count() == 0)
        ui->animButton->setVisible(false);

    connect(ui->bgButton, SIGNAL(colorChanged(QColor)), this, SLOT(changeColor(QColor)));
    connect(ui->keyWidget, SIGNAL(selectionChanged(QStringList)), this, SLOT(newSelection(QStringList)));
    connect(ui->animWidget, SIGNAL(animChanged(KbAnim*)), this, SLOT(changeAnim(KbAnim*)));
    connect(ui->animWidget, SIGNAL(didUpdateSelection(QStringList)), this, SLOT(changeAnimKeys(QStringList)));
}

KbLightWidget::~KbLightWidget(){
    delete ui;
}

void KbLightWidget::setLight(KbLight* newLight){
    ui->keyWidget->clearSelection();
    ui->keyWidget->clearAnimation();
    ui->animWidget->clearSelection();
    if(light == newLight)
        return;
    if(light)
        disconnect(light, SIGNAL(updated()), this, SLOT(updateLight()));
    connect(newLight, SIGNAL(updated()), this, SLOT(updateLight()));
    light = newLight;
    updateLight();
    ui->keyWidget->setAnimation(QStringList());
    ui->animWidget->setLight(newLight);
}

void KbLightWidget::updateLight(){
    ui->keyWidget->map(light->map());
    ui->keyWidget->colorMap(light->colorMap());
    ui->brightnessBox->setCurrentIndex(light->dimming());
}

void KbLightWidget::newSelection(QStringList selection){
    // Determine selected color (invalid color if no selection or if they're not all the same)
    QColor selectedColor;
    const QHash<QString, QRgb>& colorMap = light->colorMap();
    foreach(const QString& key, selection){
        QColor color = colorMap.value(key);
        if(!selectedColor.isValid())
            selectedColor = color;
        else if(color != selectedColor){
            selectedColor = QColor();
            break;
        }
    }

    currentSelection = selection;
    ui->animWidget->setSelectedKeys(selection);
    ui->bgButton->color(selectedColor);
    int count = selection.count();
    if(count == 0){
        ui->selLabel->setText("Click to select keys");
        return;
    } else if(count == 1)
        ui->selLabel->setText("1 key selected");
    else
        ui->selLabel->setText(QString("%1 keys selected").arg(count));
}

void KbLightWidget::changeColor(QColor newColor){
    if(light){
        foreach(QString key, currentSelection)
            light->color(key, newColor);
        ui->keyWidget->colorMap(light->colorMap());
    }
}

void KbLightWidget::changeAnim(KbAnim *newAnim){
    if(newAnim)
        ui->keyWidget->setSelection(newAnim->keys());
    else
        ui->keyWidget->clearSelection();
    ui->keyWidget->setAnimationToSelection();
}

void KbLightWidget::changeAnimKeys(QStringList keys){
    ui->keyWidget->setAnimation(keys);
}

void KbLightWidget::on_brightnessBox_activated(int index){
    if(light)
        light->dimming(index);
}

void KbLightWidget::on_bgButton_clicked(){
    if(currentSelection.isEmpty())
        ui->keyWidget->selectAll();
}

void KbLightWidget::on_animButton_clicked(){
    if(AnimScript::count() == 0)
        return;
    if(currentSelection.isEmpty())
        ui->keyWidget->selectAll();
    AnimAddDialog dialog(this, light, currentSelection);
    dialog.exec();
    if(dialog.result() != QDialog::Accepted)
        return;
    const AnimScript* script = dialog.chosenScript();
    int presetId = dialog.chosenPreset();
    ui->animWidget->addAnim(script, currentSelection, script->presets()[presetId], script->preset(presetId));
    light->restartAnimation();
}

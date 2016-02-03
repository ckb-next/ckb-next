#include "animadddialog.h"
#include "ckbsettings.h"
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
    connect(ui->keyWidget, SIGNAL(sidelightToggled()), this, SLOT(toggleSidelight())); // click on a toggle button, like sidelight
    connect(ui->animWidget, SIGNAL(animChanged(KbAnim*)), this, SLOT(changeAnim(KbAnim*)));
    connect(ui->animWidget, SIGNAL(didUpdateSelection(QStringList)), this, SLOT(changeAnimKeys(QStringList)));

    // Restore "show animated" setting
    ui->showAnimBox->setChecked(!CkbSettings::get("UI/Light/ShowBaseOnly").toBool());
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
    if(light){
        disconnect(light, SIGNAL(updated()), this, SLOT(updateLight()));
        // Disabled animated colors for previous light
        on_showAnimBox_clicked(false);
    }
    connect(newLight, SIGNAL(updated()), this, SLOT(updateLight()));
    light = newLight;
    updateLight();
    ui->keyWidget->setAnimation(QStringList());
    ui->animWidget->setLight(newLight);
    if(ui->showAnimBox->isChecked())
        on_showAnimBox_clicked(true);
}

void KbLightWidget::setMonochrome(){
    ui->keyWidget->monochrome(true);
}

void KbLightWidget::on_showAnimBox_clicked(bool checked){
    // FIXME: set globally if there's more than one KbLightWidget active
    // Connect/disconnect animation slot
    if(checked){
        if(light)
            connect(light, SIGNAL(frameDisplayed(const ColorMap&,const QSet<QString>&)), ui->keyWidget, SLOT(displayColorMap(const ColorMap&,const QSet<QString>&)));
    } else {
        if(light)
            disconnect(light, SIGNAL(frameDisplayed(const ColorMap&,const QSet<QString>&)), ui->keyWidget, SLOT(displayColorMap(const ColorMap&,const QSet<QString>&)));
        ui->keyWidget->displayColorMap(ColorMap());
    }
    CkbSettings::set("UI/Light/ShowBaseOnly", !checked);
}

void KbLightWidget::updateLight(){
    ui->keyWidget->map(light->map());
    ui->keyWidget->colorMap(light->colorMap());
    ui->brightnessBox->setCurrentIndex(light->dimming());
}

void KbLightWidget::newSelection(QStringList selection){
    // Determine selected color (invalid color if no selection or if they're not all the same)
    QColor selectedColor;
    const QColorMap& colorMap = light->colorMap();
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
        ui->selLabel->setText("Click to select");
        return;
    } else if(count == 1)
        ui->selLabel->setText(tr("1 %1 selected").arg(light->map().isMouse() ? "zone" : "key"));
    else
        ui->selLabel->setText(tr("%1 %2 selected").arg(count).arg(light->map().isMouse() ? "zones" : "keys"));
}

void KbLightWidget::changeColor(QColor newColor){
    if(light){
        foreach(QString key, currentSelection)
            light->color(key, newColor);
        ui->keyWidget->colorMap(light->colorMap());
    }
}

void KbLightWidget::toggleSidelight(){
    if(light){
        if (light->colorMap()["lsidel"] == 0xFF000000){
                light->color("lsidel",QRgb(0xFFFFFFFF));
                light->color("rsidel",QRgb(0xFFFFFFFF));
        } else {
                light->color("lsidel", QRgb(0xFF000000));
                light->color("rsidel", QRgb(0xFF000000));
        }
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

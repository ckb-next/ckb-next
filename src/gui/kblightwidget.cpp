#include "animadddialog.h"
#include "ckbsettings.h"
#include "kblightwidget.h"
#include "ui_kblightwidget.h"
#include "mainwindow.h"
#include "keywidgetlayout.h"

KbLightWidget::KbLightWidget(QWidget *parent) :
    QWidget(parent), light(nullptr),
    ui(new Ui::KbLightWidget), keyWidget(new KeyWidget(this))
{
    ui->setupUi(this);
    if(AnimScript::count() == 0)
        ui->animButton->setEnabled(false);

    connect(ui->bgButton, SIGNAL(colorChanged(QColor)), this, SLOT(changeColor(QColor)));
    connect(keyWidget, SIGNAL(selectionChanged(QStringList)), this, SLOT(newSelection(QStringList)));
    connect(keyWidget, SIGNAL(sidelightToggled()), this, SLOT(toggleSidelight())); // click on a toggle button, like sidelight
    connect(ui->animWidget, SIGNAL(animChanged(KbAnim*)), this, SLOT(changeAnim(KbAnim*)));
    connect(ui->animWidget, SIGNAL(didUpdateSelection(QStringList)), this, SLOT(changeAnimKeys(QStringList)));
    connect(keyWidget, SIGNAL(M95LightToggled()), this, SLOT(toggleM95Light()));
    connect(qApp, SIGNAL(applicationStateChanged(Qt::ApplicationState)), this, SLOT(stateChange(Qt::ApplicationState)));

    // Restore "show animated" setting
    ui->showAnimBox->setChecked(!CkbSettings::get("UI/Light/ShowBaseOnly").toBool());

    // Make a new KeyWidgetLayout that preserves the aspect ratio and add it
    KeyWidgetLayout* l = new KeyWidgetLayout();
    l->addWidget(keyWidget);
    ui->verticalLayout->removeWidget(ui->controlWidget);
    l->addWidget(ui->controlWidget);

    keyWidget->setFocusPolicy(Qt::ClickFocus);
    ui->verticalLayout->insertLayout(2, l);
    ui->verticalLayout->setStretchFactor(l, 2);
    ui->verticalLayout->setStretchFactor(ui->animWidget, 1);
}

KbLightWidget::~KbLightWidget(){
    delete ui;
}

void KbLightWidget::setLight(KbLight* newLight){
    keyWidget->clearSelection();
    keyWidget->clearAnimation();
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
    keyWidget->setAnimation(QStringList());
    ui->animWidget->setLight(newLight);
    if(ui->showAnimBox->isChecked())
        on_showAnimBox_clicked(true);
}

void KbLightWidget::setMonochrome(){
    keyWidget->monochrome(true);
}

void KbLightWidget::on_showAnimBox_clicked(bool checked){
    // FIXME: set globally if there's more than one KbLightWidget active
    // Connect/disconnect animation slot
    if(checked){
        if(light)
            startAnimationPreview();
    } else {
        if(light)
            stopAnimationPreview();
        keyWidget->displayColorMap(ColorMap(), QSet<QString>(), std::numeric_limits<quint64>::max());
    }
    CkbSettings::set("UI/Light/ShowBaseOnly", !checked);
}

void KbLightWidget::updateLight(){
    keyWidget->map(light->map());
    keyWidget->colorMap(light->colorMap());
    ui->brightnessBox->setCurrentIndex(light->dimming());
}

void KbLightWidget::newSelection(const QStringList& selection){
    if(light == nullptr)
        return;
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
        ui->selLabel->setText(tr("Click to select"));
    } else if(count == 1) {
        // This is done this way to aid translation
        if(!light->map().isKeyboard() || light->map().model() == KeyMap::K55 || light->map().model() == KeyMap::K55PRO)
            ui->selLabel->setText(tr("1 zone selected"));
        else
            ui->selLabel->setText(tr("1 key selected"));
    } else {
        if(!light->map().isKeyboard() || light->map().model() == KeyMap::K55 || light->map().model() == KeyMap::K55PRO)
            ui->selLabel->setText(tr("%1 zones selected").arg(count));
        else
            ui->selLabel->setText(tr("%1 keys selected").arg(count));
    }
}

void KbLightWidget::changeColor(const QColor& newColor){
    if(light){
        foreach(QString key, currentSelection)
            light->color(key, newColor);
        keyWidget->colorMap(light->colorMap());
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
        keyWidget->colorMap(light->colorMap());
    }
}

// TODO: Merge with above
void KbLightWidget::toggleM95Light(){
    if(light){
        if (light->colorMap()["back"] == 0xFF000000)
            light->color("back",QRgb(0xFFFFFFFF));
        else
            light->color("back", QRgb(0xFF000000));
        keyWidget->colorMap(light->colorMap());
    }
}
void KbLightWidget::changeAnim(KbAnim *newAnim){
    if(newAnim)
        keyWidget->setSelection(newAnim->keys());
    else
        keyWidget->clearSelection();
    keyWidget->setAnimationToSelection();
}

void KbLightWidget::changeAnimKeys(const QStringList& keys){
    keyWidget->setAnimation(keys);
}

void KbLightWidget::stateChange(Qt::ApplicationState state){
    if(!CkbSettings::get("Program/DisablePreviewOnFocusLoss", true).toBool())
        return;
    if (state == Qt::ApplicationActive) {
        if(light && ui->showAnimBox->isChecked())
            startAnimationPreview();
    } else {
        if(light)
            stopAnimationPreview();
        keyWidget->displayColorMap(ColorMap(), QSet<QString>(), std::numeric_limits<quint64>::max());
    }
}

void KbLightWidget::on_brightnessBox_activated(int index){
    if(light)
        light->dimming(index);
}

void KbLightWidget::on_bgButton_clicked(){
    if(currentSelection.isEmpty())
        keyWidget->selectAll();
}

void KbLightWidget::on_animButton_clicked(){
    if(AnimScript::count() == 0)
        return;
    if(currentSelection.isEmpty())
        keyWidget->selectAll();
    AnimAddDialog dialog(this, light, currentSelection);
    dialog.exec();
    if(dialog.result() != QDialog::Accepted)
        return;
    const AnimScript* script = dialog.chosenScript();
    int presetId = dialog.chosenPreset();
    QString animName = script->presets()[presetId];
    // If the preset is named "Default", then just use the script name
    if(!animName.compare("Default", Qt::CaseInsensitive))
        animName = script->name();
    else if(animName.compare(script->name(), Qt::CaseInsensitive)){
        // Append the animation script name otherwise, and only if preset and script name are not identical
        animName.append(" (");
        animName.append(script->name());
        animName.append(")");
    }
    ui->animWidget->addAnim(script, currentSelection, animName, script->preset(presetId));
    light->restartAnimation();
}

void KbLightWidget::setLegacyM95(){
    ui->animButton->setEnabled(false);
    ui->bgButton->setEnabled(false);
}

void KbLightWidget::startAnimationPreview(){
    connect(light, &KbLight::frameDisplayed, keyWidget, &KeyWidget::displayColorMap);
}

void KbLightWidget::stopAnimationPreview(){
    disconnect(light, &KbLight::frameDisplayed, keyWidget, &KeyWidget::displayColorMap);
}

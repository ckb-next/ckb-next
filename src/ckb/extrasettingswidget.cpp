#include "extrasettingswidget.h"
#include "ui_extrasettingswidget.h"
#include "kb.h"
#include "mainwindow.h"
#include "ckbsettings.h"

// KbLight
static int lastSharedDimming = -2;

ExtraSettingsWidget::ExtraSettingsWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExtraSettingsWidget)
{
    ui->setupUi(this);
    CkbSettings settings("Program");

    // Read frame rate from settings
    int rate = settings.value("framerate").toInt();
    if(rate <= 0 || rate > 60)
        rate = 30;
    ui->fpsBox->setValue(rate);
    Kb::frameRate(rate);
    ui->fpsWarnLabel->setVisible(rate > 30);

    // Read global brightness setting (default = on, 100% brightness)
    int dimming = settings.value("GlobalBrightness").toInt();
    if(dimming < -1 || dimming > KbLight::MAX_DIM)
        dimming = 0;
    lastSharedDimming = dimming;
    KbLight::shareDimming(dimming);
    // Set checkbox value (-1 = don't share)
    ui->brightnessBox->setChecked(dimming == -1);

    // Read dither
    bool dither = settings.value("Dither").toBool();
    Kb::dither(dither);
    ui->ditherBox->setChecked(dither);

#ifdef Q_OS_MACX
    // Read OSX settings
    bool noAccel = settings.value("DisableMouseAccel").toBool();
    Kb::mouseAccel(!noAccel);
    ui->mAccelBox->setChecked(noAccel);
    bool noScrollAccel = settings.value("DisableScrollAccel").toBool();
    int scrollSpeed = settings.value("ScrollSpeed", 3).toInt();
    ui->sAccelBox->setChecked(noScrollAccel);
    ui->sSpeedBox->setValue(scrollSpeed);
    Kb::scrollSpeed(noScrollAccel ? scrollSpeed : 0);
    if(!noScrollAccel)
        ui->sSpeedWidget->hide();
#else
    // Hide all OSX options on Linux
    ui->osxLabel->hide();
    ui->osxLine->hide();
    ui->mAccelBox->hide();
    ui->sAccelBox->hide();
    ui->sSpeedWidget->hide();
#endif

    // Read tray icon setting
    ui->trayBox->setChecked(settings.value("SuppressTrayIcon").toBool());

    // Update animation info
    ui->animPathLabel->setText(AnimScript::path());
    on_animScanButton_clicked();
}

ExtraSettingsWidget::~ExtraSettingsWidget(){
    delete ui;
}

void ExtraSettingsWidget::on_trayBox_clicked(bool checked){
    CkbSettings::set("Program/SuppressTrayIcon", checked);
    MainWindow::mainWindow->toggleTrayIcon(!checked);
}

void ExtraSettingsWidget::on_brightnessBox_clicked(bool checked){
    KbLight::shareDimming(checked ? -1 : 0);
    pollUpdates();
}

void ExtraSettingsWidget::on_animScanButton_clicked(){
    AnimScript::scan();
    int count = AnimScript::count();
    if(count == 0)
        ui->animCountLabel->setText("No animations found");
    else if(count == 1)
        ui->animCountLabel->setText("1 animation found");
    else
        ui->animCountLabel->setText(QString("%1 animations found").arg(count));
}

void ExtraSettingsWidget::on_fpsBox_valueChanged(int arg1){
    // Set FPS
    CkbSettings settings("Program");
    settings.setValue("framerate", arg1);
    Kb::frameRate(arg1);
    // Show warning label if FPS is above 30
    if(arg1 > 30)
        ui->fpsWarnLabel->show();
    else
        ui->fpsWarnLabel->hide();
}

void ExtraSettingsWidget::on_ditherBox_clicked(bool checked){
    CkbSettings::set("Program/Dither", checked);
    Kb::dither(checked);
}

void ExtraSettingsWidget::pollUpdates(){
    // Check for changes to shared brightness setting
    int dimming = KbLight::shareDimming();
    if(dimming != lastSharedDimming && !CkbSettings::isBusy()){
        CkbSettings::set("Program/GlobalBrightness", dimming);
        lastSharedDimming = dimming;
    }
}

void ExtraSettingsWidget::on_mAccelBox_clicked(bool checked){
    CkbSettings::set("Program/DisableMouseAccel", checked);
    Kb::mouseAccel(!checked);
}

void ExtraSettingsWidget::on_sAccelBox_clicked(bool checked){
    CkbSettings::set("Program/DisableScrollAccel", checked);
    Kb::scrollSpeed(checked ? ui->sSpeedBox->value() : 0);
    if(checked)
        ui->sSpeedWidget->show();
    else
        ui->sSpeedWidget->hide();
}

void ExtraSettingsWidget::on_sSpeedBox_valueChanged(int arg1){
    CkbSettings::set("Program/ScrollSpeed", arg1);
    Kb::scrollSpeed(ui->sAccelBox->isChecked() ? arg1 : 0);
}

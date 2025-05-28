#include "extrasettingswidget.h"
#include "ui_extrasettingswidget.h"
#include "kb.h"
#include "mainwindow.h"
#include "ckbsettings.h"
#include "animdetailsdialog.h"
#include "kbmanager.h"
#include "idletimer.h"
// KbLight
static int lastSharedDimming = -2;

ExtraSettingsWidget::ExtraSettingsWidget(QWidget *parent) :
    QWidget(parent),
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

#if defined(Q_OS_MACOS) && defined(OS_MAC_LEGACY)
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
    ui->animPathLabel->setText(AnimScript::paths().join('\n'));
    on_animScanButton_clicked();

    // Read start delay
    ui->startDelayBox->setChecked(settings.value("StartDelay").toBool());

    ui->previewBox->setChecked(settings.value("DisablePreviewOnFocusLoss", true).toBool());

#ifdef Q_OS_LINUX
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    ui->timerMinBox->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
#endif
    if(!IdleTimer::isSupported()){
        QString notSupported(tr("This feature is not supported under your current compositor"));
        ui->timerBox->setToolTip(notSupported);
        ui->timerMinBox->setToolTip(notSupported);
        ui->timerBox->setEnabled(false);
        ui->timerMinBox->setEnabled(false);
    } else {
        ui->timerBox->setChecked(settings.value("IdleTimerEnable", true).toBool());
        ui->timerMinBox->setEnabled(ui->timerBox->isChecked());
    }
    ui->timerMinBox->setValue(settings.value("IdleTimerDuration", 5).toInt());
#else
    ui->scrollWarningLabel->hide();
    ui->timerBox->hide();
    ui->timerMinBox->hide();
#endif
    ui->monochromeBox->setChecked(!settings.value("RGBIcon", true).toBool());
    ui->msaaBox->setChecked(settings.contains("GL/MSAA") && settings.value("GL/MSAA").toInt() == 0);
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
        ui->animCountLabel->setText(tr("No animations found"));
    else if(count == 1)
        ui->animCountLabel->setText(tr("1 animation found"));
    else
        ui->animCountLabel->setText(tr("%1 animations found").arg(count));
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
    // Don't do anything if the global brightness has been dimmed by the timer
    if(dimming != lastSharedDimming && !CkbSettings::isBusy() && !KbManager::kbManager()->getDeviceTimerDimmed()){
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

void ExtraSettingsWidget::on_startDelayBox_clicked(bool checked){
    CkbSettings::set("Program/StartDelay", checked);
}

void ExtraSettingsWidget::on_previewBox_clicked(bool checked){
    CkbSettings::set("Program/DisablePreviewOnFocusLoss", checked);
}

void ExtraSettingsWidget::on_detailsBtn_clicked(){
    AnimDetailsDialog* dlg = new AnimDetailsDialog(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose, true);
    dlg->exec();
}

void ExtraSettingsWidget::on_timerBox_clicked(bool checked){
#ifdef Q_OS_LINUX
    ui->timerMinBox->setEnabled(checked);
    CkbSettings::set("Program/IdleTimerEnable", checked);
    KbManager::setIdleTimer(checked);
#endif
}

void ExtraSettingsWidget::on_timerMinBox_editingFinished(){
#ifdef Q_OS_LINUX
    CkbSettings::set("Program/IdleTimerDuration", ui->timerMinBox->value());
    KbManager::setIdleTimer(false);
    KbManager::setIdleTimer(true);
#endif
}

void ExtraSettingsWidget::on_monochromeBox_toggled(bool checked){
    CkbSettings::set("Program/RGBIcon", !checked);
    MainWindow::mainWindow->syncTrayIcon();
}

void ExtraSettingsWidget::on_msaaBox_toggled(bool checked){
    CkbSettings s("Program/GL");
    if(checked)
        s.setValue("MSAA", 0);
    else
        s.remove("MSAA");
}

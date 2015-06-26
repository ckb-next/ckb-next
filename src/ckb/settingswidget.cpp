#include <unistd.h>
#include "animscript.h"
#include "autorun.h"
#include "ckbsettings.h"
#include "kblight.h"
#include "mainwindow.h"
#include "settingswidget.h"
#include "ui_settingswidget.h"

extern QString devpath;
extern QTimer* eventTimer;

// Modifier keys (OS-dependent)
static QStringList modKeys, modNames;

// KbLight
static int lastSharedDimming = -2;

SettingsWidget::SettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsWidget)
{
    ui->setupUi(this);
    CkbSettings settings("Program");

    // Load modifier remap
    KbBind::loadGlobalRemap();
    if(modKeys.isEmpty()){
        modKeys << "caps" << "lshift" << "lctrl" << "lalt" << "lwin";
#ifdef Q_OS_MACX
        modNames << "Caps Lock" << "Shift" << "Control (⌃)" << "Option (⌥)" << "Command (⌘)";
#else
        modNames << "Caps Lock" << "Shift" << "Control" << "Alt" << "Super";
#endif
    }
    foreach(const QString& name, modNames){
        ui->capsBox->addItem(name);
        ui->shiftBox->addItem(name);
        ui->ctrlBox->addItem(name);
        ui->altBox->addItem(name);
        ui->winBox->addItem(name);
    }

    int idx = modKeys.indexOf(KbBind::globalRemap("caps"));
    if(idx >= 0)
        ui->capsBox->setCurrentIndex(idx);
    idx = modKeys.indexOf(KbBind::globalRemap("lshift"));
    if(idx >= 0)
        ui->shiftBox->setCurrentIndex(idx);
    idx = modKeys.indexOf(KbBind::globalRemap("lctrl"));
    if(idx >= 0)
        ui->ctrlBox->setCurrentIndex(idx);
    idx = modKeys.indexOf(KbBind::globalRemap("lwin"));
    if(idx >= 0)
        ui->winBox->setCurrentIndex(idx);
    idx = modKeys.indexOf(KbBind::globalRemap("lalt"));
    if(idx >= 0)
        ui->altBox->setCurrentIndex(idx);
#ifdef Q_OS_MACX
    if(settings.value("osxCmdSwap").toBool()){
        // ckb <0.0.41
        settings.remove("osxCmdSwap");
        ui->ctrlBox->setCurrentIndex(modKeys.indexOf("lwin"));
        ui->winBox->setCurrentIndex(modKeys.indexOf("lctrl"));
        updateModifiers();
    }
#endif

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

    // Read auto update settings
    ui->autoFWBox->setChecked(!settings.value("DisableAutoFWCheck").toBool());

    // Read tray icon setting
    ui->trayBox->setChecked(!settings.value("SuppressTrayIcon").toBool());

    // Read auto run settings
    if(!AutoRun::available())
        ui->loginItemBox->hide();
    else {
        if(!AutoRun::once())
            // If this is the first time running the app, enable auto run by default
            AutoRun::enable();
        ui->loginItemBox->setChecked(AutoRun::isEnabled());
    }

    ui->animPathLabel->setText(AnimScript::path());
    on_animScanButton_clicked();
}

int SettingsWidget::frameRate() const {
    return ui->fpsBox->value();
}

void SettingsWidget::pollUpdates(){
    // Check for changes to shared brightness setting
    int dimming = KbLight::shareDimming();
    if(dimming != lastSharedDimming && !CkbSettings::isBusy()){
        CkbSettings::set("Program/GlobalBrightness", dimming);
        lastSharedDimming = dimming;
    }
}

SettingsWidget::~SettingsWidget(){
    delete ui;
}

void SettingsWidget::setStatus(const QString& text){
    ui->devicesLabel->setText(text);
}

void SettingsWidget::setVersion(const QString& version){
    ui->versionLabel->setText(version);
}

static inline QString right(const QString& left){
    if(left[0] != 'l')
        return left;
    return "r" + left.mid(1);
}

void SettingsWidget::updateModifiers(){
    QHash<QString, QString> newMods;
    newMods["caps"] = modKeys[ui->capsBox->currentIndex()];
    newMods["lshift"] = modKeys[ui->shiftBox->currentIndex()];
    newMods["rshift"] = right(modKeys[ui->shiftBox->currentIndex()]);
    newMods["lctrl"] = modKeys[ui->ctrlBox->currentIndex()];
    newMods["rctrl"] = right(modKeys[ui->ctrlBox->currentIndex()]);
    newMods["lalt"] = modKeys[ui->altBox->currentIndex()];
    newMods["ralt"] = right(modKeys[ui->altBox->currentIndex()]);
    newMods["lwin"] = modKeys[ui->winBox->currentIndex()];
    newMods["rwin"] = right(modKeys[ui->winBox->currentIndex()]);
    KbBind::setGlobalRemap(newMods);
    KbBind::saveGlobalRemap();
}

void SettingsWidget::on_pushButton_clicked(){
    qApp->quit();
}

void SettingsWidget::on_fpsBox_valueChanged(int framerate){
    if(!eventTimer)
        return;
    // Set FPS
    eventTimer->setInterval(1000 / framerate);
    CkbSettings settings("Program");
    settings.setValue("framerate", framerate);
    Kb::frameRate(framerate);
    // Show warning label if FPS is above 30
    if(framerate > 30)
        ui->fpsWarnLabel->show();
    else
        ui->fpsWarnLabel->hide();
}

void SettingsWidget::on_animScanButton_clicked(){
    AnimScript::scan();
    int count = AnimScript::count();
    if(count == 0)
        ui->animCountLabel->setText("No animations found");
    else if(count == 1)
        ui->animCountLabel->setText("1 animation found");
    else
        ui->animCountLabel->setText(QString("%1 animations found").arg(count));
}

void SettingsWidget::on_capsBox_activated(int index){
    updateModifiers();
}

void SettingsWidget::on_shiftBox_activated(int index){
    updateModifiers();
}

void SettingsWidget::on_ctrlBox_activated(int index){
    updateModifiers();
}

void SettingsWidget::on_altBox_activated(int index){
    updateModifiers();
}

void SettingsWidget::on_winBox_activated(int index){
    updateModifiers();
}

void SettingsWidget::on_brightnessBox_clicked(bool checked){
    KbLight::shareDimming(checked ? -1 : 0);
    pollUpdates();
}

void SettingsWidget::on_autoFWBox_clicked(bool checked){
    CkbSettings::set("Program/DisableAutoFWCheck", !checked);
}

void SettingsWidget::on_trayBox_clicked(bool checked){
    CkbSettings::set("Program/SuppressTrayIcon", !checked);
    MainWindow::mainWindow->toggleTrayIcon(checked);
}

void SettingsWidget::on_loginItemBox_clicked(bool checked){
    if(checked)
        AutoRun::enable();
    else
        AutoRun::disable();
}

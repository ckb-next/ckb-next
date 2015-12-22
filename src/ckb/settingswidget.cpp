#include <QTimer>
#include "animscript.h"
#include "autorun.h"
#include "ckbsettings.h"
#include "kb.h"
#include "layoutdialog.h"
#include "settingswidget.h"
#include "ui_settingswidget.h"

extern QString devpath;

// Modifier keys (OS-dependent)
static QStringList modKeys, modNames;

SettingsWidget::SettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsWidget)
{
    ui->setupUi(this);
    CkbSettings settings("Program");

    // Read keyboard layout
    ui->layoutBox->addItems(KeyMap::layoutNames());
    KeyMap::Layout layout = KeyMap::getLayout(settings.value("KbdLayout").toString());
    if(layout == KeyMap::NO_LAYOUT){
        // If the layout hasn't been set yet, show a dialog to let the user choose it
        layout = KeyMap::locale();
        QTimer::singleShot(1000, this, SLOT(showLayoutDialog()));   // Run the function after a delay as the dialog may not appear correctly otherwise
        settings.setValue("KbdLayout", KeyMap::getLayout(layout));
    }
    Kb::layout(layout);
    ui->layoutBox->setCurrentIndex((int)layout);

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

    // Read auto update settings
    ui->autoFWBox->setChecked(!settings.value("DisableAutoFWCheck").toBool());

    // Read auto run settings
    if(!AutoRun::available())
        ui->loginItemBox->hide();
    else {
        if(!AutoRun::once())
            // If this is the first time running the app, enable auto run by default
            AutoRun::enable();
        ui->loginItemBox->setChecked(AutoRun::isEnabled());
    }

    // Prepare extra settings
    extra = new ExtraSettingsWidget(this);
}

SettingsWidget::~SettingsWidget(){
    delete ui;
}

void SettingsWidget::showLayoutDialog(){
    LayoutDialog dialog(this);
    dialog.exec();
    // Set selected layout
    ui->layoutBox->setCurrentIndex((int)dialog.selected());
    on_layoutBox_activated((int)dialog.selected());         // Call activated() signal manually to trigger save
}

void SettingsWidget::pollUpdates(){
    extra->pollUpdates();
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

void SettingsWidget::on_autoFWBox_clicked(bool checked){
    CkbSettings::set("Program/DisableAutoFWCheck", !checked);
}

void SettingsWidget::on_loginItemBox_clicked(bool checked){
    if(checked)
        AutoRun::enable();
    else
        AutoRun::disable();
}

void SettingsWidget::on_layoutBox_activated(int index){
    KeyMap::Layout layout = (KeyMap::Layout)index;
    CkbSettings::set("Program/KbdLayout", KeyMap::getLayout(layout));
    Kb::layout(layout);
}

void SettingsWidget::on_extraButton_clicked(){
    extra->exec();
}

#include <QTimer>
#include "animscript.h"
#include "autorun.h"
#include "ckbsettings.h"
#include "kb.h"
#include "settingswidget.h"
#include "ui_settingswidget.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>
#include "mainwindow.h"
#include <ckbnextconfig.h>
#include <QProgressBar>

extern QString devpath;

// Modifier keys (OS-dependent)
static QStringList modKeys, modNames;
static bool updateRequestedByUser = false;

SettingsWidget::SettingsWidget(QWidget *parent) :
    QWidget(parent), devDetect(nullptr), devDetectProgress(nullptr),
    ui(new Ui::SettingsWidget)
{
    ui->setupUi(this);

    // Prepare extra settings
    extra = ui->extraWidget;

#ifndef Q_OS_MACOS
    ui->uninstallButton->hide();
#endif

    CkbSettings settings("Program");

    // Load modifier remap
    KbBind::loadGlobalRemap();
    if(modKeys.isEmpty()){
        modKeys << "caps" << "lshift" << "lctrl" << "lalt" << "lwin";
#ifdef Q_OS_MACOS
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
#ifdef Q_OS_MACOS
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

    QString copyrightText = QString("© 2014-2016 <a href=\"https://github.com/ccMSC/\" style=\"text-decoration:none;\">ccMSC</a>.<br/>© 2017-%1 <a href=\"https://github.com/ckb-next/ckb-next/graphs/contributors\" style=\"text-decoration:none;\">%2</a>.").arg(CKB_NEXT_COPYRIGHT_YEAR, tr("The ckb-next development team"));

#ifndef OS_MAC_LEGACY
#ifdef Q_OS_MACOS
    copyrightText.append(tr("<br/>Special thanks to <a href=\"https://github.com/tekezo\" style=\"text-decoration:none;\">tekezo</a> for <a href=\"https://github.com/tekezo/Karabiner-VirtualHIDDevice\" style=\"text-decoration:none;\">VirtualHIDDevice</a>."));
#endif
#endif

    ui->label_2->setText(copyrightText);

#ifdef DISABLE_UPDATER
    ui->autoUpdBox->hide();
    ui->pushButton_2->hide();
#else
    ui->autoUpdBox->setChecked(!settings.value("DisableAutoUpdCheck", false).toBool());
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    ui->hiDPIBox->setChecked(settings.value("HiDPI", false).toBool());
#else
    ui->hiDPIBox->hide();
#endif
}

SettingsWidget::~SettingsWidget(){
    delete ui;
}

void SettingsWidget::pollUpdates(){
    extra->pollUpdates();
}

void SettingsWidget::setStatus(const QString& text){
    ui->devicesLabel->setText(text);
}

void SettingsWidget::setVersion(const CkbVersionNumber& version){
    ui->versionLabel->setText(QString("ckb-next ") + version.toString());
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

void SettingsWidget::on_aboutQt_clicked(){
    QMessageBox::aboutQt(this);
}

void SettingsWidget::on_generateReportButton_clicked(){
    // Don't allow the script to run twice
    if(devDetect || devDetectProgress)
        return;

    QMessageBox::information(this, tr("Generate report"), tr("This will collect software logs, as well as information about the Corsair devices in your system.\n\n"
                                                             "Make sure they are plugged in and click OK."));
    devDetect = new QProcess();
    connect(devDetect, SIGNAL(finished(int)), this, SLOT(devDetectFinished(int)));
    connect(devDetect, &QProcess::destroyed, this, &SettingsWidget::devDetectDestroyed);
#if defined(Q_OS_LINUX)
    QString devDetectPath("ckb-next-dev-detect");
#elif defined(Q_OS_MACOS)
    QString devDetectPath("/Applications/ckb-next.app/Contents/Resources/ckb-next-dev-detect");
#endif
    devDetect->start(devDetectPath, QStringList() << "--nouserinput");

    // Check if it was started successfully
    if(!devDetect->waitForStarted()){
        QMessageBox::critical(this, tr("Error executing ckb-next-dev-detect"), tr("An error occurred while trying to execute ckb-next-dev-detect.\n"
                                                                                  "File not found or not executable."));
        devDetect->deleteLater();
        return;
    }
    devDetectProgress = new QProgressDialog(tr("Generating Report"), "", 0, 0, this);
    devDetectProgress->setCancelButton(nullptr);
    devDetectProgress->open();
}

void SettingsWidget::devDetectFinished(int retVal){
    devDetectProgress->close();
    delete devDetectProgress;
    devDetectProgress = nullptr;
    QFile report("/tmp/ckb-next-dev-detect-report.gz");
    if(retVal || !report.exists()){
        QString errMsg(tr("An error occurred while trying to execute ckb-next-dev-detect.\n\n"));
        errMsg.append(devDetect->readAllStandardError());
        errMsg.append("\n");
        errMsg.append(QString(tr("Return code %1")).arg(retVal));
        QMessageBox::critical(this, tr("Error executing ckb-next-dev-detect"), errMsg);
        devDetect->deleteLater();
        return;
    }

    QString newdir = QFileDialog::getExistingDirectory(this, tr("Select output directory"));
    QString newfile(newdir + "/ckb-next-report-" + QString::number(QDateTime::currentMSecsSinceEpoch()/1000) + ".gz");
    if(report.copy(newfile)){
        QMessageBox::information(this, tr("Report generated successfully"), tr("The report has been generated successfully."));
        // Try to show the newly generated file
#if defined(Q_OS_LINUX)
        if(QProcess::execute("sh", QStringList() << "-c" << "gtk-launch `xdg-mime query default inode/directory` '" + newfile + "'"))
            QProcess::execute("xdg-open", QStringList() << newdir);
#elif defined(Q_OS_MACOS)
        QProcess::execute("open", QStringList() << "-R" << newfile);
#endif
    }
    else
        QMessageBox::critical(this, tr("Error writing report"), tr("Could not write report to the selected directory.\n"
                                                                   "Please pick a different one and try again."));
    report.remove();
    devDetect->deleteLater();
}

void SettingsWidget::devDetectDestroyed(){
    devDetect = nullptr;
}

void SettingsWidget::on_autoUpdBox_clicked(bool checked)
{
    CkbSettings::set("Program/DisableAutoUpdCheck", !checked);
}

void SettingsWidget::on_pushButton_2_clicked()
{
    emit checkForUpdates();
    ui->pushButton_2->setEnabled(false);
    ui->pushButton_2->setText(tr("Checking..."));
    updateRequestedByUser = true;
}

void SettingsWidget::enableUpdateButton(){
    ui->pushButton_2->setEnabled(true);
}

void SettingsWidget::setUpdateButtonText(const QString& text){
    if(updateRequestedByUser) {
        ui->pushButton_2->setText(text);
        updateRequestedByUser = false;
    }
}

void SettingsWidget::on_uninstallButton_clicked()
{
#ifdef Q_OS_MACOS
    if(QMessageBox::warning(this, tr("Uninstall ckb-next"), tr("WARNING: Clicking OK will uninstall ckb-next and any older versions of the software from your system.\n\n"
                                                             "Your settings and lighting profiles will be preserved."), QMessageBox::Cancel, QMessageBox::Ok) != QMessageBox::Ok)
        return;

    // Open a terminal and run the uninstall script
    QProcess::execute("open", QStringList() << "-a" << "Terminal" << "/Applications/ckb-next.app/Contents/Resources/uninstall.sh");
#endif
}

void SettingsWidget::on_hiDPIBox_clicked(bool checked)
{
    CkbSettings::set("Program/HiDPI", checked);
    QMessageBox::warning(this, tr("Please restart ckb-next"), tr("Please click the Quit button and restart ckb-next for the changes to take effect."), QMessageBox::Ok);
}

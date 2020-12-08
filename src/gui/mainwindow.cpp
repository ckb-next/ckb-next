
#include "ckbsettings.h"
#include "kbmanager.h"
#include "kbfirmware.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <cstdlib>
#include <QFileInfo>
#include <QSharedMemory>
#include <QShortcut>
#include <QMessageBox>
#include <QMenuBar>
#include <unistd.h>
#include <ckbnextconfig.h>
#include <sys/socket.h>
#include <signal.h>
#include <QProcess>
#include "xcb/xwindowdetector.h"
XWindowDetector* windowDetector = nullptr;

#ifndef DISABLE_UPDATER
#include "ckbupdater.h"
#include "ckbupdaterwidget.h"
#endif

#include "ckbsystemtrayicon.h"


extern QSharedMemory appShare;
extern QString devpath;

int MainWindow::signalHandlerFd[2] = {0, 0};

MainWindow* MainWindow::mainWindow = 0;

#if defined(Q_OS_MACOS) && !defined(OS_MAC_LEGACY)
bool is_catalina_or_higher(){
    // Get macOS version. If Catalina or higher, start the daemon agent as the current user to request for HID permission.
    QString macOSver = QSysInfo::productVersion();
    // Split major/minor
    QVector<QStringRef> verVector = macOSver.splitRef('.');
    // Check if Catalina or greater
    return (verVector.count() == 2 && verVector.at(0) == QString("10") && verVector.at(1).toInt() >= 15);
}

void MainWindow::appleRequestHidTimer(){
    // Destroy the timer immediately if we have devices connected
    if(KbManager::devices().count()){
        catalinaTimer->stop();
        catalinaTimer->deleteLater();
        catalinaTimer = nullptr;
        return;
    }

    QProcess launchctl;
    launchctl.setProgram("launchctl");
    // Start the agent only if the state on the previous run wasn't "running" or pre exec
    if(!catalinaAgentStarted){
        // Start the service
        launchctl.setArguments(QStringList() << "start" << "org.ckb-next.daemon");
        launchctl.start();
        launchctl.waitForFinished();
        qDebug() << "Launchctl start returned" << launchctl.exitCode();
        catalinaAgentStarted = true;
    }

    // Get EUID
    uid_t euid = geteuid();

    // This will most likely block the UI thread, but hopefully it won't be too bad
    QThread::msleep(1500);

    // Call launchctl
    launchctl.setArguments(QStringList() << "print" << QString("gui/%1/org.ckb-next.daemon").arg(euid));
    launchctl.start();
    launchctl.waitForFinished();
    if(launchctl.exitCode() == 0){
        QString str(launchctl.readAllStandardOutput());

        int statestart = str.indexOf("state = ") + 8;
        int statelen = str.indexOf('\n', statestart) - statestart;
        if(statelen > 20)
            return;

        // Extract the state from the output
        QStringRef agentState(&str, statestart, statelen);
        if(agentState == QString("running") || agentState == QString("spawned (pre-exec)")){
            catalinaAgentStarted = true;
            return;
        }
        catalinaAgentStarted = false;

        // Extract the daemon's return code
        int statusstart = str.lastIndexOf("last exit code = ") + 17;
        int statuslen = str.indexOf('\n', statusstart) - statusstart;
        if(statuslen < 3 && statuslen > 0){
            hid_req_ret status = (hid_req_ret)QStringRef(&str, statusstart, statuslen).toInt();
            // We do not need to do anything if the request succeeds, other than wait for the loop to run again
            if(status == REQUEST_ALREADY_ALLOWED){
                qDebug() << "We have HID access!";
                catalinaTimer->stop();
                catalinaTimer->deleteLater();
                catalinaTimer = nullptr;
                // Ask user to restart daemon only if we first had to request permission
                if(prevHidRet == REQUEST_SUCCEEDED)
                    QProcess::execute("open", QStringList() << "-a" << "Terminal" << "/Applications/ckb-next.app/Contents/Resources/daemon-restart.sh");
            }
            else
                qDebug() << "HID agent encountered an unknown error";
            prevHidRet = status;
        } else if (statuslen == 14) {
            qDebug() << "Agent was never started. Something went wrong.";
        }
    }
}
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mainWindow = this;

    kbfw = new KbFirmware();

    // Start device manager
    KbManager::init(CKB_NEXT_VERSION_STR);
    connect(KbManager::kbManager(), SIGNAL(kbConnected(Kb*)), this, SLOT(addDevice(Kb*)));
    connect(KbManager::kbManager(), SIGNAL(kbDisconnected(Kb*)), this, SLOT(removeDevice(Kb*)));
    connect(KbManager::kbManager(), SIGNAL(versionUpdated()), this, SLOT(updateVersion()));
    connect(KbManager::scanTimer(), SIGNAL(timeout()), this, SLOT(timerTick()));

    // Set up tray icon
    restoreAction = new QAction(tr("Restore"), this);
    closeAction = new QAction(tr("Quit"), this);
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addAction(closeAction);
    trayIcon = new CkbSystemTrayIcon(getIcon(), getIconName(), this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->show();
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconClicked(QSystemTrayIcon::ActivationReason)));
    connect(trayIcon, &CkbSystemTrayIcon::scrollRequested, this, &MainWindow::handleTrayScrollEvt);
    toggleTrayIcon(!CkbSettings::get("Program/SuppressTrayIcon").toBool());

#ifdef Q_OS_MACOS
    // Make a custom "Close" menu action for OSX, as the default one brings up the "still running" popup unnecessarily
    QMenuBar* menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
    this->menuBar()->addMenu("ckb-next")->addAction(closeAction);
#else
    // On linux, add a handler for Ctrl+Q
    new QShortcut(QKeySequence("Ctrl+Q"), this, SLOT(quitApp()));
#endif

    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(quitApp()));
    connect(closeAction, SIGNAL(triggered()), this, SLOT(quitApp()));
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(showWindow()));
    connect(qApp, SIGNAL(applicationStateChanged(Qt::ApplicationState)), this, SLOT(stateChange(Qt::ApplicationState)));

    ui->tabWidget->addTab(settingsWidget = new SettingsWidget(this), QString(tr("Settings")));
    settingsWidget->setVersion("ckb-next " CKB_NEXT_VERSION_STR);

    // create daemon dialog as a QMessageBox
    // this will create a focussed dialog, that has to be interacted with,
    // if the daemon is not running
    // set the main and informative text to tell the user about the issue
    QMessageBox dialog;
    dialog.setText(tr("The ckb-next daemon is not running. This program will <b>not</b> work without it!"));
#ifndef Q_OS_MACOS
    QString daemonDialogText = QString(tr("Start it once with:")) +
    "<blockquote><code>sudo systemctl start ckb-next-daemon</code></blockquote>" +
    tr("Enable it for every boot:") +
    "<blockquote><code>sudo systemctl enable ckb-next-daemon</code></blockquote>";
#else
    QString daemonDialogText = QString(tr("Start and enable it with:")) +
    "<blockquote><code>sudo launchctl load -w /Library/LaunchDaemons/org.ckb-next.daemon.plist</code></blockquote>";
#endif
    dialog.setInformativeText(daemonDialogText);
    dialog.setIcon(QMessageBox::Critical);

    // Set up signal handler
    socketpair(AF_UNIX, SOCK_STREAM, 0, MainWindow::signalHandlerFd);

    sigNotifier = new QSocketNotifier(MainWindow::signalHandlerFd[1], QSocketNotifier::Read, this);
    connect(sigNotifier, &QSocketNotifier::activated, this, &MainWindow::QSignalHandler);

    signal(SIGINT, MainWindow::PosixSignalHandler);
    signal(SIGTERM, MainWindow::PosixSignalHandler);
    signal(SIGHUP, MainWindow::PosixSignalHandler);

    // check, whether daemon is running
    // the daemon creates the root device path on initialization and thus it
    // can be assumed, that the daemon is not running if doesn't exist
    // `.arg(0)` is necessary to interpolate the correct suffix into the path
    // see `./kbmanager.cpp` for details
    QFileInfo rootDevPath(devpath.arg(0));
    if (!rootDevPath.exists()) {
        // set settings widget's status
        // show the main window (otherwise only the dialog will be visible)
        // finally show the dialog
        settingsWidget->setStatus(tr("The ckb-next daemon is not running."));
        showWindow();
        dialog.exec();
    }
#ifndef DISABLE_UPDATER
    if(!CkbSettings::get("Program/DisableAutoUpdCheck", false).toBool())
        checkForCkbUpdates();

    connect(settingsWidget, &SettingsWidget::checkForUpdates, this, &MainWindow::checkForCkbUpdates);
#endif

#if defined(Q_OS_MACOS) && !defined(OS_MAC_LEGACY)
    if(is_catalina_or_higher()){
        // Load the agent as it'll be unloaded on first installation
        {
            QProcess launchctl;
            launchctl.setProgram("launchctl");
            launchctl.setArguments(QStringList() << "load" << "/Library/LaunchAgents/org.ckb-next.daemon_agent.plist");
            launchctl.start();
            launchctl.waitForFinished();
            qDebug() << "Launchctl load returned" << launchctl.exitCode();
        }

        // Start it
        catalinaTimer = new QTimer(this);
        connect(catalinaTimer, &QTimer::timeout, this, &MainWindow::appleRequestHidTimer);
        catalinaTimer->setInterval(11000);
        catalinaTimer->start();
        QTimer::singleShot(2000, this, &MainWindow::appleRequestHidTimer);
    }
#endif
#ifdef USE_XCB_EWMH
        qRegisterMetaType<XWindowInfo>("XWindowInfo");
        windowDetector = new XWindowDetector();
        windowDetector->start();
#endif
}

void MainWindow::handleTrayScrollEvt(int delta, Qt::Orientation orientation){
    bool up = delta > 0;
    emit trayIconScrolled(up);
}

void MainWindow::checkForCkbUpdates(){
#ifndef DISABLE_UPDATER
    updater = new CkbUpdater(this);
    connect(updater, &CkbUpdater::checkedForNewVer, this, &MainWindow::checkedForNewVer);
    updater->checkForNewVersion();
#endif
}

void MainWindow::toggleTrayIcon(bool visible){
    trayIcon->setVisible(visible);
}

void MainWindow::addDevice(Kb* device){
    // Connected already?
    foreach(KbWidget* w, kbWidgets){
        if(w->device == device)
            return;
    }
    // Add the keyboard
    KbWidget* widget = new KbWidget(this, device, windowDetector);
    kbWidgets.append(widget);
    // Add to tabber; switch to this device if the user is on the settings screen
    int count = ui->tabWidget->count();
    ui->tabWidget->insertTab(count - 1, widget, widget->name());
    if(ui->tabWidget->currentIndex() == count)
        ui->tabWidget->setCurrentIndex(count - 1);
    // Update connected device count
    updateVersion();
}

void MainWindow::removeDevice(Kb* device){
    foreach(KbWidget* w, kbWidgets){
        // Remove this device from the UI
        if(w->device == device){
            int i = kbWidgets.indexOf(w);
            ui->tabWidget->removeTab(i);
            kbWidgets.removeAt(i);
            w->deleteLater();
        }
    }
    // Update connected device count
    updateVersion();
}

void MainWindow::updateVersion(){
    QString daemonVersion = KbManager::ckbDaemonVersion();
    if(daemonVersion == DAEMON_UNAVAILABLE_STR){
        settingsWidget->setStatus(tr("Driver inactive"));
        return;
    }
    int count = kbWidgets.count();
    // Warn if the daemon version doesn't match the GUI
    QString daemonWarning;
    if(daemonVersion != CKB_NEXT_VERSION_STR)
        daemonWarning = tr("<br /><br /><b>Warning:</b> Driver version mismatch (") + daemonVersion + tr("). Please upgrade ckb-next") + QString(KbManager::ckbDaemonVersionF() > KbManager::ckbGuiVersionF() ? "" : "-daemon") + tr(". If the problem persists, try rebooting.");
    if(count == 0){
#if defined(Q_OS_MACOS) && !defined(OS_MAC_LEGACY)
        QProcess kextstat;
        kextstat.start("kextstat", QStringList() << "-l" << "-b" << "org.pqrs.driver.Karabiner.VirtualHIDDevice.v060800");

        if(!kextstat.waitForFinished())
            qDebug() << "Kextstat error";

        QString kextstatOut(kextstat.readAll());
        if(kextstatOut.isEmpty())
            daemonWarning.append(tr("<br /><b>Warning:</b> System Extension by \"Fumihiko Takayama\" is not allowed in Security & Privacy. Please allow it and then unplug and replug your devices."));
        if(is_catalina_or_higher())
            daemonWarning.append(tr("<br /><b>Warning:</b> Make sure ckb-next-daemon is allowed in Security & Privacy -> Input monitoring.<br />Please allow for up to 10 seconds for the daemon restart prompt to show up after allowing input monitoring."));
#elif defined(Q_OS_LINUX)
            if(!(QFileInfo("/dev/uinput").exists() || QFileInfo("/dev/input/uinput").exists())){
                QProcess modprobe;
                modprobe.start("modprobe", QStringList("uinput"));

                if(!modprobe.waitForFinished())
                    qDebug() << "Modprobe error";

                if(modprobe.exitCode())
                    daemonWarning.append(tr("<br /><b>Warning:</b> The uinput module could not be loaded. If this issue persists after rebooting, compile a kernel with CONFIG_INPUT_UINPUT=y."));
            }
#endif
        settingsWidget->setStatus(tr("No devices connected") + daemonWarning);
    }
    else if(count == 1)
        settingsWidget->setStatus(tr("1 device connected") + daemonWarning);
    else
        settingsWidget->setStatus(QString(tr("%1 devices connected")).arg(count) + daemonWarning);
}

void MainWindow::checkFwUpdates(){
    if(!mainWindow->isVisible())
        return;
    foreach(KbWidget* w, kbWidgets){
        // Display firmware upgrade notification if a new version is available
        float version = kbfw->versionForBoard(w->device->productID);
        if(version > w->device->firmware.toFloat()){
            if(w->hasShownNewFW)
                continue;
            w->hasShownNewFW = true;
            w->updateFwButton();
            // Don't run this method here because it will lock up the timer and prevent devices from working properly
            // Use a queued invocation instead
            metaObject()->invokeMethod(this, "showFwUpdateNotification", Qt::QueuedConnection, Q_ARG(QWidget*, w), Q_ARG(float, version));
            // Don't display more than one of these at once
            return;
        }
    }
}

void MainWindow::showFwUpdateNotification(QWidget* widget, float version){
    static bool isShowing = false;
    if(isShowing)
        return;
    isShowing = true;
    showWindow();
    KbWidget* w = (KbWidget*)widget;
    // Ask for update
    if(QMessageBox::information(this, "Firmware update", tr("A new firmware is available for your %1 (v%2)\nWould you like to install it now?").arg(w->device->usbModel, QString::number(version, 'f', 2)), QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No), QMessageBox::Yes) == QMessageBox::Yes){
        // If accepted, switch to the firmware tab and bring up the update window
        w->showLastTab();
        ui->tabWidget->setCurrentIndex(kbWidgets.indexOf(w));
        w->showFwUpdate();
    }
    isShowing = false;
}

void MainWindow::closeEvent(QCloseEvent *event){
    // If the window is hidden already or the event is non-spontaneous (can happen on OSX when using the Quit menu), accept it and close
    if(!event->spontaneous() || isHidden()){
        event->accept();
        return;
    }
    if(!CkbSettings::get("Popups/BGWarning").toBool()){
        QMessageBox::information(this, "ckb-next", tr("ckb-next will still run in the background.\nTo close it, choose Quit from the tray menu\nor click \"Quit\" on the Settings screen."));
        CkbSettings::set("Popups/BGWarning", true);
    }
    hide();
    event->ignore();
}

void MainWindow::timerTick(){
    // Check shared memory for changes
    if(appShare.lock()){
        void* shmData = appShare.data();
        QStringList commands = QString((const char*)shmData).split("\n");
        // Restore PID, remove all other data
        snprintf((char*)appShare.data(), appShare.size(), "PID %ld", (long)getpid());
        appShare.unlock();
        // Parse commands
        foreach(const QString& line, commands){
            // Old ckb option line - bring application to foreground
            if(line == "Open")
                showWindow();
            if(line.startsWith("Option ")){
                // New ckb option line
                QString option = line.section(' ', 1);
                if(option == "Open")
                    // Bring to foreground
                    showWindow();
                else if(option == "Close")
                    // Quit application
                    qApp->quit();
                else if(option.startsWith("SwitchToProfile"))
                    emit switchToProfileCLI(option.section(' ', 1));
                else if(option.startsWith("SwitchToMode: "))
                    emit switchToModeCLI(option.section(' ', 1));
            }
        }
    }
    // Check for firmware updates (when appropriate)
    if(!CkbSettings::get("Program/DisableAutoFWCheck").toBool()){
        kbfw->checkUpdates();
        checkFwUpdates();
    }
    // Poll for setting updates
    settingsWidget->pollUpdates();
}

void MainWindow::iconClicked(QSystemTrayIcon::ActivationReason reason){
    // On Linux, hide/show the app when the tray icon is clicked
    // On OSX this just shows the menu
#ifndef Q_OS_MACOS
    if(reason == QSystemTrayIcon::DoubleClick || reason == QSystemTrayIcon::Trigger){
        if(isVisible())
            hide();
        else
            showWindow();
    }
#endif
}

void MainWindow::showWindow(){
    showNormal();
    raise();
    activateWindow();
}

void MainWindow::stateChange(Qt::ApplicationState state){
    // On OSX it's possible for the app to be brought to the foreground without the window actually reappearing.
    // We want to make sure it's shown when this happens.
#ifdef Q_OS_MAC
    static quint64 lastStateChange = 0;
    quint64 now = QDateTime::currentMSecsSinceEpoch();
    if(state == Qt::ApplicationActive){
        // This happens once at startup so ignore it. Also don't allow it to be called more than once every 2s.
        if(lastStateChange != 0 && now >= lastStateChange + 2 * 1000)
            showWindow();
        lastStateChange = now;
    }
#endif
}

void MainWindow::quitApp(){
    qApp->quit();
}

QIcon MainWindow::getIcon() {
    // on initial launch (first time using ckb) this association checked
    // will not be present, so force it to true, as we do not want to break default behaviour
    if (CkbSettings::get("Program/RGBIcon", QVariant(true)).toBool())
        return QIcon(":/img/ckb-next.png");

    return QIcon(":/img/ckb-next-monochrome.png");
}

QString MainWindow::getIconName() {
    // Same as above but return QStrings
    if (CkbSettings::get("Program/RGBIcon", QVariant(true)).toBool())
        return "ckb-next";

    return "ckb-next-monochrome";
}

void MainWindow::setTabsEnabled(bool e){
    ui->tabWidget->tabBar()->setEnabled(e);
    QWidget* currentWidget = ui->tabWidget->currentWidget();
    if(!currentWidget){
        qDebug() << "currentWidget in setTabsEnabled is null";
        return;
    }
    KbWidget* kbw = dynamic_cast<KbWidget*>(currentWidget);
    // The cast can fail if the app is quitting
    if(!kbw)
        return;
    kbw->setTabBarEnabled(e);
}

MainWindow::~MainWindow(){
    foreach(KbWidget* w, kbWidgets)
        delete w;
    kbWidgets.clear();
    KbManager::stop();
    deinitAudioSubsystem();
    delete kbfw;
    CkbSettings::cleanUp();
#ifdef USE_XCB_EWMH
    if(windowDetector){
        windowDetector->terminateEventLoop();
        windowDetector->wait();
    }
#endif
    delete ui;
}

void MainWindow::QSignalHandler(){
    sigNotifier->setEnabled(false);
    int sig = -1;
    int ret = read(signalHandlerFd[1], &sig, sizeof(sig));

    if(ret == -1){
        qDebug() << "Error on QSignalHandler read";
        return;
    }

    qDebug() << "\nSignal" << sig << "caught. Quitting...";
    if(sig == SIGHUP){
        // Restart, but with a delay
        QProcess::startDetached(QCoreApplication::applicationFilePath(), QStringList() << "-b" << "-d");
    }
    this->quitApp();
    sigNotifier->setEnabled(true);
}

void MainWindow::PosixSignalHandler(int signal){
    int ret = write(MainWindow::signalHandlerFd[0], &signal, sizeof(signal));
    if(ret == -1)
        qDebug() << "Error on PosixSignalHandler write";
}

void MainWindow::checkedForNewVer(QString ver, QString changelog){
#ifndef DISABLE_UPDATER
    if(!ver.isEmpty()) {
        settingsWidget->setUpdateButtonText(tr("Update to v") + ver);
        showWindow();
        CkbUpdaterDialog updDialog(ver, changelog, this);
        updDialog.exec();
    } else {
        settingsWidget->setUpdateButtonText(tr("Up to date"));
    }
    updater->deleteLater();
    settingsWidget->enableUpdateButton();
#endif
}

void MainWindow::syncTrayIcon(){
    trayIcon->setIcon(getIcon(), getIconName());
}

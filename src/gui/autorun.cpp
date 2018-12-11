#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include "autorun.h"
#include "ckbsettings.h"

// >=0.3.0 (new) paths
#ifdef Q_OS_LINUX
static QDir path(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.config/autostart");
static const QString file = "ckb-next.autostart.desktop";
static const QString internalFile(":/txt/ckb-next.autostart.desktop");
#elif defined(Q_OS_MACOS)
static QDir path(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/Library/LaunchAgents");
static const QString file = "org.ckb-next.plist";
static const QString internalFile(":/txt/org.ckb-next.plist");
#else
static QDir path("");
static const QString file = "";
static const QString internalFile("");
#endif

static const QString settingPath = "Program/NewDidLoginItem";

// <=0.2.9 (old) paths
#ifdef Q_OS_LINUX
static const QString oldFile = "ckb.desktop";
#elif defined(Q_OS_MACX)
static const QString oldFile = "com.ckb.ckb.plist";
#else
static const QString oldFile = "";
#endif

static const QString oldSettingPath = "Program/DidLoginItem";

bool AutoRun::available() {
    // Allow autostart if the program is located in a system path
#ifdef Q_OS_LINUX
    return QDir::root().absoluteFilePath(QStandardPaths::findExecutable("ckb-next")) == qApp->applicationFilePath();
#elif defined(Q_OS_MACOS)
    return qApp->applicationFilePath().startsWith("/Applications/ckb-next.app", Qt::CaseInsensitive);
//#elif defined(Q_OS_WINDOWS)
#else
    return false;
#endif
}

bool AutoRun::once() {
    return CkbSettings::get(settingPath).toBool();
}

bool AutoRun::isEnabled() {
    // Check if the file exists. If not, autostart is disabled.
    if (!path.exists() || !path.exists(file))
        return false;
    // If autostart is enabled, set the flag from once() (in case it hasn't been done yet)
    CkbSettings::set(settingPath, true);
    return true;
}

void AutoRun::enable() {
    if (!available())
        return;

    // Copy file into place
    if (!path.exists())
        QDir::home().mkpath(path.absolutePath());
    QFile::copy(internalFile, path.absoluteFilePath(file));
    // Mark once() as done
    CkbSettings::set(settingPath, true);

    // If an old autostart was enabled, disable it and remove
    if (CkbSettings::get(oldSettingPath).toBool()) {
        CkbSettings::set(oldSettingPath, false);
        QFile::remove(path.absoluteFilePath(oldFile));
    }
}

void AutoRun::disable() {
    if (!available())
        return;
    // Remove file
    QFile::remove(path.absoluteFilePath(file));
}

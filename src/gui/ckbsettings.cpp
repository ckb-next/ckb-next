#include "ckbsettings.h"
#include "ckbsettingswriter.h"
#include <QThread>
#include <QMutex>
#include <QDebug>

// Shared global QSettings object
static QSettings* _globalSettings = 0;
static QThread* globalThread = 0;
QAtomicInt cacheWritesInProgress(0);
// Global one-shot settings cache, to avoid reading/writing QSettings constantly
static QMap<QString, QVariant> globalCache;
// Mutexes for accessing settings
QMutex settingsMutex(QMutex::Recursive), settingsCacheMutex(QMutex::Recursive);
#define lockMutex           QMutexLocker locker(backing == _globalSettings ? &settingsMutex : 0)
#define lockMutexStatic     QMutexLocker locker(&settingsMutex)
#define lockMutexStatic2    QMutexLocker locker2(&settingsMutex)
#define lockMutexCache      QMutexLocker locker(&settingsCacheMutex)

#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#define qInfo       qDebug
#define qWarning    qDebug
#define qFatal      qDebug
#define qCritical   qDebug
#endif

static QSettings* globalSettings(){
    if(!_globalSettings){
        lockMutexStatic;
        if(!(volatile QSettings*)_globalSettings){   // Check again after locking mutex in case another thread created the object
            _globalSettings = new QSettings;
            qInfo() << "Path to settings:" << _globalSettings->fileName();
            // Check if a config migration attempt needs to be made.
            if(!_globalSettings->value("Program/CkbMigrationChecked", false).toBool()){
                CkbSettings::migrateSettings();
            }
            // Put the settings object in a separate thread so that it won't lock up the GUI when it syncs
            globalThread = new QThread;
            globalThread->start();
            _globalSettings->moveToThread(globalThread);
        }
    }
    return _globalSettings;
}

bool CkbSettings::isBusy(){
    return cacheWritesInProgress.load() > 0;
}

void CkbSettings::migrateSettings(){
    QSettings* oldSettings = new QSettings("ckb", "ckb");
    // On macOS and iOS, allKeys() will return some extra keys for global settings that apply to all applications.
    // These keys can be read using value() but cannot be changed, only shadowed. Calling setFallbacksEnabled(false) will hide these global settings.
    // https://doc.qt.io/qt-5/qsettings.html
    oldSettings->setFallbacksEnabled(false);
    qInfo() << "Looking for old settings in:" << oldSettings->fileName();
    // Check if a basic key exists
    if(oldSettings->contains("Program/KbdLayout")){
        qInfo() << "Found, proceeding to migrate.";
        QStringList oldKeys = oldSettings->allKeys();
        foreach (const QString &key, oldKeys){
            QVariant value = oldSettings->value(key);
            _globalSettings->setValue(key, value);
        }
        // Mark settings as migrated
        _globalSettings->setValue("Program/CkbMigrationChecked", true);
        qInfo() << oldKeys.count() << "keys migrated.";
    }
    delete oldSettings;
}

void CkbSettings::cleanUp(){
    if(!_globalSettings)
        return;
    // Wait for all writers to finish
    while(cacheWritesInProgress.load() > 0)
        QThread::yieldCurrentThread();
    // Stop thread and delete objects
    globalThread->quit();
    globalThread->wait();
    delete globalThread;
    delete _globalSettings;
    globalThread = 0;
    _globalSettings = 0;
}

CkbSettings::CkbSettings() :
    backing(globalSettings()) {
}

CkbSettings::CkbSettings(const QString& basePath, bool eraseExisting) :
    backing(globalSettings()) {
    if(basePath.isEmpty()){
        if(eraseExisting)
            qDebug() << "CkbSettings created with basePath = \"\" and eraseExisting = true. This is a mistake.";
        return;
    }
    if(eraseExisting)
        remove(basePath);
    beginGroup(basePath);
}

CkbSettings::CkbSettings(QSettings& settings) :
    backing(&settings) {
}

void CkbSettings::beginGroup(const QString& prefix){
    groups.append(prefix);
}

void CkbSettings::endGroup(){
    groups.removeLast();
}

QStringList CkbSettings::childGroups() const {
    QString current = pwd();
    lockMutex;
    if(!current.isEmpty())
        backing->beginGroup(current);
    QStringList res = backing->childGroups();
    if(!current.isEmpty())
        backing->endGroup();
    return res;
}

QStringList CkbSettings::childKeys() const {
    QString current = pwd();
    lockMutex;
    if(!current.isEmpty())
        backing->beginGroup(current);
    QStringList res = backing->childKeys();
    if(!current.isEmpty())
        backing->endGroup();
    return res;
}

bool CkbSettings::contains(const QString& key) const {
    lockMutex;
    return backing->contains(pwd(key));
}

bool CkbSettings::containsGroup(const QString &group){
    QStringList components = group.split("/");
    if(components.length() > 1){
        // Find sub-group
        SGroup group(*this, components[0]);
        return containsGroup(QStringList(components.mid(1)).join('/'));
    }
    return childGroups().contains(group);
}

QVariant CkbSettings::value(const QString& key, const QVariant& defaultValue) const {
    lockMutex;
    return backing->value(pwd(key), defaultValue);
}

void CkbSettings::setValue(const QString& key, const QVariant& value){
    //qDebug() << "Setting " << key << "to value" << value;
    // Cache the write values, save them when the object is destroyed
    QString realKey = pwd(key);
    writeCache[realKey] = value;
    // Update global cache if needed
    lockMutexCache;
    if(globalCache.contains(realKey))
        globalCache[realKey] = value;
}

void CkbSettings::remove(const QString& key){
    removeCache.append(pwd(key));
}

CkbSettings::~CkbSettings(){
    if(removeCache.isEmpty() && writeCache.isEmpty())
        return;
    // Save the settings from the settings thread.
    // They have to be saved from that thread specifically to avoid performance issues
    CkbSettingsWriter* writer = new CkbSettingsWriter(backing, removeCache, writeCache);
    writer->moveToThread(globalThread);
    QObject::staticMetaObject.invokeMethod(writer, "run", Qt::QueuedConnection);
}

QVariant CkbSettings::get(const QString& key, const QVariant& defaultValue){
    // Store these settings in a memory cache
    lockMutexCache;
    if(globalCache.contains(key))
        return globalCache.value(key);
    // If it wasn't found in the memory cache, look for it in QSettings
    lockMutexStatic2;
    QSettings* settings = globalSettings();
    return globalCache[key] = settings->value(key, defaultValue);
}

void CkbSettings::set(const QString& key, const QVariant& value){
    {
        lockMutexCache;
        if(globalCache.value(key) == value)
            return;
        globalCache[key] = value;
    }
    lockMutexStatic;
    globalSettings()->setValue(key, value);
}

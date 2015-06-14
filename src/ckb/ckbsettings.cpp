#include "ckbsettings.h"
#include <QThread>

// Shared global QSettings object
static QSettings* _globalSettings = 0;
static QThread* globalThread = 0;
// Global one-shot settings cache, to avoid reading/writing QSettings constantly
static QMap<QString, QVariant> globalCache;
// Mutexes for accessing settings
static QMutex mutex(QMutex::Recursive), cacheMutex(QMutex::Recursive);
#define lockMutex           QMutexLocker locker(backing == _globalSettings ? &mutex : 0)
#define lockMutexStatic     QMutexLocker locker(&mutex)
#define lockMutexStatic2    QMutexLocker locker2(&mutex)
#define lockMutexCache      QMutexLocker locker(&cacheMutex)

static QSettings* globalSettings(){
    lockMutexStatic;
    if(!_globalSettings){
        // Put the settings object in a separate thread so that it won't lock up the GUI when it syncs
        globalThread = new QThread;
        globalThread->start();
        _globalSettings = new QSettings;
        _globalSettings->moveToThread(globalThread);
    }
    return _globalSettings;
}

bool CkbSettings::isBusy(){
    if(mutex.tryLock()){
        mutex.unlock();
        return false;
    }
    return true;
}

CkbSettings::CkbSettings() :
    backing(globalSettings()) {
}

CkbSettings::CkbSettings(const QString& basePath, bool eraseExisting) :
    backing(globalSettings()) {
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

QVariant CkbSettings::value(const QString& key, const QVariant& defaultValue) const {
    lockMutex;
    return backing->value(pwd(key), defaultValue);
}

void CkbSettings::setValue(const QString& key, const QVariant& value){
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

// Writes to the cache happen in a separate thread so that they don't slow the main application down
class CacheWriter : public QThread {
public:
    CacheWriter(const QStringList& removals, const QMap<QString, QVariant>& updates) :
        _removals(removals), _updates(updates) {
    }

    void run(){
        lockMutexStatic;
        // Process key removals
        QSettings* settings = globalSettings();
        foreach(const QString& rm, _removals){
            settings->remove(rm);
        }
        // Process writes
        QMapIterator<QString, QVariant> i(_updates);
        while(i.hasNext()){
            i.next();
            settings->setValue(i.key(), i.value());
            // Updating the global cache was done above
        }
    }

private:
    QStringList _removals;
    QMap<QString, QVariant> _updates;
};

CkbSettings::~CkbSettings(){
    if(removeCache.isEmpty() && writeCache.isEmpty())
        return;
    // Launch a new thread to save the settings
    CacheWriter* writeThread = new CacheWriter(removeCache, writeCache);
    QObject::connect(writeThread, SIGNAL(finished()), writeThread, SLOT(deleteLater()));
    writeThread->start();
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

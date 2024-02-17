#include "ckbsettingswriter.h"

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
#include <QRecursiveMutex>
#else
#include <QMutex>
#endif

// Mirror ckbsettings.cpp
extern QAtomicInt cacheWritesInProgress;
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
extern QRecursiveMutex settingsMutex, settingsCacheMutex;
#else
extern QMutex settingsMutex, settingsCacheMutex;
#endif
#define lockMutex           QMutexLocker locker(backing == _globalSettings ? &mutex : 0)
#define lockMutexStatic     QMutexLocker locker(&settingsMutex)
#define lockMutexStatic2    QMutexLocker locker2(&settingsMutex)
#define lockMutexCache      QMutexLocker locker(&settingsCacheMutex)

CkbSettingsWriter::CkbSettingsWriter(QSettings* backing, const QStringList& removals, const QMap<QString, QVariant>& updates) :
    _backing(backing), _removals(removals), _updates(updates) {
    cacheWritesInProgress.ref();
}

CkbSettingsWriter::~CkbSettingsWriter(){
    cacheWritesInProgress.deref();
}

void CkbSettingsWriter::run(){
    lockMutexStatic;
    // Process key removals
    foreach(const QString& rm, _removals){
        _backing->remove(rm);
    }
    // Process writes
    QMapIterator<QString, QVariant> i(_updates);
    while(i.hasNext()){
        i.next();
        _backing->setValue(i.key(), i.value());
        // Updating the global cache was done above
    }
    _backing->sync();
    deleteLater();
}

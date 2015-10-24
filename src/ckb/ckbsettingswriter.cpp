#include "ckbsettingswriter.h"
#include <QMutex>

// Mirror ckbsettings.cpp
extern QAtomicInt cacheWritesInProgress;
extern QMutex settingsMutex, settingsCacheMutex;
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

#ifndef CKBSETTINGSWRITER_H
#define CKBSETTINGSWRITER_H

#include <QObject>
#include <QMap>
#include <QSettings>
#include <QStringList>

// Setting de-cacher for CkbSettings
// It has to be declared in its own header or else Q_OBJECT doesn't work

class CkbSettingsWriter : public QObject {
    Q_OBJECT
public:
    CkbSettingsWriter(QSettings* backing, const QStringList& removals, const QMap<QString, QVariant>& updates);
    ~CkbSettingsWriter();

    Q_SLOT void run();

private:
    QSettings* _backing;
    QStringList _removals;
    QMap<QString, QVariant> _updates;
};

#endif // CKBSETTINGSWRITER_H

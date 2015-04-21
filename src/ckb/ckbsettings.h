#ifndef CKBSETTINGS_H
#define CKBSETTINGS_H

#include <QSettings>

// QSettings wrapper with convenience functions

class CkbSettings : public QSettings
{
public:
    // Basic settings object. No default group.
    CkbSettings();
    // Settings object with a default group. Optionally erases any existing group with the same name.
    CkbSettings(const QString& basePath, bool eraseExisting = false);
    ~CkbSettings();

    // One-shot get/set
    static QVariant get(const QString& key, const QVariant& defaultValue = QVariant());
    static void set(const QString& key, const QVariant& value);
};

#endif // CKBSETTINGS_H

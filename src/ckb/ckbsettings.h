#ifndef CKBSETTINGS_H
#define CKBSETTINGS_H

#include <QSettings>
#include <QStringList>
#include <QMap>
#include <QVariant>

// QSettings replacement with convenience functions

class CkbSettings
{
public:
    // Basic settings object. No default group.
    CkbSettings();
    // Settings object with a default group. Optionally erases any existing group with the same name.
    CkbSettings(const QString& basePath, bool eraseExisting = false);
    // CkbSettings from QSettings
    CkbSettings(QSettings& settings);

    ~CkbSettings();

    // One-shot get/set
    static QVariant get(const QString& key, const QVariant& defaultValue = QVariant());
    static void     set(const QString& key, const QVariant& value);

    // Whether or not CkbSettings is busy writing data. If busy, the constructors will block until it is not.
    // The global set() will also block if busy, but global get() will not, unless the value has never been read before.
    static bool isBusy();

    // Finalize all writes, clean up and release resources
    static void cleanUp();

    // QSettings functions
    void        beginGroup(const QString& prefix);
    void        endGroup();
    QStringList childGroups() const;
    QStringList childKeys() const;
    bool        contains(const QString& key) const;
    bool        containsGroup(const QString& group);
    QVariant    value(const QString& key, const QVariant& defaultValue = QVariant()) const;
    void        setValue(const QString& key, const QVariant& value);
    void        remove(const QString& key);

private:
    QSettings* backing;
    QStringList groups;
    QStringList removeCache;
    QMap<QString, QVariant> writeCache;

    inline QString pwd() const { return groups.join("/"); }
    inline QString pwd(const QString& key) const { return pwd() + (groups.isEmpty() ? "" : "/") + key; }
};

// Settings group wrapper. Useful for easily pushing/popping a group without worrying about everything breaking if you forgot endGroup.

class SGroup
{
public:
    inline SGroup(CkbSettings& settings, const QString& prefix) : _settings(settings) { settings.beginGroup(prefix); }
    inline ~SGroup() { _settings.endGroup(); }

private:
    CkbSettings& _settings;
};

#endif // CKBSETTINGS_H

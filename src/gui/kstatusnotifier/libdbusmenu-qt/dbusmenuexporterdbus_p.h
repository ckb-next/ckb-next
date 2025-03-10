/* This file is part of the dbusmenu-qt library
   SPDX-FileCopyrightText: 2010 Canonical
   Author: Aurelien Gateau <aurelien.gateau@canonical.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef DBUSMENUEXPORTERDBUS_P_H
#define DBUSMENUEXPORTERDBUS_P_H

// Local
#include "dbusmenutypes_p.h"

// Qt
#include <QDBusAbstractAdaptor>
#include <QDBusVariant>
#include <QObject>
#include <QVariant>

class DBusMenuExporter;

/**
 * Internal class implementing the DBus side of DBusMenuExporter
 * This avoid exposing the implementation of the DBusMenu spec to the outside
 * world.
 */
class DBusMenuExporterDBus : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.canonical.dbusmenu")
    Q_PROPERTY(uint Version READ Version)
    Q_PROPERTY(QString Status READ status)
public:
    DBusMenuExporterDBus(DBusMenuExporter *m_exporter);

    uint Version() const
    {
        return 2;
    }

    QString status() const;
    void setStatus(const QString &status);

public Q_SLOTS:
    Q_NOREPLY void Event(int id, const QString &eventId, const QDBusVariant &data, uint timestamp);
    QDBusVariant GetProperty(int id, const QString &property);
    uint GetLayout(int parentId, int recursionDepth, const QStringList &propertyNames, DBusMenuLayoutItem &item);
    DBusMenuItemList GetGroupProperties(const QList<int> &ids, const QStringList &propertyNames);
    bool AboutToShow(int id);

Q_SIGNALS:
    void ItemsPropertiesUpdated(DBusMenuItemList, DBusMenuItemKeysList);
    void LayoutUpdated(uint revision, int parentId);
    void ItemActivationRequested(int id, uint timeStamp);

private:
    DBusMenuExporter *m_exporter = nullptr;
    QString m_status;

    friend class DBusMenuExporter;
    friend class DBusMenuExporterPrivate;

    QVariantMap getProperties(int id, const QStringList &names) const;
};

#endif /* DBUSMENUEXPORTERDBUS_P_H */

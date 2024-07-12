/* This file is part of the dbusmenu-qt library
   SPDX-FileCopyrightText: 2010 Canonical
   Author: Aurelien Gateau <aurelien.gateau@canonical.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "dbusmenuexporterdbus_p.h"

// Qt
#include <QDBusMessage>
#include <QMenu>
#include <QVariant>

// Local
#include "dbusmenuadaptor.h"
#include "dbusmenuexporterprivate_p.h"
#include "dbusmenushortcut_p.h"
#include "debug_p.h"

static const char *DBUSMENU_INTERFACE = "com.canonical.dbusmenu";
static const char *FDO_PROPERTIES_INTERFACE = "org.freedesktop.DBus.Properties";

DBusMenuExporterDBus::DBusMenuExporterDBus(DBusMenuExporter *exporter)
    : QObject(exporter)
    , m_exporter(exporter)
    , m_status(QStringLiteral("normal"))
{
    DBusMenuTypes_register();
    new DbusmenuAdaptor(this);
}

uint DBusMenuExporterDBus::GetLayout(int parentId, int recursionDepth, const QStringList &propertyNames, DBusMenuLayoutItem &item)
{
    QMenu *menu = m_exporter->d->menuForId(parentId);
    DMRETURN_VALUE_IF_FAIL(menu, 0);

    // Process pending actions, we need them *now*
    QMetaObject::invokeMethod(m_exporter, "doUpdateActions");
    m_exporter->d->fillLayoutItem(&item, menu, parentId, recursionDepth, propertyNames);

    return m_exporter->d->m_revision;
}

void DBusMenuExporterDBus::Event(int id, const QString &eventType, const QDBusVariant & /*data*/, uint /*timestamp*/)
{
    if (eventType == QStringLiteral("clicked")) {
        QAction *action = m_exporter->d->m_actionForId.value(id);
        if (!action) {
            return;
        }
        // dbusmenu-glib seems to ignore the Q_NOREPLY and blocks when calling
        // Event(), so trigger the action asynchronously
        QMetaObject::invokeMethod(action, "trigger", Qt::QueuedConnection);
    } else if (eventType == QStringLiteral("hovered")) {
        QMenu *menu = m_exporter->d->menuForId(id);
        if (menu) {
            QMetaObject::invokeMethod(menu, "aboutToShow");
        }
    }
}

QDBusVariant DBusMenuExporterDBus::GetProperty(int id, const QString &name)
{
    QAction *action = m_exporter->d->m_actionForId.value(id);
    DMRETURN_VALUE_IF_FAIL(action, QDBusVariant());
    return QDBusVariant(m_exporter->d->m_actionProperties.value(action).value(name));
}

QVariantMap DBusMenuExporterDBus::getProperties(int id, const QStringList &names) const
{
    if (id == 0) {
        QVariantMap map;
        map.insert(QStringLiteral("children-display"), QStringLiteral("submenu"));
        return map;
    }
    QAction *action = m_exporter->d->m_actionForId.value(id);
    DMRETURN_VALUE_IF_FAIL(action, QVariantMap());
    const QVariantMap all = m_exporter->d->m_actionProperties.value(action);
    if (names.isEmpty()) {
        return all;
    } else {
        QVariantMap map;
        for (const QString &name : names) {
            QVariant value = all.value(name);
            if (value.isValid()) {
                map.insert(name, value);
            }
        }
        return map;
    }
}

DBusMenuItemList DBusMenuExporterDBus::GetGroupProperties(const QList<int> &ids, const QStringList &names)
{
    DBusMenuItemList list;
    for (int id : ids) {
        DBusMenuItem item;
        item.id = id;
        item.properties = getProperties(item.id, names);
        list << item;
    }
    return list;
}

/**
 * An helper class for ::AboutToShow, which sets mChanged to true if a menu
 * changes after its aboutToShow() signal has been emitted.
 */
class ActionEventFilter : public QObject
{
public:
    ActionEventFilter()
    {
    }

    bool mChanged = false;

protected:
    bool eventFilter(QObject *object, QEvent *event) override
    {
        switch (event->type()) {
        case QEvent::ActionAdded:
        case QEvent::ActionChanged:
        case QEvent::ActionRemoved:
            mChanged = true;
            // We noticed a change, no need to filter anymore
            object->removeEventFilter(this);
            break;
        default:
            break;
        }
        return false;
    }
};

bool DBusMenuExporterDBus::AboutToShow(int id)
{
    QMenu *menu = m_exporter->d->menuForId(id);
    DMRETURN_VALUE_IF_FAIL(menu, false);

    ActionEventFilter filter;
    menu->installEventFilter(&filter);
    QMetaObject::invokeMethod(menu, "aboutToShow");
    return filter.mChanged;
}

void DBusMenuExporterDBus::setStatus(const QString &status)
{
    if (m_status == status) {
        return;
    }
    m_status = status;

    QVariantMap map;
    map.insert(QStringLiteral("Status"), QVariant(status));

    QDBusMessage msg =
        QDBusMessage::createSignal(m_exporter->d->m_objectPath, QString::fromLatin1(FDO_PROPERTIES_INTERFACE), QStringLiteral("PropertiesChanged"));
    QVariantList args = QVariantList() << QString::fromLatin1(DBUSMENU_INTERFACE) << map << QStringList() // New properties: none
        ;
    msg.setArguments(args);
    QDBusConnection::sessionBus().send(msg);
}

QString DBusMenuExporterDBus::status() const
{
    return m_status;
}

#include "moc_dbusmenuexporterdbus_p.cpp"

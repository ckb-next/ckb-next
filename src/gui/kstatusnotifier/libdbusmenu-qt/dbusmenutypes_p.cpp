/* This file is part of the dbusmenu-qt library
   SPDX-FileCopyrightText: 2009 Canonical
   Author: Aurelien Gateau <aurelien.gateau@canonical.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "dbusmenutypes_p.h"

// Local
#include "dbusmenushortcut_p.h"
#include "debug_p.h"

// Qt
#include <QDBusArgument>
#include <QDBusMetaType>

//// DBusMenuItem
QDBusArgument &operator<<(QDBusArgument &argument, const DBusMenuItem &obj)
{
    argument.beginStructure();
    argument << obj.id << obj.properties;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DBusMenuItem &obj)
{
    argument.beginStructure();
    argument >> obj.id >> obj.properties;
    argument.endStructure();
    return argument;
}

//// DBusMenuItemKeys
QDBusArgument &operator<<(QDBusArgument &argument, const DBusMenuItemKeys &obj)
{
    argument.beginStructure();
    argument << obj.id << obj.properties;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DBusMenuItemKeys &obj)
{
    argument.beginStructure();
    argument >> obj.id >> obj.properties;
    argument.endStructure();
    return argument;
}

//// DBusMenuLayoutItem
QDBusArgument &operator<<(QDBusArgument &argument, const DBusMenuLayoutItem &obj)
{
    argument.beginStructure();
    argument << obj.id << obj.properties;
    argument.beginArray(qMetaTypeId<QDBusVariant>());
    const auto childrens = obj.children;
    for (const DBusMenuLayoutItem &child : childrens) {
        argument << QDBusVariant(QVariant::fromValue<DBusMenuLayoutItem>(child));
    }
    argument.endArray();
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DBusMenuLayoutItem &obj)
{
    argument.beginStructure();
    argument >> obj.id >> obj.properties;
    argument.beginArray();
    while (!argument.atEnd()) {
        QDBusVariant dbusVariant;
        argument >> dbusVariant;
        QDBusArgument childArgument = dbusVariant.variant().value<QDBusArgument>();

        DBusMenuLayoutItem child;
        childArgument >> child;
        obj.children.append(child);
    }
    argument.endArray();
    argument.endStructure();
    return argument;
}

void DBusMenuTypes_register()
{
    static bool registered = false;
    if (registered) {
        return;
    }
    qDBusRegisterMetaType<DBusMenuItem>();
    qDBusRegisterMetaType<DBusMenuItemList>();
    qDBusRegisterMetaType<DBusMenuItemKeys>();
    qDBusRegisterMetaType<DBusMenuItemKeysList>();
    qDBusRegisterMetaType<DBusMenuLayoutItem>();
    qDBusRegisterMetaType<DBusMenuLayoutItemList>();
    qDBusRegisterMetaType<DBusMenuShortcut>();
    registered = true;
}

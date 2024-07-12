/* This file is part of the dbusmenu-qt library
   SPDX-FileCopyrightText: 2009 Canonical
   Author: Aurelien Gateau <aurelien.gateau@canonical.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "dbusmenuexporter.h"

// Qt
#include <QActionGroup>
#include <QBuffer>
#include <QDateTime>
#include <QMap>
#include <QMenu>
#include <QSet>
#include <QTimer>
#include <QToolButton>
#include <QWidgetAction>

// Local
#include "dbusmenu_p.h"
#include "dbusmenuexporterdbus_p.h"
#include "dbusmenuexporterprivate_p.h"
#include "dbusmenushortcut_p.h"
#include "dbusmenutypes_p.h"
#include "debug_p.h"
#include "utils_p.h"

static const char *KMENU_TITLE = "kmenu_title";

//-------------------------------------------------
//
// DBusMenuExporterPrivate
//
//-------------------------------------------------
int DBusMenuExporterPrivate::idForAction(QAction *action) const
{
    DMRETURN_VALUE_IF_FAIL(action, -1);
    return m_idForAction.value(action, -2);
}

void DBusMenuExporterPrivate::addMenu(QMenu *menu, int parentId)
{
    if (menu->findChild<DBusMenu *>()) {
        // This can happen if a menu is removed from its parent and added back
        // See KDE bug 254066
        return;
    }
    new DBusMenu(menu, q, parentId);
    const auto actions = menu->actions();
    for (QAction *action : actions) {
        addAction(action, parentId);
    }
}

QVariantMap DBusMenuExporterPrivate::propertiesForAction(QAction *action) const
{
    DMRETURN_VALUE_IF_FAIL(action, QVariantMap());

    if (action->objectName() == QString::fromLatin1(KMENU_TITLE)) {
        // Hack: Support for KDE menu titles in a Qt-only library...
        return propertiesForKMenuTitleAction(action);
    } else if (action->isSeparator()) {
        return propertiesForSeparatorAction(action);
    } else {
        return propertiesForStandardAction(action);
    }
}

QVariantMap DBusMenuExporterPrivate::propertiesForKMenuTitleAction(QAction *action_) const
{
    QVariantMap map;
    // In case the other side does not know about x-kde-title, show a disabled item
    map.insert(QStringLiteral("enabled"), false);
    map.insert(QStringLiteral("x-kde-title"), true);

    const QWidgetAction *widgetAction = qobject_cast<const QWidgetAction *>(action_);
    DMRETURN_VALUE_IF_FAIL(widgetAction, map);
    QToolButton *button = qobject_cast<QToolButton *>(widgetAction->defaultWidget());
    DMRETURN_VALUE_IF_FAIL(button, map);
    QAction *action = button->defaultAction();
    DMRETURN_VALUE_IF_FAIL(action, map);

    map.insert(QStringLiteral("label"), swapMnemonicChar(action->text(), QLatin1Char('&'), QLatin1Char('_')));
    insertIconProperty(&map, action);
    if (!action->isVisible()) {
        map.insert(QStringLiteral("visible"), false);
    }
    return map;
}

QVariantMap DBusMenuExporterPrivate::propertiesForSeparatorAction(QAction *action) const
{
    QVariantMap map;
    map.insert(QStringLiteral("type"), QStringLiteral("separator"));
    if (!action->isVisible()) {
        map.insert(QStringLiteral("visible"), false);
    }
    return map;
}

QVariantMap DBusMenuExporterPrivate::propertiesForStandardAction(QAction *action) const
{
    QVariantMap map;
    map.insert(QStringLiteral("label"), swapMnemonicChar(action->text(), QLatin1Char('&'), QLatin1Char('_')));
    if (!action->isEnabled()) {
        map.insert(QStringLiteral("enabled"), false);
    }
    if (!action->isVisible()) {
        map.insert(QStringLiteral("visible"), false);
    }
    if (action->menu()) {
        map.insert(QStringLiteral("children-display"), QStringLiteral("submenu"));
    }
    if (action->isCheckable()) {
        bool exclusive = action->actionGroup() && action->actionGroup()->isExclusive();
        map.insert(QStringLiteral("toggle-type"), exclusive ? QStringLiteral("radio") : QStringLiteral("checkmark"));
        map.insert(QStringLiteral("toggle-state"), action->isChecked() ? 1 : 0);
    }
    insertIconProperty(&map, action);
    QKeySequence keySequence = action->shortcut();
    if (!keySequence.isEmpty()) {
        DBusMenuShortcut shortcut = DBusMenuShortcut::fromKeySequence(keySequence);
        map.insert(QStringLiteral("shortcut"), QVariant::fromValue(shortcut));
    }
    return map;
}

QMenu *DBusMenuExporterPrivate::menuForId(int id) const
{
    if (id == 0) {
        return m_rootMenu;
    }
    QAction *action = m_actionForId.value(id);
    // Action may not be in m_actionForId if it has been deleted between the
    // time it was announced by the exporter and the time the importer asks for
    // it.
    return action ? action->menu() : nullptr;
}

void DBusMenuExporterPrivate::fillLayoutItem(DBusMenuLayoutItem *item, QMenu *menu, int id, int depth, const QStringList &propertyNames)
{
    item->id = id;
    item->properties = m_dbusObject->getProperties(id, propertyNames);

    if (depth != 0 && menu) {
        const auto actions = menu->actions();
        for (QAction *action : actions) {
            int actionId = m_idForAction.value(action, -1);
            if (actionId == -1) {
                DMWARNING << "No id for action";
                continue;
            }

            DBusMenuLayoutItem child;
            fillLayoutItem(&child, action->menu(), actionId, depth - 1, propertyNames);
            item->children << child;
        }
    }
}

void DBusMenuExporterPrivate::updateAction(QAction *action)
{
    int id = idForAction(action);
    if (m_itemUpdatedIds.contains(id)) {
        return;
    }
    m_itemUpdatedIds << id;
    m_itemUpdatedTimer->start();
}

void DBusMenuExporterPrivate::addAction(QAction *action, int parentId)
{
    int id = m_idForAction.value(action, -1);
    if (id != -1) {
        DMWARNING << "Already tracking action" << action->text() << "under id" << id;
        return;
    }
    QVariantMap map = propertiesForAction(action);
    id = m_nextId++;
    QObject::connect(action, SIGNAL(destroyed(QObject *)), q, SLOT(slotActionDestroyed(QObject *)));
    m_actionForId.insert(id, action);
    m_idForAction.insert(action, id);
    m_actionProperties.insert(action, map);
    if (action->menu()) {
        addMenu(action->menu(), id);
    }
    ++m_revision;
    emitLayoutUpdated(parentId);
}

/**
 * IMPORTANT: action might have already been destroyed when this method is
 * called, so don't dereference the pointer (it is a QObject to avoid being
 * tempted to dereference)
 */
void DBusMenuExporterPrivate::removeActionInternal(QObject *object)
{
    QAction *action = static_cast<QAction *>(object);
    m_actionProperties.remove(action);
    int id = m_idForAction.take(action);
    m_actionForId.remove(id);
}

void DBusMenuExporterPrivate::removeAction(QAction *action, int parentId)
{
    removeActionInternal(action);
    QObject::disconnect(action, SIGNAL(destroyed(QObject *)), q, SLOT(slotActionDestroyed(QObject *)));
    ++m_revision;
    emitLayoutUpdated(parentId);
}

void DBusMenuExporterPrivate::emitLayoutUpdated(int id)
{
    if (m_layoutUpdatedIds.contains(id)) {
        return;
    }
    m_layoutUpdatedIds << id;
    m_layoutUpdatedTimer->start();
}

void DBusMenuExporterPrivate::insertIconProperty(QVariantMap *map, QAction *action) const
{
    // provide the icon name for per-theme lookups
    const QString iconName = q->iconNameForAction(action);
    if (!iconName.isEmpty()) {
        map->insert(QStringLiteral("icon-name"), iconName);
    }

    // provide the serialized icon data in case the icon
    // is unnamed or the name isn't supported by the theme
    const QIcon icon = action->icon();
    if (!icon.isNull()) {
        QBuffer buffer;
        icon.pixmap(16).save(&buffer, "PNG");
        map->insert(QStringLiteral("icon-data"), buffer.data());
    }
}

static void collapseSeparator(QAction *action)
{
    action->setVisible(false);
}

// Unless the separatorsCollapsible property is set to false, Qt will get rid
// of separators at the beginning and at the end of menus as well as collapse
// multiple separators in the middle. For example, a menu like this:
//
// ---
// Open
// ---
// ---
// Quit
// ---
//
// is displayed like this:
//
// Open
// ---
// Quit
//
// We fake this by setting separators invisible before exporting them.
//
// cf. https://bugs.launchpad.net/libdbusmenu-qt/+bug/793339
void DBusMenuExporterPrivate::collapseSeparators(QMenu *menu)
{
    QList<QAction *> actions = menu->actions();
    if (actions.isEmpty()) {
        return;
    }

    QList<QAction *>::Iterator it, begin = actions.begin(), end = actions.end();

    // Get rid of separators at end
    it = end - 1;
    for (; it != begin; --it) {
        if ((*it)->isSeparator()) {
            collapseSeparator(*it);
        } else {
            break;
        }
    }
    // end now points after the last visible entry
    end = it + 1;
    it = begin;

    // Get rid of separators at beginnning
    for (; it != end; ++it) {
        if ((*it)->isSeparator()) {
            collapseSeparator(*it);
        } else {
            break;
        }
    }

    // Collapse separators in between
    bool previousWasSeparator = false;
    for (; it != end; ++it) {
        QAction *action = *it;
        if (action->isSeparator()) {
            if (previousWasSeparator) {
                collapseSeparator(action);
            } else {
                previousWasSeparator = true;
            }
        } else {
            previousWasSeparator = false;
        }
    }
}

//-------------------------------------------------
//
// DBusMenuExporter
//
//-------------------------------------------------
DBusMenuExporter::DBusMenuExporter(const QString &objectPath, QMenu *menu, const QDBusConnection &_connection)
    : QObject(menu)
    , d(new DBusMenuExporterPrivate)
{
    d->q = this;
    d->m_objectPath = objectPath;
    d->m_rootMenu = menu;
    d->m_nextId = 1;
    d->m_revision = 1;
    d->m_emittedLayoutUpdatedOnce = false;
    d->m_itemUpdatedTimer = new QTimer(this);
    d->m_layoutUpdatedTimer = new QTimer(this);
    d->m_dbusObject = new DBusMenuExporterDBus(this);

    d->addMenu(d->m_rootMenu, 0);

    d->m_itemUpdatedTimer->setInterval(0);
    d->m_itemUpdatedTimer->setSingleShot(true);
    connect(d->m_itemUpdatedTimer, SIGNAL(timeout()), SLOT(doUpdateActions()));

    d->m_layoutUpdatedTimer->setInterval(0);
    d->m_layoutUpdatedTimer->setSingleShot(true);
    connect(d->m_layoutUpdatedTimer, SIGNAL(timeout()), SLOT(doEmitLayoutUpdated()));

    QDBusConnection connection(_connection);
    connection.registerObject(objectPath, d->m_dbusObject, QDBusConnection::ExportAllContents);
}

DBusMenuExporter::~DBusMenuExporter()
{
    delete d;
}

void DBusMenuExporter::doUpdateActions()
{
    if (d->m_itemUpdatedIds.isEmpty()) {
        return;
    }
    DBusMenuItemList updatedList;
    DBusMenuItemKeysList removedList;

    for (int id : d->m_itemUpdatedIds) {
        QAction *action = d->m_actionForId.value(id);
        if (!action) {
            // Action does not exist anymore
            continue;
        }

        QVariantMap &oldProperties = d->m_actionProperties[action];
        QVariantMap newProperties = d->propertiesForAction(action);
        QVariantMap updatedProperties;
        QStringList removedProperties;

        // Find updated and removed properties
        QVariantMap::ConstIterator newEnd = newProperties.constEnd();

        QVariantMap::ConstIterator oldIt = oldProperties.constBegin(), oldEnd = oldProperties.constEnd();
        for (; oldIt != oldEnd; ++oldIt) {
            QString key = oldIt.key();
            QVariantMap::ConstIterator newIt = newProperties.constFind(key);
            if (newIt != newEnd) {
                if (newIt.value() != oldIt.value()) {
                    updatedProperties.insert(key, newIt.value());
                }
            } else {
                removedProperties << key;
            }
        }

        // Find new properties (treat them as updated properties)
        QVariantMap::ConstIterator newIt = newProperties.constBegin();
        for (; newIt != newEnd; ++newIt) {
            QString key = newIt.key();
            oldIt = oldProperties.constFind(key);
            if (oldIt == oldEnd) {
                updatedProperties.insert(key, newIt.value());
            }
        }

        // Update our data (oldProperties is a reference)
        oldProperties = newProperties;
        QMenu *menu = action->menu();
        if (menu) {
            d->addMenu(menu, id);
        }

        if (!updatedProperties.isEmpty()) {
            DBusMenuItem item;
            item.id = id;
            item.properties = updatedProperties;
            updatedList << item;
        }
        if (!removedProperties.isEmpty()) {
            DBusMenuItemKeys itemKeys;
            itemKeys.id = id;
            itemKeys.properties = removedProperties;
            removedList << itemKeys;
        }
    }
    d->m_itemUpdatedIds.clear();
    if (!d->m_emittedLayoutUpdatedOnce) {
        // No need to tell the world about action changes: nobody knows the
        // menu layout so nobody knows about the actions.
        // Note: We can't stop in DBusMenuExporterPrivate::addAction(), we
        // still need to reach this method because we want our properties to be
        // updated, even if we don't announce changes.
        return;
    }
    if (!updatedList.isEmpty() || !removedList.isEmpty()) {
        d->m_dbusObject->ItemsPropertiesUpdated(updatedList, removedList);
    }
}

void DBusMenuExporter::doEmitLayoutUpdated()
{
    // Collapse separators for all updated menus
    for (int id : d->m_layoutUpdatedIds) {
        QMenu *menu = d->menuForId(id);
        if (menu && menu->separatorsCollapsible()) {
            d->collapseSeparators(menu);
        }
    }

    // Tell the world about the update
    if (d->m_emittedLayoutUpdatedOnce) {
        for (int id : std::as_const(d->m_layoutUpdatedIds)) {
            d->m_dbusObject->LayoutUpdated(d->m_revision, id);
        }
    } else {
        // First time we emit LayoutUpdated, no need to emit several layout
        // updates, signals the whole layout (id==0) has been updated
        d->m_dbusObject->LayoutUpdated(d->m_revision, 0);
        d->m_emittedLayoutUpdatedOnce = true;
    }
    d->m_layoutUpdatedIds.clear();
}

QString DBusMenuExporter::iconNameForAction(QAction *action)
{
    DMRETURN_VALUE_IF_FAIL(action, QString());
    QIcon icon = action->icon();
    if (action->isIconVisibleInMenu() && !icon.isNull()) {
        return icon.name();
    } else {
        return QString();
    }
}

void DBusMenuExporter::activateAction(QAction *action)
{
    int id = d->idForAction(action);
    DMRETURN_IF_FAIL(id >= 0);
    const uint timeStamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
    d->m_dbusObject->ItemActivationRequested(id, timeStamp);
}

void DBusMenuExporter::slotActionDestroyed(QObject *object)
{
    d->removeActionInternal(object);
}

void DBusMenuExporter::setStatus(const QString &status)
{
    d->m_dbusObject->setStatus(status);
}

QString DBusMenuExporter::status() const
{
    return d->m_dbusObject->status();
}

#include "moc_dbusmenuexporter.cpp"

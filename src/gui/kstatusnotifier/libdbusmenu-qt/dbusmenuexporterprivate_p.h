/* This file is part of the dbusmenu-qt library
   SPDX-FileCopyrightText: 2010 Canonical
   Author: Aurelien Gateau <aurelien.gateau@canonical.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef DBUSMENUEXPORTERPRIVATE_P_H
#define DBUSMENUEXPORTERPRIVATE_P_H

// Local
#include "dbusmenuexporter.h"
#include "dbusmenutypes_p.h"

// Qt
#include <QHash>
#include <QMap>
#include <QSet>
#include <QVariant>

class QMenu;
class QTimer;

class DBusMenuExporterDBus;

class DBusMenuExporterPrivate
{
public:
    DBusMenuExporter *q = nullptr;

    QString m_objectPath;

    DBusMenuExporterDBus *m_dbusObject = nullptr;

    QMenu *m_rootMenu = nullptr;
    QHash<QAction *, QVariantMap> m_actionProperties;
    QMap<int, QAction *> m_actionForId;
    QMap<QAction *, int> m_idForAction;
    int m_nextId;
    uint m_revision;
    bool m_emittedLayoutUpdatedOnce;

    QSet<int> m_itemUpdatedIds;
    QTimer *m_itemUpdatedTimer = nullptr;

    QSet<int> m_layoutUpdatedIds;
    QTimer *m_layoutUpdatedTimer = nullptr;

    int idForAction(QAction *action) const;
    void addMenu(QMenu *menu, int parentId);
    QVariantMap propertiesForAction(QAction *action) const;
    QVariantMap propertiesForKMenuTitleAction(QAction *action_) const;
    QVariantMap propertiesForSeparatorAction(QAction *action) const;
    QVariantMap propertiesForStandardAction(QAction *action) const;
    QMenu *menuForId(int id) const;
    void fillLayoutItem(DBusMenuLayoutItem *item, QMenu *menu, int id, int depth, const QStringList &propertyNames);

    void addAction(QAction *action, int parentId);
    void updateAction(QAction *action);
    void removeAction(QAction *action, int parentId);
    /**
     * Removes any reference from action in the exporter, but do not notify the
     * change outside. This is useful when a submenu is destroyed because we do
     * not receive QEvent::ActionRemoved events for its actions.
     * IMPORTANT: action might have already been destroyed when this method is
     * called, so don't dereference the pointer (it is a QObject to avoid being
     * tempted to dereference)
     */
    void removeActionInternal(QObject *action);

    void emitLayoutUpdated(int id);

    void insertIconProperty(QVariantMap *map, QAction *action) const;

    void collapseSeparators(QMenu *);
};

#endif /* DBUSMENUEXPORTERPRIVATE_P_H */

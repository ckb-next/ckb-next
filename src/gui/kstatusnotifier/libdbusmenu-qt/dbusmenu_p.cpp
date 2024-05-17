/* This file is part of the dbusmenu-qt library
   SPDX-FileCopyrightText: 2009 Canonical
   Author: Aurelien Gateau <aurelien.gateau@canonical.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "dbusmenu_p.h"

// Qt
#include <QAction>
#include <QActionEvent>
#include <QMenu>

// Local
#include "dbusmenuexporter.h"
#include "dbusmenuexporterprivate_p.h"
#include "debug_p.h"

DBusMenu::DBusMenu(QMenu *menu, DBusMenuExporter *exporter, int parentId)
    : QObject(menu)
    , m_exporter(exporter)
    , m_parentId(parentId)
{
    menu->installEventFilter(this);
    connect(m_exporter, SIGNAL(destroyed(QObject *)), SLOT(deleteMe()));
}

DBusMenu::~DBusMenu()
{
}

bool DBusMenu::eventFilter(QObject *, QEvent *event)
{
    QActionEvent *actionEvent = nullptr;
    switch (event->type()) {
    case QEvent::ActionAdded:
    case QEvent::ActionChanged:
    case QEvent::ActionRemoved:
        actionEvent = static_cast<QActionEvent *>(event);
        break;
    default:
        return false;
    }
    switch (event->type()) {
    case QEvent::ActionAdded:
        addAction(actionEvent->action());
        break;
    case QEvent::ActionChanged:
        updateAction(actionEvent->action());
        break;
    case QEvent::ActionRemoved:
        removeAction(actionEvent->action());
        break;
    default:
        break;
    }
    return false;
}

void DBusMenu::addAction(QAction *action)
{
    m_exporter->d->addAction(action, m_parentId);
}

void DBusMenu::updateAction(QAction *action)
{
    m_exporter->d->updateAction(action);
}

void DBusMenu::removeAction(QAction *action)
{
    m_exporter->d->removeAction(action, m_parentId);
}

void DBusMenu::deleteMe()
{
    delete this;
}

#include "moc_dbusmenu_p.cpp"

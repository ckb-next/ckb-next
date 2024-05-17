/* This file is part of the dbusmenu-qt library
   SPDX-FileCopyrightText: 2009 Canonical
   Author: Aurelien Gateau <aurelien.gateau@canonical.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef DBUSMENU_H
#define DBUSMENU_H

#include <QEvent>
#include <QObject>

class QAction;
class QMenu;

class DBusMenuExporter;

/**
 * Internal class responsible for tracking changes in a menu and reporting them
 * through DBusMenuExporter
 * @internal
 */
class DBusMenu : public QObject
{
    Q_OBJECT
public:
    DBusMenu(QMenu *menu, DBusMenuExporter *exporter, int parentId);
    ~DBusMenu() override;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private Q_SLOTS:
    void deleteMe();

private:
    void addAction(QAction *action);
    void updateAction(QAction *action);
    void removeAction(QAction *action);

    DBusMenuExporter *const m_exporter;
    const int m_parentId;
};

#endif /* DBUSMENU_H */

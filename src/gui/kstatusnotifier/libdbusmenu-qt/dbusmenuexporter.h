/* This file is part of the dbusmenu-qt library
   SPDX-FileCopyrightText: 2009 Canonical
   Author: Aurelien Gateau <aurelien.gateau@canonical.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef DBUSMENUEXPORTER_H
#define DBUSMENUEXPORTER_H

// Qt
#include <QDBusConnection>
#include <QObject>

class QAction;
class QMenu;

class DBusMenuExporterPrivate;

/**
 * A DBusMenuExporter instance can serialize a menu over DBus
 */
class DBusMenuExporter : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a DBusMenuExporter exporting menu at the dbus object path
     * dbusObjectPath, using the given dbusConnection.
     * The instance adds itself to the menu children.
     */
    DBusMenuExporter(const QString &dbusObjectPath, QMenu *menu, const QDBusConnection &dbusConnection = QDBusConnection::sessionBus());

    ~DBusMenuExporter() override;

    /**
     * Asks the matching DBusMenuImporter to activate @p action. For menus it
     * means popup them, for items it means triggering the associated action.
     */
    void activateAction(QAction *action);

    /**
     * The status of the menu. Can be one of "normal" or "notice". This can be
     * used to notify the other side the menu should be made more visible.
     * For example, appmenu uses it to tell Unity panel to show/hide the menubar
     * when the Alt modifier is pressed/released.
     */
    void setStatus(const QString &status);

    /**
     * Returns the status of the menu.
     * @ref setStatus
     */
    QString status() const;

protected:
    /**
     * Must extract the icon name for action. This is the name which will
     * be used to present the icon over DBus.
     * Default implementation returns action->icon().name() when built on Qt
     * >= 4.7 and a null string otherwise.
     */
    virtual QString iconNameForAction(QAction *action);

private Q_SLOTS:
    void doUpdateActions();
    void doEmitLayoutUpdated();
    void slotActionDestroyed(QObject *);

private:
    Q_DISABLE_COPY(DBusMenuExporter)
    DBusMenuExporterPrivate *const d;

    friend class DBusMenuExporterPrivate;
    friend class DBusMenuExporterDBus;
    friend class DBusMenu;
};

#endif /* DBUSMENUEXPORTER_H */

/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2009 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSTATUSNOTIFIERITEMDBUS_H
#define KSTATUSNOTIFIERITEMDBUS_H

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QList>
#include <QObject>
#include <QString>

// Custom message type for DBus
struct KDbusImageStruct {
    int width;
    int height;
    QByteArray data;
};

typedef QList<KDbusImageStruct> KDbusImageVector;

struct KDbusToolTipStruct {
    QString icon;
    KDbusImageVector image;
    QString title;
    QString subTitle;
};

class KStatusNotifierItem;

class KStatusNotifierItemDBus : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString Category READ Category)
    Q_PROPERTY(QString Id READ Id)
    Q_PROPERTY(QString Title READ Title)
    Q_PROPERTY(QString Status READ Status)
    Q_PROPERTY(int WindowId READ WindowId)
    Q_PROPERTY(bool ItemIsMenu READ ItemIsMenu)
    Q_PROPERTY(QString IconName READ IconName)
    Q_PROPERTY(KDbusImageVector IconPixmap READ IconPixmap)
    Q_PROPERTY(QString OverlayIconName READ OverlayIconName)
    Q_PROPERTY(KDbusImageVector OverlayIconPixmap READ OverlayIconPixmap)
    Q_PROPERTY(QString AttentionIconName READ AttentionIconName)
    Q_PROPERTY(KDbusImageVector AttentionIconPixmap READ AttentionIconPixmap)
    Q_PROPERTY(QString AttentionMovieName READ AttentionMovieName)
    Q_PROPERTY(KDbusToolTipStruct ToolTip READ ToolTip)
    Q_PROPERTY(QString IconThemePath READ IconThemePath)
    Q_PROPERTY(QDBusObjectPath Menu READ Menu)

    friend class KStatusNotifierItem;

public:
    explicit KStatusNotifierItemDBus(KStatusNotifierItem *parent);
    ~KStatusNotifierItemDBus() override;

    /**
     * @return the dbus connection used by this object
     */
    QDBusConnection dbusConnection() const;

    /**
     * @return the service this object is registered on the bus under
     */
    QString service() const;

    /**
     * @return the category of the application associated to this item
     * @see Category
     */
    QString Category() const;

    /**
     * @return the id of this item
     */
    QString Id() const;

    /**
     * @return the title of this item
     */
    QString Title() const;

    /**
     * @return The status of this item
     * @see Status
     */
    QString Status() const;

    /**
     * @return The id of the main window of the application that controls the item
     */
    int WindowId() const;

    /**
     * @return The item only support the context menu, the visualization should prefer sending ContextMenu() instead of Activate()
     */
    bool ItemIsMenu() const;

    /**
     * @return the name of the main icon to be displayed
     * if image() is not empty this will always return an empty string
     */
    QString IconName() const;

    /**
     * @return a serialization of the icon data
     */
    KDbusImageVector IconPixmap() const;

    /**
     * @return the name of the overlay of the main icon to be displayed
     * if image() is not empty this will always return an empty string
     */
    QString OverlayIconName() const;

    /**
     * @return a serialization of the icon data
     */
    KDbusImageVector OverlayIconPixmap() const;

    /**
     * @return the name of the icon to be displayed when the application
     * is requesting the user's attention
     * if attentionImage() is not empty this will always return an empty string
     */
    QString AttentionIconName() const;

    /**
     * @return a serialization of the requesting attention icon data
     */
    KDbusImageVector AttentionIconPixmap() const;

    /**
     * @return the name of the attention movie
     */
    QString AttentionMovieName() const;

    /**
     * all the data needed for a tooltip
     */
    KDbusToolTipStruct ToolTip() const;

    /**
     * @return path to extra icon theme, to load application specific icons
     */
    QString IconThemePath() const;

    /**
     * @return object path to the dbusmenu object
     */
    QDBusObjectPath Menu() const;

public Q_SLOTS:
    // interaction
    /**
     * Shows the context menu associated to this item
     * at the desired screen position
     */
    void ContextMenu(int x, int y);

    /**
     * Shows the main widget and try to position it on top
     * of the other windows, if the widget is already visible, hide it.
     */
    void Activate(int x, int y);

    /**
     * The user activated the item in an alternate way (for instance with middle mouse button, this depends from the systray implementation)
     */
    void SecondaryActivate(int x, int y);

    /**
     * Inform this item that the mouse wheel was used on its representation
     */
    void Scroll(int delta, const QString &orientation);

    /**
     * Provide a @p token for xdg_activation_v1
     *
     * So that the Wayland compositor knows who is requesting an activation.
     */
    //void ProvideXdgActivationToken(const QString &token);

Q_SIGNALS:
    /**
     * Inform the systemtray that the own main icon has been changed,
     * so should be reloaded
     */
    void NewIcon();

    /**
     * Inform the systemtray that there is a new icon to be used as overlay
     */
    void NewOverlayIcon();

    /**
     * Inform the systemtray that the requesting attention icon
     * has been changed, so should be reloaded
     */
    void NewAttentionIcon();

    /**
     * Inform the systemtray that a new context menu has been set.
     */
    void NewMenu();

    /**
     * Inform the systemtray that something in the tooltip has been changed
     */
    void NewToolTip();

    /**
     * Signal the new status when it has been changed
     * @see Status
     */
    void NewStatus(const QString &status);

private:
    KStatusNotifierItem *m_statusNotifierItem;
    QString m_connId;
    //QString m_xdgActivationToken;
    QDBusConnection m_dbus;
    static int s_serviceCount;
};

const QDBusArgument &operator<<(QDBusArgument &argument, const KDbusImageStruct &icon);
const QDBusArgument &operator>>(const QDBusArgument &argument, KDbusImageStruct &icon);

Q_DECLARE_METATYPE(KDbusImageStruct)

const QDBusArgument &operator<<(QDBusArgument &argument, const KDbusImageVector &iconVector);
const QDBusArgument &operator>>(const QDBusArgument &argument, KDbusImageVector &iconVector);

Q_DECLARE_METATYPE(KDbusImageVector)

const QDBusArgument &operator<<(QDBusArgument &argument, const KDbusToolTipStruct &toolTip);
const QDBusArgument &operator>>(const QDBusArgument &argument, KDbusToolTipStruct &toolTip);

Q_DECLARE_METATYPE(KDbusToolTipStruct)

#endif

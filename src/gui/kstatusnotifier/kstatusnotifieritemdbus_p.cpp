/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2009 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kstatusnotifieritemdbus_p.h"
#include "debug_p.h"
#include "kstatusnotifieritem.h"
#include "kstatusnotifieritemprivate_p.h"

#include <QMenu>

//#include <kwindowsystem.h>

#include "statusnotifierwatcher_interface.h"

#include "statusnotifieritemadaptor.h"

#ifdef Q_OS_WIN64
__inline int toInt(WId wid)
{
    return (int)((__int64)wid);
}

#else
__inline int toInt(WId wid)
{
    return (int)wid;
}
#endif

// Marshall the ImageStruct data into a D-BUS argument
const QDBusArgument &operator<<(QDBusArgument &argument, const KDbusImageStruct &icon)
{
    argument.beginStructure();
    argument << icon.width;
    argument << icon.height;
    argument << icon.data;
    argument.endStructure();
    return argument;
}

// Retrieve the ImageStruct data from the D-BUS argument
const QDBusArgument &operator>>(const QDBusArgument &argument, KDbusImageStruct &icon)
{
    qint32 width;
    qint32 height;
    QByteArray data;

    argument.beginStructure();
    argument >> width;
    argument >> height;
    argument >> data;
    argument.endStructure();

    icon.width = width;
    icon.height = height;
    icon.data = data;

    return argument;
}

// Marshall the ImageVector data into a D-BUS argument
const QDBusArgument &operator<<(QDBusArgument &argument, const KDbusImageVector &iconVector)
{
    argument.beginArray(qMetaTypeId<KDbusImageStruct>());
    for (int i = 0; i < iconVector.size(); ++i) {
        argument << iconVector[i];
    }
    argument.endArray();
    return argument;
}

// Retrieve the ImageVector data from the D-BUS argument
const QDBusArgument &operator>>(const QDBusArgument &argument, KDbusImageVector &iconVector)
{
    argument.beginArray();
    iconVector.clear();

    while (!argument.atEnd()) {
        KDbusImageStruct element;
        argument >> element;
        iconVector.append(element);
    }

    argument.endArray();

    return argument;
}

// Marshall the ToolTipStruct data into a D-BUS argument
const QDBusArgument &operator<<(QDBusArgument &argument, const KDbusToolTipStruct &toolTip)
{
    argument.beginStructure();
    argument << toolTip.icon;
    argument << toolTip.image;
    argument << toolTip.title;
    argument << toolTip.subTitle;
    argument.endStructure();
    return argument;
}

// Retrieve the ToolTipStruct data from the D-BUS argument
const QDBusArgument &operator>>(const QDBusArgument &argument, KDbusToolTipStruct &toolTip)
{
    QString icon;
    KDbusImageVector image;
    QString title;
    QString subTitle;

    argument.beginStructure();
    argument >> icon;
    argument >> image;
    argument >> title;
    argument >> subTitle;
    argument.endStructure();

    toolTip.icon = icon;
    toolTip.image = image;
    toolTip.title = title;
    toolTip.subTitle = subTitle;

    return argument;
}

int KStatusNotifierItemDBus::s_serviceCount = 0;

KStatusNotifierItemDBus::KStatusNotifierItemDBus(KStatusNotifierItem *parent)
    : QObject(parent)
    , m_statusNotifierItem(parent)
    , m_connId(QStringLiteral("org.kde.StatusNotifierItem-%1-%2").arg(QCoreApplication::applicationPid()).arg(++s_serviceCount))
    , m_dbus(QDBusConnection(m_connId))
{
    m_dbus = QDBusConnection::connectToBus(QDBusConnection::SessionBus, m_connId);

    new StatusNotifierItemAdaptor(this);
    qCDebug(LOG_KSTATUSNOTIFIERITEM) << "service is" << m_connId;
    m_dbus.registerObject(QStringLiteral("/StatusNotifierItem"), this);
}

KStatusNotifierItemDBus::~KStatusNotifierItemDBus()
{
    m_dbus.unregisterObject(QStringLiteral("/StatusNotifierItem"));
    m_dbus.disconnectFromBus(m_connId);
}

QDBusConnection KStatusNotifierItemDBus::dbusConnection() const
{
    return m_dbus;
}

QString KStatusNotifierItemDBus::service() const
{
    return m_dbus.baseService();
}

bool KStatusNotifierItemDBus::ItemIsMenu() const
{
    return (m_statusNotifierItem->d->associatedWidget == m_statusNotifierItem->d->menu);
}

// DBUS slots

QString KStatusNotifierItemDBus::Category() const
{
    return QLatin1String(m_statusNotifierItem->metaObject()
                             ->enumerator(m_statusNotifierItem->metaObject()->indexOfEnumerator("ItemCategory"))
                             .valueToKey(m_statusNotifierItem->category()));
}

QString KStatusNotifierItemDBus::Title() const
{
    return m_statusNotifierItem->title();
}

QString KStatusNotifierItemDBus::Id() const
{
    return m_statusNotifierItem->id();
}

QString KStatusNotifierItemDBus::Status() const
{
    return QLatin1String(m_statusNotifierItem->metaObject()
                             ->enumerator(m_statusNotifierItem->metaObject()->indexOfEnumerator("ItemStatus"))
                             .valueToKey(m_statusNotifierItem->status()));
}

int KStatusNotifierItemDBus::WindowId() const
{
    if (m_statusNotifierItem->d->associatedWidget && m_statusNotifierItem->d->associatedWidget != m_statusNotifierItem->d->menu) {
        return toInt(m_statusNotifierItem->d->associatedWidget->winId());
    } else {
        return 0;
    }
}

// Icon

QString KStatusNotifierItemDBus::IconName() const
{
    return m_statusNotifierItem->iconName();
}

KDbusImageVector KStatusNotifierItemDBus::IconPixmap() const
{
    return m_statusNotifierItem->d->serializedIcon;
}

QString KStatusNotifierItemDBus::OverlayIconName() const
{
    return m_statusNotifierItem->overlayIconName();
}

KDbusImageVector KStatusNotifierItemDBus::OverlayIconPixmap() const
{
    return m_statusNotifierItem->d->serializedOverlayIcon;
}

// Requesting attention icon and movie

QString KStatusNotifierItemDBus::AttentionIconName() const
{
    return m_statusNotifierItem->attentionIconName();
}

KDbusImageVector KStatusNotifierItemDBus::AttentionIconPixmap() const
{
    return m_statusNotifierItem->d->serializedAttentionIcon;
}

QString KStatusNotifierItemDBus::AttentionMovieName() const
{
    return m_statusNotifierItem->d->movieName;
}

// ToolTip

KDbusToolTipStruct KStatusNotifierItemDBus::ToolTip() const
{
    KDbusToolTipStruct toolTip;
    toolTip.icon = m_statusNotifierItem->toolTipIconName();
    toolTip.image = m_statusNotifierItem->d->serializedToolTipIcon;
    toolTip.title = m_statusNotifierItem->toolTipTitle();
    toolTip.subTitle = m_statusNotifierItem->toolTipSubTitle();

    return toolTip;
}

QString KStatusNotifierItemDBus::IconThemePath() const
{
    return m_statusNotifierItem->d->iconThemePath;
}

// Menu
QDBusObjectPath KStatusNotifierItemDBus::Menu() const
{
    return QDBusObjectPath(m_statusNotifierItem->d->menuObjectPath);
}

// Interaction

void KStatusNotifierItemDBus::ContextMenu(int x, int y)
{
    if (!m_statusNotifierItem->d->menu) {
        return;
    }

    // TODO: nicer placement, possible?
    if (!m_statusNotifierItem->d->menu->isVisible()) {
        m_statusNotifierItem->d->menu->popup(QPoint(x, y));
    } else {
        m_statusNotifierItem->d->menu->hide();
    }
}

void KStatusNotifierItemDBus::Activate(int x, int y)
{
    m_statusNotifierItem->activate(QPoint(x, y));
}

void KStatusNotifierItemDBus::SecondaryActivate(int x, int y)
{
    Q_EMIT m_statusNotifierItem->secondaryActivateRequested(QPoint(x, y));
}

void KStatusNotifierItemDBus::Scroll(int delta, const QString &orientation)
{
    Qt::Orientation dir = (orientation.toLower() == QLatin1String("horizontal") ? Qt::Horizontal : Qt::Vertical);
    Q_EMIT m_statusNotifierItem->scrollRequested(delta, dir);
}

/*void KStatusNotifierItemDBus::ProvideXdgActivationToken(const QString &token)
{
    m_xdgActivationToken = token;
    KWindowSystem::setCurrentXdgActivationToken(token);
}*/

#include "moc_kstatusnotifieritemdbus_p.cpp"

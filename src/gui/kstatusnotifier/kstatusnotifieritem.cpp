/* This file is part of the KDE libraries
   Copyright 2009 by Marco Martin <notmart@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later
   version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kstatusnotifieritem.h"
#include "kstatusnotifieritemprivate_p.h"
#include "kstatusnotifieritemdbus_p.h"

#include <QDBusConnection>
#include <QMessageBox>
#include <QPixmap>
#include <QImage>
#include <QApplication>
#include <QMenu>
#include <QMovie>
#include <QPainter>
#include <qstandardpaths.h>

#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
#define QIMAGE_SIZE(x)      (x.byteCount())
#else
#define QIMAGE_SIZE(x)      (x.sizeInBytes())
#endif

static const char s_statusNotifierWatcherServiceName[] = "org.kde.StatusNotifierWatcher";
static const int s_legacyTrayIconSize = 24;

#include <dbusmenuexporter.h>

KStatusNotifierItem::KStatusNotifierItem(QObject *parent)
    : QObject(parent),
      d(new KStatusNotifierItemPrivate(this))
{
    d->init(QString());
}

KStatusNotifierItem::KStatusNotifierItem(const QString &id, QObject *parent)
    : QObject(parent),
      d(new KStatusNotifierItemPrivate(this))
{
    d->init(id);
}

KStatusNotifierItem::~KStatusNotifierItem()
{
    delete d->statusNotifierWatcher;
    delete d->notificationsClient;
    delete d->systemTrayIcon;
    if (!qApp->closingDown()) {
        delete d->menu;
    }
    delete d;
}

QString KStatusNotifierItem::id() const
{
    //qDebug() << "id requested" << d->id;
    return d->id;
}

void KStatusNotifierItem::setCategory(const ItemCategory category)
{
    d->category = category;
}

KStatusNotifierItem::ItemStatus KStatusNotifierItem::status() const
{
    return d->status;
}

KStatusNotifierItem::ItemCategory KStatusNotifierItem::category() const
{
    return d->category;
}

void KStatusNotifierItem::setTitle(const QString &title)
{
    d->title = title;
}

void KStatusNotifierItem::setStatus(const ItemStatus status)
{
    if (d->status == status) {
        return;
    }

    d->status = status;
    emit d->statusNotifierItemDBus->NewStatus(QString::fromLatin1(metaObject()->enumerator(metaObject()->indexOfEnumerator("ItemStatus")).valueToKey(d->status)));

    if (d->systemTrayIcon) {
        d->syncLegacySystemTrayIcon();
    }
}

//normal icon

void KStatusNotifierItem::setIconByName(const QString &name)
{
    if (d->iconName == name) {
        return;
    }

    d->serializedIcon = KDbusImageVector();
    d->iconName = name;
    emit d->statusNotifierItemDBus->NewIcon();
    if (d->systemTrayIcon) {
        // It's possible to pass paths as well
        QIcon trayIcon =  QIcon::fromTheme(name);
        if(trayIcon.isNull())
            trayIcon = QIcon(name);
        d->systemTrayIcon->setIcon(trayIcon);
    }
}

QString KStatusNotifierItem::iconName() const
{
    return d->iconName;
}

void KStatusNotifierItem::setIconByPixmap(const QIcon &icon)
{
    if (d->iconName.isEmpty() && d->icon.cacheKey() == icon.cacheKey()) {
        return;
    }

    d->iconName.clear();
    d->serializedIcon = d->iconToVector(icon);
    emit d->statusNotifierItemDBus->NewIcon();

    d->icon = icon;
    if (d->systemTrayIcon) {
        d->systemTrayIcon->setIcon(icon);
    }
}

QIcon KStatusNotifierItem::iconPixmap() const
{
    return d->icon;
}

void KStatusNotifierItem::setOverlayIconByName(const QString &name)
{
    if (d->overlayIconName == name) {
        return;
    }

    d->overlayIconName = name;
    emit d->statusNotifierItemDBus->NewOverlayIcon();
    if (d->systemTrayIcon) {
        QPixmap iconPixmap = QIcon::fromTheme(d->iconName).pixmap(s_legacyTrayIconSize, s_legacyTrayIconSize);
        if (!name.isEmpty()) {
            QPixmap overlayPixmap = QIcon::fromTheme(d->overlayIconName).pixmap(s_legacyTrayIconSize / 2, s_legacyTrayIconSize / 2);
            QPainter p(&iconPixmap);
            p.drawPixmap(iconPixmap.width() - overlayPixmap.width(), iconPixmap.height() - overlayPixmap.height(), overlayPixmap);
            p.end();
        }
        d->systemTrayIcon->setIcon(iconPixmap);
    }
}

QString KStatusNotifierItem::overlayIconName() const
{
    return d->overlayIconName;
}

void KStatusNotifierItem::setOverlayIconByPixmap(const QIcon &icon)
{
    if (d->overlayIconName.isEmpty() && d->overlayIcon.cacheKey() == icon.cacheKey()) {
        return;
    }

    d->overlayIconName.clear();
    d->serializedOverlayIcon = d->iconToVector(icon);
    emit d->statusNotifierItemDBus->NewOverlayIcon();

    d->overlayIcon = icon;
    if (d->systemTrayIcon) {
        QPixmap iconPixmap = d->icon.pixmap(s_legacyTrayIconSize, s_legacyTrayIconSize);
        QPixmap overlayPixmap = d->overlayIcon.pixmap(s_legacyTrayIconSize / 2, s_legacyTrayIconSize / 2);

        QPainter p(&iconPixmap);
        p.drawPixmap(iconPixmap.width() - overlayPixmap.width(), iconPixmap.height() - overlayPixmap.height(), overlayPixmap);
        p.end();
        d->systemTrayIcon->setIcon(iconPixmap);
    }
}

QIcon KStatusNotifierItem::overlayIconPixmap() const
{
    return d->overlayIcon;
}

//Icons and movie for requesting attention state

void KStatusNotifierItem::setAttentionIconByName(const QString &name)
{
    if (d->attentionIconName == name) {
        return;
    }

    d->serializedAttentionIcon = KDbusImageVector();
    d->attentionIconName = name;
    emit d->statusNotifierItemDBus->NewAttentionIcon();
}

QString KStatusNotifierItem::attentionIconName() const
{
    return d->attentionIconName;
}

void KStatusNotifierItem::setAttentionIconByPixmap(const QIcon &icon)
{
    if (d->attentionIconName.isEmpty() && d->attentionIcon.cacheKey() == icon.cacheKey()) {
        return;
    }

    d->attentionIconName.clear();
    d->serializedAttentionIcon = d->iconToVector(icon);
    d->attentionIcon = icon;
    emit d->statusNotifierItemDBus->NewAttentionIcon();
}

QIcon KStatusNotifierItem::attentionIconPixmap() const
{
    return d->attentionIcon;
}

void KStatusNotifierItem::setAttentionMovieByName(const QString &name)
{
    if (d->movieName == name) {
        return;
    }

    d->movieName = name;

    delete d->movie;
    d->movie = nullptr;

    emit d->statusNotifierItemDBus->NewAttentionIcon();

    if (d->systemTrayIcon) {
        d->movie = new QMovie(d->movieName);
        d->systemTrayIcon->setMovie(d->movie);
    }
}

QString KStatusNotifierItem::attentionMovieName() const
{
    return d->movieName;
}

//ToolTip
static void setTrayToolTip(KStatusNotifierLegacyIcon *systemTrayIcon, const QString &title, const QString &)
{
    if (systemTrayIcon) {
        systemTrayIcon->setToolTip(title);
    }
}

void KStatusNotifierItem::setToolTip(const QString &iconName, const QString &title, const QString &subTitle)
{
    if (d->toolTipIconName == iconName &&
            d->toolTipTitle == title &&
            d->toolTipSubTitle == subTitle) {
        return;
    }

    d->serializedToolTipIcon = KDbusImageVector();
    d->toolTipIconName = iconName;

    d->toolTipTitle = title;
    setTrayToolTip(d->systemTrayIcon, title, subTitle);
    d->toolTipSubTitle = subTitle;
    emit d->statusNotifierItemDBus->NewToolTip();
}

void KStatusNotifierItem::setToolTip(const QIcon &icon, const QString &title, const QString &subTitle)
{
    if (d->toolTipIconName.isEmpty() && d->toolTipIcon.cacheKey() == icon.cacheKey() &&
            d->toolTipTitle == title &&
            d->toolTipSubTitle == subTitle) {
        return;
    }

    d->toolTipIconName.clear();
    d->serializedToolTipIcon = d->iconToVector(icon);
    d->toolTipIcon = icon;

    d->toolTipTitle = title;
    setTrayToolTip(d->systemTrayIcon, title, subTitle);

    d->toolTipSubTitle = subTitle;
    emit d->statusNotifierItemDBus->NewToolTip();
}

void KStatusNotifierItem::setToolTipIconByName(const QString &name)
{
    if (d->toolTipIconName == name) {
        return;
    }

    d->serializedToolTipIcon = KDbusImageVector();
    d->toolTipIconName = name;
    emit d->statusNotifierItemDBus->NewToolTip();
}

QString KStatusNotifierItem::toolTipIconName() const
{
    return d->toolTipIconName;
}

void KStatusNotifierItem::setToolTipIconByPixmap(const QIcon &icon)
{
    if (d->toolTipIconName.isEmpty() && d->toolTipIcon.cacheKey() == icon.cacheKey()) {
        return;
    }

    d->toolTipIconName.clear();
    d->serializedToolTipIcon = d->iconToVector(icon);
    d->toolTipIcon = icon;
    emit d->statusNotifierItemDBus->NewToolTip();
}

QIcon KStatusNotifierItem::toolTipIconPixmap() const
{
    return d->toolTipIcon;
}

void KStatusNotifierItem::setToolTipTitle(const QString &title)
{
    if (d->toolTipTitle == title) {
        return;
    }

    d->toolTipTitle = title;
    emit d->statusNotifierItemDBus->NewToolTip();
    setTrayToolTip(d->systemTrayIcon, title, d->toolTipSubTitle);
}

QString KStatusNotifierItem::toolTipTitle() const
{
    return d->toolTipTitle;
}

void KStatusNotifierItem::setToolTipSubTitle(const QString &subTitle)
{
    if (d->toolTipSubTitle == subTitle) {
        return;
    }

    d->toolTipSubTitle = subTitle;
    emit d->statusNotifierItemDBus->NewToolTip();
}

QString KStatusNotifierItem::toolTipSubTitle() const
{
    return d->toolTipSubTitle;
}

void KStatusNotifierItem::setContextMenu(QMenu *menu)
{
    if (d->menu && d->menu != menu) {
        d->menu->removeEventFilter(this);
        delete d->menu;
    }

    if (!menu) {
        d->menu = nullptr;
        return;
    }

    if (d->systemTrayIcon) {
        d->systemTrayIcon->setContextMenu(menu);
    } else if (d->menu != menu) {
        d->menuObjectPath = QStringLiteral("/MenuBar");
        new DBusMenuExporter(d->menuObjectPath, menu, d->statusNotifierItemDBus->dbusConnection());

        connect(menu, SIGNAL(aboutToShow()), this, SLOT(contextMenuAboutToShow()));
    }

    d->menu = menu;
    Qt::WindowFlags oldFlags = d->menu->windowFlags();
    d->menu->setParent(nullptr);
    d->menu->setWindowFlags(oldFlags);
}

QMenu *KStatusNotifierItem::contextMenu() const
{
    return d->menu;
}

void KStatusNotifierItem::setAssociatedWidget(QWidget *associatedWidget)
{
    if (associatedWidget) {
        d->associatedWidget = associatedWidget->window();
        d->associatedWidgetPos = QPoint(-1, -1);
    } else if (d->associatedWidget) {
        d->associatedWidget = nullptr;
    }

    if (d->systemTrayIcon) {
        delete d->systemTrayIcon;
        d->systemTrayIcon = nullptr;
        d->setLegacySystemTrayEnabled(true);
    }
}

QWidget *KStatusNotifierItem::associatedWidget() const
{
    return d->associatedWidget;
}

QList<QAction *> KStatusNotifierItem::actionCollection() const
{
    return d->actionCollection.values();
}

void KStatusNotifierItem::addAction(const QString &name, QAction *action)
{
    d->actionCollection.insert(name, action);
}

void KStatusNotifierItem::removeAction(const QString &name)
{
    d->actionCollection.remove(name);
}

QAction* KStatusNotifierItem::action(const QString &name) const
{
    return d->actionCollection.value(name);
}

void KStatusNotifierItem::showMessage(const QString &title, const QString &message, const QString &icon, int timeout)
{
    if (!d->notificationsClient) {
        d->notificationsClient = new org::freedesktop::Notifications(QStringLiteral("org.freedesktop.Notifications"), QStringLiteral("/org/freedesktop/Notifications"),
                QDBusConnection::sessionBus());
    }

    uint id = 0;
    {
        QVariantMap hints;

#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
        QString desktopFileName = QGuiApplication::desktopFileName();
#else
        QString desktopFileName = QFileInfo(QCoreApplication::applicationFilePath()).fileName();
#endif
        if (!desktopFileName.isEmpty()) {
            // handle apps which set the desktopFileName property with filename suffix,
            // due to unclear API dox (https://bugreports.qt.io/browse/QTBUG-75521)
            if (desktopFileName.endsWith(QLatin1String(".desktop"))) {
                desktopFileName.chop(8);
            }
            hints.insert(QStringLiteral("desktop-entry"), desktopFileName);
        }

        d->notificationsClient->Notify(d->title, id, icon, title, message, QStringList(), hints, timeout);
    }
}

QString KStatusNotifierItem::title() const
{
    return d->title;
}

void KStatusNotifierItem::activate(const QPoint &pos)
{
    //if the user activated the icon the NeedsAttention state is no longer necessary
    //FIXME: always true?
    if (d->status == NeedsAttention) {
        d->status = Active;
        emit d->statusNotifierItemDBus->NewStatus(QString::fromLatin1(metaObject()->enumerator(metaObject()->indexOfEnumerator("ItemStatus")).valueToKey(d->status)));
    }

    if (d->associatedWidget && d->associatedWidget == d->menu) {
        d->statusNotifierItemDBus->ContextMenu(pos.x(), pos.y());
        return;
    }

    if (d->menu && d->menu->isVisible()) {
        d->menu->hide();
    }

    if (!d->associatedWidget) {
        emit activateRequested(true, pos);
        return;
    }

    d->checkVisibility(pos);
}

bool KStatusNotifierItemPrivate::checkVisibility(QPoint pos, bool perform)
{
    const bool unmapped = !(associatedWidget->isVisible() && !associatedWidget->isMinimized());
    if (perform) {
        minimizeRestore(unmapped);
        emit q->activateRequested(unmapped, pos);
    }
    return unmapped;
}

bool KStatusNotifierItem::eventFilter(QObject *watched, QEvent *event)
{
    if (d->systemTrayIcon == nullptr) {
        //FIXME: ugly ugly workaround to weird QMenu's focus problems
        if (watched == d->menu &&
                (event->type() == QEvent::WindowDeactivate || (event->type() == QEvent::MouseButtonRelease && static_cast<QMouseEvent *>(event)->button() == Qt::LeftButton))) {
            //put at the back of even queue to let the action activate anyways
            QTimer::singleShot(0, this, [this]() { d->hideMenu(); });
        }
    }
    return false;
}

//KStatusNotifierItemPrivate

const int KStatusNotifierItemPrivate::s_protocolVersion = 0;

KStatusNotifierItemPrivate::KStatusNotifierItemPrivate(KStatusNotifierItem *item)
    : q(item),
      category(KStatusNotifierItem::ApplicationStatus),
      status(KStatusNotifierItem::Passive),
      movie(nullptr),
      menu(nullptr),
      associatedWidget(nullptr),
      titleAction(nullptr),
      statusNotifierWatcher(nullptr),
      notificationsClient(nullptr),
      systemTrayIcon(nullptr)
{
}

void KStatusNotifierItemPrivate::init(const QString &extraId)
{
    qDBusRegisterMetaType<KDbusImageStruct>();
    qDBusRegisterMetaType<KDbusImageVector>();
    qDBusRegisterMetaType<KDbusToolTipStruct>();

    statusNotifierItemDBus = new KStatusNotifierItemDBus(q);
    q->setAssociatedWidget(qobject_cast<QWidget *>(q->parent()));

    QDBusServiceWatcher *watcher = new QDBusServiceWatcher(QString::fromLatin1(s_statusNotifierWatcherServiceName),
            QDBusConnection::sessionBus(),
            QDBusServiceWatcher::WatchForOwnerChange,
            q);
    QObject::connect(watcher, SIGNAL(serviceOwnerChanged(QString,QString,QString)),
                     q, SLOT(serviceChange(QString,QString,QString)));

    //create a default menu, just like in KSystemtrayIcon
    QMenu *m = new QMenu(associatedWidget);

    title = QGuiApplication::applicationDisplayName();
    if (title.isEmpty()) {
        title = QCoreApplication::applicationName();
    }
    titleAction = m->addSection(qApp->windowIcon(), title);
    m->setTitle(title);
    q->setContextMenu(m);

    QAction *action = new QAction(q);
    action->setText(KStatusNotifierItem::tr("Quit"));
    action->setIcon(QIcon::fromTheme(QStringLiteral("application-exit")));
    // cannot yet convert to function-pointer-based connect:
    // some apps like kalarm or korgac have a hack to rewire the connection
    // of the "quit" action to a own slot, and rely on the name-based slot to disconnect
    // TODO: extend KStatusNotifierItem API to support such needs
    QObject::connect(action, SIGNAL(triggered()), q, SLOT(maybeQuit()));
    actionCollection.insert(QStringLiteral("quit"), action);

    id = title;
    if (!extraId.isEmpty()) {
        id.append(QLatin1Char('_')).append(extraId);
    }

    // Init iconThemePath to the app folder for now
    iconThemePath = QStandardPaths::locate(QStandardPaths::DataLocation, QStringLiteral("icons"), QStandardPaths::LocateDirectory);

    registerToDaemon();
}

void KStatusNotifierItemPrivate::registerToDaemon()
{
    //qDebug() << "Registering a client interface to the KStatusNotifierWatcher";
    if (!statusNotifierWatcher) {
        statusNotifierWatcher = new org::kde::StatusNotifierWatcher(QString::fromLatin1(s_statusNotifierWatcherServiceName), QStringLiteral("/StatusNotifierWatcher"),
                QDBusConnection::sessionBus());
    }

    if (statusNotifierWatcher->isValid()) {
        // get protocol version in async way
        QDBusMessage msg = QDBusMessage::createMethodCall(QString::fromLatin1(s_statusNotifierWatcherServiceName),
                                                          QStringLiteral("/StatusNotifierWatcher"),
                                                          QStringLiteral("org.freedesktop.DBus.Properties"),
                                                          QStringLiteral("Get"));
        msg.setArguments(QVariantList{QStringLiteral("org.kde.StatusNotifierWatcher"), QStringLiteral("ProtocolVersion")});
        QDBusPendingCall async = QDBusConnection::sessionBus().asyncCall(msg);
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, q);
        QObject::connect(watcher, &QDBusPendingCallWatcher::finished, q,
            [this, watcher] {
                watcher->deleteLater();
                QDBusPendingReply<QVariant> reply = *watcher;
                if (reply.isError()) {
                    // If we get here, it's possible we're dealing with the non compliant GNOME extension
                    // Said implementation has a ProtocolVersion() method that returns some identifier other than the protocol version
                    // Call the method async, and if it succeeds, register the StatusNotifierItem
                    QDBusMessage extmsg = QDBusMessage::createMethodCall(QString::fromLatin1(s_statusNotifierWatcherServiceName),
                                                                      QStringLiteral("/StatusNotifierWatcher"),
                                                                      QString::fromLatin1(s_statusNotifierWatcherServiceName),
                                                                      QStringLiteral("ProtocolVersion"));
                    QDBusPendingCall extasync = QDBusConnection::sessionBus().asyncCall(extmsg);
                    QDBusPendingCallWatcher *extwatcher = new QDBusPendingCallWatcher(extasync, q);
                    QObject::connect(extwatcher, &QDBusPendingCallWatcher::finished, q,
                        [this, extwatcher] {
                            extwatcher->deleteLater();
                            QDBusPendingReply<QString> pendingWatcherReply = *extwatcher;
                            if (pendingWatcherReply.isError()) {
                                // Failed, fall back
                                qDebug() << "Failed to get KStatusNotifierWatcher protocol version:" << pendingWatcherReply.error();
                                setLegacySystemTrayEnabled(true);
                            } else {
                                qDebug() << "Detected non compliant KStatusNotifierWatcher implementation:" << pendingWatcherReply.value();
                                statusNotifierWatcher->RegisterStatusNotifierItem(statusNotifierItemDBus->service());
                                setLegacySystemTrayEnabled(false);
                            }
                        }
                    );
                } else {
                    bool ok = false;
                    const int protocolVersion = reply.value().toInt(&ok);
                    if (ok && protocolVersion == s_protocolVersion) {
                        statusNotifierWatcher->RegisterStatusNotifierItem(statusNotifierItemDBus->service());
                        setLegacySystemTrayEnabled(false);
                    } else {
                        qDebug() << "KStatusNotifierWatcher has incorrect protocol version";
                        setLegacySystemTrayEnabled(true);
                    }
                }
            }
        );
    } else {
        qDebug() << "KStatusNotifierWatcher not reachable";
        setLegacySystemTrayEnabled(true);
    }
}

void KStatusNotifierItemPrivate::serviceChange(const QString &name, const QString &oldOwner, const QString &newOwner)
{
    Q_UNUSED(name)
    if (newOwner.isEmpty()) {
        //unregistered
        qDebug() << "Connection to the KStatusNotifierWatcher lost";
        setLegacyMode(true);
        delete statusNotifierWatcher;
        statusNotifierWatcher = nullptr;
    } else if (oldOwner.isEmpty()) {
        //registered
        setLegacyMode(false);
    }
}

void KStatusNotifierItemPrivate::setLegacyMode(bool legacy)
{
    if (legacy) {
        //unregistered
        setLegacySystemTrayEnabled(true);
    } else {
        //registered
        registerToDaemon();
    }
}

void KStatusNotifierItemPrivate::legacyWheelEvent(int delta)
{
    statusNotifierItemDBus->Scroll(-delta, QStringLiteral("vertical"));
}

void KStatusNotifierItemPrivate::legacyActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::MiddleClick) {
        emit q->secondaryActivateRequested(systemTrayIcon->geometry().topLeft());
    } else if (reason == QSystemTrayIcon::Trigger) {
        q->activate(systemTrayIcon->geometry().topLeft());
    }
}

void KStatusNotifierItemPrivate::setLegacySystemTrayEnabled(bool enabled)
{
    if (enabled == (systemTrayIcon != nullptr)) {
        // already in the correct state
        return;
    }

    if (enabled) {
        bool isKde = !qEnvironmentVariableIsEmpty("KDE_FULL_SESSION") || qgetenv("XDG_CURRENT_DESKTOP") == "KDE";
        if (!systemTrayIcon && !isKde) {
            if (!QSystemTrayIcon::isSystemTrayAvailable()) {
                return;
            }
            systemTrayIcon = new KStatusNotifierLegacyIcon(associatedWidget);
            syncLegacySystemTrayIcon();
            systemTrayIcon->setToolTip(toolTipTitle);
            // silence the "icon not set" warning
            systemTrayIcon->setIcon(QIcon(QPixmap(16, 16)));
            systemTrayIcon->show();
            QObject::connect(systemTrayIcon, SIGNAL(wheel(int)), q, SLOT(legacyWheelEvent(int)));
            QObject::connect(systemTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), q, SLOT(legacyActivated(QSystemTrayIcon::ActivationReason)));
        } else if (isKde) {
            // prevent infinite recursion if the KDE platform plugin is loaded
            // but SNI is not available; see bug 350785
            qDebug() << "env says KDE is running but SNI unavailable -- check "
                                             "KDE_FULL_SESSION and XDG_CURRENT_DESKTOP";
            return;
        }

        if (menu) {
            menu->setWindowFlags(Qt::Popup);
        }
    } else {
        delete systemTrayIcon;
        systemTrayIcon = nullptr;

        if (menu) {
            menu->setWindowFlags(Qt::Window);
        }
    }

    if (menu) {
        QMenu *m = menu;
        menu = nullptr;
        q->setContextMenu(m);
    }
}

void KStatusNotifierItemPrivate::syncLegacySystemTrayIcon()
{
    // Hide the icon if passive
    systemTrayIcon->setVisible(status != KStatusNotifierItem::Passive);
    if (status == KStatusNotifierItem::NeedsAttention) {
        {
            if (!movieName.isNull()) {
                if (!movie) {
                    movie = new QMovie(movieName);
                }
                systemTrayIcon->setMovie(movie);
            } else if (!attentionIconName.isNull()) {
                systemTrayIcon->setIcon(QIcon::fromTheme(attentionIconName));
            } else {
                systemTrayIcon->setIcon(attentionIcon);
            }
        }
    } else {
        if (!iconName.isNull()) {
            systemTrayIcon->setIcon(QIcon::fromTheme(iconName));
        } else {
            systemTrayIcon->setIcon(icon);
        }
    }

    systemTrayIcon->setToolTip(toolTipTitle);
}

void KStatusNotifierItemPrivate::contextMenuAboutToShow()
{

}

void KStatusNotifierItemPrivate::maybeQuit()
{

}

void KStatusNotifierItemPrivate::minimizeRestore()
{
    q->activate(systemTrayIcon ? systemTrayIcon->geometry().topLeft() : QPoint(0, 0));
}

void KStatusNotifierItemPrivate::hideMenu()
{
    menu->hide();
}

void KStatusNotifierItemPrivate::minimizeRestore(bool show)
{
    if (show) {
        auto state = (associatedWidget->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive;
        associatedWidget->setWindowState(state);
        associatedWidget->show();
        associatedWidget->raise();
    } else {
        associatedWidget->hide();
    }
}

KDbusImageStruct KStatusNotifierItemPrivate::imageToStruct(const QImage &image)
{
    KDbusImageStruct structIcon;
    structIcon.width = image.size().width();
    structIcon.height = image.size().height();
    if (image.format() == QImage::Format_ARGB32) {
        structIcon.data = QByteArray((char *)image.bits(), QIMAGE_SIZE(image));
    } else {
        QImage image32 = image.convertToFormat(QImage::Format_ARGB32);
        structIcon.data = QByteArray((char *)image32.bits(), QIMAGE_SIZE(image32));
    }

    //swap to network byte order if we are little endian
    if (QSysInfo::ByteOrder == QSysInfo::LittleEndian) {
        const QByteArray& ba = structIcon.data;
        const int size = ba.size() / sizeof(quint32);
        const int sizeBytes = size * sizeof(quint32);
        quint32 *uintBuf = (uint32_t*)malloc(sizeBytes);
        memcpy(uintBuf, ba.data(), sizeBytes);
        for (int i = 0; i < size; ++i)
            uintBuf[i] = qToBigEndian(uintBuf[i]);
        structIcon.data = QByteArray((const char*)uintBuf, sizeBytes);
        free(uintBuf);
    }

    return structIcon;
}

KDbusImageVector KStatusNotifierItemPrivate::iconToVector(const QIcon &icn)
{
    KDbusImageVector iconVector;

    QPixmap iconPixmap;

    //if an icon exactly that size wasn't found don't add it to the vector
    const auto lstSizes = icn.availableSizes();
    for (QSize size : lstSizes) {
        iconPixmap = icn.pixmap(size);
        iconVector.append(imageToStruct(iconPixmap.toImage()));
    }
    return iconVector;
}

#include "moc_kstatusnotifieritem.cpp"
#include "moc_kstatusnotifieritemprivate_p.cpp"

/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2009 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSTATUSNOTIFIERITEMPRIVATE_H
#define KSTATUSNOTIFIERITEMPRIVATE_H

#include <QEventLoopLocker>
#include <QMovie>
#include <QObject>
#include <QString>
#include <QSystemTrayIcon>
#include <QWheelEvent>

#include "kstatusnotifieritem.h"

#ifdef QT_DBUS_LIB
#include "kstatusnotifieritemdbus_p.h"

#include "notifications_interface.h"
#include "statusnotifierwatcher_interface.h"
#endif

class KSystemTrayIcon;
class QMenu;
class QAction;

// this class is needed because we can't just put an event filter on it:
// the events that are passed to QSystemTrayIcon are done so in a way that
// bypasses the usual event filtering mechanisms *sigh*
class KStatusNotifierLegacyIcon : public QSystemTrayIcon
{
    Q_OBJECT

public:
    KStatusNotifierLegacyIcon(QObject *parent)
        : QSystemTrayIcon(parent)
    {
    }

    bool event(QEvent *e) override
    {
        if (e->type() == QEvent::Wheel) {
            QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(e);
            Q_EMIT wheel(wheelEvent->angleDelta().y());
        }

        return false;
    }

    void setMovie(QMovie *movie)
    {
        if (m_movie.data() == movie) {
            return;
        }

        delete m_movie.data();
        m_movie = movie;

        if (!movie) {
            return;
        }

        movie->setParent(this);
        movie->setCacheMode(QMovie::CacheAll);
        connect(movie, &QMovie::frameChanged, this, &KStatusNotifierLegacyIcon::slotNewFrame);
    }

    void setIconWithMask(QIcon &icon, bool isMask)
    {
        icon.setIsMask(isMask);
        QSystemTrayIcon::setIcon(icon);
    }

Q_SIGNALS:
    void wheel(int);

private Q_SLOTS:
    void slotNewFrame()
    {
        if (m_movie) {
            setIcon(QIcon(m_movie.data()->currentPixmap()));
        }
    }

private:
    QPointer<QMovie> m_movie;
};

class KStatusNotifierItemPrivate
{
public:
    KStatusNotifierItemPrivate(KStatusNotifierItem *item);

    void init(const QString &extraId);
    void registerToDaemon();
    void serviceChange(const QString &name, const QString &oldOwner, const QString &newOwner);
    void setLegacySystemTrayEnabled(bool enabled);
    void syncLegacySystemTrayIcon();
    void contextMenuAboutToShow();
    void maybeQuit();
    void minimizeRestore();
    void minimizeRestore(bool show);
    void hideMenu();
    void setLegacyMode(bool legacy);
    void checkForRegisteredHosts();
    void legacyWheelEvent(int delta);
    void legacyActivated(QSystemTrayIcon::ActivationReason reason);

    bool checkVisibility(QPoint pos, bool perform = true);

    static const int s_protocolVersion;

    KStatusNotifierItem *q;

#ifdef QT_DBUS_LIB
    KDbusImageStruct imageToStruct(const QImage &image);
    KDbusImageVector iconToVector(const QIcon &icn);

    KDbusImageVector serializedIcon;
    KDbusImageVector serializedAttentionIcon;
    KDbusImageVector serializedOverlayIcon;
    KDbusImageVector serializedToolTipIcon;

    org::kde::StatusNotifierWatcher *statusNotifierWatcher = nullptr;
    org::freedesktop::Notifications *notificationsClient = nullptr;

    KStatusNotifierItemDBus *statusNotifierItemDBus;
#endif

    KStatusNotifierItem::ItemCategory category;
    QString id;
    QString title;
    KStatusNotifierItem::ItemStatus status;

    QString iconName;
    QIcon icon;

    QString overlayIconName;
    QIcon overlayIcon;

    QString attentionIconName;
    QIcon attentionIcon;
    QString movieName;
    QPointer<QMovie> movie;

    QString toolTipIconName;
    QIcon toolTipIcon;
    QString toolTipTitle;
    QString toolTipSubTitle;
    QString iconThemePath;
    QString menuObjectPath;
    KStatusNotifierLegacyIcon *systemTrayIcon;

    QMenu *menu;
    QHash<QString, QAction *> actionCollection;
    QPointer<QWidget> associatedWidget;
    QPoint associatedWidgetPos;
    QAction *titleAction;

    // Ensure that closing the last KMainWindow doesn't exit the application
    // if a system tray icon is still present.
    QEventLoopLocker eventLoopLocker;

    bool hasQuit : 1;
    bool onAllDesktops : 1;
    bool standardActionsEnabled : 1;
};

#endif

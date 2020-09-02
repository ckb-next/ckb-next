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

#ifndef KSTATUSNOTIFIERITEMPRIVATE_H
#define KSTATUSNOTIFIERITEMPRIVATE_H

#include <QObject>
#include <QString>
#include <QMovie>
#include <QSystemTrayIcon>
#include <QWheelEvent>

#include "kstatusnotifieritem.h"
#include "kstatusnotifieritemdbus_p.h"

#include "statusnotifierwatcher_interface.h"
#include "notifications_interface.h"

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
            emit wheel(wheelEvent->angleDelta().y());
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
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
        icon.setIsMask(isMask);
#endif
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

    KDbusImageStruct imageToStruct(const QImage &image);
    KDbusImageVector iconToVector(const QIcon &icn);
    bool checkVisibility(QPoint pos, bool perform = true);

    static const int s_protocolVersion;

    KStatusNotifierItem *q;

    KStatusNotifierItem::ItemCategory category;
    QString id;
    QString title;
    KStatusNotifierItem::ItemStatus status;

    QString iconName;
    KDbusImageVector serializedIcon;
    QIcon icon;

    QString overlayIconName;
    KDbusImageVector serializedOverlayIcon;
    QIcon overlayIcon;

    QString attentionIconName;
    QIcon attentionIcon;
    KDbusImageVector serializedAttentionIcon;
    QString movieName;
    QPointer<QMovie> movie;

    QString toolTipIconName;
    KDbusImageVector serializedToolTipIcon;
    QIcon toolTipIcon;
    QString toolTipTitle;
    QString toolTipSubTitle;
    QString iconThemePath;
    QString menuObjectPath;

    QMenu *menu;
    QHash<QString, QAction *> actionCollection;
    QWidget *associatedWidget;
    QPoint associatedWidgetPos;
    QAction *titleAction;
    org::kde::StatusNotifierWatcher *statusNotifierWatcher;
    org::freedesktop::Notifications *notificationsClient;

    KStatusNotifierLegacyIcon *systemTrayIcon;
    KStatusNotifierItemDBus *statusNotifierItemDBus;
};

#endif

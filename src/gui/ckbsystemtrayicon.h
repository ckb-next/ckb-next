#ifndef CKBSYSTEMTRAYICON_H
#define CKBSYSTEMTRAYICON_H
#include <QIcon>
#include <QSystemTrayIcon>

#ifdef USE_DBUS_MENU
#include "kstatusnotifier/kstatusnotifieritem.h"
class CkbSystemTrayIcon : public KStatusNotifierItem {
#else
#include <QEvent>
#include <QWheelEvent>
class CkbSystemTrayIcon : public QSystemTrayIcon {
#endif
    Q_OBJECT

public:
#ifdef USE_DBUS_MENU
    void setIcon(QIcon icon, QString name);
    CkbSystemTrayIcon(const QIcon& icon, const QString iconName, QObject* parent = 0);
    inline void show() { setStatus(KStatusNotifierItem::Active); }
    inline void hide() { setStatus(KStatusNotifierItem::Passive); }
    inline void setVisible(bool visible) { setStatus((visible ? KStatusNotifierItem::Active : KStatusNotifierItem::Passive)); }
    ~CkbSystemTrayIcon();
signals:
    // This is never emitted by KStatusNotifierItem
    void activated(QSystemTrayIcon::ActivationReason);
private:
    QString previousPath;
#else
    CkbSystemTrayIcon(const QIcon& icon, const QString iconName, QObject* parent = 0) :  QSystemTrayIcon(icon, parent) {}
    virtual bool event(QEvent* evt)
    {
        if(evt->type() == QEvent::Wheel) {
            QWheelEvent* wheelEvt = static_cast<QWheelEvent*>(evt);
            emit scrollRequested(wheelEvt->delta(), wheelEvt->orientation());
            return true;
        }
        return QSystemTrayIcon::event(evt);
    }

signals:
        void scrollRequested(int delta, Qt::Orientation orientation);

public:
        void setIcon(QIcon icon, QString name) { QSystemTrayIcon::setIcon(icon); }
#endif
};

#endif // CKBSYSTEMTRAYICON_H

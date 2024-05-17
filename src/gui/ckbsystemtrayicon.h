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
    CkbSystemTrayIcon(const QIcon& icon, const QString iconName, QObject* parent = nullptr);
    inline void show() { setStatus(KStatusNotifierItem::Active); }
    inline void hide() { setStatus(KStatusNotifierItem::Passive); }
    inline void setVisible(bool visible) { setStatus((visible ? KStatusNotifierItem::Active : KStatusNotifierItem::Passive)); }
    ~CkbSystemTrayIcon();
signals:
    // This is never emitted by KStatusNotifierItem
    void activated(QSystemTrayIcon::ActivationReason);
    void scrollRequested(QPoint delta);
private:
    QString previousPath;
#else
    // setToolTip implementations for feature parity
    // They discard the icon
    void setToolTip(const QString& iconName, const QString& title, const QString& subTitle) { QSystemTrayIcon::setToolTip(QString("%1: %2").arg(title, subTitle)); }
    void setToolTip(const QIcon& icon, const QString& title, const QString& subTitle) { QSystemTrayIcon::setToolTip(QString("%1: %2").arg(title, subTitle)); }
    CkbSystemTrayIcon(const QIcon& icon, const QString iconName, QObject* parent = nullptr) :  QSystemTrayIcon(icon, parent) {}
    virtual bool event(QEvent* evt)
    {
        if(evt->type() == QEvent::Wheel) {
            QWheelEvent* wheelEvt = static_cast<QWheelEvent*>(evt);
            emit scrollRequested(wheelEvt->angleDelta());
            return true;
        }
        return QSystemTrayIcon::event(evt);
    }

signals:
        void scrollRequested(QPoint delta);

public:
        void setIcon(QIcon icon, QString name) { QSystemTrayIcon::setIcon(icon); }
#endif
};

#endif // CKBSYSTEMTRAYICON_H

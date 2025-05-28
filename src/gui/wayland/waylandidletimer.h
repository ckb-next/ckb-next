#ifndef WAYLANDIDLETIMER_HPP
#define WAYLANDIDLETIMER_HPP

#ifdef USE_WAYLAND_IDLE_NOTIFY
#include "idletimer.h"

#include <QScopedPointer>

class WaylandIdleNotifier;
class WaylandIdleNotification;
class WaylandIdleTimer : public IdleTimerImpl
{
public:
    WaylandIdleTimer();

    int getIdleTime() const override;
    bool isSupported() const override;

private:
    bool m_supported;
    QScopedPointer<WaylandIdleNotifier> m_notifier;
    QScopedPointer<WaylandIdleNotification> m_notification;
};
#endif

#endif // WAYLANDIDLETIMER_HPP

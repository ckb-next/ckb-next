#include "waylandidletimer.h"

#ifdef USE_WAYLAND_IDLE_NOTIFY
#include "wayland/waylandutils.h"
#include "qwayland-ext-idle-notify-v1.h"

#include <QGuiApplication>
#include <QDateTime>
#include <QWaylandClientExtensionTemplate>

class WaylandIdleNotification : public QtWayland::ext_idle_notification_v1 {
public:
    WaylandIdleNotification(struct ::ext_idle_notification_v1 *object)
        : QtWayland::ext_idle_notification_v1(object)
    {}

    ~WaylandIdleNotification()
    {
        if (qGuiApp) {
            destroy();
        }
    }

    qint64 m_idleAt = -1;

protected:
    void ext_idle_notification_v1_idled() override
    {
        m_idleAt = QDateTime::currentMSecsSinceEpoch();
    }

    void ext_idle_notification_v1_resumed() override
    {
        m_idleAt = -1;
    }
};

class WaylandIdleNotifier : public QWaylandClientExtensionTemplate<WaylandIdleNotifier>, public QtWayland::ext_idle_notifier_v1 {
public:
    WaylandIdleNotifier()
        : QWaylandClientExtensionTemplate<WaylandIdleNotifier>(2)
    {
        initialize();
    }

    ~WaylandIdleNotifier()
    {
        if (qGuiApp && isActive()) {
            destroy();
        }
    }
};

WaylandIdleTimer::WaylandIdleTimer()
    : wl_utils(new WaylandUtils), m_supported(wl_utils->hasInterface("ext_idle_notifier_v1"))
{
    if (m_supported) {
        auto seat = WaylandUtils::seat();
        m_notifier.reset(new WaylandIdleNotifier());
#ifdef USE_WAYLAND_INPUT_IDLE_NOTIFY
        if (wl_utils->hasInterface("ext_idle_notifier_v1", 2)) {
            m_notification.reset(new WaylandIdleNotification(m_notifier->get_input_idle_notification(0, seat)));
        } else {
            m_notification.reset(new WaylandIdleNotification(m_notifier->get_idle_notification(0, seat)));
        }
#else
        m_notification.reset(new WaylandIdleNotification(m_notifier->get_idle_notification(0, seat)));
#endif
    }
}

int WaylandIdleTimer::getIdleTime() const
{
    if (m_supported) {
        const auto idleAt = m_notification->m_idleAt;
        if (idleAt >= 0) {
            const auto idleTime = QDateTime::currentMSecsSinceEpoch() - idleAt;
            if (idleTime > INT_MAX) {
                return INT_MAX;
            }
            return (int)idleTime;
        }
    }
    return 0;
}

bool WaylandIdleTimer::isSupported() const
{
    return m_supported;
}

#endif


#include "idletimer.h"
#include "wayland/waylandutils.h"
#include <QScopedPointer>

#if QT_VERSION < QT_VERSION_CHECK(6, 3, 0)
#include <QGlobalStatic>
#else
#include <qapplicationstatic.h>
#endif

#ifdef USE_XCB_SCREENSAVER
#include "xcb/xcbidletimer.h"
#endif

#ifdef USE_WAYLAND_IDLE_NOTIFY
#include "wayland/waylandidletimer.h"
#endif

class IdleTimerHelper {
public:
    QScopedPointer<IdleTimerImpl> m_impl;

    IdleTimerHelper() {
        if (WaylandUtils::isWayland()) {
#ifdef USE_WAYLAND_IDLE_NOTIFY
            m_impl.reset(new WaylandIdleTimer());
#endif
        } else {
#ifdef USE_XCB_SCREENSAVER
            m_impl.reset(new XcbIdleTimer());
#endif
        }
    };
};

#if QT_VERSION < QT_VERSION_CHECK(6, 3, 0)
Q_GLOBAL_STATIC(IdleTimerHelper, g_helper)
#else
Q_APPLICATION_STATIC(IdleTimerHelper, g_helper)
#endif

int IdleTimer::getIdleTime()
{
    return isSupported() ? g_helper->m_impl->getIdleTime() : 0;
}

bool IdleTimer::isSupported()
{
    return g_helper->m_impl.isNull() ? false : g_helper->m_impl->isSupported();
}

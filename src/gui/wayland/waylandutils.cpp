#include "waylandutils.h"

#include <QGuiApplication>

#ifdef USE_WAYLAND
#include <QGuiApplication>
#if QT_VERSION < QT_VERSION_CHECK(6, 5 ,0)
#include <qpa/qplatformnativeinterface.h>
#endif

#include <wayland-client-protocol.h>
#include <wayland-client.h>

#if QT_VERSION >= QT_VERSION_CHECK(6, 3 ,0)
#include <qapplicationstatic.h>
#else
#include <QGlobalStatic>
#endif
#endif

namespace WaylandUtils
{

bool isWayland()
{
    if (qGuiApp->platformName() == "wayland") {
        return true;
    }

    if(!qEnvironmentVariableIsEmpty("WAYLAND_DESKTOP")) {
        return true;
    }

    if(qEnvironmentVariable("XDG_SESSION_TYPE") == "wayland") {
        return true;
    }

    return false;
}

#ifdef USE_WAYLAND
struct RegistryData {
    QString interfaceName;
    quint32 version;
};

class WaylandUtilsHelper {
public:
    wl_display* m_display = nullptr;
    wl_registry* m_registry = nullptr;
    wl_seat *m_seat = nullptr;

    QList<RegistryData> m_registerData;

    WaylandUtilsHelper()
    {
#if QT_VERSION < QT_VERSION_CHECK(6, 5 ,0)
        QPlatformNativeInterface *nativeInterface = qGuiApp->platformNativeInterface();
        if (!nativeInterface) {
            return;
        }

        m_display = static_cast<wl_display *>(nativeInterface->nativeResourceForIntegration("wl_display"));
        if (!m_display) {
            return;
        }

        m_seat = static_cast<wl_seat *>(nativeInterface->nativeResourceForIntegration("m_seat"));
        if (!m_seat) {
            m_display = nullptr;
            return;
        }
#else
        auto nativeInterface = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
        if (!nativeInterface) {
            return;
        }

        m_display = nativeInterface->display();
        m_seat = nativeInterface->seat();
#endif

        m_registry = wl_display_get_registry(m_display);
        if (!m_registry) {
            return;
        }

        auto global_handler = [](void* data_ptr, wl_registry*, uint32_t name, const char* interface, uint32_t version) {
            auto registerData = static_cast<QList<RegistryData>*>(data_ptr);
            registerData->append({QString::fromLatin1(interface), version});
        };

        auto global_remover_handler = [](void*, wl_registry*, uint32_t) { };

        wl_registry_listener registry_listener = {
            global_handler,
            global_remover_handler
        };

        wl_registry_add_listener(m_registry, &registry_listener, &m_registerData);

        wl_display_dispatch(m_display);
        wl_display_roundtrip(m_display);
    }
};

#if QT_VERSION >= QT_VERSION_CHECK(6, 3 ,0)
Q_APPLICATION_STATIC(WaylandUtilsHelper, g_helper,)
#else
Q_GLOBAL_STATIC(WaylandUtilsHelper, g_helper)
#endif

bool hasInterface(const QString &name, quint32 version)
{
    auto it = std::find_if(g_helper->m_registerData.constBegin(), g_helper->m_registerData.constEnd(), [&name, &version](const RegistryData& data){
        const bool matched = data.interfaceName == name;
        if (version == 0) {
            return matched;
        }
        return matched && data.version == version;
    });
    return it != g_helper->m_registerData.constEnd();
}

wl_display *display() {
    return g_helper->m_display;
}

wl_seat *seat() {
    return g_helper->m_seat;
}
#endif

} // WaylandUtils

#include "waylandutils.h"

#include <QGuiApplication>

#ifdef USE_WAYLAND
#if QT_VERSION < QT_VERSION_CHECK(6, 5 ,0)
#include <qpa/qplatformnativeinterface.h>
#endif

#include <wayland-client-protocol.h>
#include <wayland-client.h>

#endif

bool WaylandUtils::isWayland()
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

WaylandUtils::WaylandUtils()
{
    wl_display* m_display = nullptr;
#if QT_VERSION < QT_VERSION_CHECK(6, 5 ,0)
    QPlatformNativeInterface* nativeInterface = qGuiApp->platformNativeInterface();
    if (!nativeInterface)
        return;

    m_display = static_cast<wl_display *>(nativeInterface->nativeResourceForIntegration("wl_display"));
#else
    auto nativeInterface = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
    if (!nativeInterface)
        return;

    m_display = nativeInterface->display();
#endif
    if (!m_display)
        return;

    wl_registry* m_registry = wl_display_get_registry(m_display);
    if (!m_registry)
        return;

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

    // HACK: Destroy the registry proxy to indirectly remove the listener
    // This way we avoid crashing when the display disappears/changes
    wl_proxy_destroy(reinterpret_cast<wl_proxy*>(m_registry));
}

wl_seat* WaylandUtils::seat() {
#if QT_VERSION < QT_VERSION_CHECK(6, 5 ,0)
    QPlatformNativeInterface* nativeInterface = qGuiApp->platformNativeInterface();
    if (!nativeInterface)
        return nullptr;

    wl_seat* s = static_cast<wl_seat*>(nativeInterface->nativeResourceForIntegration("m_seat"));
#else
    auto nativeInterface = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
    if (!nativeInterface)
        return nullptr;

    wl_seat* s = nativeInterface->seat();
#endif
    return s;
}

bool WaylandUtils::hasInterface(const QString &name, quint32 version)
{
    auto it = std::find_if(m_registerData.constBegin(), m_registerData.constEnd(), [&name, &version](const RegistryData& data){
        const bool matched = data.interfaceName == name;
        if (version == 0) {
            return matched;
        }
        return matched && data.version == version;
    });
    return it != m_registerData.constEnd();
}

#endif

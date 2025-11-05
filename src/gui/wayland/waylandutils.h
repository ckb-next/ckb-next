#ifndef WAYLANDUTILS_HPP
#define WAYLANDUTILS_HPP

#include <QString>

#ifdef USE_WAYLAND
class wl_display;
class wl_seat;
#include <QList>
#endif

class WaylandUtils
{

public:
    static bool isWayland();

#ifdef USE_WAYLAND
    WaylandUtils();
    static wl_seat* seat();

    bool hasInterface(const QString& name, quint32 version = 0);

private:
    struct RegistryData {
        QString interfaceName;
        quint32 version;
    };
    QList<RegistryData> m_registerData;
#endif

}; // WaylandUtils

#endif // WAYLANDUTILS_HPP

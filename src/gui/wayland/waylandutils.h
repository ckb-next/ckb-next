#ifndef WAYLANDUTILS_HPP
#define WAYLANDUTILS_HPP

#include <QString>

#ifdef USE_WAYLAND
class wl_display;
class wl_seat;
#endif

namespace WaylandUtils
{

bool isWayland();

#ifdef USE_WAYLAND
bool hasInterface(const QString &name, quint32 version);

wl_display *display();
wl_seat *seat();
#endif

} // WaylandUtils

#endif // WAYLANDUTILS_HPP

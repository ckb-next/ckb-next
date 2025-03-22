#include "waylandutils.h"

#include <QGuiApplication>

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

} // WaylandUtils

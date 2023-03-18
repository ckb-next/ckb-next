#include "idletimer.h"
#include <stdint.h>
#include <QtGlobal>
#include <QByteArray>
#include <QString>

#ifdef USE_XCB_SCREENSAVER
#include <xcb/xcb.h>
#include <QX11Info>
#include <xcb/screensaver.h>
#include <limits.h>

int IdleTimer::getIdleTime(){
    xcb_connection_t* conn = QX11Info::connection();
    const int display = QX11Info::appScreen();

    xcb_screensaver_query_info_cookie_t replyCookie = xcb_screensaver_query_info(conn, QX11Info::appRootWindow(display));
    xcb_screensaver_query_info_reply_t* replyInfo = xcb_screensaver_query_info_reply(conn, replyCookie, nullptr);

    uint32_t userIdle = replyInfo->ms_since_user_input;
    free(replyInfo);

    // Clamp the value
    if(userIdle > INT_MAX)
        userIdle = INT_MAX;

    return (int)userIdle;
}

#else

int IdleTimer::getIdleTime(){
    return 0;
}
#endif

bool IdleTimer::isWayland(){
    QByteArray wdesktop = qgetenv("WAYLAND_DESKTOP");
    if(!wdesktop.isEmpty())
        return true;
    QString stype(qgetenv("XDG_SESSION_TYPE"));
    if(stype == "wayland")
        return true;
    return false;
}

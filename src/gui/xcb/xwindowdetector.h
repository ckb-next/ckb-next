#ifndef XWINDOWDETECTOR_H
#define XWINDOWDETECTOR_H

#include <QObject>
#include <QThread>
#include "xwindowinfo.h"
#ifdef USE_XCB_EWMH
#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>
#endif

class XWindowDetector : public QThread
{
    Q_OBJECT
#ifdef USE_XCB_EWMH
public:
    explicit XWindowDetector(QObject *parent = 0);
    void terminateEventLoop();

signals:
    void activeWindowChanged(XWindowInfo info);
public slots:

void run();

private:
    static QString getExePathByPID(uint32_t pid);
    QString xcbGetString(xcb_connection_t* conn, xcb_window_t win, xcb_atom_enum_t atom, xcb_atom_enum_t type);
    xcb_generic_event_t* xcbWaitForEventInterruptible(xcb_connection_t* conn, int xcbFd, fd_set* fds);
    void fetchNewWinInfo(xcb_window_t win, xcb_ewmh_connection_t* ewmh, xcb_connection_t* conn, bool& window_has_ewmh);
#endif
};

#endif // XWINDOWDETECTOR_H

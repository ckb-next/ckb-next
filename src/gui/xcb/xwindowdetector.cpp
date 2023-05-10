#include "xwindowdetector.h"
#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <QDebug>
#include "xwindowinfo.h"
#include <QMetaType>
#include <QFileInfo>
#include <sys/eventfd.h>
#include <unistd.h>

// Gets the current focused window, listens for focus out events on it, and repeats on new events.
const uint32_t values = XCB_EVENT_MASK_PROPERTY_CHANGE;
const uint32_t values_unsubscribe = 0;

xcb_window_t appwindow = 0;
int efd;

// The following are/must be called in the parent thread
XWindowDetector::XWindowDetector(QObject *parent) :
    QThread(parent)
{
    efd = eventfd(0, EFD_NONBLOCK);
}

void XWindowDetector::terminateEventLoop()
{
    uint64_t val = 1;
    if(write(efd, &val, sizeof(uint64_t)) == -1)
        qDebug() << "XCB Terminate event loop errno" << errno;
}

// The following are to be called inside QThread::run() (the new thread)
QString XWindowDetector::getExePathByPID(uint32_t pid)
{
    return QFileInfo(QString("/proc/") + QString::number(pid) + "/exe").canonicalFilePath();
}

void listenForWindowEvents(xcb_connection_t* conn, xcb_window_t win, const uint32_t* val)
{
    xcb_change_window_attributes(conn, win, XCB_CW_EVENT_MASK, val);
    xcb_flush(conn);
}

QString XWindowDetector::xcbGetString(xcb_connection_t* conn, xcb_window_t win, xcb_atom_enum_t atom, xcb_atom_enum_t type)
{
    QString xcb_string;
    xcb_get_property_cookie_t prop = xcb_get_property(conn, 0, win, atom, type, 0, 128);
    xcb_get_property_reply_t* reply = xcb_get_property_reply(conn, prop, nullptr);
    if(!reply)
        return xcb_string;
    int len = xcb_get_property_value_length(reply);
    if(len)
        xcb_string = QString::fromUtf8((const char*)xcb_get_property_value(reply), len);
    free(reply);
    return xcb_string;
}

xcb_generic_event_t* XWindowDetector::xcbWaitForEventInterruptible(xcb_connection_t* conn, int xcbFd, fd_set* fds)
{
    FD_ZERO(fds);
    FD_SET(xcbFd, fds);
    FD_SET(efd, fds);
    if(pselect(((xcbFd > efd) ? xcbFd : efd) + 1, fds, nullptr, nullptr, nullptr, nullptr) > 0)
    {
        // If we received data from the eventfd, then we need to tell the thread to close. Otherwise, we have an event from xcb.
        if(FD_ISSET(efd, fds))
            return nullptr;
        return xcb_poll_for_event(conn);
    }
    return nullptr;
}

void XWindowDetector::fetchNewWinInfo(xcb_window_t win, xcb_ewmh_connection_t* ewmh, xcb_connection_t* conn, bool& window_has_ewmh)
{
    uint32_t pid = 0;
    QStringList wm_class;
    QString window_name;
    int cnt = 0;
    // Some windows don't have the information ready, so if there's no meaningful data, keep trying
    do
    {
        // get PID if possible
        xcb_get_property_reply_t* pid_reply = xcb_get_property_reply(conn, xcb_ewmh_get_wm_pid(ewmh, win), nullptr);
        if(pid_reply)
        {
            xcb_ewmh_get_wm_pid_from_reply(&pid, pid_reply);
            free(pid_reply);
        }

        // Get WM_CLASS
        wm_class = xcbGetString(conn, win, XCB_ATOM_WM_CLASS, XCB_ATOM_STRING).split(QChar('\0'),
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
                                                                                     QString::SkipEmptyParts
#else
                                                                                     Qt::SkipEmptyParts
#endif
                                                                                     );

        // Get _NET_WM_NAME using EWMH
        xcb_ewmh_get_utf8_strings_reply_t _net_wm_name;
        if(xcb_ewmh_get_wm_name_from_reply(ewmh, &_net_wm_name, xcb_get_property_reply(conn, xcb_ewmh_get_wm_name(ewmh, win), nullptr)))
        {
            window_name = QString::fromUtf8(_net_wm_name.strings, _net_wm_name.strings_len);
            xcb_ewmh_get_utf8_strings_reply_wipe(&_net_wm_name);
            window_has_ewmh = true;
        }
        else
        {
            // Fall back to the legacy WM_NAME property
            window_name = xcbGetString(conn, win, XCB_ATOM_WM_NAME, XCB_ATOM_ANY);
            window_has_ewmh = false;
        }
        cnt++;
        msleep(100);
    } while (window_name.isEmpty() && wm_class.isEmpty() && pid == 0 && cnt < 5);

    if(!window_name.isEmpty())
    {
        QString wm_instance_name, wm_class_name;
        if(wm_class.length() == 2)
        {
            wm_instance_name = wm_class.at(0);
            wm_class_name = wm_class.at(1);
        }
        const XWindowInfo info = {pid, window_name, getExePathByPID(pid), wm_instance_name, wm_class_name};
        emit activeWindowChanged(info);
    }
}

static inline xcb_screen_t* getPreferredScreen(xcb_connection_t* conn, int preferred_screen){
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(xcb_get_setup(conn));
    for(int i = 0; i < preferred_screen; i++){
        if(!iter.rem)
            return nullptr;
        xcb_screen_next(&iter);
    }
    return iter.data;
}

static inline xcb_window_t getActiveWindow(xcb_ewmh_connection_t* ewmh, int preferred_screen){
    xcb_window_t win = 0;
    xcb_ewmh_get_active_window_reply(ewmh, xcb_ewmh_get_active_window(ewmh, preferred_screen), &win, nullptr);
    while(!win) {
        QThread::msleep(50);
        xcb_ewmh_get_active_window_reply(ewmh, xcb_ewmh_get_active_window(ewmh, preferred_screen), &win, nullptr);
        // Read from the eventfd to allow breaking from the loop to quit
        uint64_t buf;
        if(read(efd, &buf, sizeof(buf)) == -1 && errno == EAGAIN)
            continue;
        return 0;
    }
    return win;
}

// From main.cpp
extern const char* DISPLAY;

void XWindowDetector::run()
{
    int preferred_screen = 0;
    // XCB
    xcb_connection_t* conn = xcb_connect(DISPLAY, &preferred_screen);
    xcb_screen_t* scr = getPreferredScreen(conn, preferred_screen);

    // EWMH
    xcb_ewmh_connection_t ewmh_c;
    xcb_ewmh_connection_t* ewmh = &ewmh_c;
    xcb_generic_error_t* err = nullptr;
    if(!xcb_ewmh_init_atoms_replies(ewmh, xcb_ewmh_init_atoms(conn, ewmh), &err))
    {
        qDebug() << "Error initialising EWMH";
        if(err)
            qDebug() << "XCB error code" << err->error_code;
        free(err);
        xcb_disconnect(conn);
        return;
    }

    // Enable notifications for the root window
    listenForWindowEvents(conn, scr->root, &values);

    xcb_window_t cur_win = 0;
    bool window_has_ewmh = false; // Used to skip window title change events
    int xcbFd = xcb_get_file_descriptor(conn);
    fd_set fds;
    xcb_generic_event_t* evt;
    while((evt = xcbWaitForEventInterruptible(conn, xcbFd, &fds)))
    {
        const uint8_t eventtype = evt->response_type & ~0x80;
        if (eventtype == XCB_PROPERTY_NOTIFY) {
            xcb_property_notify_event_t* notifyevt = reinterpret_cast<xcb_property_notify_event_t*>(evt);

            // Root window events are used to monitor for the active window
            if(notifyevt->window == scr->root){
                // Ignore notifications for all atoms other than the active window
                if(notifyevt->atom != ewmh->_NET_ACTIVE_WINDOW)
                    continue;

                xcb_window_t new_win = getActiveWindow(ewmh, preferred_screen);
                if(!new_win){
                    free(evt);
                    xcb_ewmh_connection_wipe(ewmh);
                    xcb_disconnect(conn);
                    return;
                }

                if(cur_win != new_win){
                    if(cur_win)
                        listenForWindowEvents(conn, cur_win, &values_unsubscribe);
                    cur_win = new_win;

                    listenForWindowEvents(conn, new_win, &values);
                    fetchNewWinInfo(new_win, ewmh, conn, window_has_ewmh);
                }
            } else if(notifyevt->window == cur_win) {
                // Only accept legacy WM_NAME if we haven't gotten an ewmh name
                if(notifyevt->atom == ewmh->_NET_WM_NAME || (!window_has_ewmh && notifyevt->atom == XCB_ATOM_WM_NAME)
                        || notifyevt->atom == XCB_ATOM_WM_CLASS){
                    fetchNewWinInfo(cur_win, ewmh, conn, window_has_ewmh);
                }
            }
        }
        free(evt);
    }

    qDebug() << "Disconnecting from X";
    xcb_ewmh_connection_wipe(ewmh);
    xcb_disconnect(conn);
    qDebug() << "XCB EWMH Thread Quitting...";
}

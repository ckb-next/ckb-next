#include "ckbsystemtrayicon.h"
#include <QEvent>
#include <QWheelEvent>

bool CkbSystemTrayIcon::event(QEvent* evt){
    if(evt->type() == QEvent::Wheel) {
        QWheelEvent* wheelEvt = static_cast<QWheelEvent*>(evt);
        if(wheelEvt->delta() > 0)
            emit wheelScrolled(true);
        else
            emit wheelScrolled(false);
        return true;
    }
    return QSystemTrayIcon::event(evt);
}

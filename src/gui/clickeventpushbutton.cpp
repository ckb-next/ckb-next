#include "clickeventpushbutton.h"
#include <QMouseEvent>
void ClickEventPushButton::mouseReleaseEvent(QMouseEvent* e){
    // We have to check manually, because underMouse() returns false
    // and mouseLeaveEvent is processed after this event
    if(e->button() == Qt::LeftButton && rect().contains(e->pos()))
        clickedByMouse = true;
    QToolButton::mouseReleaseEvent(e);
}

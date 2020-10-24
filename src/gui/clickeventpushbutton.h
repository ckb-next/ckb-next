#ifndef CLICKEVENTPUSHBUTTON_H
#define CLICKEVENTPUSHBUTTON_H

#include <QToolButton>
#include <QDebug>

class ClickEventPushButton : public QToolButton{
    Q_OBJECT
public:
    ClickEventPushButton(QWidget* parent = nullptr) : QToolButton(parent), clickedByMouse(false) {}
    inline bool isClickedByMouse() {
        bool temp = clickedByMouse;
        clickedByMouse = false;
        return temp;
    }
protected:
    virtual void mouseReleaseEvent(QMouseEvent* e) override;
private:
    bool clickedByMouse;
};
#endif // CLICKEVENTPUSHBUTTON_H

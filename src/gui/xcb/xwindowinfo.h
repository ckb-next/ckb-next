#ifndef XWINDOWINFO_H
#define XWINDOWINFO_H

#include <QString>

class XWindowInfo
{
public:
    uint32_t pid;
    QString windowTitle;
    QString program;
    QString wm_instance_name;
    QString wm_class_name;

    inline bool isEmpty() { return windowTitle.isEmpty() && program.isEmpty() && wm_instance_name.isEmpty() && wm_class_name.isEmpty(); }
};

#endif // XWINDOWINFO_H

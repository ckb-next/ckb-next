#ifndef XCBIDLETIMER_HPP
#define XCBIDLETIMER_HPP

#include "idletimer.h"

class XcbIdleTimer : public IdleTimerImpl
{
public:
    int getIdleTime() override;
};

#endif // XCBIDLETIMER_HPP

#ifndef XCBIDLETIMER_HPP
#define XCBIDLETIMER_HPP

#include "idletimer.h"

class XcbIdleTimer : public IdleTimerImpl
{
public:
    int getIdleTime() const override;
};

#endif // XCBIDLETIMER_HPP

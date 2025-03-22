#ifndef IDLETIMER_H
#define IDLETIMER_H

class IdleTimerImpl {
public:
    virtual int getIdleTime() = 0;
};

class IdleTimer
{
public:
    static int getIdleTime();
    static bool isSupported();
};

#endif // IDLETIMER_H

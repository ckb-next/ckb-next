#ifndef IDLETIMER_H
#define IDLETIMER_H

class IdleTimerImpl {
public:
    virtual int getIdleTime() const = 0;
    virtual inline bool isSupported() const {
        return true;
    }
    virtual ~IdleTimerImpl() = default;
};

class IdleTimer
{
public:
    static int getIdleTime();
    static bool isSupported();
};

#endif // IDLETIMER_H

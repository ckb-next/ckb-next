#ifndef MONOTONICCLOCK_H
#define MONOTONICCLOCK_H
#include <chrono>
#include <QtGlobal>
using namespace std::chrono;

struct MonotonicClock {
    static inline qint64 msecs() {
        steady_clock::time_point tp = steady_clock::now();
        milliseconds ms = duration_cast<milliseconds>(tp.time_since_epoch());
        return ms.count();
    }
};

#endif // MONOTONICCLOCK_H

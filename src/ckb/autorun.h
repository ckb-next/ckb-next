#ifndef AUTORUN_H
#define AUTORUN_H

#include <QObject>

// Class for running ckb at login

class AutoRun
{
public:
    // Whether or not run at login is possible
    static bool available();

    // Whether or not ckb has been set to run at login at least once before
    static bool once();

    // Enable/disable launch at login
    static bool isEnabled();
    static void enable();
    static void disable();
};

#endif // AUTORUN_H

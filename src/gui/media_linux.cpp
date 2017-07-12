#ifndef __APPLE__

#include <QDateTime>
#include <QProcess>
#include "media.h"

static QProcess muteProcess;

muteState getMuteState(){
    // Get default sink mute state from pulseaudio
    static muteState lastKnown = UNKNOWN;
    static quint64 lastTime = 0;
    // Instead of running a command to check the state and waiting for it to finish, run the command now but wait to check it until the next frame
    // (locking up the GUI thread is bad)
    if(lastTime > 0 && muteProcess.state() == QProcess::NotRunning){
        if(muteProcess.exitCode() != 0)
            lastKnown = UNKNOWN;
        QString output = muteProcess.readLine().trimmed();
        if(output == "yes")
            lastKnown = MUTED;
        else if(output == "no")
            lastKnown = UNMUTED;
    }
    quint64 time = QDateTime::currentMSecsSinceEpoch();
    if(time - lastTime < 33)
        // Don't run it than 30 times per second
        return lastKnown;
    lastTime = time;
    if(muteProcess.state() == QProcess::NotRunning)
        // Shamelessly taken from pulseaudio-ctl
        muteProcess.start("sh", QStringList() << "-c" << "pacmd list-sinks|grep -A 15 '* index'|awk '/muted:/{ print $2 }'");
    return lastKnown;
}

#endif

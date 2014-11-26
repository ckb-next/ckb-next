#ifndef __APPLE__

#include <QDateTime>
#include <QProcess>
#include "media.h"

muteState getMuteState(){
    // Get default sink mute state from pulseaudio
    static muteState lastKnown = UNKNOWN;
    static quint64 lastTime = 0;
    quint64 time = QDateTime::currentMSecsSinceEpoch();
    if(time - lastTime < 50)
        // Don't run more than 20 times per second
        return lastKnown;
    lastTime = time;
    QProcess process;
    // Shamelessly taken from pulseaudio-ctl
    process.start("sh", QStringList() << "-c" << "pacmd list-sinks|grep -A 15 '* index'|awk '/muted:/{ print $2 }'");
    // It needs to finish very quickly (and might not do so), so if it dies too early return the last known state
    process.waitForFinished(10);
    QString output = process.readLine().trimmed();
    if(output == "yes")
        return lastKnown = MUTED;
    else if(output == "no")
        return lastKnown = UNMUTED;
    return lastKnown;
}

#endif

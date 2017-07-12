#include "kbmanager.h"

#ifndef Q_OS_MACX
QString devpath = "/dev/input/ckb%1";
#else
QString devpath = "/var/run/ckb%1";
#endif

QString KbManager::_guiVersion, KbManager::_daemonVersion = DAEMON_UNAVAILABLE_STR;
KbManager* KbManager::_kbManager = 0;

void KbManager::init(QString guiVersion){
    _guiVersion = guiVersion;
    if(_kbManager)
        return;
    _kbManager = new KbManager();
}

void KbManager::stop(){
    if(!_kbManager)
        return;
    delete _kbManager;
    _kbManager = 0;
}

KbManager::KbManager(QObject *parent) : QObject(parent){
    // Set up the timers
    _eventTimer = new QTimer(this);
    _eventTimer->setTimerType(Qt::PreciseTimer);
    _scanTimer = new QTimer(this);
    _scanTimer->start(100);
    connect(_scanTimer, SIGNAL(timeout()), this, SLOT(scanKeyboards()));
}

void KbManager::fps(int framerate){
    QTimer* timer = eventTimer();
    if(!timer)
        return;
    if(timer->isActive())
        timer->setInterval(1000 / framerate);
    else
        timer->start(1000 / framerate);
}

float KbManager::parseVersionString(QString version){
    // Remove extraneous info (anything after a +, anything non-numeric)
    QStringList dots = version.replace(QRegExp("\\+.+"), "").replace(QRegExp("[^\\d\\.]"), "").split(".");
    float base = 1.f;
    float res = 0.f;
    // A number like "1.2.3" will result in 1.0203
    // NB: will fail if a point version goes over 99 or if using more than two dots. floats can only reliably encode 7 decimal digits.
    foreach(const QString& dot, dots){
        res += dot.toFloat() * base;
        base /= 100.f;
    }
    return res;
}

void KbManager::scanKeyboards(){
    QString rootdev = devpath.arg(0);
    QFile connected(rootdev + "/connected");
    if(!connected.open(QIODevice::ReadOnly)){
        // No root controller - remove all keyboards
        foreach(Kb* kb, _devices){
            emit kbDisconnected(kb);
            kb->save();
            delete kb;
        }
        _devices.clear();
        if(_daemonVersion != DAEMON_UNAVAILABLE_STR){
            _daemonVersion = DAEMON_UNAVAILABLE_STR;
            emit versionUpdated();
        }
        return;
    }
    // Check daemon version
    QFile version(rootdev + "/version");
    QString vString;
    if(version.open(QIODevice::ReadOnly)){
        vString = QString::fromUtf8(version.readLine()).trimmed();
        version.close();
    } else
        vString = DAEMON_UNAVAILABLE_STR;
    if(_daemonVersion != vString){
        _daemonVersion = vString;
        emit versionUpdated();
    }

    // Scan connected devices
    QList<QStringList> lines;
    while(1){
        QString line = connected.readLine().trimmed();
        if(line.isEmpty())
            break;
        QStringList components = line.split(" ");
        if(components.length() < 2)             // "<path> <serial> <name>" (we're only interested in the first two)
            continue;
        lines.append(components);
    }
    connected.close();

    // Remove any active devices not in the list
    QMutableSetIterator<Kb*> i(_devices);
    while(i.hasNext()){
        Kb* kb = i.next();
        bool matched = false;
        foreach(const QStringList& line, lines){
            if(kb->matches(line[0], line[1])){
                matched = true;
                break;
            }
        }
        if(matched)
            continue;
        // Device not found, remove
        i.remove();
        emit kbDisconnected(kb);
        kb->save();
        delete kb;
    }

    // Add any new devices found in the list
    foreach(const QStringList& line, lines){
        bool matched = false;
        foreach(Kb* kb, _devices){
            if(kb->matches(line[0], line[1])){
                matched = true;
                break;
            }
        }
        if(matched)
            continue;
        // Device not found, create new
        Kb* kb = new Kb(this, line[0]);
        if(!kb->isOpen()){
            delete kb;
            continue;
        }
        _devices.insert(kb);
        // Load preferences and send signal
        emit kbConnected(kb);
        kb->load();
        connect(_eventTimer, SIGNAL(timeout()), kb, SLOT(frameUpdate()));
        connect(_scanTimer, SIGNAL(timeout()), kb, SLOT(autoSave()));
    }
}


#include "kbmanager.h"
#include "idletimer.h"
#include <limits>

QString devpath = "/dev/input/ckb%1";

#ifdef DEBUG_IDLE_TIMER
#define IDLE_TIMER_DURATION 5000
#else
#define IDLE_TIMER_DURATION (CkbSettings::get("Program/IdleTimerDuration", 5).toInt() * 60000)
#endif

QString KbManager::_guiVersion, KbManager::_daemonVersion = DAEMON_UNAVAILABLE_STR;
KbManager* KbManager::_kbManager = 0;

#ifdef USE_XCB_SCREENSAVER
QTimer* KbManager::_idleTimer = nullptr;
void KbManager::setIdleTimer(bool enable){
    if(!_idleTimer){
        // This won't go well if there are multiple instances of KbManager since the pointer is static.
        // The rest of the code seems to only be able to handle only one instance too, so it should be fine.
        _idleTimer = new QTimer(_kbManager);
        connect(_idleTimer, &QTimer::timeout, _kbManager, &KbManager::idleTimerTick);
    }
    if(enable)
        _idleTimer->start(IDLE_TIMER_DURATION);
    else
        _idleTimer->stop();
}
void KbManager::idleTimerTick(){
    // Get user idle time
    const int idle = IdleTimer::getIdleTime();
    const int settingsidle = IDLE_TIMER_DURATION;
    const int devidleres = settingsidle - getLastUsedDeviceIdleTime();
    int systemidleres = settingsidle - idle;
    if(systemidleres > 0 || devidleres > 0)
    {
        if(devidleres > systemidleres)
            systemidleres = devidleres;

        // Turn the lights back on (if applicable)
        foreach(Kb* kb, _devices){
            // Check if the dimming state on this device could actually be restored
            // If not, skip over it to avoid a failed brightness setting restoration with shared dimming
            if(kb->currentLight()->timerDimRestore() == -1)
                continue;
            if(KbLight::shareDimming() != -1)
                break;
        }

        // Reschedule the timer if there's still time left
        _idleTimer->start(systemidleres);
        return;
    }
    // Turn off all the lights
    foreach(Kb* kb, _devices){
        kb->currentLight()->timerDim();
        if(KbLight::shareDimming() != -1)
            break;
    }
    // Start checking for activity every half a second
    _idleTimer->start(500);
}
#endif

void KbManager::init(QString guiVersion){
    _guiVersion = guiVersion;
    if(_kbManager)
        return;
    _kbManager = new KbManager();
#ifdef USE_XCB_SCREENSAVER
    if(!IdleTimer::isWayland() && CkbSettings::get("Program/IdleTimerEnable", true).toBool()){
        setIdleTimer(true);
    }
#endif
}

void KbManager::stop(){
    if(!_kbManager)
        return;
    delete _kbManager;
    _kbManager = nullptr;
}

KbManager::KbManager(QObject *parent) : QObject(parent){
    // Set up the timers
    _eventTimer = new QTimer(this);
    _eventTimer->setTimerType(Qt::PreciseTimer);
    _scanTimer = new QTimer(this);
    _scanTimer->start(1000);
    connect(_scanTimer, SIGNAL(timeout()), this, SLOT(scanKeyboards()));

    // Scan for keyboards immediately so they show up as soon as the GUI appears.
    QTimer::singleShot(0,this,SLOT(scanKeyboards()));
}

int KbManager::getLastUsedDeviceIdleTime(){
    qint64 last = std::numeric_limits<qint64>::max();

    for(const Kb* const dev : _devices){
        const qint64 cur = dev->getDeviceIdleTime();
        if(last > cur)
            last = cur;
    }

    if(last > INT_MAX)
        last = INT_MAX;
    return (int)last;
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
    // Remove extraneous info (anything after + or -, anything non-numeric)
    QStringList dots = version.replace(QRegExp("\\+|-.+"), "").replace(QRegExp("[^\\d\\.]"), "").split(".");
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

void KbManager::brightnessScroll(int delta, Qt::Orientation orientation){
    const bool up = delta > 0;
    int dimming = KbLight::shareDimming();

    // Only run this if shared dimming is enabled
    if(KbLight::shareDimming() == -1)
        return;

    QSet<Kb*>::const_iterator i = _devices.begin();
    if(_devices.empty() || i == _devices.end())
        return;

    dimming += (up ? -1 : 1);

    if(dimming < 0)
        dimming = 0;
    if(dimming > KbLight::MAX_DIM)
        dimming = KbLight::MAX_DIM;

    (*i)->currentLight()->dimming(dimming);
}

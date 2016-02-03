#include <cmath>
#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QUrl>
#include "animscript.h"

QHash<QUuid, AnimScript*> AnimScript::scripts;

AnimScript::AnimScript(QObject* parent, const QString& path) :
    QObject(parent), _path(path), initialized(false), process(0)
{
}

AnimScript::AnimScript(QObject* parent, const AnimScript& base) :
    QObject(parent), _info(base._info), _path(base._path), initialized(false), process(0)
{
}

AnimScript::~AnimScript(){
    if(process){
        process->kill();
        process->waitForFinished(1000);
        delete process;
    }
}

QString AnimScript::path(){
#ifdef __APPLE__
    return QDir(QApplication::applicationDirPath() + "/../Resources").absoluteFilePath("ckb-animations");
#else
    return QDir(QApplication::applicationDirPath()).absoluteFilePath("ckb-animations");
#endif
}

void AnimScript::scan(){
    QDir dir(path());
    foreach(AnimScript* script, scripts)
        delete script;
    scripts.clear();
    foreach(QString file, dir.entryList(QDir::Files | QDir::Executable)){
        AnimScript* script = new AnimScript(qApp, dir.absoluteFilePath(file));
        if(script->load() && !scripts.contains(script->_info.guid))
            scripts[script->_info.guid] = script;
        else
            delete script;
    }
}

QList<const AnimScript*> AnimScript::list(){
    // Gather the animations into an alphabetically-sorted list
    QMap<QString, const AnimScript*> result;
    foreach(AnimScript* script, scripts.values()){
        QString name = script->name();
        if(result.contains(name)){
            // If duplicate names exist, make them unique by including their GUIDs
            AnimScript* last = (AnimScript*)result[name];
            last->_info.name += " " + last->guid().toString().toUpper();
            script->_info.name += " " + script->guid().toString().toUpper();
        }
        result[script->name()] = script;
    }
    return result.values();
}

AnimScript* AnimScript::copy(QObject* parent, const QUuid& id){
     return scripts.contains(id) ? new AnimScript(parent, *scripts.value(id)) : 0;
}

inline QString urlParam(const QString& param){
    return QUrl::fromPercentEncoding(param.trimmed().toLatin1()).trimmed();
}

const static double ONE_DAY = 24. * 60. * 60.;

bool AnimScript::load(){
    // Run the process to get script info
    QProcess infoProcess;
    infoProcess.start(_path, QStringList("--ckb-info"));
    qDebug() << "Scanning " << _path;
    infoProcess.waitForFinished(1000);
    if(infoProcess.state() == QProcess::Running){
        // Kill the process if it takes more than 1s
        infoProcess.kill();
        return false;
    }
    // Set defaults for performance info
    _info.kpMode = KP_NONE;
    _info.absoluteTime = _info.preempt = _info.liveParams = false;
    _info.repeat = true;
    // Read output
    QString line;
    while((line = infoProcess.readLine()) != ""){
        line = line.trimmed();
        QStringList components = line.split(" ");
        int count = components.count();
        if(count < 2)
            continue;
        QString param = components[0].trimmed();
        if(param == "guid")
            _info.guid = QUuid(urlParam(components[1]));
        else if(param == "name")
            _info.name = urlParam(components[1]);
        else if(param == "version")
            _info.version = urlParam(components[1]);
        else if(param == "year")
            _info.year = urlParam(components[1]);
        else if(param == "author")
            _info.author = urlParam(components[1]);
        else if(param == "license")
            _info.license = urlParam(components[1]);
        else if(param == "description")
            _info.description = urlParam(components[1]);
        else if(param == "kpmode")
            _info.kpMode = (components[1] == "position") ? KP_POSITION : (components[1] == "name") ? KP_NAME : KP_NONE;
        else if(param == "time")
            _info.absoluteTime = (components[1] == "absolute");
        else if(param == "repeat")
            _info.repeat = (components[1] == "on");
        else if(param == "preempt")
            _info.preempt = (components[1] == "on");
        else if(param == "parammode")
            _info.liveParams = (components[1] == "live");
        else if(param == "param"){
            // Read parameter
            if(count < 3)
                continue;
            while(components.count() < 8)
                components.append("");
            Param::Type type = Param::INVALID;
            QString sType = components[1].toLower();
            if(sType == "long")
                type = Param::LONG;
            else if(sType == "double")
                type = Param::DOUBLE;
            else if(sType == "bool")
                type = Param::BOOL;
            else if(sType == "rgb")
                type = Param::RGB;
            else if(sType == "argb")
                type = Param::ARGB;
            else if(sType == "gradient")
                type = Param::GRADIENT;
            else if(sType == "agradient")
                type = Param::AGRADIENT;
            else if(sType == "angle")
                type = Param::ANGLE;
            else if(sType == "string")
                type = Param::STRING;
            else if(sType == "label")
                type = Param::LABEL;
            else
                continue;
            // "param <type> <name> <prefix> <postfix> <default>"
            QString name = components[2].toLower();
            // Make sure it's not present already
            if(hasParam(name))
                continue;
            QString prefix = urlParam(components[3]), postfix = urlParam(components[4]);
            QVariant def = urlParam(components[5]), minimum = urlParam(components[6]), maximum = urlParam(components[7]);
            // Don't allow predefined params
            if(name == "trigger" || name == "kptrigger" || name == "kpmode" || name == "duration" || name == "delay" || name == "kpdelay" || name == "repeat" || name == "kprepeat" || name == "stop" || name == "kpstop" || name == "kpmodestop" || name == "kprelease")
                continue;
            Param param = { type, name, prefix, postfix, def, minimum, maximum };
            _info.params.append(param);
        } else if(param == "preset"){
            // Add preset
            QString name = urlParam(components.at(1));
            QMap<QString, QVariant> preset;
            for(int i = 2; i < count; i++){
                // Scan name/value setting pairs
                QString setting = components.at(i);
                QStringList sComponents = setting.split("=");
                if(sComponents.count() != 2)
                    continue;
                QString param = sComponents.first().trimmed();
                QString value = urlParam(sComponents.last());
                preset[param] = value;
            }
            // If the preset contains a duration, set the repeat time to the same value
            if(preset.contains("duration")){
                QVariant duration = preset.value("duration");
                preset["repeat"] = duration;
                preset["kprepeat"] = duration;
            }
            _presets.append(name);
            _presetValues.append(preset);
        }
    }
    // Make sure the required parameters are filled out
    if(_info.guid.isNull() || _info.name == "" || _info.version == "" || _info.year == "" || _info.author == "" || _info.license == "")
        return false;
    // Add timing parameters
    double defaultDuration = -1.;
    if(!_info.absoluteTime){
        defaultDuration = 1.;
        Param duration = { Param::DOUBLE, "duration", "", "", defaultDuration, 0.1, ONE_DAY };
        _info.params.append(duration);
    }
    Param trigger = { Param::BOOL, "trigger", "", "", true, 0, 0 };
    _info.params.append(trigger);
    Param kptrigger = { Param::BOOL, "kptrigger", "", "", false, 0, 0 };
    _info.params.append(kptrigger);
    if(_info.absoluteTime || !_info.repeat)
        _info.preempt = false;
    Param kpmode = { Param::LONG, "kpmode", "", "", 1, 0, 0 };
    if(_info.kpMode)
        kpmode.defaultValue = 0;
    _info.params.append(kpmode);
    Param delay = { Param::DOUBLE, "delay", "", "", 0., 0., ONE_DAY };
    Param kpdelay = { Param::DOUBLE, "kpdelay", "", "", 0., 0., ONE_DAY };
    _info.params.append(delay);
    _info.params.append(kpdelay);
    Param kpmodestop = { Param::BOOL, "kpmodestop", "", "", false, 0, 0 };
    Param kprelease = { Param::BOOL, "kprelease", "", "", false, 0, 0 };
    _info.params.append(kpmodestop);
    _info.params.append(kprelease);
    if(_info.repeat){
        Param repeat = { Param::DOUBLE, "repeat", "", "", defaultDuration, 0.1, ONE_DAY };
        Param kprepeat = { Param::DOUBLE, "kprepeat", "", "", defaultDuration, 0.1, ONE_DAY };
        // When repeats are enabled, stop and kpstop are LONG values (number of repeats)
        Param stop = { Param::LONG, "stop", "", "", -1, 0, 1000 };
        Param kpstop = { Param::LONG, "kpstop", "", "", 0, 0, 1000 };
        _info.params.append(repeat);
        _info.params.append(kprepeat);
        _info.params.append(stop);
        _info.params.append(kpstop);
    } else {
        // When repeats are disabled, stop and kpstop are DOUBLE values (seconds)
        Param stop = { Param::DOUBLE, "stop", "", "", -1., 0.1, ONE_DAY };
        Param kpstop = { Param::DOUBLE, "kpstop", "", "", -1., 0.1, ONE_DAY };
        _info.params.append(stop);
        _info.params.append(kpstop);
    }
    return true;
}

void AnimScript::init(const KeyMap& map, const QStringList& keys, const QMap<QString, QVariant>& paramValues){
    if(_path == "")
        return;
    end();
    _map = map;
    _colors.init(map);
    _colorBuffer.init(map);
    _keys = keys;
    _paramValues = paramValues;
    setDuration();
    stopped = firstFrame = false;
    initialized = true;
}

void AnimScript::setDuration(){
    if(_info.absoluteTime){
        durationMsec = 1000;
        repeatMsec = 0;
    } else {
        durationMsec = round(_paramValues.value("duration").toDouble() * 1000.);
        if(durationMsec <= 0)
            durationMsec = -1;
        repeatMsec = round(_paramValues.value("repeat").toDouble() * 1000.);
    }
}

void AnimScript::parameters(const QMap<QString, QVariant>& paramValues){
    if(!initialized || !process || !_info.liveParams)
        return;
    _paramValues = paramValues;
    setDuration();
    printParams();
}

void AnimScript::printParams(){
    process->write("begin params\n");
    QMapIterator<QString, QVariant> i(_paramValues);
    while(i.hasNext()){
        i.next();
        process->write("param ");
        process->write(i.key().toLatin1());
        process->write(" ");
        process->write(QUrl::toPercentEncoding(i.value().toString()));
        process->write("\n");
    }
    process->write("end params\n");
}

void AnimScript::begin(quint64 timestamp){
    if(!initialized)
        return;
    end();
    stopped = firstFrame = readFrame = readAnyFrame = inFrame = false;
    // Determine the upper left corner of the given keys
    QStringList keysCopy = _keys;
    minX = INT_MAX;
    minY = INT_MAX;
    foreach(const QString& key, keysCopy){
        const Key& pos = _map.key(key);
        if(!pos){
            keysCopy.removeAll(key);
            continue;
        }
        if(pos.x < minX)
            minX = pos.x;
        if(pos.y < minY)
            minY = pos.y;
    }
    if(keysCopy.isEmpty()){
        // If the key list is empty, don't actually start the animation but pretend it's running anyway
        firstFrame = readFrame = readAnyFrame = true;
        return;
    }
    process = new QProcess(this);
    connect(process, SIGNAL(readyRead()), this, SLOT(readProcess()));
    process->start(_path, QStringList("--ckb-run"));
    qDebug() << "Starting " << _path;
    // Write the keymap to the process
    process->write("begin keymap\n");
    process->write(QString("keycount %1\n").arg(keysCopy.count()).toLatin1());
    foreach(const QString& key, keysCopy){
        const Key& pos = _map.key(key);
        process->write(QString("key %1 %2,%3\n").arg(key).arg(pos.x - minX).arg(pos.y - minY).toLatin1());
    }
    process->write("end keymap\n");
    // Write parameters
    printParams();
    // Begin animating
    process->write("begin run\n");
    lastFrame = timestamp;
}

void AnimScript::retrigger(quint64 timestamp, bool allowPreempt){
    if(!initialized)
        return;
    if(allowPreempt && _info.preempt && repeatMsec > 0)
        // If preemption is wanted, trigger the animation 1 duration in the past first
        retrigger(timestamp - repeatMsec);
    if(!process)
        begin(timestamp);
    advance(timestamp);
    if(process)
        process->write("start\n");
}

void AnimScript::stop(quint64 timestamp){
    if(!initialized)
        return;
    if(!process)
        begin(timestamp);
    advance(timestamp);
    if(process)
        process->write("stop\n");
}

void AnimScript::keypress(const QString& key, bool pressed, quint64 timestamp){
    if(!initialized)
        return;
    if(!process)
        begin(timestamp);
    int kpMode = _info.kpMode;
    if(_paramValues.value("kpmode", 0).toInt() != 0)
        // Disable KP mode according to user preferences
        kpMode = KP_NONE;
    switch(kpMode){
    case KP_NONE:
        // If KPs aren't allowed, call retrigger/stop instead
        if(pressed)
            retrigger(timestamp);
        else if(_paramValues.value("kprelease", false).toBool())
            stop(timestamp);
        break;
    case KP_NAME:
        // Print keypress by name
        advance(timestamp);
        process->write(("key " + key + (pressed ? " down\n" : " up\n")).toLatin1());
        break;
    case KP_POSITION:
        // Print keypress by position
        const Key& kp = _map.key(key);
        if(!kp)
            return;
        advance(timestamp);
        process->write(("key " + QString("%1,%2").arg(kp.x - minX).arg(kp.y - minY) + (pressed ? " down\n" : " up\n")).toLatin1());
        break;
    }
}

void AnimScript::end(){
    _colors.clear();
    if(process){
        process->kill();
        connect(process, SIGNAL(finished(int)), process, SLOT(deleteLater()));
        disconnect(process, SIGNAL(readyRead()), this, SLOT(readProcess()));
        process = 0;
    }
}

void AnimScript::readProcess(){
    while(process->canReadLine()){
        QByteArray line = process->readLine().trimmed();
        if(!inFrame){
            // Ignore anything not between "begin frame" and "end frame", except for "end run", which indicates that the program is done.
            if(line == "begin frame")
                inFrame = true;
            else if(line == "end run"){
                stopped = true;
                return;
            }
            continue;
        }
        if(line.startsWith("argb ")){
            // Add a color to the buffer
            char keyName[31];
            QRgb keyColor = 0;
            if(sscanf(line, "argb %30s %x", keyName, &keyColor) != 2)
                continue;
            QRgb* inMap = _colorBuffer.colorForName(keyName);
            if(!inMap)
                continue;
            *inMap = keyColor;
        }
        if(line == "end frame"){
            // Frame is finished. Copy color buffer back to the atomic map
            memcpy(_colors.colors(), _colorBuffer.colors(), sizeof(QRgb) * _colors.count());
            inFrame = false;
            readFrame = readAnyFrame = true;
        }
    }
}

void AnimScript::frame(quint64 timestamp){
    if(!initialized || stopped)
        return;
    // Start the animation if it's not running yet
    if(!process)
        begin(timestamp);

    advance(timestamp);
    if((readFrame || !firstFrame) && process)
        // Don't ask for a new frame if the animation hasn't delivered the last one yet
        process->write("frame\n");
    firstFrame = true;
    readFrame = false;
}

void AnimScript::advance(quint64 timestamp){
    if(timestamp <= lastFrame || !process)
        // Don't do anything if the time hasn't actually advanced.
        return;
    double delta = (timestamp - lastFrame) / (double)durationMsec;
    if(!_info.absoluteTime){
        // Skip any complete durations
        while(delta > 1.){
            process->write("time 1\n");
            delta--;
        }
    }
    process->write(QString("time %1\n").arg(delta).toLatin1());
    lastFrame = timestamp;
}

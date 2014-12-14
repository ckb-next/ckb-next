#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QUrl>
#include "animscript.h"

QHash<QUuid, AnimScript*> AnimScript::scripts;

AnimScript::AnimScript(QObject* parent, const QString& path) :
    QObject(parent), _path(path), initialized(false)
{
}

AnimScript::AnimScript(QObject* parent, const AnimScript& base) :
    QObject(parent), _guid(base._guid), _name(base._name), _version(base._version), _copyright(base._copyright), _license(base._license), _path(base._path), _currentPos(0.), initialized(false)
{
}

AnimScript::~AnimScript(){
    stop();
}

QString AnimScript::path(){
#ifdef __APPLE
    return QDir(QApplication::applicationDirPath() + "../../../../").absoluteFilePath("animations");
#else
    return QDir().absoluteFilePath("animations");
#endif
}

void AnimScript::scan(){
    QDir dir(path());
    foreach(AnimScript* script, scripts)
        delete script;
    scripts.clear();
    foreach(QString file, dir.entryList(QDir::Files | QDir::Executable)){
        AnimScript* script = new AnimScript(0, dir.absoluteFilePath(file));
        if(script->load() && !scripts.contains(script->_guid))
            scripts[script->_guid] = script;
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
            last->_name += " " + last->guid().toString().toUpper();
            script->_name += " " + script->guid().toString().toUpper();
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

bool AnimScript::load(){
    // Run the process to get script info
    QProcess infoProcess;
    infoProcess.start(_path, QStringList("--ckb-info"));
    infoProcess.waitForFinished(100);
    if(infoProcess.state() == QProcess::Running)
        infoProcess.kill();
    // Read output
    QString line;
    while((line = infoProcess.readLine()) != ""){
        line = line.trimmed();
        QStringList components = line.split(" ");
        if(components.count() < 2)
            continue;
        QString param = components[0].trimmed();
        if(param == "guid")
            _guid = QUuid(urlParam(components[1]));
        else if(param == "name")
            _name = urlParam(components[1]);
        else if(param == "version")
            _version = "v" + urlParam(components[1]);
        else if(param == "copyright")
            _copyright = "Copyright © " + urlParam(components[1]);
        else if(param == "license")
            _license = urlParam(components[1]);
    }
    // Make sure the required parameters are filled out
    if(_guid.isNull() || _name == "" || _version == "" || _copyright == "" || _license == "")
        return false;
    return true;
}

void AnimScript::init(const KeyMap& map, const QStringList& keys, double duration){
    if(_path == "")
        return;
    stop();
    _map = map;
    _keys = keys;
    _duration = duration * 1000.;
    initialized = true;
    stopped = false;
    clearNext = true;
}

void AnimScript::start(){
    if(!initialized || process.state() != QProcess::NotRunning)
        return;
    process.start(_path, QStringList("--ckb-run"));
    if(!process.waitForStarted(100))
        return;
    // Determine the upper left corner of the given keys
    QStringList keysCopy = _keys;
    int minX = INT_MAX, minY = INT_MAX;
    foreach(QString key, _keys){
        const KeyPos* pos = _map.key(key);
        if(!pos){
            keysCopy.removeAll(key);
            continue;
        }
        if(pos->x < minX)
            minX = pos->x;
        if(pos->y < minY)
            minY = pos->y;
    }
    // Write the keymap to the process
    process.write("begin keymap\n");
    process.write(QString("keycount %1\n").arg(keysCopy.count()).toLatin1());
    foreach(QString key, keysCopy){
        const KeyPos* pos = _map.key(key);
        process.write(QString("key %1 %2,%3\n").arg(key).arg(pos->x - minX).arg(pos->y - minY).toLatin1());
    }
    process.write("end keymap\n");
    process.write("begin run\n");
    process.write("start\n");
    process.write(QString("frame %1\n").arg(_currentPos).toLatin1());
    lastFrame = QDateTime::currentMSecsSinceEpoch();
}

void AnimScript::retrigger(){
    if(!initialized)
        return;
    if(process.state() == QProcess::NotRunning)
        start();
    else
        process.write("start\n");
}

void AnimScript::stop(){
    if(!initialized || process.state() == QProcess::NotRunning)
        return;
    process.write("end run\n");
    if(!process.waitForFinished(100))
        process.kill();
    clearNext = true;
}

void AnimScript::frame(){
    if(!initialized || stopped)
        return;
    if(process.state() == QProcess::NotRunning)
        start();
    bool skipped = true;
    // Buffer the current output.
    while(process.canReadLine()){
        QString line = process.readLine().trimmed();
        if(inputBuffer.length() == 0 && line != "begin frame"){
            // Ignore anything not between "begin frame" and "end frame", except for "end run", which indicates that the program is done.
            if(line == "end run"){
                stopped = true;
                return;
            }
            continue;
        }
        if(line == "end frame"){
            // Process this frame
            skipped = false;
            if(clearNext){
                _colors.clear();
                clearNext = false;
            }
            foreach(QString input, inputBuffer){
                QStringList split = input.split(" ");
                if(split.length() != 3 || split[0] != "argb")
                    continue;
                _colors[split[1]] = split[2].toUInt(0, 16);
            }
            inputBuffer.clear();
            continue;
        }
        inputBuffer += line;
    }
    // If at least one frame was read, advance the animation
    if(!skipped){
        quint64 nextFrame = QDateTime::currentMSecsSinceEpoch();
        double delta = (double)(nextFrame - lastFrame) / _duration;
        if(delta > 1.)
            delta = 1.;
        else if(delta < 0.)
            delta = 0.;
        _currentPos += delta;
        if(_currentPos >= 1.)
            _currentPos -= 1.;
        lastFrame = nextFrame;
        process.write(QString("frame %1\n").arg(delta).toLatin1());
    }

}

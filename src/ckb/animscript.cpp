#include <cmath>
#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QUrl>
#include "animscript.h"

QHash<QUuid, AnimScript*> AnimScript::scripts;

AnimScript::AnimScript(QObject* parent, const QString& path) :
    QObject(parent), _path(path), initialized(false)
{
}

AnimScript::AnimScript(QObject* parent, const AnimScript& base) :
    QObject(parent), _guid(base._guid), _name(base._name), _version(base._version), _year(base._year), _author(base._author), _license(base._license), _description(base._description), _path(base._path), _params(base._params), initialized(false), process(0)
{
}

AnimScript::~AnimScript(){
    if(process){
        process->kill();
        process->waitForFinished(1000);
    }
}

QString AnimScript::path(){
#ifdef __APPLE__
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
    qDebug() << "Scanning " << _path;
    infoProcess.waitForFinished(1000);
    if(infoProcess.state() == QProcess::Running){
        // Kill the process if it takes more than 1s
        infoProcess.kill();
        return false;
    }
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
            _version = urlParam(components[1]);
        else if(param == "year")
            _year = urlParam(components[1]);
        else if(param == "author")
            _author = urlParam(components[1]);
        else if(param == "license")
            _license = urlParam(components[1]);
        else if(param == "description")
            _description = urlParam(components[1]);
        else if(param == "param"){
            // Read parameter
            if(components.count() < 3)
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
            else if(sType == "string")
                type = Param::STRING;
            else
                continue;
            // "param <type> <name> <prefix> <postfix> <default>"
            QString name = components[2].toLower();
            // Make sure it's not present already
            if(hasParam(name))
                continue;
            QString prefix = urlParam(components[3]), postfix = urlParam(components[4]);
            QVariant def = urlParam(components[5]), minimum = urlParam(components[6]), maximum = urlParam(components[7]);
            // Check types of predefined parameters
            if((name == "trigger" || name == "kptrigger") && type != Param::BOOL)
                continue;
            else if((name == "duration" || name == "repeat" || name == "kprepeat") && type != Param::DOUBLE)
                continue;
            Param param = { type, name, prefix, postfix, def, minimum, maximum };
            _params.append(param);
        }
    }
    // Make sure the required parameters are filled out
    if(_guid.isNull() || _name == "" || _version == "" || _year == "" || _author == "" || _license == "")
        return false;
    return true;
}

void AnimScript::init(const KeyMap& map, const QStringList& keys, const QMap<QString, QVariant>& paramValues){
    if(_path == "")
        return;
    stop();
    _map = map;
    _keys = keys;
    _duration = paramValues.value("duration").toDouble() * 1000.;
    if(_duration <= 0.)
        _duration = -1.;
    _paramValues = paramValues;
    stopped = firstFrame = false;
    initialized = true;
}

void AnimScript::start(){
    if(!initialized)
        return;
    stop();
    stopped = firstFrame = false;
    process = new QProcess(this);
    process->start(_path, QStringList("--ckb-run"));
    qDebug() << "Starting " << _path;
    // Determine the upper left corner of the given keys
    QStringList keysCopy = _keys;
    int minX = INT_MAX, minY = INT_MAX;
    foreach(const QString& key, _keys){
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
    process->write("begin keymap\n");
    process->write(QString("keycount %1\n").arg(keysCopy.count()).toLatin1());
    foreach(const QString& key, keysCopy){
        const KeyPos* pos = _map.key(key);
        process->write(QString("key %1 %2,%3\n").arg(key).arg(pos->x - minX).arg(pos->y - minY).toLatin1());
    }
    process->write("end keymap\n");
    // Write parameters
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
    // Begin animating
    process->write("begin run\n");
    lastFrame = QDateTime::currentMSecsSinceEpoch();
}

void AnimScript::retrigger(){
    if(!initialized)
        return;
    if(!process)
        start();
    process->write("start\n");
    _frame(false);
}

void AnimScript::keypress(const QString& key, bool pressed){
    if(!initialized)
        return;
    if(!process)
        start();
    process->write(("key " + key + (pressed ? " down\n" : " up\n")).toLatin1());
    _frame(false);
}

void AnimScript::stop(){
    _colors.clear();
    if(process){
        process->kill();
        connect(process, SIGNAL(finished(int)), process, SLOT(deleteLater()));
        process = 0;
    }
}

void AnimScript::_frame(bool parseOutput){
    if(!initialized || stopped)
        return;
    // Start the animation if it's not running yet
    if(!process)
        start();

    // Buffer the current output.
    bool skipped = true;
    if(parseOutput){
        while(process->canReadLine()){
            QString line = process->readLine().trimmed();
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
    }
    // If at least one frame was read, advance the animation
    if(!skipped || !firstFrame){
        quint64 nextFrame = QDateTime::currentMSecsSinceEpoch();
        double delta = (double)(nextFrame - lastFrame) / _duration;
        if(delta > 1.)
            delta = 1.;
        else if(delta < 0.)
            delta = 0.;
        lastFrame = nextFrame;
        process->write(QString("frame %1\n").arg(delta).toLatin1());
        firstFrame = true;
    }

}

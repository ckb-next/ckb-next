#ifndef ANIMSCRIPT_H
#define ANIMSCRIPT_H

#include <QHash>
#include <QObject>
#include <QMap>
#include <QProcess>
#include <QUuid>
#include "keymap.h"

class AnimScript : public QObject
{
    Q_OBJECT
public:
    static QString path();
    static void scan();
    static inline int count() { return scripts.count(); }
    static QList<const AnimScript*> list();

    static AnimScript* copy(QObject* parent, const QUuid& id);

    void init(const KeyMap& map, const QStringList& keys, double duration);
    void retrigger();
    void frame();
    void stop();
    inline void resetPosition() { _currentPos = 0.; }
    const QHash<QString, QRgb>& colors() const { return _colors; }

    inline const QUuid& guid() const { return _guid; }
    inline const QString& name() const { return _name; }
    inline const QString& version() const { return _version; }
    inline const QString& copyright() const { return _copyright; }
    inline const QString& license() const { return _license; }

    ~AnimScript();
private:
    bool load();

    QUuid _guid;
    QString _name;
    QString _version;
    QString _copyright;
    QString _license;

    QString _path;

    KeyMap _map;
    QStringList _keys;
    QHash<QString, QRgb> _colors;

    quint64 lastFrame;
    double _duration;
    double _currentPos;

    bool initialized, stopped, clearNext;
    QProcess process;
    QStringList inputBuffer;
    void start();

    static QHash<QUuid, AnimScript*> scripts;

    AnimScript(QObject* parent, const QString& path);
    AnimScript(QObject* parent, const AnimScript& base);
};

#endif // ANIMSCRIPT_H

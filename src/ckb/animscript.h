#ifndef ANIMSCRIPT_H
#define ANIMSCRIPT_H

#include <QHash>
#include <QObject>
#include <QMap>
#include <QProcess>
#include <QUuid>
#include <QVariant>
#include "keymap.h"

class AnimScript : public QObject
{
    Q_OBJECT
public:
    // Animation parameters
    struct Param {
        // Parameter type
        enum Type {
            INVALID = -1,
            LONG,
            DOUBLE,
            BOOL,
            RGB,
            ARGB,
            GRADIENT,
            AGRADIENT,
            STRING
        };
        Type type;
        // Internal name
        QString name;
        // Parameter friendly name/description
        QString prefix;
        QString postfix;
        // Default value (if any)
        QVariant defaultValue;
        // Minimum and maximum values (apply to int and double)
        QVariant minimum;
        QVariant maximum;
    };

    // Global animation path
    static QString path();
    // Scan the animation path for scripts
    static void scan();
    // Loaded script count and alphabetical list
    static inline int count() { return scripts.count(); }
    static QList<const AnimScript*> list();

    // Script properties
    inline const QUuid& guid() const { return _guid; }
    inline const QString& name() const { return _name; }
    inline const QString& version() const { return _version; }
    inline QString copyright() const { return "Copyright © " + _year + " " + _author; }
    inline const QString& year() const { return _year; }
    inline const QString& author() const { return _author; }
    inline const QString& license() const { return _license; }
    inline const QString& description() const { return _description; }

    // Parameters, in the order they were given
    inline QListIterator<Param> paramIterator() const { return _params; }
    inline Param param(const QString& name) const { QListIterator<Param> i(_params); while(i.hasNext()){ Param p = i.next(); if(p.name == name) return p; } return Param(); }
    inline bool hasParam(const QString& name) const { QListIterator<Param> i(_params); while(i.hasNext()){ if(i.next().name == name) return true; } return false; }

    // Creates a usable script object with the given parent object. Returns null if no such script exists.
    static AnimScript* copy(QObject* parent, const QUuid& id);

    // Initializes or re-initializes a script. Must be called at least once.
    // paramValues should contain parameter name/value pairs to run the script with.
    void init(const KeyMap& map, const QStringList& keys, const QMap<QString, QVariant>& paramValues);
    // Starts or restarts the animation.
    void retrigger();
    // Triggers a keypress event.
    void keypress(const QString& key, bool pressed);
    void keypress(int x, int y, bool pressed);
    // Executes the next frame of the animation.
    inline void frame() { _frame(true); }
    // Stops the animation.
    void stop();

    // Colors returned from the last executed frame.
    const QHash<QString, QRgb>& colors() const { return _colors; }

    ~AnimScript();
private:
    bool load();

    QUuid _guid;
    QString _name;
    QString _version;
    QString _year;
    QString _author;
    QString _license;
    QString _description;

    QString _path;

    KeyMap _map;
    QStringList _keys;
    QHash<QString, QRgb> _colors;
    QList<Param> _params;
    QMap<QString, QVariant> _paramValues;

    quint64 lastFrame;
    double _duration;

    bool initialized, firstFrame, stopped;
    QProcess* process;
    QStringList inputBuffer;
    void start();
    void _frame(bool parseOutput);

    static QHash<QUuid, AnimScript*> scripts;

    AnimScript(QObject* parent, const QString& path);
    AnimScript(QObject* parent, const AnimScript& base);
};

#endif // ANIMSCRIPT_H

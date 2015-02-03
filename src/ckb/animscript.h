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
            ANGLE,
            STRING,
            LABEL
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
    inline const QUuid& guid() const { return _info.guid; }
    inline const QString& name() const { return _info.name; }
    inline const QString& version() const { return _info.version; }
    inline QString copyright() const { return "Copyright © " + _info.year + " " + _info.author; }
    inline const QString& year() const { return _info.year; }
    inline const QString& author() const { return _info.author; }
    inline const QString& license() const { return _info.license; }
    inline const QString& description() const { return _info.description; }

    // Parameters, in the order they were given
    inline QListIterator<Param> paramIterator() const { return _info.params; }
    inline Param param(const QString& name) const { QListIterator<Param> i(_info.params); while(i.hasNext()){ Param p = i.next(); if(p.name == name) return p; } return ((Param[]){ { Param::INVALID, "", "", "", 0, 0, 0 } })[0]; }
    inline bool hasParam(const QString& name) const { QListIterator<Param> i(_info.params); while(i.hasNext()){ if(i.next().name == name) return true; } return false; }

    // Creates a usable script object with the given parent object. Returns null if no such script exists.
    static AnimScript* copy(QObject* parent, const QUuid& id);

    // Initializes or re-initializes a script. Must be called at least once.
    // paramValues should contain parameter name/value pairs to run the script with.
    void init(const KeyMap& map, const QStringList& keys, const QMap<QString, QVariant>& paramValues);
    // Updates parameters without a complete re-initialization. Does nothing if the animation does not support live params.
    void parameters(const QMap<QString, QVariant>& paramValues);
    // Starts or restarts the animation.
    void retrigger(quint64 timestamp, bool allowPreempt = false);
    // Triggers a keypress event.
    void keypress(const QString& key, bool pressed, quint64 timestamp);
    // Executes the next frame of the animation.
    void frame(quint64 timestamp);
    // Stops the animation.
    void stop();

    // Whether or not the animation has processed any frames yet.
    inline bool hasFrame() const { return initialized && readAnyFrame; }

    // Colors returned from the last executed frame.
    const QHash<QString, QRgb>& colors() const { return _colors; }

    ~AnimScript();

private slots:
    void readProcess();

private:
    bool load();

    // Basic info
    struct {
        QUuid guid;
        QString name;
        QString version;
        QString year;
        QString author;
        QString license;
        QString description;
        // Parameter list
        QList<Param> params;
        // Playback flags
        int kpMode :3;
        bool absoluteTime :1, repeat :1, preempt :1, liveParams :1;
    } _info;
    const static int KP_NONE = 0, KP_NAME = 1, KP_POSITION = 2;
    // Script path
    QString _path;
    // Key map (positions)
    KeyMap _map;
    int minX, minY;
    // Keys in use
    QStringList _keys;
    // Current colors
    QHash<QString, QRgb> _colors;
    QMap<QString, QVariant> _paramValues;

    // Animation state
    quint64 lastFrame;
    int durationMsec;
    bool initialized :1, firstFrame :1, readFrame :1, readAnyFrame :1, stopped :1;
    QProcess* process;
    QStringList inputBuffer;

    // Helper functions
    void setDuration();
    void printParams();
    void start(quint64 timestamp);
    void nextFrame(quint64 timestamp);

    // Global script list
    static QHash<QUuid, AnimScript*> scripts;

    AnimScript(QObject* parent, const QString& path);
    AnimScript(QObject* parent, const AnimScript& base);
};

#endif // ANIMSCRIPT_H

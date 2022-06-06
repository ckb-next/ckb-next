#ifndef KEYACTION_H
#define KEYACTION_H

#include <QObject>
#include <QProcess>
#include "keymap.h"

class KbBind;
class KbAnim;

// Class for managing an action associated with a keypress

class KeyAction : public QObject
{
    Q_OBJECT
public:
    // Action/string conversion
    KeyAction(const QString& action, QObject* parent = nullptr);
    inline QString  value()     const { return _value; }
    inline operator QString ()  const { return _value; }
    // Empty action
    explicit KeyAction(QObject* parent = nullptr);

    // No action
    static inline QString   noAction()    { return QString(); }
    // Default action (usually the same as the key name, but not always)
    static QString          defaultAction(const QString& key, KeyMap::Model model);

    // Friendly action name
    QString friendlyName(const KeyMap& map) const;
    // Name to send to driver (empty string for unbind)
    QString driverName() const;

    // Returns the string that's passed to the daemon
    inline QString macroContent() const {
        return _value.split(":")[1];
    }

    // Mode-switch action.
    // 0 for first mode, 1 for second, etc. Constants below for movement options
    const static int MODE_PREV = -2, MODE_NEXT = -1;
    const static int MODE_PREV_WRAP = -4, MODE_NEXT_WRAP = -3;
    static QString  modeAction(int mode);
    // DPI action. 0 for sniper, 1 for first DPI, etc
    const static int DPI_CYCLE_UP = -4, DPI_CYCLE_DOWN = -3;
    const static int DPI_UP = -2, DPI_DOWN = -1;
    const static int DPI_SNIPER = 0, DPI_CUSTOM = 6;
    static QString  dpiAction(int level, int customX = 0, int customY = 0);
    // Brightness control
    const static int LIGHT_UP = 0, LIGHT_DOWN = 1;
    const static int LIGHT_UP_WRAP = 2, LIGHT_DOWN_WRAP = 3;
    static QString  lightAction(int type = LIGHT_UP_WRAP);
    // Win lock control
    const static int LOCK_TOGGLE = 0, LOCK_ON = 1, LOCK_OFF = 2;
    static QString  lockAction(int type = LOCK_TOGGLE);
    // Key to launch a program. stop should be (<press stop> | <release stop>)
    static const int PROGRAM_PR_MULTI = 0x04, PROGRAM_PR_INDEF = 0x00, PROGRAM_PR_KRSTOP = 0x01, PROGRAM_PR_KPSTOP = 0x02;
    static const int PROGRAM_RE_MULTI = 0x40, PROGRAM_RE_INDEF = 0x00, PROGRAM_RE_KPSTOP = 0x20;
    static QString  programAction(const QString& onPress, const QString& onRelease, int stop);
    // Key to start an animation
    static QString animAction(const QUuid& guid, bool onlyOnce, bool stopOnRelease);
    static QString macroAction(const QString& macroDef);

    // Action type
    enum Type {
        UNBOUND,
        NORMAL,
        SPECIAL,
    };
    Type type() const;
    inline bool isUnbound() const       { return type() == UNBOUND; }
    inline bool isNormal() const        { return type() == NORMAL; }
    inline bool isSpecial() const       { return type() == SPECIAL; }
    // Media is a type of normal key
    inline bool isMedia() const         { return _value == "mute" || _value == "volup" || _value == "voldn" || _value == "stop" || _value == "prev" || _value == "play" || _value == "next"; }
    // Macro, program and animation are types of special key
    inline bool isProgram() const       { return _value.startsWith("$program:"); }
    inline bool isAnim() const          { return _value.startsWith("$anim:"); }
    inline bool isMacro() const         { return _value.startsWith("$macro:"); }
    // Mouse is some normal keys plus DPI
    inline bool isDPI() const           { return _value.startsWith("$dpi:"); }
    inline bool isMouse() const         { return (isNormal() && (_value.startsWith("mouse") || _value.startsWith("wheel"))) || isDPI(); }

    // Splits a special action into action and parameter.
    QString specialInfo(int& parameter)                         const;
    // Get program key info (onPress, onRelease = programs, return = stop)
    int     programInfo(QString& onPress, QString& onRelease)   const;
    // Get DPI info. custom is only set if return == DPI_CUSTOM.
    int     dpiInfo(QPoint& custom)                             const;
    // Get animation info.
    QUuid   animInfo(bool& onlyOnce, bool& stopOnRelease)       const;

    // Perform keydown action (if any)
    void keyEvent(KbBind* bind, bool down);
    // Perform keyup action (if any)
    void keyRelease(KbBind* bind);
    // Adjusts the DISPLAY variable to the mouse's screen. Needed to ensure that programs launch on the correct screen in multihead.
    void adjustDisplay();

    ~KeyAction();
private:
    /// ccMSC: Don't copy key actions (the old one needs to be deleted first)
    /// frickler24: statement left as described, but copying is done in KbBind copy constructor
    inline void operator=(const KeyAction& rhs) {}
    inline KeyAction(const KeyAction& rhs) : QObject() {}

    QString _value;

    // Currently-running programs
    QProcess* preProgram;
    QProcess* relProgram;

    // Mouse sniper mode (0 = inactive)
    quint64 sniperValue;
};

#endif // KEYACTION_H

#ifndef KBLIGHT_H
#define KBLIGHT_H

#include <QFile>
#include <QObject>
#include <QSettings>
#include "keymap.h"

class KbLight : public QObject
{
    Q_OBJECT
public:
    KbLight(QObject* parent, int modeIndex, const KeyMap& map);

    inline KeyMap& map() { return _map; }
    inline void map(const KeyMap& map) { _map = map; }

    inline const QColor& fgColor() { return _fgColor; }
    inline void fgColor(const QColor& color) { _fgColor = color; }

    inline int modeIndex() { return _modeIndex - 1; }
    inline void modeIndex(int index) { _modeIndex = index + 1; }

    static const int MAX_BRIGHTNESS = 3;
    inline int brightness() { return _brightness; }
    inline void brightness(int bright) { _brightness = bright; }

    static const int MAX_INACTIVE = 2;
    inline int inactive() { return _inactive; }
    inline void inactive(int in) { _inactive = in; }

    inline int animation() { return _animation; }
    inline void animation(int anim) { _animation = anim; animPos = -36.f; }

    inline bool winLock() { return _winLock; }
    void winLock(QFile& cmd, bool lock);

    inline QStringList animated() { return _animated; }
    inline void animated(const QStringList& newAnimated) { _animated = newAnimated; }

    void frameUpdate(QFile& cmd, bool dimMute);
    void close(QFile& cmd);

    void load(QSettings& settings);
    void save(QSettings& settings);

private:
    QColor _fgColor;
    KeyMap _map;
    int _modeIndex;
    int _brightness;
    int _inactive;
    int _animation;
    QStringList _animated;
    bool _winLock;

    float animPos;

    void animWave(const QStringList& keys, KeyMap& colorMap);
    void animRipple(const QStringList& keys, KeyMap& colorMap);

    void printRGB(QFile& cmd, KeyMap& colorMap);
};

#endif // KBLIGHT_H

#ifndef KBLIGHT_H
#define KBLIGHT_H

#include <QFile>
#include <QObject>
#include <QSettings>
#include "animscript.h"
#include "kbanim.h"
#include "keymap.h"

class KbLight : public QObject
{
    Q_OBJECT
public:
    KbLight(QObject* parent, int modeIndex, const KeyMap& map);

    inline const KeyMap& map() { return _map; }
    void map(const KeyMap& map);
    inline void color(const QString& key, const QColor& newColor) { _map.color(key, newColor); }
    inline void colorAll(const QColor& newColor) { _map.colorAll(newColor); }
    void layout(KeyMap::Layout newLayout);

    inline int modeIndex() { return _modeIndex - 1; }
    inline void modeIndex(int index) { _modeIndex = index + 1; }

    static const int MAX_BRIGHTNESS = 3;
    inline int brightness() { return _brightness; }
    inline void brightness(int bright) { _brightness = bright; }

    static const int MAX_INACTIVE = 2;
    inline int inactive() { return _inactive; }
    inline void inactive(int in) { _inactive = in; }

    inline bool winLock() { return _winLock; }
    void winLock(QFile& cmd, bool lock);

    inline bool showMute() { return _showMute; }
    inline void showMute(bool newShowMute) { _showMute = newShowMute; }

    KbAnim* addAnim(const AnimScript* base, const QStringList& keys);
    QList<KbAnim*> animList;

    void frameUpdate(QFile& cmd, bool dimMute);
    void close(QFile& cmd);

    void load(QSettings& settings);
    void save(QSettings& settings);

signals:
    void didLoad();

private:
    KeyMap _map;
    int _modeIndex;
    int _brightness;
    int _inactive;
    bool _winLock;
    bool _showMute;

    void animWave(const QStringList& keys, KeyMap& colorMap);
    void animRipple(const QStringList& keys, KeyMap& colorMap);

    void printRGB(QFile& cmd, KeyMap& colorMap);
};

#endif // KBLIGHT_H

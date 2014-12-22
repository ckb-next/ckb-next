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
    KbLight(QObject* parent, int modeIndex, const KeyMap& keyMap);

    // Key map
    inline const KeyMap& map() { return _map; }
    void map(const KeyMap& map);
    // Key -> color map
    inline const QHash<QString, QRgb>& colorMap() { return _colorMap; }
    // Color a key
    inline void color(const QString& key, const QColor& newColor) { _colorMap[key] = newColor.rgb(); }
    // Color all keys in the current map
    void color(const QColor& newColor);

    // Index of the mode that this belongs to (zero-indexed, converted to one-indexed internally)
    inline int modeIndex() { return _modeIndex - 1; }
    inline void modeIndex(int index) { _modeIndex = index + 1; }

    // Overally brightness. 0 = max, 3 = min
    static const int MAX_BRIGHTNESS = 3;
    inline int brightness() { return _brightness; }
    inline void brightness(int bright) { _brightness = bright; }

    // Inactive indicator level. -1 for no dimming, 2 for off
    static const int MAX_INACTIVE = 2;
    inline int inactive() { return _inactive; }
    inline void inactive(int in) { _inactive = in; }

    // Whether or not to indicate the mute key
    inline bool showMute() { return _showMute; }
    inline void showMute(bool newShowMute) { _showMute = newShowMute; }

    // Windows lock
    inline bool winLock() { return _winLock; }
    void winLock(QFile& cmd, bool lock);

    // Lighting animations
    KbAnim* addAnim(const AnimScript* base, const QStringList& keys);
    QList<KbAnim*> animList;
    // Sends a keypress event to active animations
    void animKeypress(const QString& keyEvent);

    // Start the mode
    void open();
    // Write a new frame to the keyboard
    void frameUpdate(QFile& cmd, bool dimMute);
    // Make the lighting idle, stopping any animations
    void close(QFile& cmd);

    // Load and save from stored settings
    void load(QSettings& settings);
    void save(QSettings& settings);

signals:
    void didLoad();

private:
    KeyMap _map;
    QHash<QString, QRgb> _colorMap;
    int _modeIndex;
    int _brightness;
    int _inactive;
    bool _winLock;
    bool _showMute;

    void printRGB(QFile& cmd, const QHash<QString, QRgb>& animMap);
};

#endif // KBLIGHT_H

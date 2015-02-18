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
    // New lighting setup
    KbLight(QObject* parent, const KeyMap& keyMap);
    // Copy a lighting setup
    KbLight(QObject *parent, const KeyMap& keyMap, const KbLight& other);

    // Key map
    inline const KeyMap& map() { return _map; }
    void map(const KeyMap& map);
    // Key -> color map
    inline const QHash<QString, QRgb>& colorMap() { return _colorMap; }
    // Color a key
    inline void color(const QString& key, const QColor& newColor) { _colorMap[key] = newColor.rgb(); }
    // Color all keys in the current map
    void color(const QColor& newColor);

    // Overall dimming. 0 = max brightness, 3 = off
    static const int MAX_DIM = 3;
    inline int dimming() { return _dimming; }
    inline void dimming(int newDimming) { _dimming = newDimming; emit updated(); }

    // Inactive indicator level. -1 for no dimming, 2 for off
    static const int MAX_INACTIVE = 2;
    inline int inactive() { return _inactive; }
    inline void inactive(int in) { _inactive = in; emit updated(); }

    // Whether or not to indicate the mute key
    inline bool showMute() { return _showMute; }
    inline void showMute(bool newShowMute) { _showMute = newShowMute; emit updated(); }

    // Lighting animations
    KbAnim* addAnim(const AnimScript* base, const QStringList& keys);
    KbAnim* duplicateAnim(KbAnim* oldAnim);
    QList<KbAnim*> animList;
    // Stops and restarts all animations
    void restartAnimation();
    // Sends a keypress event to active animations
    void animKeypress(const QString& key, bool down);

    // Start the mode
    void open();
    // Whether or not all animations have started
    bool isStarted();
    // Write a new frame to the keyboard.
    void frameUpdate(QFile& cmd, int modeIndex, bool dimMute, bool dimLock);
    // Make the lighting idle, stopping any animations.
    void close();
    // Write the mode's base colors without any animation
    void base(QFile& cmd, int modeIndex);

    // Load and save from stored settings
    void load(QSettings& settings);
    void save(QSettings& settings);

signals:
    void didLoad();
    void updated();

private:
    KeyMap _map;
    QHash<QString, QRgb> _colorMap;
    int _dimming;
    int _inactive;
    bool _showMute;
    bool _start;

    void printRGB(QFile& cmd, const QHash<QString, QRgb>& animMap);
};

#endif // KBLIGHT_H

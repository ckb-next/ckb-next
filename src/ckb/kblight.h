#ifndef KBLIGHT_H
#define KBLIGHT_H

#include <QFile>
#include <QObject>
#include <QSet>
#include <QSettings>
#include "animscript.h"
#include "kbanim.h"
#include "keymap.h"
#include "colormap.h"

class KbMode;

// Keyboard lighting setup

class KbLight : public QObject
{
    Q_OBJECT
public:
    // New lighting setup
    KbLight(KbMode* parent, const KeyMap& keyMap);
    // Copy a lighting setup
    KbLight(KbMode *parent, const KeyMap& keyMap, const KbLight& other);
    ~KbLight();

    // Key map
    inline const KeyMap&    map() { return _map; }
    void                    map(const KeyMap& map);
    // Key -> color map
    inline const QColorMap& colorMap() { return _qColorMap; }
    // Color a key
    void                    color(const QString& key, const QColor& newColor);
    // Color all keys in the current map
    void                    color(const QColor& newColor);

    // Overall dimming. 0 = max brightness, 3 = off
    static const int    MAX_DIM = 3;
    inline int          dimming()               { return _dimming; }
    void                dimming(int newDimming);
    // Shared brightness between all modes (-1 for sharing disabled)
    static int          shareDimming();
    static void         shareDimming(int newShareDimming);

    // Lighting animations
    typedef QList<KbAnim*> AnimList;
    KbAnim*             addAnim(const AnimScript* base, const QStringList& keys, const QString& name, const QMap<QString, QVariant>& preset);
    KbAnim*             duplicateAnim(KbAnim* oldAnim);
    const AnimList&     animList()                              { return _animList; }
    void                animList(const AnimList& newAnimList)   { _needsSave = true; _animList = newAnimList; }
    KbAnim*             findAnim(const QUuid& guid) const       { foreach(KbAnim* anim, _animList) { if(anim->guid() == guid) return anim; } return 0; }
    int                 findAnimIdx(const QUuid& guid) const    { return _animList.indexOf(findAnim(guid)); }
    // Preview animation - temporary animation displayed at the top of the animation list
    void previewAnim(const AnimScript* base, const QStringList& keys, const QMap<QString, QVariant>& preset);
    void stopPreview();
    // Stops and restarts all animations
    void restartAnimation();
    // Sends a keypress event to active animations
    void animKeypress(const QString& key, bool down);

    // Start the mode
    void open();
    // Whether or not all animations have started
    bool isStarted();
    // Make the lighting idle, stopping any animations.
    void close();

    // Reset indicator state
    void resetIndicators();
    // Set an indicator to a given ARGB value
    void setIndicator(const char* name, QRgb argb);

    // Write a new frame to the keyboard. Write "mode %d" first. Optionally provide a list of keys to use as indicators and overwrite the lighting
    void frameUpdate(QFile& cmd, bool monochrome = false);
    // Write the mode's base colors without any animation
    void base(QFile& cmd, bool ignoreDim = false, bool monochrome = false);

    // Load and save from stored settings
    void load(CkbSettings& settings);
    void save(CkbSettings& settings);
    bool needsSave() const;

signals:
    void didLoad();
    void updated();
    void frameDisplayed(const ColorMap& animatedColors, const QSet<QString>& indicatorList);

private:
    AnimList        _animList;
    KbAnim*         _previewAnim;
    KeyMap          _map;
    QColorMap       _qColorMap;
    ColorMap        _colorMap, _animMap, _indicatorMap;
    QSet<QString>   _indicatorList;
    quint64         lastFrameSignal;
    int             _dimming;
    bool            _start;
    bool            _needsSave, _needsMapRefresh;

    // Rebuild base ColorMap (if needed)
    void rebuildBaseMap();
    // Print RGB values to cmd node
    void printRGB(QFile& cmd, const ColorMap& animMap);
};

#endif // KBLIGHT_H

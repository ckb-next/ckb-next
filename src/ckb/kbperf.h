#ifndef KBPERF_H
#define KBPERF_H
#include <QFile>
#include <QMap>
#include <QPoint>
#include "ckbsettings.h"
#include "keymap.h"

class KbMode;
class KbBind;
class KbLight;

// DPI/performance settings. Also stores indicator colors. Created as part of KbMode.

class KbPerf : public QObject
{
    Q_OBJECT
public:
    // New setup with default settings
    explicit KbPerf(KbMode* parent);
    // Copy a setup
    KbPerf(KbMode* parent, const KbPerf& other);
    const KbPerf& operator= (const KbPerf& rhs);

    // Load and save from stored settings
    void        load(CkbSettings& settings);
    void        save(CkbSettings& settings);
    inline bool needsSave() const { return _needsSave; }

    // Mouse lift height
    enum height {
        LOW = 1,
        LOWMED,
        MEDIUM,
        MEDHIGH,
        HIGH
    };
    inline height   liftHeight() const              { return _liftHeight; }
    void            liftHeight(height newHeight);

    // Mouse angle snap
    inline bool angleSnap() const { return _angleSnap; }
    void        angleSnap(bool newAngleSnap);

    // Stored DPI settings (X/Y)
    const static int DPI_COUNT = 6, SNIPER = 0;
    const static int DPI_MIN = 100, DPI_MAX = 12000;
    inline QPoint   dpi(int index) const                    { if(index < 0 || index >= DPI_COUNT) return QPoint(); return QPoint(dpiX[index], dpiY[index]); }
    void            dpi(int index, const QPoint& newValue);
    inline QPoint   sniperDpi() const                       { return dpi(SNIPER); }
    inline void     sniperDpi(const QPoint& newValue)       { dpi(SNIPER, newValue); }
    // Set both X and Y
    inline void     dpi(int index, int newValue)    { dpi(index, QPoint(newValue, newValue)); }
    inline void     sniperDpi(int newValue)         { sniperDpi(QPoint(newValue, newValue)); }

    // Current DPI
    inline QPoint   curDpi() const                  { return QPoint(dpiCurX, dpiCurY); }
    void            curDpi(const QPoint& newDpi);
    inline void     curDpi(int newDpi)              { curDpi(QPoint(newDpi, newDpi)); }
    // DPI index (updated automatically by curDpi). -1 if custom.
    inline int      curDpiIdx() const                       { return dpiCurIdx; }
    inline void     curDpiIdx(int newIdx) { curDpi(dpi(newIdx)); }
    void            dpiUp();
    void            dpiDown();
    // DPI stages enabled (default all). Disabled stages will be bypassed when invoking dpiUp/dpiDown (but not any other functions).
    inline bool     dpiEnabled(int index) const             { return dpiOn[index]; }
    inline void     dpiEnabled(int index, bool newEnabled)  { if(index <= 0) return; dpiOn[index] = newEnabled; _needsUpdate = _needsSave = true; }
    // Push/pop a DPI state. Useful for toggling custom DPI. pushDpi returns an index which must be passed back to popDpi.
    // Note that calling curDpi will empty the stack, so any previously-pushed DPIs are automatically popped.
    quint64         pushDpi(const QPoint& newDpi);
    inline quint64  pushDpi(int newDpi)             { return pushDpi(QPoint(newDpi, newDpi)); }
    inline quint64  pushSniper()                    { return pushDpi(sniperDpi()); }
    void            popDpi(quint64 pushIdx);

    // Indicator opacity [0, 1]
    inline float    iOpacity() const                            { return _iOpacity; }
    inline void     iOpacity(float newIOpacity)                 { _iOpacity = newIOpacity; _needsSave = true; }
    // DPI indicator colors
    inline bool     dpiIndicator() const                        { return _dpiIndicator; }
    inline void     dpiIndicator(bool newDpiIndicator)          { _dpiIndicator = newDpiIndicator; _needsSave = true; }
    const static int OTHER = DPI_COUNT;     // valid only with dpiColor
    inline QColor   dpiColor(int index) const                   { return dpiClr[index]; }
    inline void     dpiColor(int index, const QColor& newColor) { dpiClr[index] = newColor; _needsUpdate = _needsSave = true; }
    // KB indicator colors
    enum indicator {
        // Hardware
        NUM,
        CAPS,
        SCROLL, HW_IMAX = SCROLL,
        // Software
        MODE,
        MACRO,
        LIGHT,
        LOCK,
        MUTE
    };
    // Hardware indicator state
    enum i_hw {
        NONE = -1,  // For non-hardware indicators
        NORMAL,
        ON,
        OFF
    };
    const static int I_COUNT = (int)MUTE + 1, HW_I_COUNT = (int)HW_IMAX + 1;
    // Indicator color and settings. For MUTE, color1 = on, color2 = off, color3 = don't know. For LIGHT, color1 = 33%, color2 = 67%, color3 = 100%.
    // For all others, color1 = on, color2 = off, color3 unused
    void getIndicator(indicator index, QColor& color1, QColor& color2, QColor& color3, bool& software_enable, i_hw& hardware_enable);
    void setIndicator(indicator index, const QColor& color1, const QColor& color2, const QColor& color3 = QColor(), bool software_enable = true, i_hw hardware_enable = NORMAL);

    // Updates settings to the driver. Write "mode %d" first. Disable saveCustomDpi when writing a hardware profile or other permanent storage.
    // By default, nothing will be written unless the settings have changed. Use force = true or call setNeedsUpdate() to override.
    void        update(QFile& cmd, bool force = false, bool saveCustomDpi = true);
    inline void setNeedsUpdate()        { _needsUpdate = true; }

    // Get indicator status to send to KbLight
    void applyIndicators(int modeIndex, const bool indicatorState[HW_I_COUNT]);

signals:
    void didLoad();
    void settingsUpdated();

private:
    // Related objects
    inline KbMode*  modeParent() const { return (KbMode*)parent(); }
    KbBind*         bind() const;
    KbLight*        light() const;

    // Send indicator state to KbLight, taking current opacity into account
    void lightIndicator(const char* name, QRgb rgba);

    // DPI
    int dpiX[DPI_COUNT];
    int dpiY[DPI_COUNT];
    int dpiCurX, dpiCurY, dpiCurIdx;
    QColor dpiClr[DPI_COUNT + 1];
    bool dpiOn[DPI_COUNT];
    // Last-set DPI that was on the DPI list, not counting any pushed DPIs or sniper.
    int dpiLastIdx;

    // Current DPI stack. If non-empty, pushedDpis[0] represents the last DPI set by curDpi.
    // (not necessarily the same as dpi(dpiLastIdx), since the last-set DPI might not have been on the DPI list)
    QMap<quint64, QPoint> pushedDpis;
    uint runningPushIdx;

    // Update DPI without popping stack
    void _curDpi(const QPoint& newDpi);

    // Indicators
    float _iOpacity;
    QColor iColor[I_COUNT][2];
    QColor light100Color, muteNAColor;
    bool iEnable[I_COUNT];
    i_hw hwIType[HW_I_COUNT];
    bool _dpiIndicator;

    // Mouse settings
    height _liftHeight;
    bool _angleSnap;
    // Misc
    bool _needsUpdate, _needsSave;
};

#endif // KBPERF_H

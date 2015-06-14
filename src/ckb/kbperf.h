#ifndef KBPERF_H
#define KBPERF_H
#include <QFile>
#include <QMap>
#include <QPoint>
#include "ckbsettings.h"
#include "keymap.h"

class KbMode;

// DPI/performance settings. Also stores indicator colors. Created as part of KbMode.

class KbPerf : public QObject
{
    Q_OBJECT
public:
    // New setup with default settings
    explicit KbPerf(KbMode* parent);
    // Copy a setup
    KbPerf(KbMode* parent, const KbPerf& other);

    // Load and save from stored settings
    void        load(CkbSettings& settings);
    void        save(CkbSettings& settings);
    inline bool needsSave() const { return _needsSave; }

    // Stored DPI settings (X/Y)
    const static int DPI_COUNT = 6, SNIPER = 0;
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
    inline int      curDpiIdx() const   { return dpiCurIdx; }
    inline void     curDpiIdx(int newIdx) { curDpi(dpi(newIdx)); }
    inline void     dpiUp()             { int idx = curDpiIdx() + 1; if(idx <= SNIPER) idx = 1; if(idx < DPI_COUNT) curDpiIdx(idx); }
    inline void     dpiDown()           { int idx = curDpiIdx() - 1; if(idx > SNIPER) curDpiIdx(idx); }
    // Push/pop a DPI state. Useful for toggling custom DPI. pushDpi returns an index which must be passed back to popDpi.
    // Note that calling curDpi will empty the stack, so any previously-pushed DPIs are automatically popped.
    quint64         pushDpi(const QPoint& newDpi);
    inline quint64  pushDpi(int newDpi)             { return pushDpi(QPoint(newDpi, newDpi)); }
    inline quint64  pushSniper()                    { return pushDpi(sniperDpi()); }
    void            popDpi(quint64 pushIdx);

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

    // Updates settings to the driver. Write "mode %d" first. Disable saveCustomDpi when writing a hardware profile or other permanent storage.
    // By default, nothing will be written unless the settings have changed. Use force = true to overwrite.
    void update(QFile& cmd, bool force = false, bool saveCustomDpi = true);

signals:
    void didLoad();
    void settingsUpdated();


private:
    int dpiX[DPI_COUNT];
    int dpiY[DPI_COUNT];
    int dpiCurX, dpiCurY, dpiCurIdx;
    // Last-set DPI that was on the DPI list, not counting any pushed DPIs or sniper.
    int dpiLastIdx;

    height _liftHeight;
    bool _angleSnap;
    bool _needsUpdate, _needsSave;

    // Current DPI stack. If non-empty, pushedDpis[0] represents the last DPI set by curDpi.
    // (not necessarily the same as dpi(dpiLastIdx), since the last-set DPI might not have been on the DPI list)
    QMap<quint64, QPoint> pushedDpis;
    uint runningPushIdx;

    // Update DPI without popping stack
    void _curDpi(const QPoint& newDpi);
};

#endif // KBPERF_H

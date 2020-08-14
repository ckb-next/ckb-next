#ifndef BATTERYSYSTEMTRAYICON_H
#define BATTERYSYSTEMTRAYICON_H
#include "ckbsystemtrayicon.h"

class BatteryStatusTrayIcon : public CkbSystemTrayIcon {
    Q_OBJECT

    public:
        BatteryStatusTrayIcon( QString _name, QObject* parent = 0 )
                             : CkbSystemTrayIcon(QIcon(), "", parent), name(_name) {}

        // Battery strings for indexes
        const static QString BATTERY_VALUES[5];
        const static QString BATTERY_CHARGING_VALUES[3];
    public slots:
        void setBattery(uint battery, uint charging);
    private:
        QString name;
};
#endif

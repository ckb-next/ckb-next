#ifndef BATTERYSYSTEMTRAYICON_H
#define BATTERYSYSTEMTRAYICON_H
#include "ckbsystemtrayicon.h"
#include <QAction>
#include <QMenu>

enum BatteryStatus {
    BATT_STATUS_UNKNOWN,
    BATT_STATUS_CHARGING,
    BATT_STATUS_DISCHARGING,
    BATT_STATUS_CHARGED,
    BATT_STATUS_INVALID, // Used for safety checks
};

class BatteryStatusTrayIcon : public CkbSystemTrayIcon {
    Q_OBJECT

    public:
        BatteryStatusTrayIcon( QString _name, QObject* parent = nullptr )
                             : CkbSystemTrayIcon(QIcon(":/img/ckb-next-monochrome.png"), "ckb-next-monochrome", parent), name(_name) {
            QMenu* menu = new QMenu();
            QAction* nameAct = new QAction(name);
            nameAct->setDisabled(true);
            menu->addAction(nameAct);
            menu->addSeparator();

            batLevel = new QAction();
            batLevel->setEnabled(false);

            batStatus = new QAction();
            batStatus->setEnabled(false);

            menu->addAction(batLevel);
            menu->addAction(batStatus);
            setContextMenu(menu);
        }

        // Battery strings for indexes
        const static QString BATTERY_VALUES[5];
        const static QString BATTERY_STATUS_VALUES[4];
    static_assert(sizeof(BATTERY_STATUS_VALUES)/sizeof(*BATTERY_STATUS_VALUES) == BATT_STATUS_INVALID, "BatteryStatus enum is not the same length as BATTERY_STATUS_VALUES");
        const static QString batTrayLevel;
        const static QString batTrayStatus;
        static int getBatteryString(uint bat) {
            if(bat > 50)
                return 4;
            if(bat > 30)
                return 3;
            if(bat > 15)
                return 2;
            return 1;
        }
    public slots:
        void setBattery(uint battery, BatteryStatus charging);
    private:
        QString name;
        QAction* batLevel;
        QAction* batStatus;
};
#endif

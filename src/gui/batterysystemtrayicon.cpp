#include <QIcon>
#include "batterysystemtrayicon.h"
#include "kb.h"

void BatteryStatusTrayIcon::setBattery(uint battery, BatteryStatus charging){
    const int chglvl = getBatteryString(battery);
    const QString chglvlstr = BATTERY_VALUES[chglvl];
    const QString chgstatstr = BATTERY_STATUS_VALUES[charging];


    QString iconName = charging == BatteryStatus::BATT_STATUS_CHARGING ? QString("charging") : QString::number(chglvl);
    QIcon icon = QIcon(QString(":/img/ckb-next_battery_%1.png").arg(iconName));
    setIcon(icon, QString("ckb-next_battery_%1").arg(iconName));

    const QString tooltip = QString("%1:").arg(name);
    const QString batlevelstr = batTrayLevel.arg(chglvlstr, QString::number(battery));
    const QString subtooltip = QString("%1, %2").arg(batlevelstr, chgstatstr);
    setToolTip(icon, tooltip, subtooltip);

    batLevel->setText(batlevelstr);
    batStatus->setText(batTrayStatus.arg(chgstatstr));
}

const QString BatteryStatusTrayIcon::BATTERY_VALUES[5] = {tr("Not connected"), tr("Critical"), tr("Low"), tr("Medium"), tr("High")};
const QString BatteryStatusTrayIcon::BATTERY_STATUS_VALUES[4] = {tr("N/A"), tr("Discharging"), tr("Charging"), tr("Fully Charged")};
const QString BatteryStatusTrayIcon::batTrayLevel = tr("Battery: %1 (%2%)");
const QString BatteryStatusTrayIcon::batTrayStatus = tr("Status: %1");

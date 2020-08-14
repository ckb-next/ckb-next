#include <QIcon>
#include "batterysystemtrayicon.h"
#include "kb.h"

void BatteryStatusTrayIcon::setBattery(uint battery, uint charging){
  QString iconName = charging == 2 ? QString("charging") : QString::number(battery);
  QIcon icon = QIcon(QString(":/img/battery_%1.png")
    .arg(iconName));
  QString tooltip = QString("%1: %2, %3")
    .arg(name)
    .arg(BATTERY_VALUES[battery])
    .arg(BATTERY_CHARGING_VALUES[charging]);

  setIcon(icon, QString("ckb-next-battery-%1").arg(iconName));
  setToolTip(icon, tooltip, "");
}

const QString BatteryStatusTrayIcon::BATTERY_VALUES[5] = {tr("Not connected"), tr("Critical"), tr("Low"), tr("Medium"), tr("High")};
const QString BatteryStatusTrayIcon::BATTERY_CHARGING_VALUES[3] = {tr("N/A"), tr("Not charging"), tr("Charging")};

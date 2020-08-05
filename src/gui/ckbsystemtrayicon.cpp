// Cmake will not build this file if USE_DBUS_MENU is not set
#include "ckbsystemtrayicon.h"
#include <QDir>
#include <QDebug>
CkbSystemTrayIcon::CkbSystemTrayIcon(const QIcon& icon, QObject*parent) : KStatusNotifierItem("ckb-next", parent), previousPath("")
{
    setIcon(icon);
}

// Not all implementations support passing icons by pixmap
// Save the icon to /tmp/ and use that instead
void CkbSystemTrayIcon::setIcon(QIcon icon)
{
    QList<QSize> availSizes = icon.availableSizes();
    if(!availSizes.length())
        return;

    QPixmap pm = icon.pixmap(availSizes.at(0));
    QString path;
    // Keep trying until we find a path that doesn't conflict
    int i;
    for(i = 0; i < 5; i++)
    {
        path = QDir::tempPath() + QString("/ckb-next-tray-%1.png").arg(qrand());
        if(!QFile::exists(path))
            break;
    }

    if(i == 5)
        return;

    if(!pm.save(path))
        return;

    setIconByName(path);

    // Delete the old file
    QFile f(previousPath);
    if(!previousPath.isEmpty() && f.exists())
        f.remove();

    previousPath = path;
}

CkbSystemTrayIcon::~CkbSystemTrayIcon()
{
    QFile f(previousPath);
    if(f.exists())
        f.remove();
}

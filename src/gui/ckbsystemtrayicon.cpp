// Cmake will not build this file if USE_DBUS_MENU is not set
#include "ckbsystemtrayicon.h"
#include <QDir>
#include <QDebug>
#include "compat/qrand.h"

CkbSystemTrayIcon::CkbSystemTrayIcon(const QIcon& icon, const QString iconName, QObject*parent) : KStatusNotifierItem("ckb-next", parent), previousPath("")
{
    setIcon(icon, iconName);
    connect(this, &KStatusNotifierItem::scrollRequested, [this](int delta, Qt::Orientation orientation) {
        QPoint data;
        if(orientation == Qt::Horizontal)
            data.setX(delta);
        else
            data.setY(delta);

        emit scrollRequested(data);
    });
    // KDE added extra tray options but ckb-next already has them, so turn them off.
    KStatusNotifierItem::setStandardActionsEnabled(false);
}

// Not all implementations support passing icons by pixmap
// Save the icon to /tmp/ and use that instead
void CkbSystemTrayIcon::setIcon(QIcon icon, QString name)
{
    // Check if the icon can be found in the theme
    QIcon::setThemeName("hicolor");
    QIcon themeicon = QIcon::fromTheme(name);

    // GTK's icon search can return icons even if the name doesn't fully match
    if(!themeicon.isNull() && themeicon.name() == name){
        // If the icon was found, just pass the name to kstatusnotifier. The DE should do the rest
        KStatusNotifierItem::setIconByName(name);
        return;
    }

    // If not, load the internal ones
    qDebug() << "Indicator icon not found in theme hicolor";
    QList<QSize> availSizes = icon.availableSizes();
    if(!availSizes.length())
        return;

    // Make sure to scale down to 22x22, otherwise some desktop environments will just display the whole
    // 512x512 in the tray...
    QPixmap pm = icon.pixmap(availSizes.at(0)).scaled(22, 22, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QString path;
    // Keep trying until we find a path that doesn't conflict
    int i;
    for(i = 0; i < 5; i++){
        path = QDir::tempPath() + QString("/ckb-next-tray-%1.png").arg(Q_RAND());
        if(!QFile::exists(path))
            break;
    }

    if(i == 5)
        return;

    if(!pm.save(path))
        return;

    KStatusNotifierItem::setIconByName(path);

    // Delete the old file
    QFile f(previousPath);
    if(!previousPath.isEmpty() && f.exists())
        f.remove();

    previousPath = path;
    return;
}

CkbSystemTrayIcon::~CkbSystemTrayIcon()
{
    if(previousPath.isEmpty())
        return;
    QFile f(previousPath);
    if(f.exists())
        f.remove();
    disconnect(this, &KStatusNotifierItem::scrollRequested, nullptr, nullptr);
}

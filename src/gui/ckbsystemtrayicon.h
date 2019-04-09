#ifndef CKBSYSTEMTRAYICON_H
#define CKBSYSTEMTRAYICON_H
#include <QSystemTrayIcon>

class CkbSystemTrayIcon : public QSystemTrayIcon {
    Q_OBJECT

    public:
        CkbSystemTrayIcon(const QIcon& icon, QObject* parent = 0)
                    : QSystemTrayIcon(icon, parent) {}

    virtual bool event(QEvent* evt);

    signals:
        void wheelScrolled(bool up);

};
#endif // CKBSYSTEMTRAYICON_H

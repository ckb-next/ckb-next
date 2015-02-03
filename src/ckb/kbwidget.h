#ifndef KBWIDGET_H
#define KBWIDGET_H

#include <QFile>
#include <QListWidgetItem>
#include <QWidget>
#include "kb.h"

namespace Ui {
class KbWidget;
}

class KbWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KbWidget(QWidget *parent, const QString& path, const QString& prefsBase);
    ~KbWidget();

    Kb* device;

    inline bool isActive() { return _active && device && device->isOpen(); }
    inline void active(bool newActive) { _active = newActive; }

    inline QString name() { return device ? device->usbModel : ""; }

private:
    Ui::KbWidget *ui;

    QString prefsPath;
    bool _active;

    KbMode* currentMode;

    const static int GUID = Qt::UserRole;
    const static int NEW_FLAG = Qt::UserRole + 1;

private slots:
    void updateProfileList();
    void profileChanged();
    void on_profileBox_activated(int index);

    QIcon modeIcon(int i);
    void addNewModeItem();

    void modeChanged(bool spontaneous = true);
    void on_modesList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void modesList_reordered();
    void on_modesList_itemChanged(QListWidgetItem *item);
    void on_modesList_itemClicked(QListWidgetItem *item);
    void on_modesList_customContextMenuRequested(const QPoint &pos);

    void devUpdate();
    void modeUpdate();
    void on_hwSaveButton_clicked();
    void on_inactiveSwitchCheck_clicked(bool checked);
    void on_inactiveSwitchBox_activated(int index);
    void on_muteCheck_clicked(bool checked);
    void on_layoutBox_activated(int index);
};

#endif // KBWIDGET_H

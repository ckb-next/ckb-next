#ifndef KBWIDGET_H
#define KBWIDGET_H

#include <QFile>
#include <QListWidgetItem>
#include <QWidget>
#include "kb.h"

// Central widget for displaying/controlling a device

namespace Ui {
class KbWidget;
}

class KbWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KbWidget(QWidget *parent, Kb* _device);
    ~KbWidget();

    // Device handle
    Kb* device;
    inline QString name() const { return device->usbModel; }

    // Has the "there is a firmware upgrade for this device..." screen already been shown?
    bool hasShownNewFW;
    // Update the "Check for updates" label with the current status
    void updateFwButton();

public slots:
    // Show a tab
    void showFirstTab();
    void showLastTab();

    // Display firmware update dialog
    inline void showFwUpdate()          { on_fwUpdButton_clicked(); }

private:
    Ui::KbWidget *ui;
    quint64 lastAutoSave;

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
    void on_tabWidget_currentChanged(int index);
    void on_fwUpdButton_clicked();
};

#endif // KBWIDGET_H

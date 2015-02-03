#ifndef KBPROFILEDIALOG_H
#define KBPROFILEDIALOG_H

#include <QDialog>
#include "kbwidget.h"

namespace Ui {
class KbProfileDialog;
}

class KbProfileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit KbProfileDialog(KbWidget *parent = 0);
    ~KbProfileDialog();

private slots:
    void profileList_reordered();
    void on_profileList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_profileList_itemClicked(QListWidgetItem *item);
    void on_profileList_itemChanged(QListWidgetItem *item);
    void on_profileList_customContextMenuRequested(const QPoint &pos);

private:
    Ui::KbProfileDialog *ui;

    Kb* device;
    const static int GUID = Qt::UserRole;
    const static int NEW_FLAG = Qt::UserRole + 1;

    void repopulate();
    void addNewProfileItem();
};

#endif // KBPROFILEDIALOG_H

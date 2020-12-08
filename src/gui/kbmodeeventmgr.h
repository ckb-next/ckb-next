#ifndef KBMODEEVENTMGR_H
#define KBMODEEVENTMGR_H

#include <QWidget>
#include <QDialog>
#include "kbwindowinfo.h"
#include <QTableWidgetItem>
#include "kbwindowinfomodel.h"
#include <QVector>

namespace Ui {
class KbModeEventMgr;
}

class KbModeEventMgr : public QDialog
{
    Q_OBJECT

public:
    explicit KbModeEventMgr(QWidget* parent, KbMode* m);
    ~KbModeEventMgr();
    virtual void closeEvent(QCloseEvent* evt);

private slots:
    void on_cancelBtn_clicked();
    void on_okBtn_clicked();
    void on_addbtn_clicked();
    void on_removebtn_clicked();
    void rowsChanged(const QModelIndex& parent, int first, int last);
    void on_clearbtn_2_clicked();

private:
    KbWindowInfo* info;
    Ui::KbModeEventMgr* ui;
    void handleClose();
    KbWindowInfoModel* model;
    QVector<KbWindowInfo::MatchPair> backup;
    KbWindowInfoModelDropdownDelegate* delegate;
};

#endif // KBMODEEVENTMGR_H

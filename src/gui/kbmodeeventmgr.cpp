#include "kbmodeeventmgr.h"
#include "ui_kbmodeeventmgr.h"
#include <QFileDialog>
#include <QDebug>
#include "kbwidget.h"
#include <QCloseEvent>
#include "idletimer.h"
#include "kbwindowinfomodel.h"

KbModeEventMgr::KbModeEventMgr(QWidget* parent, KbMode* m) :
    QDialog(parent), info(m->winInfo()),
    ui(new Ui::KbModeEventMgr), model(new KbWindowInfoModel(info, this)), backup(info->items),
    delegate(new KbWindowInfoModelDropdownDelegate)
{
    connect(model, &KbWindowInfoModel::rowsInserted, this, &KbModeEventMgr::rowsChanged);
    connect(model, &KbWindowInfoModel::rowsRemoved, this, &KbModeEventMgr::rowsChanged);
    ui->setupUi(this);
    if(!IdleTimer::isWayland())
        ui->waylandWarning->hide();
    ui->modeLabel->setText(QString(tr("Switch to mode \"%1\" when:")).arg(m->name()));
    ui->tableView->setModel(model);
    ui->tableView->horizontalHeader()->setSectionResizeMode(KbWindowInfoModel::COL_MATCH_TYPE, QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(KbWindowInfoModel::COL_VERB, QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(KbWindowInfoModel::COL_TARGET, QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(KbWindowInfoModel::COL_CASE_INSENSITIVE, QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(KbWindowInfoModel::COL_OPERATOR, QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->hideSection(KbWindowInfoModel::COL_OPERATOR);
    ui->enableBox->setChecked(info->isEnabled());
    rowsChanged(QModelIndex(), 0, 0);
    ui->tableView->setItemDelegateForColumn(KbWindowInfoModel::COL_MATCH_TYPE, delegate);
    ui->tableView->setItemDelegateForColumn(KbWindowInfoModel::COL_CASE_INSENSITIVE, delegate);
    ui->tableView->setItemDelegateForColumn(KbWindowInfoModel::COL_VERB, delegate);
    ui->tableView->setItemDelegateForColumn(KbWindowInfoModel::COL_OPERATOR, delegate);
    connect(model, &KbWindowInfoModel::dataChanged, this, [this]
            (const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles){
        const int col = topLeft.column();
        if(col != KbWindowInfoModel::COL_TARGET)
            ui->tableView->resizeColumnToContents(col);
    });
    if(model->rowCount() > 0)
        ui->tableView->setCurrentIndex(model->index(0, 0));
}

KbModeEventMgr::~KbModeEventMgr()
{
    delete ui;
    delete delegate;
}

void KbModeEventMgr::on_cancelBtn_clicked()
{
    info->items = backup;
    this->close();
}

void KbModeEventMgr::closeEvent(QCloseEvent* evt)
{
    // Disable the mode event if empty
    if(info->isEmpty())
        info->setEnabled(false);

    // Continue handling the event
    evt->accept();
}

void KbModeEventMgr::on_okBtn_clicked()
{
    info->setEnabled(ui->enableBox->isChecked());
    this->close();
}

void KbModeEventMgr::on_addbtn_clicked()
{
    const int newitem = model->addItem();
    const QModelIndex idx = model->index(newitem, KbWindowInfoModel::COL_TARGET);
    ui->tableView->setCurrentIndex(idx);
    ui->tableView->edit(idx);
}

void KbModeEventMgr::on_removebtn_clicked()
{
    QModelIndexList selected = ui->tableView->selectionModel()->selectedRows();
    if(selected.isEmpty())
        return;
    model->removeItem(selected.at(0).row());
}

void KbModeEventMgr::rowsChanged(const QModelIndex& parent, int first, int last)
{
    if(model->rowCount() > 1)
        ui->tableView->horizontalHeader()->showSection(KbWindowInfoModel::COL_OPERATOR);
    else
        ui->tableView->horizontalHeader()->hideSection(KbWindowInfoModel::COL_OPERATOR);
}

void KbModeEventMgr::on_clearbtn_2_clicked()
{
    model->clear();
    ui->okBtn->setFocus();
}

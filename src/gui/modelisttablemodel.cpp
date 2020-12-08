#include "modelisttablemodel.h"
#include <QIcon>
#include <QFont>
#include <QApplication>
#include <QStyle>
#include <QStyledItemDelegate>

ModeListTableModel::ModeListTableModel(Kb *dev, QObject* parent) : QAbstractTableModel(parent), device(dev),
    highlightedRow(-1), _hasFocus(false)
{
    connect(device, &Kb::profileAboutToChange, this, &ModeListTableModel::profileAboutToChange);
    connect(device, &Kb::profileChanged, this, &ModeListTableModel::profileChanged);
}

void ModeListTableModel::profileAboutToChange(){
    KbProfile* currentProfile = device->currentProfile();
    emit beginResetModel();
    if(!currentProfile)
        return;
    for(int i = 0; i < currentProfile->modeCount(); i++)
        disconnect(currentProfile->at(i)->winInfo(), nullptr, this, nullptr);
}

void ModeListTableModel::profileChanged(){
    // Signals will have always been connected when this is called
    // and we'll always have a valid profile
    emit endResetModel();
    KbProfile* currentProfile = device->currentProfile();
    for(int i = 0; i < currentProfile->modeCount(); i++){
        connect(currentProfile->at(i)->winInfo(), &KbWindowInfo::enableStateChanged, this, [this, i](){
            QModelIndex changed = index(i, COL_EVENT_ICON);
            emit dataChanged(changed, changed, {Qt::DecorationRole});
        });
    }
}

int ModeListTableModel::rowCount(const QModelIndex& parent) const {
    const KbProfile* const prof = device->currentProfile();
    if(!prof)
        return 0;
    // +1 for the "New mode" item
    return prof->modeCount() + 1;
}

int ModeListTableModel::columnCount(const QModelIndex& parent) const {
    return COL_MODE_MAX;
}

QIcon ModeListTableModel::modeIcon(const int i) const{
    const KbProfile* const currentProfile = device->currentProfile();
    const KbProfile* const hwProfile = device->hwProfile();
    int hwModeCount = device->hwModeCount;
    if(i >= hwModeCount)
        return QIcon(":/img/icon_mode.png");
    else
        return QIcon(QString(currentProfile == hwProfile ? ":/img/icon_mode%1_hardware.png" : ":/img/icon_mode%1.png").arg(i + 1));
}

QIcon ModeListTableModel::eventIcon(KbMode* mode){
    if(mode->winInfo()->isEnabled())
        return QIcon(":/img/lightning.svg");
    return QIcon(":/img/lightning_disabled.svg");
}

QVariant ModeListTableModel::data(const QModelIndex& index, int role) const{
    // Make sure the profile is valid
    const KbProfile* const prof = device->currentProfile();
    if(!prof)
        return QVariant();
    const int row = index.row();
    const int col = index.column();
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        // If the index is past modeCount, then we are adding the New mode... button
        if(row > prof->modeCount() - 1){
            if(col == COL_MODE_NAME)
                return QString(tr("New mode..."));
            return QVariant();
        }
        if(col == COL_MODE_NAME)
            return prof->at(row)->name();
    } else if (role == Qt::DecorationRole ) {
        // Add the + for the new mode
        if(row > prof->modeCount() - 1){
            if(col == COL_MODE_ICON)
                return QIcon(":/img/icon_plus.png");
            return QVariant();
        }

        switch(col){
        case COL_MODE_ICON:
            return modeIcon(row);
        case COL_EVENT_ICON:
            return eventIcon(prof->at(row));
        }
    } else if (role == Qt::FontRole && row > prof->modeCount() - 1 && col == COL_MODE_NAME) {
        QFont font;
        font.setItalic(true);
        return font;
    } else if(role == Qt::BackgroundRole) {
        if(col == COL_EVENT_ICON && row == activeRow && _hasFocus)
            return QApplication::style()->standardPalette().highlight();
        else if(row == highlightedRow)
            return QApplication::style()->standardPalette().window();
    }
    return QVariant();
}

Qt::ItemFlags ModeListTableModel::flags(const QModelIndex& index) const {
    if(!index.isValid())
        return Qt::ItemIsDropEnabled;
    const int col = index.column();
    const int row = index.row();
    const KbProfile* const prof = device->currentProfile();
    if(row > prof->modeCount() - 1) // New mode...
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if(col == COL_MODE_NAME)
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsEditable;
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled;
}

bool ModeListTableModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if(role != Qt::EditRole)
        return false;
    const KbProfile* const prof = device->currentProfile();
    if(!index.isValid() || !prof || index.column() != COL_MODE_NAME)
        return false;

    const int row = index.row();
    if(row > prof->modeCount() - 1)
        return false;

    prof->at(row)->name(value.toString());
    emit dataChanged(index, index, {Qt::EditRole, Qt::DisplayRole});

    return true;
}

int ModeListTableModel::addNewMode(){
    KbMode* newMode = device->newMode();
    // "Add" the new mode item back
    emit beginInsertRows(QModelIndex(), rowCount()-1, rowCount()-1);
    device->currentProfile()->append(newMode);
    device->setCurrentMode(newMode);
    const int newrow = rowCount() - 2;
    // Update the previous new mode item with the new mode
    emit dataChanged(index(newrow, 0), index(newrow, columnCount()-1), {Qt::DisplayRole, Qt::EditRole, Qt::DecorationRole});
    emit endInsertRows();
    return newrow;
}

Qt::DropActions ModeListTableModel::supportedDropActions() const{
    return Qt::MoveAction;
}

bool ModeListTableModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int dstrow, int column, const QModelIndex& parent){
    // Don't allow dropping after the new mode item
    // it has to be done here as there's no way to check for it in flags()
    if(dstrow == -1 || action != Qt::MoveAction || dstrow > rowCount() - 1 || !data->hasFormat("application/x-qabstractitemmodeldatalist"))
        return false;
    QByteArray e = data->data("application/x-qabstractitemmodeldatalist");
    QDataStream stream(&e, QIODevice::ReadOnly);

    // Read only the first item. We only need column 0 and there is only a single row selected.
    int srcrow = -1;
    int srccol = -1;
    QMap<int,  QVariant> roledata;
    stream >> srcrow >> srccol >> roledata;
    if(srcrow < dstrow)
        dstrow--;
    if(srcrow == dstrow)
        return false;

    emit layoutAboutToBeChanged();
    device->currentProfile()->move(srcrow, dstrow);
    emit layoutChanged();

    return true;
}

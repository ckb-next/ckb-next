#ifndef MODELISTTABLEMODEL_H
#define MODELISTTABLEMODEL_H
#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include "kb.h"
#include <QMimeData>

class ModeListTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    ModeListTableModel(Kb* dev, QObject* parent = nullptr);
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    static const enum {
        COL_MODE_ICON,
        COL_MODE_NAME,
        COL_EVENT_ICON,
        COL_MODE_MAX,
    } ModeListColumns;
    Qt::ItemFlags flags(const QModelIndex& index) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role);
    inline void setHighlightedRow(const int row) {
        highlightedRow = row;
        emit dataChanged(index(0, 0), index(rowCount()-1, columnCount()-1), {Qt::BackgroundRole});
    }
    inline KbMode* modeForIndex(const QModelIndex& index) const {
        return device->currentProfile()->at(index.column());
    }
    int addNewMode();
    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int dstrow, int column, const QModelIndex& parent);
    Qt::DropActions supportedDropActions() const;
    inline void setHasFocus(const bool f){
        _hasFocus = f;
        emit dataChanged(index(0, COL_EVENT_ICON), index(rowCount()-1, COL_EVENT_ICON), {Qt::BackgroundRole});
    }
    inline bool hasFocus() const { return _hasFocus; }
    inline void setActiveRow(const int row) {
        activeRow = row;
        emit dataChanged(index(0, COL_EVENT_ICON), index(rowCount()-1, COL_EVENT_ICON), {Qt::BackgroundRole});
    }
public slots:
    void profileAboutToChange();
    void profileChanged();
private:
    Kb* device;
    QIcon modeIcon(const int i) const;
    static QIcon eventIcon(KbMode* mode);
    int highlightedRow;
    bool _hasFocus;
    int activeRow;
};

#endif // MODELISTTABLEMODEL_H

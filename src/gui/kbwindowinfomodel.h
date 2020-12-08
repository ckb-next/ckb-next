#ifndef KBWINDOWINFOMODEL_H
#define KBWINDOWINFOMODEL_H

#include <QAbstractTableModel>
#include "kbwindowinfo.h"
#include <QStyledItemDelegate>
#include <QComboBox>

class KbWindowInfoModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    KbWindowInfoModel(KbWindowInfo* i, QObject* parent);
    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int dstrow, int column, const QModelIndex& parent);
    Qt::DropActions supportedDropActions() const;
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex& index) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role);
    enum WININFO_COLUMN {
        COL_MATCH_TYPE,
        COL_VERB,
        COL_TARGET,
        COL_CASE_INSENSITIVE,
        COL_OPERATOR,
    };
    void removeItem(const int row);
    int addItem();
    void clear();
    static QString caseSensitiveStr;
    static QString caseInsensitiveStr;
    static QString orStr;
    static QString andStr;
    static QString isStr;
    static QString containsStr;
    static QString classStr;
    static QString instanceStr;
    static QString pathStr;
    static QString titleStr;
    static QString startsStr;
    static QString endsStr;

private:
    KbWindowInfo* wininfo;
    static QString operatorIndexToString(KbWindowInfo::MatchOperator idx);
    static QString matchTypeIndexToString(KbWindowInfo::MatchType idx);
    static QString verbIndexToString(int idx);
};

class KbWindowInfoModelDropdownDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const {

        QWidget* editor = new QComboBox(parent);
        return editor;
    }
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;
    void setEditorData(QWidget* editor, const QModelIndex& index) const;
};
#endif // KBWINDOWINFOMODEL_H

#ifndef MACROTABLEMODEL_H
#define MACROTABLEMODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include "macroline.h"
#include <QDebug>
#include <QComboBox>
#include <QStyledItemDelegate>

class MacroTableModel : public QAbstractTableModel{
    Q_OBJECT
public:
    MacroTableModel(QObject* parent = nullptr) : QAbstractTableModel(parent), defaultDelay(false) {}
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    inline void append(const MacroLine& ml) {
        emit beginInsertRows(QModelIndex(), macroLines.length(), macroLines.length());
        macroLines.append(ml);
        emit endInsertRows();
    }
    inline QModelIndex addBlankElement(){
        const MacroLine ml;
        append(ml);
        return index(length() - 1, 1);
    }
    inline void clear() {
        if(!length())
            return;
        emit beginRemoveRows(QModelIndex(), 0, macroLines.length() - 1);
        macroLines.clear();
        emit endRemoveRows();
    }
    inline const MacroLine& at(int i) const {
        return macroLines.at(i);
    }
    inline int length() const {
        return macroLines.length();
    }
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex& index) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role);
    QString toString(bool rawData);
    QString fromString(const QString& input, const bool stopOnError);
    inline void setDefaultDelay(bool set){
        defaultDelay = set;
        // Update UI to disable editing the delay in the table
        emit dataChanged(index(0, 2), index(length() - 1, 3));
    }
    void removeLastMouseLeftClick();
    static const QSet<QString> validMacroKeys;
    int isMacroMatched();
    void removeMultipleColumns(QModelIndexList l);
    Qt::DropActions supportedDropActions() const;
    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int dstrow, int column, const QModelIndex& parent);
    void removeEmptyRowAtEnd();

private:
    QVector<MacroLine> macroLines;
    bool defaultDelay;
};

class MacroDropdown : public QComboBox{
    Q_OBJECT
public:
    MacroDropdown(QWidget* p) : QComboBox(p) {
        insertItem(0, "↓");
        insertItem(1, "↑");
    }
};

class MacroDropdownDelegate : public QStyledItemDelegate{
    Q_OBJECT
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const {
        QWidget* editor = new MacroDropdown(parent);
        return editor;
    }
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;
    void setEditorData(QWidget* editor, const QModelIndex& index) const;
};

#endif // MACROTABLEMODEL_H

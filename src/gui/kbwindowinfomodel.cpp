#include "kbwindowinfomodel.h"
#include "kbwindowinfo.h"
#include <QMimeData>
#include <algorithm>

// Awful
QString KbWindowInfoModel::caseSensitiveStr;
QString KbWindowInfoModel::caseInsensitiveStr;
QString KbWindowInfoModel::orStr;
QString KbWindowInfoModel::andStr;
QString KbWindowInfoModel::isStr;
QString KbWindowInfoModel::containsStr;
QString KbWindowInfoModel::startsStr;
QString KbWindowInfoModel::endsStr;
QString KbWindowInfoModel::titleStr;
QString KbWindowInfoModel::pathStr;
QString KbWindowInfoModel::instanceStr;
QString KbWindowInfoModel::classStr;

KbWindowInfoModel::KbWindowInfoModel(KbWindowInfo* i, QObject* parent) :
    QAbstractTableModel(parent), wininfo(i) {
    if(caseSensitiveStr.isEmpty()) {
        caseSensitiveStr = tr("Case Sensitive");
        caseInsensitiveStr = tr("Case Insensitive");
        orStr = tr("OR");
        andStr = tr("AND");
        isStr = tr("is");
        containsStr = tr("contains");
        startsStr = tr("starts with");
        endsStr = tr("ends with");
        titleStr = tr("Window Title");
        pathStr = tr("Program Path");
        instanceStr = tr("Instance Name");
        classStr = tr("Class Name");
    }
}

int KbWindowInfoModel::rowCount(const QModelIndex& parent) const {
    return wininfo->items.length();
}

int KbWindowInfoModel::columnCount(const QModelIndex& parent) const {
    return 5;
}

QString KbWindowInfoModel::matchTypeIndexToString(KbWindowInfo::MatchType idx){
    switch(idx){
    case KbWindowInfo::MATCH_TYPE_WINDOW_TITLE:
        return titleStr;
    case KbWindowInfo::MATCH_TYPE_PROGRAM_PATH:
        return pathStr;
    case KbWindowInfo::MATCH_TYPE_WM_INSTANCE_NAME:
        return instanceStr;
    case KbWindowInfo::MATCH_TYPE_WM_CLASS_NAME:
        return classStr;
    default:
        return QString();
    }
}

QString KbWindowInfoModel::operatorIndexToString(KbWindowInfo::MatchOperator idx){
    switch(idx){
    case KbWindowInfo::MATCH_OP_OR:
        return orStr;
    case KbWindowInfo::MATCH_OP_AND:
        return andStr;
    default:
        return QString();
    }
}

QVariant KbWindowInfoModel::data(const QModelIndex& index, int role) const {
    if(!index.isValid() || (role != Qt::DisplayRole && role != Qt::EditRole))
        return QVariant();
    const int row = index.row();
    const int col = index.column();
    const KbWindowInfo::MatchPair& mp = wininfo->items.at(row);

    if(role == Qt::EditRole){
        switch(col){
        case COL_MATCH_TYPE:
            return static_cast<int>(mp.type);
        case COL_VERB:
            if(mp.flags.testFlag(KbWindowInfo::MATCH_SUBSTRING))
                return 1;
            else if(mp.flags.testFlag(KbWindowInfo::MATCH_STARTS_WITH))
                return 2;
            else if(mp.flags.testFlag(KbWindowInfo::MATCH_ENDS_WITH))
                return 3;
            else
                return 0;
        case COL_TARGET:
            return mp.item;
        case COL_CASE_INSENSITIVE:
            return mp.flags.testFlag(KbWindowInfo::MATCH_CASE_INSENSITIVE) ? 1 : 0;
        case COL_OPERATOR:
            return static_cast<int>(wininfo->items.at(row).op);
        }
    } else { // Only two roles are allowed down here, so this one will always be DisplayRole
        switch(col){
        case COL_MATCH_TYPE:
            return matchTypeIndexToString(mp.type);
        case COL_VERB:
            if(mp.flags.testFlag(KbWindowInfo::MATCH_SUBSTRING))
                return containsStr;
            else if(mp.flags.testFlag(KbWindowInfo::MATCH_STARTS_WITH))
                return startsStr;
            else if(mp.flags.testFlag(KbWindowInfo::MATCH_ENDS_WITH))
                return endsStr;
            else
                return isStr;
        case COL_TARGET:
            return mp.item;
        case COL_CASE_INSENSITIVE:
            return mp.flags.testFlag(KbWindowInfo::MATCH_CASE_INSENSITIVE) ? caseInsensitiveStr : caseSensitiveStr;
        case COL_OPERATOR:
            return operatorIndexToString(wininfo->items.at(row).op);
        }
    }

    return QVariant();
}

Qt::ItemFlags KbWindowInfoModel::flags(const QModelIndex& index) const
{
    if(!index.isValid())
        return Qt::ItemIsDropEnabled;

    // Always disable the bottom right operator
    if(index.column() == COL_OPERATOR && index.row() == rowCount() - 1)
        return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled;
}

bool KbWindowInfoModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if(!index.isValid())
        return false;

    const int row = index.row();
    const int col = index.column();
    switch(col){
    case COL_TARGET:
        wininfo->items[row].item = value.toString();
        break;
    case COL_CASE_INSENSITIVE:
#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
        if(value.toInt() == 1)
            wininfo->items[row].flags |= KbWindowInfo::MATCH_CASE_INSENSITIVE;
        else
            wininfo->items[row].flags &= ~KbWindowInfo::MATCH_CASE_INSENSITIVE;
#else
        wininfo->items[row].flags.setFlag(KbWindowInfo::MATCH_CASE_INSENSITIVE, value.toInt() == 1);
#endif
        break;
    case COL_MATCH_TYPE:
        wininfo->items[row].type = static_cast<KbWindowInfo::MatchType>(value.toInt());
        break;
    case COL_OPERATOR:
        wininfo->items[row].op = static_cast<KbWindowInfo::MatchOperator>(value.toInt());
        break;
    case COL_VERB: {
        const int val = value.toInt();
#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
        if(val == 1) {
            wininfo->items[row].flags |= KbWindowInfo::MATCH_SUBSTRING;
            wininfo->items[row].flags &= ~KbWindowInfo::MATCH_STARTS_WITH;
            wininfo->items[row].flags &= ~KbWindowInfo::MATCH_ENDS_WITH;
        } else if(val == 2) {
            wininfo->items[row].flags &= ~KbWindowInfo::MATCH_SUBSTRING;
            wininfo->items[row].flags |= KbWindowInfo::MATCH_STARTS_WITH;
            wininfo->items[row].flags &= ~KbWindowInfo::MATCH_ENDS_WITH;
        } else if(val == 3) {
            wininfo->items[row].flags &= ~KbWindowInfo::MATCH_SUBSTRING;
            wininfo->items[row].flags &= ~KbWindowInfo::MATCH_STARTS_WITH;
            wininfo->items[row].flags |= KbWindowInfo::MATCH_ENDS_WITH;
        } else {
            wininfo->items[row].flags &= ~KbWindowInfo::MATCH_SUBSTRING;
            wininfo->items[row].flags &= ~KbWindowInfo::MATCH_STARTS_WITH;
            wininfo->items[row].flags &= ~KbWindowInfo::MATCH_ENDS_WITH;
        }
#else
        wininfo->items[row].flags.setFlag(KbWindowInfo::MATCH_SUBSTRING, val == 1);
        wininfo->items[row].flags.setFlag(KbWindowInfo::MATCH_STARTS_WITH, val == 2);
        wininfo->items[row].flags.setFlag(KbWindowInfo::MATCH_ENDS_WITH, val == 3);
#endif
        break;
    }
    default:
        return false;
    }
    emit dataChanged(index, index, {role});
    return true;
}

void KbWindowInfoModel::removeItem(const int row)
{
    emit beginRemoveRows(QModelIndex(), row, row);
    wininfo->items.remove(row);
    emit endRemoveRows();
}

int KbWindowInfoModel::addItem()
{
    const int row = rowCount();
    emit beginInsertRows(QModelIndex(), row, row);
    QString name("SuperTuxKart");
    switch(row){
    case 8:
        name = tr("Click");
        break;
    case 9:
        name = tr("Click Click");
        break;
    case 10:
        name = tr("Click Click Click");
        break;
    case 11:
        name = tr("Good Job! Have a cookie 🍪");
        break;
    }
    wininfo->items.append({KbWindowInfo::MATCH_TYPE_WINDOW_TITLE, name,
                           KbWindowInfo::MatchFlags(), KbWindowInfo::MATCH_OP_OR});
    emit endInsertRows();
    return row;
}

void KbWindowInfoModel::clear()
{
    emit beginRemoveRows(QModelIndex(), 0, rowCount()-1);
    wininfo->items.clear();
    emit endRemoveRows();
}

Qt::DropActions KbWindowInfoModel::supportedDropActions() const{
    return Qt::MoveAction;
}

bool KbWindowInfoModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int dstrow, int column, const QModelIndex& parent){
    if(dstrow == -1 || action != Qt::MoveAction || !data->hasFormat("application/x-qabstractitemmodeldatalist"))
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
#if QT_VERSION < QT_VERSION_CHECK(5, 6, 0)
    QVector<KbWindowInfo::MatchPair>::iterator iter = wininfo->items.begin();
    if (srcrow < dstrow)
        std::rotate(iter + srcrow, iter + srcrow + 1, iter + dstrow + 1);
    else
        std::rotate(iter + dstrow, iter + srcrow, iter + srcrow + 1);
#else
    wininfo->items.move(srcrow, dstrow);
#endif
    emit layoutChanged();

    return true;
}

void KbWindowInfoModelDropdownDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
                                const QModelIndex& index) const
{
    QComboBox* e = qobject_cast<QComboBox*>(editor);
    model->setData(index, e->currentIndex());
}

void KbWindowInfoModelDropdownDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
    QComboBox* e = qobject_cast<QComboBox*>(editor);
    e->clear();
    // Set all available options for each column
    switch (index.column()) {
    case KbWindowInfoModel::COL_OPERATOR:
        e->addItem(KbWindowInfoModel::orStr);
        e->addItem(KbWindowInfoModel::andStr);
        break;
    case KbWindowInfoModel::COL_CASE_INSENSITIVE:
        e->addItem(KbWindowInfoModel::caseSensitiveStr);
        e->addItem(KbWindowInfoModel::caseInsensitiveStr);
        break;
    case KbWindowInfoModel::COL_VERB:
        e->addItem(KbWindowInfoModel::isStr);
        e->addItem(KbWindowInfoModel::containsStr);
        e->addItem(KbWindowInfoModel::startsStr);
        e->addItem(KbWindowInfoModel::endsStr);
        break;
    case KbWindowInfoModel::COL_MATCH_TYPE:
        e->addItem(KbWindowInfoModel::titleStr);
        e->addItem(KbWindowInfoModel::pathStr);
        e->addItem(KbWindowInfoModel::instanceStr);
        e->addItem(KbWindowInfoModel::classStr);
        break;
    }
    // data() returns the appropriate ints for EditRole
    e->setCurrentIndex(index.data(Qt::EditRole).toInt());
}

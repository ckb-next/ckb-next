#include "macrotablemodel.h"
#include <QRegularExpression>
#include <QStringBuilder>
#include <QMimeData>
#include <QIODevice>

const QSet<QString> MacroTableModel::validMacroKeys =
        QSet<QString>{"esc",
                    "f1",
                    "f2",
                    "f3",
                    "f4",
                    "f5",
                    "f6",
                    "f7",
                    "f8",
                    "f9",
                    "f10",
                    "f11",
                    "grave",
                    "1",
                    "2",
                    "3",
                    "4",
                    "5",
                    "6",
                    "7",
                    "8",
                    "9",
                    "0",
                    "minus",
                    "tab",
                    "q",
                    "w",
                    "e",
                    "r",
                    "t",
                    "y",
                    "u",
                    "i",
                    "o",
                    "p",
                    "lbrace",
                    "caps",
                    "a",
                    "s",
                    "d",
                    "f",
                    "g",
                    "h",
                    "j",
                    "k",
                    "l",
                    "colon",
                    "quote",
                    "lshift",
                    "bslash_iso",
                    "z",
                    "x",
                    "c",
                    "v",
                    "b",
                    "n",
                    "m",
                    "comma",
                    "dot",
                    "slash",
                    "lctrl",
                    "lwin",
                    "lalt",
                    "hanja",
                    "space",
                    "hangul",
                    "katahira",
                    "ralt",
                    "rwin",
                    "rmenu",
                    "f12",
                    "prtscn",
                    "scroll",
                    "pause",
                    "ins",
                    "home",
                    "pgup",
                    "rbrace",
                    "bslash",
                    "hash",
                    "enter",
                    "ro",
                    "equal",
                    "yen",
                    "bspace",
                    "del",
                    "end",
                    "pgdn",
                    "rshift",
                    "rctrl",
                    "up",
                    "left",
                    "down",
                    "right",
                    "mute",
                    "stop",
                    "prev",
                    "play",
                    "next",
                    "numlock",
                    "numslash",
                    "numstar",
                    "numminus",
                    "numplus",
                    "numenter",
                    "num7",
                    "num8",
                    "num9",
                    "num4",
                    "num5",
                    "num6",
                    "num1",
                    "num2",
                    "num3",
                    "num0",
                    "numdot",
                    "volup",
                    "voldn",
                    "muhenkan",
                    "henkan",
                    "fn",
                    "lightup",
                    "lightdn",
                    "eject",
                    "power",
                    "f13",
                    "f14",
                    "f15",
                    "f16",
                    "f17",
                    "f18",
                    "f19",
                    "f20",
                    "f21",
                    "f22",
                    "f23",
                    "f24",
                    "mouse1",
                    "mouse2",
                    "mouse3",
                    "mouse4",
                    "mouse5",
                    "wheelup",
                    "wheeldn",
                    "mouse6",
                    "mouse7",
                    "mouse8"
    };


int MacroTableModel::rowCount(const QModelIndex& parent) const{
    return length();
}

int MacroTableModel::columnCount(const QModelIndex& parent) const{
    return 4;
}

QVariant MacroTableModel::data(const QModelIndex& index, int role) const{
    if (role == Qt::DisplayRole || role == Qt::EditRole){
        const int row = index.row();
        if(row < length()){
            const MacroLine ml = macroLines.at(row);
            const int col = index.column();
            switch(col){
                case 0:
                    return QString((ml.keyDown ? "↓" : "↑"));
                case 1:
                    return QString(ml.key);
                case 2:
                case 3:
                    if(index.row() == 0)
                        return QString(tr("None"));
                    else if(defaultDelay || ml.usTime == MacroLine::MACRO_DELAY_DEFAULT)
                        return QString(tr("Default"));
                    else
                        return QString::number(col == 2 ? ml.usTime : ml.usTimeMax);
                default:
                    return tr("Unknown");
            }
        }
    } else if(role == Qt::TextAlignmentRole){
        if(index.column() == 0)
            return Qt::AlignCenter;
    } else if(role == Qt::ToolTipRole){
        if(index.column() == 2 || index.column() == 3){
            if(index.row() == 0)
                return QString(tr("You can not set a delay before the first key event"));
            else if(defaultDelay)
                return QString(tr("To set a delay, please switch the delay mode to \"as typed\""));
        }
    }
    return QVariant();
}

QVariant MacroTableModel::headerData(int section, Qt::Orientation orientation, int role) const{
    if(orientation != Qt::Horizontal)
        return QVariant();

    if(role == Qt::DisplayRole) {
        switch(section){
            case 0:
                return QString("↓");
            case 1:
                return QString(tr("Key"));
            case 2:
                return QString(tr("Min. Delay"));
            case 3:
                return QString(tr("Max. Delay"));
            default:
                return QVariant();
        }
    }
    return QVariant();

}

Qt::ItemFlags MacroTableModel::flags(const QModelIndex &idx) const{
    Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled;

    // Allow drops only inbetween rows
    if(!idx.isValid())
        f |= Qt::ItemIsDropEnabled;

    // Disable the right column if needed
    if(idx == index(0, 2) || idx == index(0,3) || ((idx.column() == 2 || idx.column() == 3) && defaultDelay))
        f &= ~Qt::ItemIsEnabled;

    return f;
}

Qt::DropActions MacroTableModel::supportedDropActions() const{
    return Qt::MoveAction;
}

bool MacroTableModel::setData(const QModelIndex& index, const QVariant& value, int role){
    if (!index.isValid() || role != Qt::EditRole)
        return false;
    MacroLine& ml = macroLines[index.row()];
    const int col = index.column();
    switch(col){
    case 0:
        ml.keyDown = !value.toInt(); // toInt() because the variant has an index int in it, which will get converted to bool
        break;
    case 1:
    {
        const QString key = value.toString();
        const bool isNewEntry = ml.key.isEmpty();
        if(!validMacroKeys.contains(key)){
            // If the key was blank (means the user is editing a new row), then delete it if we couldn't add it
            if(isNewEntry){
                const int row = index.row();
                emit beginRemoveRows(QModelIndex(), row, row);
                macroLines.removeAt(row);
                emit endRemoveRows();
            }
            return false;
        }
        ml.key = key;
        if(isNewEntry){
            const MacroLine newml(key, MacroLine::MACRO_DELAY_DEFAULT, MacroLine::MACRO_DELAY_DEFAULT, false);
            append(newml);
        }
    }
        break;
    case 2:
    case 3:
    {
        const QString valstr = value.toString();
        if(valstr.isEmpty()){
            ml.usTime = ml.usTimeMax = MacroLine::MACRO_DELAY_DEFAULT;
            return true;
        }

        bool ok;
        const qint64 delay = valstr.toLongLong(&ok);
        if(!ok || delay < 0)
            return false;

        if(col == 2) {
            if(delay > ml.usTimeMax)
                ml.usTimeMax = delay;

            ml.usTime = delay;
        } else {
            if(delay < ml.usTime && ml.usTimeMax != MacroLine::MACRO_DELAY_DEFAULT)
                return false;

            if(ml.usTime == MacroLine::MACRO_DELAY_DEFAULT)
                ml.usTime = delay;

            ml.usTimeMax = delay;
        }
    }
        break;
    default:
        qDebug() << "Unknown column in setData";
    }
    emit dataChanged(index, index, {role});
    return true;
}

QString MacroTableModel::toString(bool rawData){
    QString l;
    const int mlength = macroLines.length();
    for(int i = 0; i < mlength; i++){
        const MacroLine& ml = macroLines.at(i);
        l.append((ml.keyDown ? '+' : '-'));
        l.append(ml.key);

        if(i + 1 < mlength){
            const MacroLine& nextMl = macroLines.at(i+1);
            const qint64 usTime = nextMl.usTime;
            if(usTime != MacroLine::MACRO_DELAY_DEFAULT && (!defaultDelay || rawData)){
                l.append("=" + QString::number(usTime));
                if(nextMl.usTimeMax != nextMl.usTime)
                    l.append("_" + QString::number(nextMl.usTimeMax));
            }
            l.append(",");
        }
    }

    return l;
}
#if QT_VERSION < QT_VERSION_CHECK(6, 0 ,0)
#define MACRO_ERROR(end, start)  return QString::number(end) % ":<br>" % str.leftRef(end) \
                                        % "<b><span style=\"color: #ff0000;\">" % str.midRef(end, start - end) % "</span></b>" \
                                        % str.midRef(start)
#else
#define MACRO_ERROR(end, start)  return QString::number(end) % ":<br>" % strView.first(end) \
                                        % "<b><span style=\"color: #ff0000;\">" % strView.sliced(end, start - end) % "</span></b>" \
                                        % strView.sliced(start)
#endif
#define MACRO_ERROR_RET()       MACRO_ERROR(previousEnd, currentStart)
#define MACRO_ERROR_RET_SUFFIX(end, start, suffix) MACRO_ERROR(end, start) % "<br><br>" % suffix
QString MacroTableModel::fromString(const QString& input, const bool stopOnError){
    // Replace all "whitespace" characters with ' ', and then remove that as well
    QString str = input.simplified();
    str.replace(QChar(' '), QString(""));
    QStringView strView(str);
    QVector<MacroLine> newMacroLines;
    QRegularExpression re("(\\+|-)([a-z0-9_]+)(=(\\d+)(_(\\d+))?)?(,|$)");
    QRegularExpressionMatchIterator i = re.globalMatch(strView.toString());
    qint64 prevDelay = MacroLine::MACRO_DELAY_DEFAULT, prevMaxDelay = MacroLine::MACRO_DELAY_DEFAULT;
    int previousEnd = 0;
    while (i.hasNext()) {
        QRegularExpressionMatch m = i.next();
        // Make sure the match started from the previous end. If it didn't, we have a parse error
        const int currentStart = m.capturedStart();
        if(previousEnd != currentStart && stopOnError){
            MACRO_ERROR_RET();
        }
        QStringView act = m.capturedView(1);
        QString key = m.captured(2);
#if QT_VERSION < QT_VERSION_CHECK(6, 0 ,0)
        QStringRef delaystr = m.capturedRef(4);
        QStringRef maxDelayStr = m.capturedRef(6);
#else
        QStringView delaystr = m.capturedView(4);
        QStringView maxDelayStr = m.capturedView(6);
#endif

        qint64 delay = MacroLine::MACRO_DELAY_DEFAULT, maxDelay = MacroLine::MACRO_DELAY_DEFAULT;
        if(!delaystr.isNull()){
            bool ok;
            delay = delaystr.toLongLong(&ok);
            if(!ok){
                if(stopOnError)
                    MACRO_ERROR_RET_SUFFIX(m.capturedStart(4), m.capturedEnd(4), tr("Delay is too large"));
                delay = MacroLine::MACRO_DELAY_DEFAULT;
            }
            maxDelay = delay;
            if(!maxDelayStr.isNull()){
                maxDelay = maxDelayStr.toLongLong(&ok);
                if(!ok){
                    if(stopOnError)
                        MACRO_ERROR_RET_SUFFIX(m.capturedStart(6), m.capturedEnd(6), tr("Max random delay is too large"));
                    maxDelay = delay;
                } else if(maxDelay <= delay) {
                    if(stopOnError)
                        MACRO_ERROR_RET_SUFFIX(m.capturedStart(6), m.capturedEnd(6), tr("Max random delay is less or equal to the minimum"));
                    maxDelay = delay;
                }
            }
        }
        // Act and key will always be valid strings
        const bool down = (act == QStringLiteral("+"));

        // Make sure the key actually exists
        if(!validMacroKeys.contains(key)){
            if(stopOnError)
                MACRO_ERROR_RET_SUFFIX(m.capturedStart(2), m.capturedEnd(2), tr("Invalid key ") % key);
            else
                continue;
        }

        newMacroLines.append(MacroLine(key, prevDelay, prevMaxDelay, down));
        prevDelay = delay;
        prevMaxDelay = maxDelay;
        previousEnd = m.capturedEnd();
    }
    // If we get down here, make sure we reached the end
    const int len = input.length();
    if(previousEnd != len && stopOnError)
        MACRO_ERROR(previousEnd, len);
    emit beginResetModel();
    macroLines = newMacroLines;
    emit endResetModel();
    return QString();
}

void MacroTableModel::removeLastMouseLeftClick(){
    // Go through the vector in reverse, find the last two mouse events and remove them
    bool foundKeyUp = false;
    for(int i = macroLines.length() - 1; i >= 0; i--){
        const MacroLine& ml = macroLines.at(i);
        // Once the Key Up was found, start searching for a keydown
        if(foundKeyUp && ml.keyDown && ml.key == "mouse1"){
            emit beginRemoveRows(QModelIndex(), i, i);
            macroLines.removeAt(i);
            emit endRemoveRows();
            return;
        } else if(!ml.keyDown && ml.key == "mouse1") {
            emit beginRemoveRows(QModelIndex(), i, i);
            macroLines.removeAt(i);
            emit endRemoveRows();
            foundKeyUp = true;
        }
    }
}

void MacroDropdownDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
                                const QModelIndex& index) const
{
    MacroDropdown* e = qobject_cast<MacroDropdown*>(editor);
    model->setData(index, e->currentIndex());
}

void MacroDropdownDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
    MacroDropdown* e = qobject_cast<MacroDropdown*>(editor);
    e->setCurrentIndex(index.data().toString() == "↑");
}

// Check if all the key events are matched
// For example, +key,-key returns -1
// +key returns 0, which is the position of +key in the vector
int MacroTableModel::isMacroMatched(){
    QSet<int> visitedIndices;
    const int mLength = macroLines.length();
    // While we could check if the number of items is odd, and if so, return,
    // we won't be able to know where the issue is, so we might as well parse it anyway.
    if(mLength == 1)
        return 0;
    for(int i = 0; i < mLength; i++){
        // If we already visited this key, ignore it
        if(visitedIndices.contains(i))
            continue;
        const MacroLine& ml_i = macroLines.at(i);
        // Outer loop checks for keydowns, inner loop checks for keyups
        // If we get a keyup on the outer loop that wasn't skipped, then there's an error
        if(!ml_i.keyDown)
            return i;
        // No need to add indices visited with i, as neither loop goes back
        int j;
        for(j = i + 1; j < mLength; j++){
            if(visitedIndices.contains(j))
                continue;
            const MacroLine& ml_j = macroLines.at(j);
            if(ml_j.keyDown)
                continue;

            if(ml_i.key == ml_j.key){
                visitedIndices.insert(j);
                break;
            }
        }
        // No match
        if(j == mLength)
            return i;
    }
    // If we get out here, everything matched
    return -1;
}

void MacroTableModel::removeMultipleColumns(QModelIndexList l){
    // The list is not always in order, so sort it from lowest to highest
    std::sort(l.begin(), l.end(), [](const QModelIndex& a, const QModelIndex& b) { return a < b; });
    // and then iterate counting down
    for(int i = l.length(); i--;){
        const int row = l.at(i).row();
        emit beginRemoveRows(QModelIndex(), row, row);
        macroLines.remove(row);
        emit endRemoveRows();
    }
}

bool MacroTableModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int dstrow, int column, const QModelIndex& parent){
    if(dstrow == -1 || action != Qt::MoveAction || !data->hasFormat("application/x-qabstractitemmodeldatalist"))
        return false;
    QByteArray e = data->data("application/x-qabstractitemmodeldatalist");
    QDataStream stream(&e, QIODevice::ReadOnly);

    // Set to keep track of selected items in the vector
    QSet<const MacroLine* const> rows;

    while (!stream.atEnd()){
        // Get the source row, and ignore columns other than 0
        int srcrow = -1;
        int srccol = -1;
        QMap<int,  QVariant> roledata;
        stream >> srcrow >> srccol >> roledata;

        // Only process moves on col == 0, as that will always exist and we move whole rows
        // Make sure r is valid and within range
        if(srccol || srcrow == -1 || srcrow > length() - 1)
            continue;

        // Finally add the address of the item to the set
        const MacroLine& ml = macroLines.at(srcrow);
        rows.insert(&ml);
    }

    if(rows.isEmpty())
        return true;

    emit beginResetModel();

    // Move the items to the destination index
    std::stable_partition(macroLines.begin(), macroLines.begin() + dstrow, [&rows](const MacroLine& ml){return !rows.contains(&ml);});
    std::stable_partition(macroLines.begin() + dstrow, macroLines.end(), [&rows](const MacroLine& ml){return rows.contains(&ml);});

    emit endResetModel();
    return true;
}

void MacroTableModel::removeEmptyRowAtEnd(){
    const int i = macroLines.length() - 1;
    if(!macroLines.at(i).key.isEmpty())
        return;

    emit beginRemoveRows(QModelIndex(), i, i);
    macroLines.remove(i);
    emit endRemoveRows();
}

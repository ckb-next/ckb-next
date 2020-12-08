#ifndef MODELISTWIDGET_H
#define MODELISTWIDGET_H

#include <QTableView>
#include <QResizeEvent>
#include <QTimer>
#include "kb.h"
#include "modelisttablemodel.h"

class ModeListWidget : public QTableView
{
    Q_OBJECT
public:
    explicit ModeListWidget(QWidget* parent = 0);
    void setDevice(Kb* dev);
    inline void setIgnoreFocusLoss(const bool e) { ignoreFocusLoss = e; }
    ~ModeListWidget();
signals:
    void orderChanged();

private:
    void leaveEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    Kb* device;
    QTableView* floatingView;
    void updateFloatingGeometry();
    ModeListTableModel* modelptr;
    bool ignoreFocusLoss;
};

class FloatingDelegate : public QStyledItemDelegate
{
protected:
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

#endif // MODELISTWIDGET_H

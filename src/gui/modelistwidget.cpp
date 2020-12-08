#include <QUuid>
#include "modelistwidget.h"
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QDropEvent>
#include "modelisttablemodel.h"
#include <QAbstractSlider>
#include <QScrollBar>
#include <QApplication>

ModeListWidget::ModeListWidget(QWidget *parent) :
    QTableView(parent), device(nullptr), floatingView(nullptr), modelptr(nullptr), ignoreFocusLoss(false)
{
    connect(this, &QTableView::entered, this, [this](const QModelIndex& index){
        modelptr->setHighlightedRow(index.row());
    });
}

void ModeListWidget::setDevice(Kb* dev){
    modelptr = new ModeListTableModel(dev, this);
    device = dev;
    setModel(modelptr);
    // Highlight the new item on reorder
    connect(modelptr, &QAbstractItemModel::layoutChanged, this, [this](){
        const int index = device->currentProfile()->indexOf(device->currentMode());
        if(index < 0)
            return;
        setCurrentIndex(modelptr->index(index, 0));
    });

    // Model needs to know the current row to set the appropriate bg colour on the lightning table
    connect(selectionModel(), &QItemSelectionModel::currentRowChanged, this, [this](const QModelIndex& index){
        modelptr->setActiveRow(index.row());
    });

    QHeaderView* const hh = horizontalHeader();
    hh->setSectionResizeMode(ModeListTableModel::COL_MODE_ICON, QHeaderView::ResizeToContents);
    hh->setSectionResizeMode(ModeListTableModel::COL_MODE_NAME, QHeaderView::Fixed);
    hh->hideSection(ModeListTableModel::COL_EVENT_ICON);

    floatingView = new QTableView(this);
    floatingView->setMouseTracking(true);
    connect(floatingView, &QTableView::entered, this, [this](const QModelIndex& index){
        modelptr->setHighlightedRow(index.row());
    });

    floatingView->setModel(modelptr);
    floatingView->setSelectionModel(selectionModel());
    floatingView->setSelectionBehavior(QAbstractItemView::SelectRows);
    floatingView->setSelectionMode(QAbstractItemView::SingleSelection);
    floatingView->setItemDelegateForColumn(ModeListTableModel::COL_EVENT_ICON, new FloatingDelegate);

    floatingView->setShowGrid(false);
    floatingView->horizontalHeader()->hideSection(ModeListTableModel::COL_MODE_ICON);
    floatingView->horizontalHeader()->hideSection(ModeListTableModel::COL_MODE_NAME);
    floatingView->setFocusPolicy(Qt::NoFocus);
    floatingView->verticalHeader()->hide();
    floatingView->horizontalHeader()->hide();
    floatingView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    floatingView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    floatingView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    floatingView->setHorizontalScrollMode(ScrollPerPixel);
    floatingView->setVerticalScrollMode(ScrollPerPixel);
    floatingView->setStyleSheet("QTableView { border: none; }");
    floatingView->horizontalHeader()->setMinimumSectionSize(0);
    floatingView->horizontalHeader()->setDefaultSectionSize(65);
    floatingView->verticalHeader()->setMinimumSectionSize(28);
    floatingView->verticalHeader()->setDefaultSectionSize(28);
    viewport()->stackUnder(floatingView);
    floatingView->setIconSize(iconSize());
    floatingView->setContextMenuPolicy(Qt::CustomContextMenu);

    updateFloatingGeometry();

    connect(floatingView->verticalScrollBar(), &QAbstractSlider::valueChanged, [this](int value){
        floatingView->verticalScrollBar()->blockSignals(true);
        verticalScrollBar()->setValue(value);
        floatingView->verticalScrollBar()->blockSignals(false);
    });
    connect(verticalScrollBar(), &QAbstractSlider::valueChanged, [this](int value){
        verticalScrollBar()->blockSignals(true);
        floatingView->verticalScrollBar()->setValue(value);
        verticalScrollBar()->blockSignals(false);
    });

    // Pass through some needed signals
    connect(floatingView, &QTableView::clicked, this, [this](const QModelIndex& idx) { emit clicked(idx); });
    connect(floatingView, &QTableView::doubleClicked, this, [this](const QModelIndex& idx) { emit doubleClicked(idx); });
    // Don't lose "focus" when right clicking
    connect(floatingView, &QTableView::customContextMenuRequested, this, [this](const QPoint& p) {
        emit customContextMenuRequested(p);
    });

#ifndef USE_XCB_EWMH
    floatingView->horizontalHeader()->setSectionHidden(2, true);
    floatingView->hide();
#endif
}

ModeListWidget::~ModeListWidget(){
    delete floatingView;
}

// Hide the hover background effect when leaving the widget
void ModeListWidget::leaveEvent(QEvent* event) {
    modelptr->setHighlightedRow(-1);
    QTableView::leaveEvent(event);
}

void ModeListWidget::resizeEvent(QResizeEvent* event) {
    QTableView::resizeEvent(event);
    updateFloatingGeometry();

    // Calculate the width of the model name column
    // It "stretches" if it's smaller than the free space, or expands to its full size if larger
    // to create a scrollbar
    int maxWidth = viewport()->width() - frameWidth()*2 - verticalHeader()->sectionSize(ModeListTableModel::COL_MODE_ICON);
    if(verticalScrollBar()->isVisible())
        maxWidth -= verticalScrollBar()->width();

    const int actualWidth = sizeHintForColumn(ModeListTableModel::COL_MODE_NAME) + floatingView->horizontalHeader()->sectionSize(ModeListTableModel::COL_EVENT_ICON);
    horizontalHeader()->resizeSection(ModeListTableModel::COL_MODE_NAME, qMax<int>(maxWidth, actualWidth));
}

// Keep track of focus events to control the colour of the active row on the floating widget
void ModeListWidget::focusOutEvent(QFocusEvent* event) {
    QTableView::focusOutEvent(event);
    if(ignoreFocusLoss)
        return;
    modelptr->setHasFocus(false);
}

void ModeListWidget::focusInEvent(QFocusEvent* event) {
    QTableView::focusInEvent(event);
    modelptr->setHasFocus(true);
}

void ModeListWidget::updateFloatingGeometry() {
    const int width = floatingView->horizontalHeader()->sectionSize(2);
    floatingView->setGeometry(horizontalHeader()->width() + frameWidth() - width,
                              frameWidth(), width,
                              viewport()->height() + horizontalHeader()->height());
}

void FloatingDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    // Trick the style into drawing the item as enabled but not selected so that we can control its background
    // using the model
    if(opt.backgroundBrush != Qt::NoBrush && dynamic_cast<const ModeListTableModel*>(index.model())->hasFocus())
        opt.state &= ~QStyle::State_Selected;

    QStyle* style = (option.widget ? option.widget->style() : QApplication::style());
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, option.widget);
}

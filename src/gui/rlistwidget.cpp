#include <QUuid>
#include "rlistwidget.h"

RListWidget::RListWidget(QWidget *parent) :
    QListWidget(parent)
{
    setDragDropMode(QAbstractItemView::InternalMove);
    setMovement(QListView::Snap);

    reorderTimer.setSingleShot(true);
    reorderTimer.setInterval(100);
    connect(&reorderTimer, SIGNAL(timeout()), this, SLOT(timerTick()));

    connect(this, SIGNAL(itemEntered(QListWidgetItem*)), this, SLOT(enter(QListWidgetItem*)));
    connect(this, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(change(QListWidgetItem*)));
}

void RListWidget::timerTick(){
    bool reordered = false;
    QListWidgetItem* reselect = 0;
    QList<QVariant> newItems;
    // Scan the item list to see if they changed
    int c = count();
    for(int i = 0; i < c; i++){
        QListWidgetItem* itm = item(i);
        QVariant data = itm->data(DATA_ROLE);
        newItems.append(data);
        if(i >= previousItems.count() || data != previousItems[i])
            reordered = true;
        // Re-select the dragged item (if any)
        if(data == dragged)
            reselect = itm;
    }
    if(previousItems.length() != newItems.length())
        return;
    if(reordered){
        previousItems = newItems;
        emit orderChanged();
        if(reselect){
            reselect->setSelected(true);
            setCurrentItem(reselect);
            dragged = QVariant();
        }
    }
}

void RListWidget::enter(QListWidgetItem* item){
    rescanItems();
    // Check for drag+drop setup
    if(item)
        dragged = item->data(DATA_ROLE);
}

void RListWidget::change(QListWidgetItem* item){
    reorderTimer.stop();
    reorderTimer.start();
}

void RListWidget::rescanItems(){
    QList<QVariant> newItems;
    int c = count();
    for(int i = 0; i < c; i++){
        QVariant data = this->item(i)->data(DATA_ROLE);
        if(data.isNull()){
            // Generate the ID for this item if it doesn't already exist
            data = QUuid::createUuid();
            this->item(i)->setData(DATA_ROLE, data);
        }
        newItems.append(data);
    }
    previousItems = newItems;
}

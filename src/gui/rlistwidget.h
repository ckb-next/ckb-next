#ifndef RLISTWIDGET_H
#define RLISTWIDGET_H

#include <QListWidget>
#include <QTimer>

// Reorderable list widget

class RListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit RListWidget(QWidget *parent = 0);

Q_SIGNALS:
    void orderChanged();

private Q_SLOTS:
    void timerTick();
    void enter(QListWidgetItem* item);
    void change(QListWidgetItem* item);

private:
    QVariant        currentData;
    QList<QVariant> previousItems;
    QVariant        dragged;
    QTimer          reorderTimer;

    const static int DATA_ROLE = Qt::UserRole + 100;

    void rescanItems();
};

#endif // RLISTWIDGET_H

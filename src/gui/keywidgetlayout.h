#ifndef KEYWIDGETLAYOUT_H
#define KEYWIDGETLAYOUT_H

#include <QLayout>
#include <QLayoutItem>
#include <QRect>
#include <typeinfo>
#include "keywidget.h"
#include <cmath>
#include <cassert>

class KeyWidgetLayout : public QLayout
{
public:
    KeyWidgetLayout(QWidget* parent = nullptr) : QLayout(parent), keyWidget(nullptr), controls(nullptr) {
    }
    ~KeyWidgetLayout(){
        delete keyWidget;
    }

    QLayoutItem* itemAt(int i) const override {
        switch(i){
        case 0:
            return keyWidget;
        case 1:
            return controls;
        default:
            return nullptr;
        }
    }

    QLayoutItem* takeAt(int i) override {
        QLayoutItem* tempItem;
        switch(i){
        case 0:
            tempItem = keyWidget;
            keyWidget = nullptr;
            break;
        case 1:
            tempItem = controls;
            controls = nullptr;
            break;
        default:
            return nullptr;
        }
        return tempItem;
    }

    void addItem(QLayoutItem* i) override {
        assert(keyWidget == nullptr || controls == nullptr);
        if(keyWidget){
            controls = i;
            return;
        }
        keyWidget = i;
    }

    bool isEmpty() const override {
        return !(keyWidget || controls);
    }

    int count() const override {
        if(keyWidget && controls)
            return 2;
        if(keyWidget)
            return 1;
        return 0;
    }

    QSize minimumSize() const override {
        return (keyWidget && controls) ? (keyWidget->minimumSize() + controls->minimumSize()) : QSize(0, 0);
    }

    QSize sizeHint() const override {
        return (keyWidget && controls) ? (keyWidget->sizeHint() + controls->sizeHint()) : QSize(0, 0);
    }

    void setGeometry(const QRect& r) override {
        QLayout::setGeometry(r);

        if(!(keyWidget && controls))
            return;

        KeyWidget* w = dynamic_cast<KeyWidget*>(keyWidget->widget());
        // Calculate the width/height while preserving the aspect ratio
        // This is needed because Qt does not support any kind of "widthForHeight"
        int keyWidgetHeight = keyWidget->heightForWidth(r.width());
        if(keyWidgetHeight > r.height()){
            keyWidgetHeight = r.height();
        }

        const int controlHeight = controls->sizeHint().height();
        // Make sure there is enough space vertically for the control widget to fit
        if(keyWidgetHeight + controlHeight > r.height()){
            keyWidgetHeight = r.height() - controlHeight;
        }

        // Calculate the final width
        int commonWidth = std::round(keyWidgetHeight * w->aspectRatio());

        // If the calculated width is not enough for the control widget, force it to be that
        const int controlWidthHint = controls->sizeHint().width();

        int keyWidgetWidth = commonWidth;
        if(commonWidth < controlWidthHint){
            commonWidth = controlWidthHint;
            keyWidgetHeight = keyWidget->heightForWidth(keyWidgetWidth);
        }

        // Calculate the horizontal offset to centre the widgets
        const int keyWidgetXOff = (r.width() - keyWidgetWidth) / 2;
        const int controlXOff = (r.width() - commonWidth) / 2;

        QRect keyWidgetRect = QRect(r.x() + keyWidgetXOff, r.y(), keyWidgetWidth, keyWidgetHeight);
        QRect controlRect = QRect(r.x() + controlXOff, r.y() + keyWidgetRect.height(), commonWidth, controlHeight);

        keyWidget->setGeometry(keyWidgetRect);
        controls->setGeometry(controlRect);
    }

private:
    QLayoutItem* keyWidget;
    QLayoutItem* controls;
};

#endif // KEYWIDGETLAYOUT_H

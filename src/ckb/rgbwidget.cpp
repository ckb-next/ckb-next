#include <cmath>
#include <QPainter>
#include "rgbwidget.h"

static const int KEY_SIZE = 12;

RgbWidget::RgbWidget(QWidget *parent) :
    QWidget(parent), mouseDownX(-1), mouseDownY(-1), mouseCurrentX(-1), mouseCurrentY(-1)
{
}

void RgbWidget::map(const KeyMap& newMap){
    keyMap = newMap;
    selection = QBitArray(keyMap.count());
    newSelection = QBitArray(keyMap.count());
    setFixedSize((keyMap.width() + KEY_SIZE) * 2, (keyMap.height() + KEY_SIZE) * 2);
    update();
}

void RgbWidget::set(const QString& name, const QColor& color){
    keyMap.color(name, color);
    update();
}

void RgbWidget::set(const QColor& color){
    keyMap.colorAll(color);
    update();
}

void RgbWidget::setSelected(const QColor& color){
    uint count = keyMap.count();
    for(uint i = 0; i < count; i++){
        if(selection.testBit(i))
            keyMap.color(i, color);
    }
}

void RgbWidget::paintEvent(QPaintEvent*){
    const QColor bColor(64, 60, 60);
    const QColor kColor(112, 110, 110);
    const QColor hColor(160, 168, 240);

    float xScale = (float)width() / (keyMap.width() + KEY_SIZE);
    float yScale = (float)height() / (keyMap.height() + KEY_SIZE);
    uint count = keyMap.count();

    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QBitArray highlight;
    switch(mouseDownMode){
    case SET:
        highlight = newSelection;
        break;
    case ADD:
        highlight = selection | newSelection;
        break;
    case SUBTRACT:
        highlight = selection & ~newSelection;
        break;
    default:
        highlight = selection;
    }
    painter.setBrush(QBrush(bColor));
    painter.drawRect(0, 0, width(), height());
    if(mouseDownMode != NONE && (mouseDownX != mouseCurrentX || mouseDownY != mouseCurrentY)){
        int x1 = (mouseDownX > mouseCurrentX) ? mouseCurrentX : mouseDownX;
        int x2 = (mouseDownX > mouseCurrentX) ? mouseDownX : mouseCurrentX;
        int y1 = (mouseDownY > mouseCurrentY) ? mouseCurrentY : mouseDownY;
        int y2 = (mouseDownY > mouseCurrentY) ? mouseDownY : mouseCurrentY;
        QColor hColor2 = hColor;
        hColor2.setAlpha(128);
        painter.setBrush(QBrush(hColor2));
        painter.drawRect(x1, y1, x2 - x1, y2 - y1);
    }

    for(uint i = 0; i < count; i++){
        const KeyPos& key = *keyMap.key(i);
        float x = key.x + 6.f - key.width / 2.f + 1.f;
        float y = key.y + 6.f - key.height / 2.f + 1.f;
        float w = key.width - 2.f;
        float h = key.height - 2.f;
        if(!strcmp(key.name, "enter"))
            y = key.y + 1.f;
        if(highlight.testBit(i))
            painter.setBrush(QBrush(hColor));
        else
            painter.setBrush(QBrush(kColor));
        if(!strcmp(key.name, "mr") || !strcmp(key.name, "m1") || !strcmp(key.name, "m2") || !strcmp(key.name, "m3")
                || !strcmp(key.name, "light") || !strcmp(key.name, "lock")){
            x += w / 8.f;
            y += h / 8.f;
            w *= 0.75f;
            h *= 0.75f;
            painter.drawEllipse(QRectF(x * xScale, y * yScale, w * xScale, h * yScale));
        } else {
            // UK enter key isn't rectangular
            if(key.height == 24 && !strcmp(key.name, "enter")){
                h = 10.f;
                painter.drawRect(QRectF((x + w - 13.f) * xScale, y * yScale, 13.f * xScale, 22.f * yScale));
            }
            painter.drawRect(QRectF(x * xScale, y * yScale, w * xScale, h * yScale));
        }
        x += w / 2.f - 2.f;
        y += h / 2.f - 2.f;
        w = 4.f;
        h = 4.f;
        painter.setBrush(QBrush(QColor(255, 255, 255)));
        painter.drawEllipse(QRectF(x * xScale, y * yScale, w * xScale, h * yScale));
        x += w / 2.f - 1.5f;
        y += h / 2.f - 1.5f;
        w = 3.f;
        h = 3.f;
        painter.setBrush(QBrush(keyMap.color(i)));
        painter.drawEllipse(QRectF(x * xScale, y * yScale, w * xScale, h * yScale));
    }
}

void RgbWidget::mousePressEvent(QMouseEvent* event){
    mouseDownMode = (event->modifiers() & Qt::AltModifier) ? SUBTRACT : (event->modifiers() & Qt::ControlModifier) ? ADD : SET;
    // If shift is held down and the previous mousedown didn't have any movement, select all keys between the two clicks
    if(event->modifiers() & Qt::ShiftModifier && mouseCurrentX >= 0 && mouseCurrentY >= 0){
        // Use mouseMoveEvent to make the selection
        mouseDownMode = SET;
        mouseMoveEvent(event);
        // And immediately close it off
        mouseReleaseEvent(event);
        mouseDownX = mouseCurrentX = event->x();
        mouseDownY = mouseCurrentY = event->y();
        return;
    }
    mouseDownX = mouseCurrentX = event->x();
    mouseDownY = mouseCurrentY = event->y();
    // See if the event hit any keys
    float xScale = (float)width() / (keyMap.width() + KEY_SIZE);
    float yScale = (float)height() / (keyMap.height() + KEY_SIZE);
    float mx = mouseCurrentX / xScale, my = mouseCurrentY / yScale;
    uint count = keyMap.count();
    for(uint i = 0; i < count; i++){
        const KeyPos& key = *keyMap.key(i);
        if(fabs(key.x - mx + 6.f) <= 5.f && fabs(key.y - my + 6.f) <= 5.f){
            newSelection.setBit(i);
            update();
            break;
        }
    }
}

void RgbWidget::mouseMoveEvent(QMouseEvent* event){
    if(mouseDownMode == NONE)
        return;
    // Find selection rectangle
    mouseCurrentX = event->x();
    mouseCurrentY = event->y();
    float xScale = (float)width() / (keyMap.width() + KEY_SIZE);
    float yScale = (float)height() / (keyMap.height() + KEY_SIZE);
    float mx1, mx2, my1, my2;
    if(mouseCurrentX >= mouseDownX){
        mx1 = mouseDownX / xScale - 5.f;
        mx2 = mouseCurrentX / xScale + 5.f;
    } else {
        mx1 = mouseCurrentX / xScale - 5.f;
        mx2 = mouseDownX / xScale + 5.f;
    }
    if(mouseCurrentY >= mouseDownY){
        my1 = mouseDownY / yScale - 5.f;
        my2 = mouseCurrentY / yScale + 5.f;
    } else {
        my1 = mouseCurrentY / yScale - 5.f;
        my2 = mouseDownY / yScale + 5.f;
    }
    // Clear new selection
    newSelection.fill(false);
    // See if the event hit any keys
    uint count = keyMap.count();
    for(uint i = 0; i < count; i++){
        const KeyPos& key = *keyMap.key(i);
        if(key.x + 6.f >= mx1 && key.x + 6.f <= mx2
                && key.y + 6.f >= my1 && key.y + 6.f <= my2){
            newSelection.setBit(i);
        }
    }
    update();
}

void RgbWidget::mouseReleaseEvent(QMouseEvent* event){
    // Apply the new selection
    switch(mouseDownMode){
    case SET:
        selection = newSelection;
        break;
    case ADD:
        selection |= newSelection;
        break;
    case SUBTRACT:
        selection &= ~newSelection;
        break;
    default:;
    }
    // Clear mousedown state.
    newSelection.fill(false);
    mouseDownMode = NONE;
    // Leave mouseCurrentX and mouseCurrentY alone if the mouse didn't move.
    // Used for shift+click above
    if(event->x() != mouseDownX || event->y() != mouseDownY)
        mouseCurrentX = mouseCurrentY = 0;
    update();
    // Determine the color of the selected keys (invalid color if there are any conflicts)
    QColor color = QColor();
    uint count = keyMap.count();
    for(uint i = 0; i < count; i++){
        if(selection.testBit(i)){
            if(!color.isValid()){
                color = keyMap.color(i);
                continue;
            }
            if(keyMap.color(i) != color){
                color = QColor();
                break;
            }
        }
    }
    emit selectionChanged(color, selection.count(true));
}

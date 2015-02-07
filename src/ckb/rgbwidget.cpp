#include <cmath>
#include <QPainter>
#include "rgbwidget.h"

static const int KEY_SIZE = 12;

RgbWidget::RgbWidget(QWidget *parent) :
    QWidget(parent), mouseDownX(-1), mouseDownY(-1), mouseCurrentX(-1), mouseCurrentY(-1), mouseDownMode(NONE)
{
    setMouseTracking(true);
}

void RgbWidget::map(const KeyMap& newMap){
    keyMap = newMap;
    selection = QBitArray(keyMap.count());
    newSelection = QBitArray(keyMap.count());
    animation = QBitArray(keyMap.count());
    setFixedSize((keyMap.width() + KEY_SIZE) * 2, (keyMap.height() + KEY_SIZE) * 2);
    update();
}

void RgbWidget::colorMap(const QHash<QString, QRgb>& newColorMap){
    _colorMap = newColorMap;
    update();
}

void RgbWidget::paintEvent(QPaintEvent*){
    const QColor bColor(64, 60, 60);
    const QColor kColor(112, 110, 110);
    const QColor hColor(136, 176, 240);
    const QColor haColor(136, 200, 240);
    const QColor aColor(112, 200, 110);

    float xScale = (float)width() / (keyMap.width() + KEY_SIZE);
    float yScale = (float)height() / (keyMap.height() + KEY_SIZE);
    uint count = keyMap.count();

    // Determine which keys to highlight
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
    case TOGGLE:
        highlight = selection ^ newSelection;
        break;
    default:
        highlight = selection;
    }

    QPainter painter(this);
    // Draw background
    painter.setPen(Qt::NoPen);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(QBrush(bColor));
    painter.drawRect(0, 0, width(), height());

    // Draw key backgrounds
    KeyMap::Model model = keyMap.model();
    for(uint i = 0; i < count; i++){
        const KeyPos& key = *keyMap.key(i);
        float x = key.x + 6.f - key.width / 2.f + 1.f;
        float y = key.y + 6.f - key.height / 2.f + 1.f;
        float w = key.width - 2.f;
        float h = key.height - 2.f;
        // Set color based on key highlight
        if(highlight.testBit(i)){
            if(animation.testBit(i))
                painter.setBrush(QBrush(haColor));
            else
                painter.setBrush(QBrush(hColor));
        } else if(animation.testBit(i))
            painter.setBrush(QBrush(aColor));
        else
            painter.setBrush(QBrush(kColor));
        if(!strcmp(key.name, "mr") || !strcmp(key.name, "m1") || !strcmp(key.name, "m2") || !strcmp(key.name, "m3")
                || !strcmp(key.name, "light") || !strcmp(key.name, "lock") || (model == KeyMap::K65 && !strcmp(key.name, "mute"))){
            // Switch keys are circular
            x += w / 8.f;
            y += h / 8.f;
            w *= 0.75f;
            h *= 0.75f;
            painter.drawEllipse(QRectF(x * xScale, y * yScale, w * xScale, h * yScale));
        } else {
            if(!strcmp(key.name, "enter")){
                if(key.height == 24){
                    // ISO enter key isn't rectangular
                    y = key.y + 1.f;
                    h = 10.f;
                    painter.drawRect(QRectF((x + w - 13.f) * xScale, y * yScale, 13.f * xScale, 22.f * yScale));
                } else {
                    // US enter key isn't perfectly centered, needs an extra pixel on the left to appear correctly
                    x -= 1.f;
                    w += 1.f;
                }
            } else if(!strcmp(key.name, "rshift") || !strcmp(key.name, "stop")){
                // A few other keys also need extra pixels
                x -= 1.f;
                w += 1.f;
            } else if(!strcmp(key.name, "caps") || !strcmp(key.name, "lshift") || !strcmp(key.name, "next")){
                w += 1.f;
            }
            painter.drawRect(QRectF(x * xScale, y * yScale, w * xScale, h * yScale));
        }
    }

    // Draw mouse highlight (if any)
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

    // Draw key colors
    for(uint i = 0; i < count; i++){
        const KeyPos& key = *keyMap.key(i);
        float x = key.x + 6.f - 2.f;
        float y = key.y + 6.f - 2.f;
        float w = 4.f;
        float h = 4.f;
        painter.setBrush(QBrush(QColor(255, 255, 255)));
        painter.drawEllipse(QRectF(x * xScale, y * yScale, w * xScale, h * yScale));
        x += w / 2.f - 1.5f;
        y += h / 2.f - 1.5f;
        w = 3.f;
        h = 3.f;
        painter.setBrush(QBrush(_colorMap[key.name]));
        painter.drawEllipse(QRectF(x * xScale, y * yScale, w * xScale, h * yScale));
    }
}

void RgbWidget::mousePressEvent(QMouseEvent* event){
    event->accept();
    mouseDownMode = (event->modifiers() & Qt::AltModifier) ? SUBTRACT : (event->modifiers() & Qt::ShiftModifier) ? ADD : (event->modifiers() & Qt::ControlModifier) ? TOGGLE : SET;
    mouseDownX = mouseCurrentX = event->x();
    mouseDownY = mouseCurrentY = event->y();
    // See if the event hit a key
    float xScale = (float)width() / (keyMap.width() + KEY_SIZE);
    float yScale = (float)height() / (keyMap.height() + KEY_SIZE);
    float mx = mouseCurrentX / xScale - 6.f, my = mouseCurrentY / yScale - 6.f;
    uint count = keyMap.count();
    for(uint i = 0; i < count; i++){
        const KeyPos& key = *keyMap.key(i);
        if(fabs(key.x - mx) <= key.width / 2.f - 2.f && fabs(key.y - my) <= key.height / 2.f - 2.f){
            newSelection.setBit(i);
            update();
            break;
        }
    }
}

void RgbWidget::mouseMoveEvent(QMouseEvent* event){
    event->accept();
    if(mouseDownMode != NONE){
        // Find selection rectangle
        mouseCurrentX = event->x();
        mouseCurrentY = event->y();
        float xScale = (float)width() / (keyMap.width() + KEY_SIZE);
        float yScale = (float)height() / (keyMap.height() + KEY_SIZE);
        float mx1, mx2, my1, my2;
        if(mouseCurrentX >= mouseDownX){
            mx1 = mouseDownX / xScale - 6.f;
            mx2 = mouseCurrentX / xScale - 6.f;
        } else {
            mx1 = mouseCurrentX / xScale - 6.f;
            mx2 = mouseDownX / xScale - 6.f;
        }
        if(mouseCurrentY >= mouseDownY){
            my1 = mouseDownY / yScale - 6.f;
            my2 = mouseCurrentY / yScale - 6.f;
        } else {
            my1 = mouseCurrentY / yScale - 6.f;
            my2 = mouseDownY / yScale - 6.f;
        }
        // Clear new selection
        newSelection.fill(false);
        // See if the event hit any keys
        uint count = keyMap.count();
        for(uint i = 0; i < count; i++){
            // Find key rectangle
            const KeyPos& key = *keyMap.key(i);
            float kx1 = key.x - key.width / 2.f + 1.f;
            float ky1 = key.y - key.height / 2.f + 1.f;
            float kx2 = kx1 + key.width - 2.f;
            float ky2 = ky1 + key.height - 2.f;
            // If they overlap, add the key to the selection
            if(!(mx1 >= kx2 || kx1 >= mx2)
                    && !(my1 >= ky2 || ky1 >= my2))
                newSelection.setBit(i);
        }
        update();
    }
    // Update tooltip with the moused-over key (if any)
    float xScale = (float)width() / (keyMap.width() + KEY_SIZE);
    float yScale = (float)height() / (keyMap.height() + KEY_SIZE);
    float mx = event->x() / xScale - 6.f, my = event->y() / yScale - 6.f;
    uint count = keyMap.count();
    for(uint i = 0; i < count; i++){
        const KeyPos& key = *keyMap.key(i);
        if(fabs(key.x - mx) <= key.width / 2.f - 2.f && fabs(key.y - my) <= key.height / 2.f - 2.f){
            setToolTip(key.friendlyName ? QString(key.friendlyName) : QString(key.name).toUpper());
            return;
        }
    }
    setToolTip("");
}

void RgbWidget::mouseReleaseEvent(QMouseEvent* event){
    event->accept();
    if(mouseDownMode == NONE)
        return;
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
    case TOGGLE:
        selection ^= newSelection;
        break;
    default:;
    }
    // Clear mousedown state.
    newSelection.fill(false);
    mouseDownMode = NONE;
    QStringList selectedNames;
    uint count = keyMap.count();
    for(uint i = 0; i < count; i++){
        if(selection.testBit(i))
            selectedNames << keyMap.key(i)->name;
    }
    emit selectionChanged(selectedColor(), selectedNames);
    update();
}

void RgbWidget::setSelection(const QStringList& keys){
    selection.fill(false);
    foreach(QString key, keys){
        int index = keyMap.index(key);
        if(index >= 0)
            selection.setBit(index);
    }
    newSelection.fill(false);
    mouseDownMode = NONE;
    update();
    emit selectionChanged(selectedColor(), keys);
}

void RgbWidget::clearSelection(){
    selection.fill(false);
    newSelection.fill(false);
    mouseDownMode = NONE;
    update();
    emit selectionChanged(QColor(), QStringList());
}

void RgbWidget::setAnimation(const QStringList& keys){
    animation.fill(false);
    foreach(QString key, keys){
        int index = keyMap.index(key);
        if(index >= 0)
            animation.setBit(index);
    }
    update();
}

void RgbWidget::setAnimationToSelection(){
    animation = selection;
    update();
}

void RgbWidget::clearAnimation(){
    animation.fill(false);
    update();
}

QColor RgbWidget::selectedColor(){
    QColor color = QColor();
    uint count = keyMap.count();
    for(uint i = 0; i < count; i++){
        if(selection.testBit(i)){
            QColor newColor = _colorMap[keyMap.key(i)->name];
            if(!color.isValid()){
                color = newColor;
                continue;
            }
            if(newColor != color)
                return QColor();
        }
    }
    return color;
}

#include <cmath>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QPainter>
#include "keywidget.h"
#include "kbbind.h"

static const int KEY_SIZE = 12;

KeyWidget::KeyWidget(QWidget *parent, bool rgbMode) :
    QWidget(parent), mouseDownX(-1), mouseDownY(-1), mouseCurrentX(-1), mouseCurrentY(-1), mouseDownMode(NONE), _rgbMode(rgbMode)
{
    setMouseTracking(true);
}

void KeyWidget::map(const KeyMap& newMap){
    keyMap = newMap;
    selection = QBitArray(keyMap.count());
    newSelection = QBitArray(keyMap.count());
    animation = QBitArray(keyMap.count());
    setFixedSize((keyMap.width() + KEY_SIZE) * 2.3, (keyMap.height() + KEY_SIZE) * 2.3);
    update();
}

void KeyWidget::colorMap(const QHash<QString, QRgb>& newColorMap){
    _colorMap = newColorMap;
    update();
}

void KeyWidget::bindMap(const QHash<QString, QString>& newBindMap){
    _bindMap = newBindMap;
    update();
}

void KeyWidget::paintEvent(QPaintEvent*){
    const QColor bgColor(64, 60, 60);
    const QColor keyColor(112, 110, 110);
    const QColor highlightColor(136, 176, 240);
    const QColor highlightAnimColor(136, 200, 240);
    const QColor animColor(112, 200, 110);

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
    painter.setBrush(QBrush(bgColor));
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
                painter.setBrush(QBrush(highlightAnimColor));
            else
                painter.setBrush(QBrush(highlightColor));
        } else if(animation.testBit(i))
            painter.setBrush(QBrush(animColor));
        else
            painter.setBrush(QBrush(keyColor));
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
        QColor hColor2 = highlightColor;
        hColor2.setAlpha(128);
        painter.setBrush(QBrush(hColor2));
        painter.drawRect(x1, y1, x2 - x1, y2 - y1);
    }

    if(_rgbMode){
        // Draw key colors (RGB mode)
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
    } else {
        // Render the text to a separate pixmap so that it can be drawn with a drop shadow
        int wWidth = width(), wHeight = height();
        QPixmap txt(wWidth, wHeight);
        txt.fill(QColor(0, 0, 0, 0));
        QPainter txtPainter(&txt);
        // Draw key names
        txtPainter.setBrush(Qt::NoBrush);
        QFont font = painter.font();
        font.setBold(true);
        font.setPixelSize(4.8f * yScale);
        QFont font0 = font;
        for(uint i = 0; i < count; i++){
            const KeyPos& key = *keyMap.key(i);
            float x = key.x + 6.f - key.width / 2.f + 1.f;
            float y = key.y + 6.f - key.height / 2.f;
            float w = key.width - 2.f;
            float h = key.height;
            // Print the key's friendly name (with some exceptions)
            QString name = key.friendlyName(false);
            name = name.split(" ").last();
            struct _names {
                const char* keyName, *displayName;
            };
            _names names[] = {
                {"light", "☼"}, {"lock", "☒"}, {"mute", "◖⊘"}, {"volup", "◖))"}, {"voldn", "◖)"},
                {"prtscn",  "PrtScn\nSysRq"}, {"scroll", "Scroll\nLock"}, {"pause", "Pause\nBreak"}, {"stop", "▪"}, {"prev", "|◂◂"}, {"play", "▸||"}, {"next", "▸▸|"},
                {"pgup", "Page\nUp"}, {"pgdn", "Page\nDown"}, {"numlock", "Num\nLock"}, {"caps", "Caps"},
#ifdef Q_OS_MACX
                {"lctrl", "⌃"}, {"rctrl", "⌃"}, {"lwin", "⌘"}, {"rwin", "⌘"}, {"lalt", "⌥"}, {"ralt", "⌥"},
#else
                {"lwin", "❖"}, {"rwin", "❖"},
#endif
                {"rmenu", "▤"}, {"up", "▲"}, {"left", "◀"}, {"down", "▼"}, {"right", "▶"}
            };
            for(uint k = 0; k < sizeof(names) / sizeof(_names); k++){
                const char* keyName = key.name;
#ifdef Q_OS_MACX
                if(KeyPos::osxCmdSwap){
                    if(!strcmp(keyName, "lctrl")) keyName = "lwin";
                    else if(!strcmp(keyName, "rctrl")) keyName = "rwin";
                    else if(!strcmp(keyName, "lwin")) keyName = "lctrl";
                    else if(!strcmp(keyName, "rwin")) keyName = "rctrl";
                }
#endif
                if(!strcmp(keyName, names[k].keyName)){
                    name = names[k].displayName;
                    break;
                }
            }
            if((key.name[0] == 'm' && (key.name[1] == 'r' || key.name[1] <= '3')) || !strcmp(key.name, "up") || !strcmp(key.name, "down") || !strcmp(key.name, "left") || !strcmp(key.name, "right"))
                // Use a smaller size for MR, M1 - M3, and arrow keys
                font.setPixelSize(font.pixelSize() * 0.85);
            else if(!strcmp(key.name, "end"))
                // Use a smaller size for "End" to match everything else in that area
                font.setPixelSize(font.pixelSize() * 0.72);
            else if(!strcmp(key.name, "light")
#ifndef Q_OS_MACX
                    || !strcmp(key.name, "lwin") || !strcmp(key.name, "rwin")
#endif
                    )
                // Use a larger font size for Win and Brightness to compensate for the unicode symbols looking smaller
                font.setPixelSize(font.pixelSize() * 1.3);
            // Determine the appropriate size to draw the text at
            txtPainter.setFont(font);
            QRectF rect(x * xScale, y * yScale - 1, w * xScale, h * yScale);
            int flags = Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextWordWrap;
            QRectF bounds = txtPainter.boundingRect(rect, flags, name);
            while((bounds.height() >= rect.height() - 6. || bounds.width() >= rect.width() - 2.) && font.pixelSize() >= 5){
                // Scale font size down until it fits inside the key
                font.setPixelSize(font.pixelSize() - 2);
                txtPainter.setFont(font);
                bounds = txtPainter.boundingRect(rect, flags, name);
            }
            // Pick color based on key function
            QString bind = _bindMap.value(key.name);
            QString def = KbBind::defaultAction(key.name);
            if(bind.isEmpty())
                // Unbound - red
                txtPainter.setPen(QColor(255, 136, 136));
            else if(KbBind::isSpecial(bind) && (bind == def || !KbBind::isSpecial(def)))
                // Special function - blue (only if not mapped to a different function - if a special function is remapped, color it yellow)
                txtPainter.setPen(QColor(128, 224, 255));
            else if(KbBind::isMedia(bind) && (bind == def || !KbBind::isMedia(def)))
                // Media key - green
                txtPainter.setPen(QColor(160, 255, 168));
            else if(bind == key.name)
                // Standard key - white
                txtPainter.setPen(QColor(255, 255, 255));
            else
                // Remapped key - yellow
                txtPainter.setPen(QColor(255, 248, 128));
            txtPainter.drawText(rect, flags, name);
            font = font0;
        }
        // Apply the drop shadow effect and paint the text
        QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect;  // Have to use "new", creating this on the stack causes a crash...
        effect->setBlurRadius(4.);
        effect->setColor(QColor(0, 0, 0, 192));
        effect->setOffset(0, 1);
        QGraphicsScene scene;
        QGraphicsPixmapItem item;
        item.setPixmap(txt);
        item.setGraphicsEffect(effect);
        scene.addItem(&item);
        scene.render(&painter, QRectF(), QRectF(0, 0, wWidth, wHeight));
        delete effect;
    }
}

void KeyWidget::mousePressEvent(QMouseEvent* event){
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
        if(fabs(key.x - mx) <= key.width / 2.f - 1.f && fabs(key.y - my) <= key.height / 2.f - 1.f){
            newSelection.setBit(i);
            update();
            break;
        }
    }
}

void KeyWidget::mouseMoveEvent(QMouseEvent* event){
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
            setToolTip(key.friendlyName(false));
            return;
        }
    }
    setToolTip("");
}

void KeyWidget::mouseReleaseEvent(QMouseEvent* event){
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
    emit selectionChanged(selectedNames);
    update();
}

void KeyWidget::setSelection(const QStringList& keys){
    selection.fill(false);
    foreach(QString key, keys){
        int index = keyMap.index(key);
        if(index >= 0)
            selection.setBit(index);
    }
    newSelection.fill(false);
    mouseDownMode = NONE;
    update();
    emit selectionChanged(keys);
}

void KeyWidget::clearSelection(){
    selection.fill(false);
    newSelection.fill(false);
    mouseDownMode = NONE;
    update();
    emit selectionChanged(QStringList());
}

void KeyWidget::setAnimation(const QStringList& keys){
    animation.fill(false);
    foreach(QString key, keys){
        int index = keyMap.index(key);
        if(index >= 0)
            animation.setBit(index);
    }
    update();
}

void KeyWidget::setAnimationToSelection(){
    animation = selection;
    update();
}

void KeyWidget::clearAnimation(){
    animation.fill(false);
    update();
}

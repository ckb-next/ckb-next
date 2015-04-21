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

void KeyWidget::colorMap(const ColorMap& newColorMap){
    _colorMap = newColorMap;
    update();
}

void KeyWidget::displayColorMap(ColorMap newDisplayMap){
    if(!isVisible())
        return;
    _displayColorMap = newDisplayMap;
    update();
}

void KeyWidget::bindMap(const BindMap& newBindMap){
    _bindMap = newBindMap;
    update();
}

void KeyWidget::paintEvent(QPaintEvent*){
    const QColor bgColor(68, 64, 64);
    const QColor keyColor(112, 110, 110);
    const QColor highlightColor(136, 176, 240);
    const QColor highlightAnimColor(136, 200, 240);
    const QColor animColor(112, 200, 110);

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
#if QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
    int ratio = painter.device()->devicePixelRatio();
#else
    int ratio = 1;
#endif
    float xScale = (float)width() / (keyMap.width() + KEY_SIZE) * ratio;
    float yScale = (float)height() / (keyMap.height() + KEY_SIZE) * ratio;
    // Draw background
    painter.setPen(Qt::NoPen);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(QBrush(bgColor));
    painter.drawRect(0, 0, width(), height());

    // Draw mouse highlight (if any)
    if(mouseDownMode != NONE && (mouseDownX != mouseCurrentX || mouseDownY != mouseCurrentY)){
        int x1 = (mouseDownX > mouseCurrentX) ? mouseCurrentX : mouseDownX;
        int x2 = (mouseDownX > mouseCurrentX) ? mouseDownX : mouseCurrentX;
        int y1 = (mouseDownY > mouseCurrentY) ? mouseCurrentY : mouseDownY;
        int y2 = (mouseDownY > mouseCurrentY) ? mouseDownY : mouseCurrentY;
        painter.setPen(QPen(highlightColor, 0.5));
        QColor bColor = highlightColor;
        bColor.setAlpha(128);
        painter.setBrush(QBrush(bColor));
        painter.drawRect(x1, y1, x2 - x1, y2 - y1);
    }

    // Draw key backgrounds on a separate pixmap so that a drop shadow can be applied to them.
    int wWidth = width(), wHeight = height();
    KeyMap::Model model = keyMap.model();
    QPixmap keyBG(wWidth * ratio, wHeight * ratio);
    keyBG.fill(QColor(0, 0, 0, 0));
    QPainter bgPainter(&keyBG);
    bgPainter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    bgPainter.setPen(Qt::NoPen);
    for(uint i = 0; i < count; i++){
        const KeyPos& key = *keyMap.key(i);
        float x = key.x + 6.f - key.width / 2.f + 1.f;
        float y = key.y + 6.f - key.height / 2.f + 1.f;
        float w = key.width - 2.f;
        float h = key.height - 2.f;
        // In RGB mode, ignore volume wheel on K70/K95
        if(model != KeyMap::K65 && _rgbMode && (!strcmp(key.name, "volup") || !strcmp(key.name, "voldn")))
            continue;
        // Set color based on key highlight
        if(highlight.testBit(i)){
            if(animation.testBit(i))
                bgPainter.setBrush(QBrush(highlightAnimColor));
            else
                bgPainter.setBrush(QBrush(highlightColor));
        } else if(animation.testBit(i))
            bgPainter.setBrush(QBrush(animColor));
        else
            bgPainter.setBrush(QBrush(keyColor));
        if(!strcmp(key.name, "mr") || !strcmp(key.name, "m1") || !strcmp(key.name, "m2") || !strcmp(key.name, "m3")
                || !strcmp(key.name, "light") || !strcmp(key.name, "lock") || (model == KeyMap::K65 && !strcmp(key.name, "mute"))){
            // Switch keys are circular
            x += w / 8.f;
            y += h / 8.f;
            w *= 0.75f;
            h *= 0.75f;
            bgPainter.drawEllipse(QRectF(x * xScale, y * yScale, w * xScale, h * yScale));
        } else {
            if(!strcmp(key.name, "enter")){
                if(key.height == 24){
                    // ISO enter key isn't rectangular
                    y = key.y + 1.f;
                    h = 10.f;
                    bgPainter.drawRect(QRectF((x + w - 13.f) * xScale, y * yScale, 13.f * xScale, 22.f * yScale));
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
            bgPainter.drawRect(QRectF(x * xScale, y * yScale, w * xScale, h * yScale));
        }
    }

    // Render the key decorations (RGB -> light circles, binding -> key names) on yet another layer
    QPixmap decoration(wWidth * ratio, wHeight * ratio);
    decoration.fill(QColor(0, 0, 0, 0));
    QPainter decPainter(&decoration);
    decPainter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    if(_rgbMode){
        // Draw key colors (RGB mode)
        decPainter.setPen(QPen(QColor(255, 255, 255), 1.5));
        for(uint i = 0; i < count; i++){
            const KeyPos& key = *keyMap.key(i);
            if(model != KeyMap::K65 && _rgbMode && (!strcmp(key.name, "volup") || !strcmp(key.name, "voldn")))
                continue;
            float x = key.x + 6.f - 1.8f;
            float y = key.y + 6.f - 1.8f;
            float w = 3.6f;
            float h = 3.6f;
            if(_displayColorMap.contains(key.name))
                decPainter.setBrush(QBrush(_displayColorMap.value(key.name)));
            else
                decPainter.setBrush(QBrush(_colorMap.value(key.name)));
            decPainter.drawEllipse(QRectF(x * xScale, y * yScale, w * xScale, h * yScale));
        }
    } else {
        // Draw key names
        decPainter.setBrush(Qt::NoBrush);
        QFont font = painter.font();
        font.setBold(true);
        font.setPixelSize(5.25f * yScale);
        QFont font0 = font;
        for(uint i = 0; i < count; i++){
            const KeyPos& key = *keyMap.key(i);
            float x = key.x + 6.f - key.width / 2.f + 1.f;
            float y = key.y + 6.f - key.height / 2.f;
            float w = key.width - 2.f;
            float h = key.height;
            // Print the key's friendly name (with some exceptions)
            QString keyName = KbBind::globalRemap(key.name);
            QString name = key.friendlyName(false);
            name = name.split(" ").last();
            struct _names {
                const char* keyName, *displayName;
            };
            _names names[] = {
                {"light", "☼"}, {"lock", "☒"}, {"mute", "◖⊘"}, {"volup", keyMap.model() == KeyMap::K65 ? "◖))" : "▲"}, {"voldn", keyMap.model() == KeyMap::K65 ? "◖)" : "▼"},
                {"prtscn",  "PrtScn\nSysRq"}, {"scroll", "Scroll\nLock"}, {"pause", "Pause\nBreak"}, {"stop", "▪"}, {"prev", "|◂◂"}, {"play", "▸||"}, {"next", "▸▸|"},
                {"pgup", "Page\nUp"}, {"pgdn", "Page\nDown"}, {"numlock", "Num\nLock"},
                {"caps", "Caps"}, {"lshift", "Shift"}, {"rshift", "Shift"},
#ifdef Q_OS_MACX
                {"lctrl", "⌃"}, {"rctrl", "⌃"}, {"lwin", "⌘"}, {"rwin", "⌘"}, {"lalt", "⌥"}, {"ralt", "⌥"},
#else
                {"lctrl", "Ctrl"}, {"rctrl", "Ctrl"}, {"lwin", "❖"}, {"rwin", "❖"}, {"lalt", "Alt"}, {"ralt", "Alt"},
#endif
                {"rmenu", "▤"}, {"up", "▲"}, {"left", "◀"}, {"down", "▼"}, {"right", "▶"}
            };
            for(uint k = 0; k < sizeof(names) / sizeof(_names); k++){
                if(keyName == names[k].keyName){
                    name = names[k].displayName;
                    break;
                }
            }
            if(keyName == "mr" || keyName == "m1" || keyName == "m2" || keyName == "m3" || keyName == "up" || keyName == "down" || keyName == "left" || keyName == "right")
                // Use a smaller size for MR, M1 - M3, and arrow keys
                font.setPixelSize(font.pixelSize() * 0.75);
            else if(keyName == "end")
                // Use a smaller size for "End" to match everything else in that area
                font.setPixelSize(font.pixelSize() * 0.65);
            else if(keyName == "light"
#ifndef Q_OS_MACX
                    || keyName == "lwin" || keyName == "rwin"
#endif
                    )
                // Use a larger font size for Win and Brightness to compensate for the unicode symbols looking smaller (Linux only)
                font.setPixelSize(font.pixelSize() * 1.3);
            // Determine the appropriate size to draw the text at
            decPainter.setFont(font);
            QRectF rect(x * xScale, y * yScale - 1, w * xScale, h * yScale);
            int flags = Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextWordWrap;
            QRectF bounds = decPainter.boundingRect(rect, flags, name);
            while((bounds.height() >= rect.height() - 8. || bounds.width() >= rect.width() - 2.) && font.pixelSize() >= 5){
                // Scale font size down until it fits inside the key
                font.setPixelSize(font.pixelSize() - 2);
                decPainter.setFont(font);
                bounds = decPainter.boundingRect(rect, flags, name);
            }
            // Pick color based on key function
            QString bind = _bindMap.value(key.name);
            QString def = KbBind::defaultAction(key.name);
            if(bind.isEmpty())
                // Unbound - red
                decPainter.setPen(QColor(255, 136, 136));
            else if(KbBind::isProgram(bind))
                // Custom program - orange
                decPainter.setPen(QColor(255, 224, 192));
            else if(KbBind::isSpecial(bind) && (bind == def || !KbBind::isSpecial(def)))
                // Special function - blue (only if not mapped to a different function - if a special function is remapped, color it yellow)
                decPainter.setPen(QColor(128, 224, 255));
            else if(KbBind::isMedia(bind) && (bind == def || !KbBind::isMedia(def)))
                // Media key - green
                decPainter.setPen(QColor(160, 255, 168));
            else if(bind == def)
                // Standard key - white
                decPainter.setPen(QColor(255, 255, 255));
            else
                // Remapped key - yellow
                decPainter.setPen(QColor(255, 248, 128));
            decPainter.drawText(rect, flags, name);
            font = font0;
        }
    }
    // Create drop shadow effects
    QGraphicsDropShadowEffect* bgEffect = new QGraphicsDropShadowEffect;  // Have to use "new", creating these on the stack causes a crash...
    bgEffect->setBlurRadius(2.);
    bgEffect->setColor(QColor(0, 0, 0, 32));
    bgEffect->setOffset(0, 1);
    QGraphicsDropShadowEffect* decEffect = new QGraphicsDropShadowEffect;
    decEffect->setBlurRadius(4.);
    decEffect->setColor(QColor(0, 0, 0, 104));
    decEffect->setOffset(0, 1);
    // Apply them to the pixmaps
    QGraphicsPixmapItem* bgItem = new QGraphicsPixmapItem(keyBG);
    bgItem->setGraphicsEffect(bgEffect);
    QGraphicsPixmapItem* decItem = new QGraphicsPixmapItem(decoration);
    decItem->setGraphicsEffect(decEffect);
    // Render everything
    QGraphicsScene* scene = new QGraphicsScene;
    scene->addItem(bgItem);
    scene->addItem(decItem);
    // It has to be rendered onto yet another pixmap or else DPI scaling will look terrible...
    QPixmap final(wWidth * ratio, wHeight * ratio);
    final.fill(QColor(0, 0, 0, 0));
    QPainter finalPainter(&final);
    scene->render(&finalPainter, QRectF(0, 0, wWidth * ratio, wHeight * ratio), QRectF(0, 0, wWidth * ratio, wHeight * ratio));
    delete scene;   // <- Automatically cleans up the rest of the objects
#if QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
    final.setDevicePixelRatio(ratio);
#endif
    painter.drawPixmap(QPointF(0., 0.), final);
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
        if(keyMap.model() != KeyMap::K65 && _rgbMode && (!strcmp(key.name, "volup") || !strcmp(key.name, "voldn")))
            continue;
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
            // Ignore volume wheel on K70/K95 in RGB mode
            if(keyMap.model() != KeyMap::K65 && _rgbMode && (!strcmp(key.name, "volup") || !strcmp(key.name, "voldn")))
                continue;
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
        if(keyMap.model() != KeyMap::K65 && _rgbMode && (!strcmp(key.name, "volup") || !strcmp(key.name, "voldn")))
            continue;
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

void KeyWidget::selectAll(){
    selection.fill(true);
    newSelection.fill(false);
    mouseDownMode = NONE;
    update();
    emit selectionChanged(keyMap.allKeys());
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

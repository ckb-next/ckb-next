#include <cmath>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QPainter>
#include "keywidget.h"
#include "keyaction.h"
#include "kbbind.h"

static const int KEY_SIZE = 12;

static QImage* m65Overlay = 0, *sabOverlay = 0, *scimOverlay = 0;

// KbLight.cpp
extern QRgb monoRgb(float r, float g, float b);

KeyWidget::KeyWidget(QWidget *parent, bool rgbMode) :
    QWidget(parent), mouseDownX(-1), mouseDownY(-1), mouseCurrentX(-1), mouseCurrentY(-1), mouseDownMode(NONE), _rgbMode(rgbMode), _monochrome(false)
{
    setMouseTracking(true);
    setAutoFillBackground(false);
}

void KeyWidget::map(const KeyMap& newMap){
    keyMap = newMap;
    selection = QBitArray(keyMap.count());
    newSelection = QBitArray(keyMap.count());
    animation = QBitArray(keyMap.count());
    int width, height;
    if(keyMap.isMouse()){
        width = (keyMap.width() + KEY_SIZE) * 2.6;
        height = (keyMap.height() + KEY_SIZE) * 2.6;
    } else {
        width = (keyMap.width() + KEY_SIZE) * 2.3;
        height = (keyMap.height() + KEY_SIZE) * 2.3;
    }
    if(width < 500)
        width = 500;
    setFixedSize(width, height);
    update();
}

void KeyWidget::drawInfo(float& scale, float& offsetX, float& offsetY, int ratio){
    int w = width() * ratio, h = height() * ratio;
    float xScale = (float)w / (keyMap.width() + KEY_SIZE);
    float yScale = (float)h / (keyMap.height() + KEY_SIZE);
    scale = fmin(xScale, yScale);
    offsetX = (w / scale - keyMap.width()) / 2.f;
    offsetY = (h / scale - keyMap.height()) / 2.f;
}

void KeyWidget::colorMap(const QColorMap& newColorMap){
    _colorMap = newColorMap;
    update();
}

void KeyWidget::displayColorMap(const ColorMap &newDisplayMap, const QSet<QString> &indicators){
    if(!isVisible())
        return;
    _displayColorMap = newDisplayMap;
    _indicators = indicators;
    update();
}

void KeyWidget::bindMap(const BindMap& newBindMap){
    _bindMap = newBindMap;
    update();
}

void KeyWidget::paintEvent(QPaintEvent*){
    const QColor bgColor(68, 64, 64);
    const QColor keyColor(112, 110, 110);
    const QColor sniperColor(130, 90, 90);
    const QColor thumbColor(34, 32, 32);
    const QColor transparentColor(0, 0, 0, 0);
    const QColor highlightColor(136, 176, 240);
    const QColor highlightAnimColor(136, 200, 240);
    const QColor animColor(112, 200, 110);

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
    int wWidth = width(), wHeight = height();
    KeyMap::Model model = keyMap.model();
    KeyMap::Layout layout = keyMap.layout();
    float scale, offX, offY;
    drawInfo(scale, offX, offY, ratio);
    // Draw background
    painter.setPen(Qt::NoPen);
    painter.setRenderHint(QPainter::Antialiasing, true);

    if(keyMap.isMouse()){
        // Draw mouse overlays
        const QImage* overlay = 0;
        float xpos = 0.f, ypos = 0.f;
        if(model == KeyMap::M65){
            if(!m65Overlay)
                m65Overlay = new QImage(":/img/overlay_m65.png");
            overlay = m65Overlay;
            xpos = 2.f;
            ypos = -2.f;
        } else if(model == KeyMap::SABRE){
            if(!sabOverlay)
                sabOverlay = new QImage(":/img/overlay_sabre.png");
            overlay = sabOverlay;
            xpos = 1.f;
            ypos = -2.f;
        } else if(model == KeyMap::SCIMITAR){
            if(!scimOverlay)
                scimOverlay = new QImage(":/img/overlay_scimitar.png");
            overlay = scimOverlay;
            xpos = 3.5f;
            ypos = -2.f;
        }
        if(overlay){
            painter.setBrush(palette().brush(QPalette::Window));
            painter.drawRect(0, 0, width(), height());
            float oXScale = scale / 9.f, oYScale = scale / 9.f;             // The overlay has a resolution of 9px per keymap unit
            float x = (xpos + offX) * scale, y = (ypos + offY) * scale;
            int w = overlay->width() * oXScale, h = overlay->height() * oYScale;
            // We need to transform the image with QImage::scaled() because painter.drawImage() will butcher it, even with smoothing enabled
            // However, the width/height need to be rounded to integers
            int iW = round(w), iH = round(h);
            painter.drawImage(QRectF((x - (iW - w) / 2.f) / ratio, (y - (iH - h) / 2.f) / ratio, iW / ratio, iH / ratio), overlay->scaled(iW, iH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        }
    } else {
        // Otherwise, draw a solid background
        painter.setBrush(QBrush(bgColor));
        painter.drawRect(0, 0, width(), height());
    }

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
    QPixmap keyBG(wWidth * ratio, wHeight * ratio);
    keyBG.fill(QColor(0, 0, 0, 0));
    QPainter bgPainter(&keyBG);
    bgPainter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    bgPainter.setPen(Qt::NoPen);
    QHashIterator<QString, Key> k(keyMap);
    uint i = -1;
    while(k.hasNext()){
        k.next();
        i++;
        const Key& key = k.value();
        float x = key.x + offX - key.width / 2.f + 1.f;
        float y = key.y + offY - key.height / 2.f + 1.f;
        float w = key.width - 2.f;
        float h = key.height - 2.f;
        // In RGB mode, ignore keys without LEDs
        if((_rgbMode && !key.hasLed)
                || (!_rgbMode && !key.hasScan))
            continue;
        // Set color based on key highlight
        bgPainter.setOpacity(1.);
        if(highlight.testBit(i)){
            if(animation.testBit(i))
                bgPainter.setBrush(QBrush(highlightAnimColor));
            else
                bgPainter.setBrush(QBrush(highlightColor));
        } else if(animation.testBit(i)){
            bgPainter.setBrush(QBrush(animColor));
        } else {
            if(!strcmp(key.name, "sniper"))
                // Sniper key uses a reddish base color instead of the usual grey
                bgPainter.setBrush(QBrush(sniperColor));
            else if(model == KeyMap::SCIMITAR && !strncmp(key.name, "thumb", 5) && strcmp(key.name, "thumb"))
                // Thumbgrid keys use a black color
                bgPainter.setBrush(QBrush(thumbColor));
            else if(!strcmp(key.name, "lsidel") || !strcmp(key.name, "rsidel") || !strcmp(key.name, "logo"))
                // Strafe side lights have different background
                bgPainter.setBrush(QBrush(transparentColor));
            else {
                bgPainter.setBrush(QBrush(keyColor));
                if(KeyMap::isMouse(model))
                    bgPainter.setOpacity(0.7);
            }
        }
        if(model != KeyMap::STRAFE && (!strcmp(key.name, "mr") || !strcmp(key.name, "m1") || !strcmp(key.name, "m2") || !strcmp(key.name, "m3")
                || !strcmp(key.name, "light") || !strcmp(key.name, "lock") || (model == KeyMap::K65 && !strcmp(key.name, "mute")))){
            // Switch keys are circular except for Strafe. All Strafe keys are square
            x += w / 8.f;
            y += h / 8.f;
            w *= 0.75f;
            h *= 0.75f;
            bgPainter.drawEllipse(QRectF(x * scale, y * scale, w * scale, h * scale));
        } else {
            if(!strcmp(key.name, "enter")){
                if(key.height == 24){
                    // ISO enter key isn't rectangular
                    y = key.y + 1.f;
                    h = 10.f;
                    bgPainter.drawRect(QRectF((x + w - 13.f) * scale, y * scale, 13.f * scale, 22.f * scale));
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
            bgPainter.drawRect(QRectF(x * scale, y * scale, w * scale, h * scale));
        }
    }

    // Render the key decorations (RGB -> light circles, binding -> key names) on yet another layer
    QPixmap decoration(wWidth * ratio, wHeight * ratio);
    decoration.fill(QColor(0, 0, 0, 0));
    QPainter decPainter(&decoration);
    decPainter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    if(_rgbMode){
        // Draw key colors (RGB mode)
        QHashIterator<QString, Key> k(keyMap);
        uint i = -1;
        while(k.hasNext()){
            k.next();
            i++;
            const Key& key = k.value();
            if(!key.hasLed)
                continue;
            float x = key.x + offX - 1.8f;
            float y = key.y + offY - 1.8f;
            float w = 3.6f;
            float h = 3.6f;
            // Display a white circle around regular keys, red circle around indicators
            if(_indicators.contains(key.name))
                decPainter.setPen(QPen(QColor(255, 248, 136), 1.5));
            else
                decPainter.setPen(QPen(QColor(255, 255, 255), 1.5));
            QRgb color;
            const QRgb* inDisplay = _displayColorMap.colorForName(key.name);
            if(inDisplay)
                // Color in display map? Grab it from there
                // (monochrome conversion not necessary as this would have been done by the animation)
                color = *inDisplay;
            else {
                // Otherwise, read from base map
                color = _colorMap.value(k.key());
                if(_monochrome)
                    color = monoRgb(qRed(color), qGreen(color), qBlue(color));
            }
            decPainter.setBrush(QBrush(color));
            if (model == KeyMap::STRAFE) { // STRAFE custom design and special keys
                float kx = key.x + offX - key.width / 2.f + 1.f;
                float ky = key.y + offY - key.height / 2.f + 1.f;
                float kw = key.width - 2.f;
                float kh = key.height - 2.f;
                decPainter.setPen(QPen(QColor(255, 255, 255), 1.2)); // less invasive outline to show the key color better
                if(!strcmp(key.name, "logo")) { // stylized logo
                    float lx = key.x + offX - key.width / 2.f + 2.f;
                    float ly = key.y + offY - key.height / 2.f + 2.f;
                    float lw = key.width - 4.f;
                    float lh = key.height - 4.f;
                    QPainterPath logo;
                    logo.moveTo(lx*scale,(ly+lh)*scale);
                    logo.quadTo((lx+2.f)*scale,(ly+lh/2.f)*scale,lx*scale,ly*scale);
                    logo.quadTo((lx+lw)*scale,ly*scale,(lx+lw)*scale,(ly+lh)*scale);
                    logo.quadTo((lx+lw/2.f)*scale,(ly+lh-4.f)*scale,lx*scale,(ly+lh)*scale);
                    decPainter.drawPath(logo);
                    //decPainter.setPen(QPen(Qt::green, 1.2)); //QColor(125,125,125)
                    //decPainter.drawRect(QRectF(lx * scale, ly * scale, lw * scale, lh * scale)); // don't really know why the 12 and 24 make it work here, but they do
                } else if(!strcmp(key.name, "lsidel") || !strcmp(key.name, "rsidel")) { // Strafe side lights (toggle lights with no animation)
                    QRadialGradient gradient(QPointF(wWidth/2.f * ratio, wHeight/2.f * ratio), wWidth/2.f * ratio);//,QPointF(10, 5));
                    gradient.setColorAt(0, color);
                    gradient.setColorAt(0.9, color); // bring up intensity
                    gradient.setColorAt(1, bgColor);
                    decPainter.setBrush(QBrush(gradient));
                    decPainter.setPen(QPen(keyColor, 1.2)); //QColor(125,125,125)
                    decPainter.drawRect(QRectF(kx * scale, ky * scale - 12 , kw * scale, kh * scale+24)); // don't really know why the 12 and 24 make it work here, but they do
                } else if(_indicators.contains(key.name)) { // FIX: This check fails whenever _indicators is empty because "show animated" is unchecked
                    decPainter.setPen(QPen(QColor(0,0,0,0), 1));    // no outline for the indicators, you can't change their color the standard way
                    decPainter.drawRect(QRectF((kx+2.f) * scale, (ky+2.f) * scale, (kw-4.f) * scale, (kh-4.f) * scale)); // square indicators
               } else //everything else is a circle, just a tad bigger to show the key color better
                    decPainter.drawEllipse(QRectF((x-1.f) * scale, (y-1.f) * scale, (w+2.f) * scale, (h+2.f) * scale));
            } else
                decPainter.drawEllipse(QRectF(x * scale, y * scale, w * scale, h * scale));
        }
    } else {
        // Draw key names
        decPainter.setBrush(Qt::NoBrush);
        QFont font = painter.font();
        font.setBold(true);
        font.setPixelSize(5.25f * scale);
        QFont font0 = font;
        QHashIterator<QString, Key> k(keyMap);
        uint i = -1;
        while(k.hasNext()){
            k.next();
            i++;
            const Key& key = k.value();
            if(!key.hasScan)
                continue;
            float x = key.x + offX - key.width / 2.f + 1.f;
            float y = key.y + offY - key.height / 2.f;
            float w = key.width - 2.f;
            float h = key.height;
            // Print the key's friendly name (with some exceptions)
            QString keyName = KbBind::globalRemap(key.name);
            QString name = key.friendlyName(false);
            name = name.split(" ").last();
            struct {
                const char* keyName, *displayName;
            } names[] = {
                {"light", "☼"}, {"lock", "☒"}, {"mute", "◖⊘"}, {"volup", keyMap.model() == KeyMap::K65 ? "◖))" : "▲"}, {"voldn", keyMap.model() == KeyMap::K65 ? "◖)" : "▼"},
                {"prtscn",  "PrtScn\nSysRq"}, {"scroll", "Scroll\nLock"}, {"pause", "Pause\nBreak"}, {"stop", "▪"}, {"prev", "|◂◂"}, {"play", "▸||"}, {"next", "▸▸|"},
                {"pgup", "Page\nUp"}, {"pgdn", "Page\nDown"}, {"numlock", "Num\nLock"},
                {"caps", "Caps"}, {"lshift", "Shift"}, {"rshift", "Shift"},
#ifdef Q_OS_MACX
                {"lctrl", "⌃"}, {"rctrl", "⌃"}, {"lwin", "⌘"}, {"rwin", "⌘"}, {"lalt", "⌥"}, {"ralt", "⌥"},
#else
                {"lctrl", "Ctrl"}, {"rctrl", "Ctrl"}, {"lwin", "❖"}, {"rwin", "❖"}, {"lalt", "Alt"}, {"ralt", "Alt"},
#endif
                {"rmenu", "▤"}, {"up", "▲"}, {"left", "◀"}, {"down", "▼"}, {"right", "▶"}, {"fn","Fn"},
                {"mouse1", ""}, {"mouse2", ""}, {"mouse3", "∙"}, {"dpiup", "▲"}, {"dpidn", "▼"}, {"wheelup", "▲"}, {"wheeldn", "▼"}, {"dpi", "◉"}, {"mouse5", "▲"}, {"mouse4", "▼"}, {"sniper", "⊕"}
            };
            for(uint k = 0; k < sizeof(names) / sizeof(names[0]); k++){
                if(keyName == names[k].keyName){
                    name = names[k].displayName;
                    break;
                }
            }
            if(keyName == "thumb1" && model == KeyMap::SABRE)
                name = "∙";
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
                // Use a larger font size for Super (Linux only) and Brightness to compensate for the unicode symbols looking smaller
                font.setPixelSize(font.pixelSize() * 1.3);
            if((layout == KeyMap::EU || layout == KeyMap::EU_DVORAK) && (keyName == "hash" || keyName == "bslash_iso"))
                // Don't differentiate backslashes on the EU layout
                name = "\\";
            // Determine the appropriate size to draw the text at
            decPainter.setFont(font);
            QRectF rect(x * scale, y * scale - 1, w * scale, h * scale);
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
            else if(KeyAction(bind).isProgram())
                // Custom program - orange
                decPainter.setPen(QColor(255, 224, 192));
            else if(KeyAction(bind).isSpecial() && (bind == def || !KeyAction(def).isSpecial()))
                // Special function - blue (only if not mapped to a different function - if a special function is remapped, color it yellow)
                decPainter.setPen(QColor(128, 224, 255));
            else if(KeyAction(bind).isMedia() && (bind == def || !KeyAction(def).isMedia()))
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
    float scale, offX, offY;
    drawInfo(scale, offX, offY);
    float mx = mouseCurrentX / scale - offX, my = mouseCurrentY / scale - offY;
    QHashIterator<QString, Key> k(keyMap);
    uint i = -1;
    while(k.hasNext()){
        k.next();
        i++;
        const Key& key = k.value();
        if((_rgbMode && !key.hasLed)
                || (!_rgbMode && !key.hasScan))
            continue;
        if(fabs(key.x - mx) <= key.width / 2.f - 1.f && fabs(key.y - my) <= key.height / 2.f - 1.f){
            // Sidelights can't have a color, but they can be toggled
            if(!strcmp(key.name, "lsidel") || !strcmp(key.name, "rsidel")){
                emit sidelightToggled(); // get the kblightwidget to record it
                update();
                break;
            }
            newSelection.setBit(i);
            update();
            break;
        }
    }
}

void KeyWidget::mouseMoveEvent(QMouseEvent* event){
    event->accept();
    QString tooltip;

    // Find selection rectangle
    mouseCurrentX = event->x();
    mouseCurrentY = event->y();
    float scale, offX, offY;
    drawInfo(scale, offX, offY);
    float mx = mouseCurrentX / scale - offX, my = mouseCurrentY / scale - offY;
    float mx1, mx2, my1, my2;
    if(mouseCurrentX >= mouseDownX){
        mx1 = mouseDownX / scale - offX;
        mx2 = mouseCurrentX / scale - offX;
    } else {
        mx1 = mouseCurrentX / scale - offX;
        mx2 = mouseDownX / scale - offX;
    }
    if(mouseCurrentY >= mouseDownY){
        my1 = mouseDownY / scale - offY;
        my2 = mouseCurrentY / scale - offY;
    } else {
        my1 = mouseCurrentY / scale - offY;
        my2 = mouseDownY / scale - offY;
    }
    // Clear new selection
    if(mouseDownMode != NONE)
        newSelection.fill(false);
    // See if the event hit any keys
    QHashIterator<QString, Key> k(keyMap);
    uint i = -1;
    while(k.hasNext()){
        k.next();
        i++;
        const Key& key = k.value();
        if((_rgbMode && !key.hasLed)
                || (!_rgbMode && !key.hasScan))
            continue;
        // Update tooltip with the mouse hover (if any), even if it's not selectable
        if(fabs(key.x - mx) <= key.width / 2.f - 1.f && fabs(key.y - my) <= key.height / 2.f - 1.f
                && tooltip.isEmpty())
            tooltip = key.friendlyName(false);
        // on STRAFE Sidelights and indicators can't be assigned color the way other keys are colored
        if(keyMap.model() == KeyMap::STRAFE && (!strcmp(key.name, "lsidel") || !strcmp(key.name, "rsidel") || _indicators.contains(key.name))) // FIX: _indicators check fails whenever _indicators is empty because "show animated" is unchecked
            continue;
        float kx1 = key.x - key.width / 2.f + 1.f;
        float ky1 = key.y - key.height / 2.f + 1.f;
        float kx2 = kx1 + key.width - 2.f;
        float ky2 = ky1 + key.height - 2.f;
        // If they overlap, add the key to the selection
        if(!(mx1 >= kx2 || kx1 >= mx2)
                && !(my1 >= ky2 || ky1 >= my2)
                && mouseDownMode != NONE)
            newSelection.setBit(i);
    }

    if(mouseDownMode != NONE)
        update();
    setToolTip(tooltip);
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
    // Emit signal with the names of the keys
    QStringList selectedNames;
    uint i = 0;
    foreach(const QString& key, keyMap.keys()){
        if(selection.testBit(i++))
            selectedNames << key;
    }
    emit selectionChanged(selectedNames);
    update();
}

void KeyWidget::setSelection(const QStringList& keys){
    selection.fill(false);
    QStringList allNames = keyMap.keys();
    foreach(const QString& key, keys){
        int index = allNames.indexOf(key);
        if(index >= 0)
            selection.setBit(index);
    }
    newSelection.fill(false);
    mouseDownMode = NONE;
    update();
    emit selectionChanged(keys);
}

void KeyWidget::selectAll(){
    selection.fill(false);
    // Fill the selection with all keys that have an LED/scancode (depending on widget mode)
    int i = 0;
    QStringList selectedNames;
    foreach(const Key& key, keyMap.positions()){
        // Sidelights can't be selected
        if(strcmp(key.name, "lsidel") && strcmp(key.name, "rsidel")
           && ((_rgbMode && key.hasLed) || !(_rgbMode && key.hasScan))){
            selection.setBit(i);
            selectedNames << key.name;
        }
        i++;
    }
    // Clear mousedown state
    newSelection.fill(false);
    mouseDownMode = NONE;
    update();
    emit selectionChanged(selectedNames);
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
    QStringList allNames = keyMap.keys();
    foreach(const QString& key, keys){
        // Sidelights can't be selected
        if(!strcmp(key.toLatin1(), "lsidel") || !strcmp(key.toLatin1(), "rsidel"))
            continue;
        int index = allNames.indexOf(key);
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

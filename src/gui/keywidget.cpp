#include <cmath>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QPainter>
#include "keywidget.h"
#include "keyaction.h"
#include "kbbind.h"
#include <QToolTip>
#include <limits>
#include <QLabel>
#include "ckbmainbackgroundcolour.h"
#include <ckbnextconfig.h>

static const int KEY_SIZE = 12;

static const QColor bgColor(68, 64, 64);
static const QColor bgColorA(68, 64, 64, 35);
static const QColor keyColor(112, 110, 110);
static const QColor sniperColor(130, 90, 90);
static const QColor thumbColor(34, 32, 32);
static const QColor transparentColor(0, 0, 0, 0);
static const QColor highlightAnimColor(136, 200, 240);
static const QColor animColor(112, 200, 110);
static const QColor highlightColor(136, 176, 240);
static const QBrush bgHighlightBrush(QColor(136, 176, 240, 128));
static const QColor red(255, 136, 136);
static const QColor orange(255, 224, 192);
static const QColor blue(128, 224, 255);
static const QColor green(160, 255, 168);
static const QColor white(255, 255, 255);
static const QColor yellow(255, 248, 128);

#ifndef NDEBUG
static const QBrush hitboxBrush(QColor(255, 136, 136, 128));
#endif

// KbLight.cpp
extern QRgb monoRgb(float r, float g, float b);

static const QMap<QString, QString> keyNames {
    {"light", "☼"}, {"lock", "☒"}, {"mute", "🔇"}, {"volup", "▲"}, {"voldn", "▼"},
    {"prtscn",  "PrtScn\nSysRq"}, {"scroll", "Scroll\nLock"}, {"pause", "Pause\nBreak"}, {"stop", "■"}, {"prev", "⏮"}, {"play", "⏯"}, {"next", "⏭"},
    {"pgup", "Page\nUp"}, {"pgdn", "Page\nDown"}, {"numlock", "Num\nLock"},
    {"caps", "Caps"}, {"lshift", "Shift"}, {"rshift", "Shift"},
#ifdef Q_OS_MACOS
    {"lctrl", "⌃"}, {"rctrl", "⌃"}, {"lwin", "⌘"}, {"rwin", "⌘"}, {"lalt", "⌥"}, {"ralt", "⌥"},
#else
    {"lctrl", "Ctrl"}, {"rctrl", "Ctrl"}, {"lwin", "❖"}, {"rwin", "❖"}, {"lalt", "Alt"}, {"ralt", "Alt"},
#endif
    {"rmenu", "▤"}, {"up", "▲"}, {"left", "◀"}, {"down", "▼"}, {"right", "▶"}, {"fn","Fn"},
    {"mouse1", ""}, {"mouse2", ""}, {"mouse3", "∙"}, {"dpiup", "▲"}, {"dpidn", "▼"}, {"wheelup", "▲"}, {"wheeldn", "▼"}, {"dpi", "◉"}, {"mouse5", "▲"}, {"mouse4", "▼"}, {"sniper", "⊕"},
    {"lghtpgm", "☼P"}
};

KeyWidget::KeyWidget(QWidget* parent) :
    QOpenGLWidget(parent), mouseDownMode(NONE), _rgbMode(true), _monochrome(false), _aspectRatio(0.5), drawInfoScale(0.f), _debug(false)
{
    setMouseTracking(true);
#ifdef FPS_COUNTER
    glFpsTimer.start();
    kbLoopElapsed = 0.0;
#endif
}

static inline int calculateControlWheelOffset(int d){
    //return abs((d % 7) - 3);
    switch(d)
    {
    case 1:
    case 8:
    case 9:
        return 0;
    case 2:
    case 7:
    case 10:
        return 1;
    case 3:
    case 6:
        return 2;
    case 4:
    case 5:
        return 3;
    default:
        return -1;
    }
}

static inline QRectF getKeyRect(const Key& key){
    QRectF keyRect(QPointF(key.x, key.y) - QPointF(key.width, key.height) / 2.f + QPointF(1.f, 1.f), QSize(key.width, key.height) - QSize(2, 2));
    // Special case for the pie-shaped ctrl wheel segments
    // We pretend they are small rectangles instead
    if(!strncmp(key.name, "ctrlwheel", 9)){
        int d;
        if(sscanf(key.name + 9, "%d", &d) == 1) {
            const int offX = calculateControlWheelOffset(d + 2);
            const int offY = calculateControlWheelOffset(d);
            float newX = keyRect.x() + offX * 5.5f - 6.5f;
            float newY = keyRect.y() + offY * 5.5f - 6.5f;

            // These are intended to "stack"
            if(offX > 0)
                newX -= 2.7f;
            if(offX == 3)
                newX -= 4.f;

            if(offY > 0)
                newY -= 2.7f;
            if(offY == 3)
                newY -= 4.f;

            keyRect.moveTo(newX, newY);
        }
    }
    return keyRect;
}

void KeyWidget::map(const KeyMap& newMap){
    keyMap = newMap;
    selection = QBitArray(keyMap.count());
    newSelection = QBitArray(keyMap.count());
    animation = QBitArray(keyMap.count());
    _aspectRatio = keyMap.width() / (float)keyMap.height();
    if(keyMap.isKeyboard())
        _aspectRatio -= 0.35;
    else if(keyMap.isMousepad())
        _aspectRatio += 0.35;
    update();
    updateGeometry();

    const KeyMap::Model model = keyMap.model();
    if(!keyMap.isKeyboard()){
        // Prepare image overlays
        _overlayPos.setY(-2.f);
        switch(model){
        case KeyMap::M65:
        case KeyMap::M65E:
            _currentOverlay.load(":/img/overlay_m65.png");
            _overlayPos.setX(2.f);
            break;
        case KeyMap::SABRE:
            _currentOverlay.load(":/img/overlay_sabre.png");
            _overlayPos.setX(1.f);
            break;
        case KeyMap::SCIMITAR:
            _currentOverlay.load(":/img/overlay_scimitar.png");
            _overlayPos.setX(10.3f);
            break;
        case KeyMap::HARPOON:
            _currentOverlay.load(":/img/overlay_harpoon.png");
            _overlayPos.setX(4.3f);
            break;
        case KeyMap::GLAIVE:
            _currentOverlay.load(":/img/overlay_glaive.png");
            _overlayPos.setX(3.5f);
            break;
        case KeyMap::GLAIVEPRO:
            _currentOverlay.load(":/img/overlay_glaivepro.png");
            _overlayPos.setX(3.5f);
            break;
        case KeyMap::KATAR:
        case KeyMap::KATARPROXT:
            _currentOverlay.load(":/img/overlay_katar.png");
            _overlayPos.setX(3.7f);
            _overlayPos.setY(-2.f);
            break;
        case KeyMap::DARKCORE:
            _currentOverlay.load(":/img/overlay_darkcore.png");
            _overlayPos.setX(-5.f);
            _overlayPos.setY(-2.f);
            break;
        case KeyMap::POLARIS:
            _currentOverlay.load(":/img/overlay_polaris.png");
            _overlayPos.setX(-19.5f);
            break;
        case KeyMap::M95:
            _currentOverlay.load(":/img/overlay_m95.png");
            _overlayPos.setX(35.3f);
            break;
        case KeyMap::IRONCLAW:
            _currentOverlay.load(":/img/overlay_ironclaw.png");
            _overlayPos.setX(2.f);
            break;
        case KeyMap::NIGHTSWORD:
            _currentOverlay.load(":/img/overlay_nightsword.png");
            _overlayPos.setX(2.f);
            break;
        case KeyMap::IRONCLAW_W:
            _currentOverlay.load(":/img/overlay_ironclaw.png");
            _overlayPos.setX(2.f);
            break;
        case KeyMap::M55:
            _currentOverlay.load(":/img/overlay_m55.png");
            _overlayPos.setX(5.7f);
            break;
        case KeyMap::DARKCORERGBPRO:
            _currentOverlay.load(":/img/overlay_darkcore_rgb_pro.png");
            _overlayPos.setX(-5.f);
            _overlayPos.setY(-2.f);
            break;
        case KeyMap::ST100: // Entries without overlays should not be added here. This will (hopefully) be moved.
            _overlayPos.setX(-18.5);
            // fallthrough
        default:
            {
                QImage blank(810, 700, QImage::Format_ARGB32);
                blank.fill(bgColorA);
                _currentOverlay = blank;
                break;
            }
        }
    } else {
        _currentOverlay = QImage();
    }
    calculateDrawInfo(size());
    setDebug(_debug);
}

void KeyWidget::calculateDrawInfo(const QSize& size){
    const float w = size.width();
    const float h = size.height();
    float xScale = w / (keyMap.width() + KEY_SIZE);
    float yScale = h / (keyMap.height() + KEY_SIZE);
    drawInfoScale = std::fmin(xScale, yScale);
    // FIXME: cleanup
    drawInfoOffset.setX((w / drawInfoScale - keyMap.width()) / 2.f);
    drawInfoOffset.setY((h / drawInfoScale - keyMap.height()) / 2.f);
    if(_currentOverlay.isNull()) {
        _currentOverlayScaled = QImage();
    } else {
        QSize sz = _currentOverlay.size() * drawInfoScale / 9.f;
        // We need to transform the image with QImage::scaled() because painter.drawImage() will butcher it, even with smoothing enabled
        _currentOverlayScaled = _currentOverlay.scaled(sz, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
}

bool KeyWidget::event(QEvent* e){
    if(e->type() != QEvent::ToolTip)
        return QOpenGLWidget::event(e);

    QHelpEvent* he = static_cast<QHelpEvent*>(e);
    QPointF mouseCurrentScaled = he->pos() / drawInfoScale - drawInfoOffset;

    for(const Key& key: keyMap){

        // Don't show tooltips for keys in the wrong mode
        if((_rgbMode && !key.hasLed)
                || (!_rgbMode && !key.hasScan))
            continue;

        // Get the name of the key under the cursor
        QRectF keyRect(QPointF(key.x, key.y) - QPointF(key.width, key.height) / 2.f + QPointF(1.f, 1.f), QSize(key.width, key.height) - QSize(2, 2));
        if(keyRect.contains(mouseCurrentScaled)){
            QToolTip::showText(he->globalPos(), key.friendlyName(false));
            return true;
        }
    }
    // No key was found under the cursor
    QToolTip::hideText();
    e->ignore();
    return true;
}

void KeyWidget::setDebug(bool debug){
#ifndef NDEBUG
    _debug = debug;
    hitboxes.clear();
    if(debug)
        for(const Key& key : keyMap)
            hitboxes[key.name] = getKeyRect(key);
    update();
#endif
}

void KeyWidget::colorMap(const QColorMap& newColorMap){
    _colorMap = newColorMap;
    update();
}

void KeyWidget::displayColorMap(const ColorMap& newDisplayMap, const QSet<QString>& indicators, quint64 renderInterval){
    if(!isVisible())
        return;
    _displayColorMap = newDisplayMap;
    _indicators = indicators;
#ifdef FPS_COUNTER
    if(renderInterval != std::numeric_limits<quint64>::max())
        kbLoopElapsed = renderInterval;
#endif
    update();
}

void KeyWidget::bindMap(const BindMap& newBindMap){
    _bindMap = newBindMap;
    update();
}

void KeyWidget::paintGL(){
    // Don't bother painting anything if the scale is 0
    if(drawInfoScale <= 0.f)
        return;
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
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    const KeyMap::Model model = keyMap.model();
    const KeyMap::Layout layout = keyMap.layout();

    painter.setPen(Qt::NoPen);

    // Clear everything
    painter.setBrush(QBrush(CkbMainBackgroundColour::getColour()));
    painter.drawRect(rect());

    // Draw background
    if(!_currentOverlayScaled.isNull()){
        // FIXME: Properly centre this instead of relying on a pre-set offset
        // The overlay has a resolution of 9px per keymap unit
        QPointF pos = (_overlayPos + drawInfoOffset) * drawInfoScale;
        painter.drawImage(QRectF(pos, _currentOverlayScaled.size()), _currentOverlayScaled);
    } else {
        // Otherwise, draw a solid background
        painter.setBrush(QBrush(bgColor));
        painter.drawRect(rect());
    }

    // Draw key backgrounds
    painter.setPen(Qt::NoPen);
    int i = -1;
    int d;
    for(const Key& key : keyMap){
        i++;
        float x = key.x + drawInfoOffset.x() - key.width / 2.f + 1.f;
        float y = key.y + drawInfoOffset.y() - key.height / 2.f + 1.f;
        float w = key.width - 2.f;
        float h = key.height - 2.f;
        // In RGB mode, ignore keys without LEDs
        if((_rgbMode && !key.hasLed)
                || (!_rgbMode && !key.hasScan))
            continue;
        // Set color based on key highlight
        painter.setOpacity(1.);
        if(highlight.testBit(i)){
            if(animation.testBit(i))
                painter.setBrush(QBrush(highlightAnimColor));
            else
                painter.setBrush(QBrush(highlightColor));
        } else if(animation.testBit(i)){
            painter.setBrush(QBrush(animColor));
        } else {
            if(!strcmp(key.name, "sniper"))
                // Sniper key uses a reddish base color instead of the usual grey
                painter.setBrush(QBrush(sniperColor));
            else if(model == KeyMap::SCIMITAR && !strncmp(key.name, "thumb", 5) && strcmp(key.name, "thumb"))
                // Thumbgrid keys use a black color
                painter.setBrush(QBrush(thumbColor));
            else if(!strcmp(key.name, "lsidel") || !strcmp(key.name, "rsidel") || !strcmp(key.name, "logo"))
                // Strafe side lights have different background
                painter.setBrush(QBrush(transparentColor));
            else {
                painter.setBrush(QBrush(keyColor));
                if(KeyMap::isMouse(model))
                    painter.setOpacity(0.7);
            }
        }
        if(((model != KeyMap::STRAFE && model != KeyMap::K95P && model != KeyMap::K100 && model != KeyMap::K70MK2 && model != KeyMap::STRAFE_MK2 && model != KeyMap::K70_TKL && model != KeyMap::K70_PRO) && (!strcmp(key.name, "mr") || !strcmp(key.name, "m1") || !strcmp(key.name, "m2") || !strcmp(key.name, "m3")
                || !strcmp(key.name, "light") || !strcmp(key.name, "lock") || !strcmp(key.name, "lghtpgm") || (model == KeyMap::K65 && !strcmp(key.name, "mute")))) ||
                !strcmp(key.name, "ctrlwheelb")){
            // Not all devices have circular buttons
            x += w / 8.f;
            y += h / 8.f;
            w *= 0.75f;
            h *= 0.75f;
            painter.drawEllipse(QRectF(x * drawInfoScale, y * drawInfoScale, w * drawInfoScale, h * drawInfoScale));
        } else if (model == KeyMap::POLARIS) {
            // Draw the edges as polygons
            if(!strcmp(key.name, "zone11")){
               drawBottomLeftCorner(&painter, x, y, w, h+2.f, drawInfoScale);
            } else if(!strcmp(key.name, "zone5")){
                drawBottomRightCorner(&painter, x, y, w, h+2.f, drawInfoScale);
            } else
                painter.drawRect(QRectF(x * drawInfoScale, y * drawInfoScale, w * drawInfoScale, h * drawInfoScale));
        } else if(model == KeyMap::ST100){
            // Draw the edges as polygons
            if(!strcmp(key.name, "zone7")){
                drawBottomLeftCorner(&painter, x, y, w, h, drawInfoScale);
             } else if(!strcmp(key.name, "zone4")){
                drawBottomRightCorner(&painter, x, y, w, h, drawInfoScale);
            } else if (!strcmp(key.name, "zone2")){
                drawTopRightCorner(&painter, x, y, w, h, drawInfoScale);
            } else if (!strcmp(key.name, "zone9")){
                drawTopLeftCorner(&painter, x, y, w, h, drawInfoScale);
            } else
                painter.drawRect(QRectF(x * drawInfoScale, y * drawInfoScale, w * drawInfoScale, h * drawInfoScale));
        } else if ((model == KeyMap::K70MK2 || model == KeyMap::STRAFE_MK2 || model == KeyMap::K70_TKL || model == KeyMap::K70_PRO) && key.friendlyName().startsWith("Logo")) {
            w += 10.f;
            x -= 5.f;
            painter.drawRect(QRectF(x * drawInfoScale, y * drawInfoScale, w * drawInfoScale, h * drawInfoScale));
        } else if (model == KeyMap::M95) {
            painter.drawRect(QRectF(x * drawInfoScale, y * drawInfoScale, w * drawInfoScale, h * drawInfoScale));
        } else if (sscanf(key.name, "ctrlwheel%d", &d) == 1) {
            // Undo the offset in the keymap definition
            // Hardcode (20, 20) here so that key.width and key.height can be used for the hover rectangles
            x = key.x - calculateControlWheelOffset(d + 2) + drawInfoOffset.x() - 20 / 2.f + 1.f;
            y = key.y - calculateControlWheelOffset(d) + drawInfoOffset.y() - 20 / 2.f + 1.f;
            w = h = 20 - 2.f;
            float bx = x + 5.f;
            float by = y + 5.f;
            float bw = w - 10.f;
            float bh = h - 10.f;

            int angle = 90 - 45 * (d - 1);

            // Create a pie
            QPainterPath p;
            QRectF rect = QRectF(x * drawInfoScale, y * drawInfoScale, w * drawInfoScale, h * drawInfoScale);
            p.moveTo(rect.center());
            p.arcTo(rect, angle, -45);

            // Create a smaller inner circle
            QPainterPath p2;
            QRectF rect2 = QRectF(bx * drawInfoScale, by * drawInfoScale, bw * drawInfoScale, bh * drawInfoScale);
            p2.moveTo(rect2.center());
            p2.arcTo(rect2, angle, 360);

            // Draw their difference
            painter.drawPath(p-p2);
            //QPainterPath diff = p - p2;

            //bgPainter.drawRect(diff.boundingRect() - QMarginsF(1*scale, 1*scale, 1*scale, 1*scale));
        } else {
            if(!strcmp(key.name, "enter")){
                if(key.height == 24){
                    // ISO enter key isn't rectangular
                    h = 10.f;
                    painter.drawRect(QRectF((x + w - 13.f) * drawInfoScale, y * drawInfoScale, 13.f * drawInfoScale, 22.f * drawInfoScale));
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
            painter.drawRect(QRectF(x * drawInfoScale, y * drawInfoScale, w * drawInfoScale, h * drawInfoScale));
        }
    }

    painter.setOpacity(1.0);

    // Render the key decorations (RGB -> light circles, binding -> key names)
    if(_rgbMode){
        // Draw key colors (RGB mode)
        for(const Key& key : keyMap){
            if(!key.hasLed)
                continue;
            float x = key.x + drawInfoOffset.x() - 1.8f;
            float y = key.y + drawInfoOffset.y() - 1.8f;
            float w = 3.6f;
            float h = 3.6f;
            /*if(model == KeyMap::K55){
                x = key.x;
                y = key.y;
                w = key.width;
                h = key.height;
            }*/
            // Display a white circle around regular keys, red circle around indicators
            if(_indicators.contains(key.name))
                painter.setPen(QPen(QColor(255, 248, 136), 1.5));
            else
                painter.setPen(QPen(QColor(255, 255, 255), 1.5));
            QRgb color;
            const QRgb* inDisplay = _displayColorMap.colorForName(key.name);
            if(inDisplay)
                // Color in display map? Grab it from there
                // (monochrome conversion not necessary as this would have been done by the animation)
                color = *inDisplay;
            else {
                // Otherwise, read from base map
                color = _colorMap.value(key.name);
                if(_monochrome)
                    color = monoRgb(qRed(color), qGreen(color), qBlue(color));
            }
            painter.setBrush(QBrush(color));
            // Strafe side lights (toggle lights with no animation)
            if(!strcmp(key.name, "lsidel") || !strcmp(key.name, "rsidel")) {
                drawStrafeSidelights(&key, &painter, keyColor, color, bgColor);
            } else if(!strcmp(key.name, "logo") || key.friendlyName() == QLatin1String("Logo 1") || !strcmp(key.name, "back")) { // Logos
                drawLogo(&key, &painter);
            } else if (model == KeyMap::POLARIS) {
                float kx = key.x + drawInfoOffset.x() - key.width / 2.f + 2.f;
                float ky = key.y + drawInfoOffset.y() - key.height / 2.f + 2.f;
                float kw = key.width - 4.f;
                float kh = key.height - 4.f;
                // No border
                painter.setPen(QPen(QColor(0,0,0,0), 1));
                // Draw the edges as polygons
                if(!strcmp(key.name, "zone11")){
                    drawBottomLeftCorner(&painter, kx, ky, kw, kh+2.f, drawInfoScale);
                 } else if(!strcmp(key.name, "zone5")){
                    drawBottomRightCorner(&painter, kx, ky, kw, kh+2.f, drawInfoScale);
                 } else
                    painter.drawRect(QRectF(kx * drawInfoScale, ky * drawInfoScale, kw * drawInfoScale, kh * drawInfoScale));
            } else if (model == KeyMap::ST100) {
                float kx = key.x + drawInfoOffset.x() - key.width / 2.f + 2.f;
                float ky = key.y + drawInfoOffset.y() - key.height / 2.f + 2.f;
                float kw = key.width - 4.f;
                float kh = key.height - 4.f;
                // No border
                painter.setPen(QPen(QColor(0,0,0,0), 1));
                // Draw the edges as polygons
                if(!strcmp(key.name, "zone2")){
                    drawTopRightCorner(&painter, kx, ky, kw, kh, drawInfoScale);
                } else if(!strcmp(key.name, "zone4")){
                    drawBottomRightCorner(&painter, kx, ky, kw, kh, drawInfoScale);
                } else if(!strcmp(key.name, "zone5")){
                    drawLogo(&key, &painter);
                } else if(!strcmp(key.name, "zone7")){
                    drawBottomLeftCorner(&painter, kx, ky, kw, kh, drawInfoScale);
                } else if(!strcmp(key.name, "zone9")){
                    drawTopLeftCorner(&painter, kx, ky, kw, kh, drawInfoScale);
                } else
                    painter.drawRect(QRectF(kx * drawInfoScale, ky * drawInfoScale, kw * drawInfoScale, kh * drawInfoScale));
            } else if (model == KeyMap::K55 || model == KeyMap::K55PRO)
                painter.drawRect(QRectF(x * drawInfoScale, y * drawInfoScale, w * drawInfoScale, h * drawInfoScale));
            else if ((model == KeyMap::K70MK2 || model == KeyMap::STRAFE_MK2) && key.friendlyName() == "Logo 2")
                painter.drawRect(QRectF((key.x + drawInfoOffset.x() - key.width / 2.f - 2.f) * drawInfoScale, y * drawInfoScale, (key.width + 4.f) * drawInfoScale, h * drawInfoScale));
            else if (sscanf(key.name, "ctrlwheel%d", &d) == 1){
                // Hardcode (20, 20) here so that key.width and key.height can be used for the hover rectangles
                x = key.x - calculateControlWheelOffset(d + 2) + drawInfoOffset.x() - 20 / 2.f + 2.5f;
                y = key.y - calculateControlWheelOffset(d) + drawInfoOffset.y() - 20 / 2.f + 2.5f;
                w = 20 - 5.f;
                h = 20 - 5.f;

                float bx = x + 2.f;
                float by = y + 2.f;
                float bw = w - 4.f;
                float bh = h - 4.f;

                int angle = 90 - 45 * (d - 1);

                // Create a pie
                QPainterPath p;
                QRectF rect = QRectF(x * drawInfoScale, y * drawInfoScale, w * drawInfoScale, h * drawInfoScale);
                p.moveTo(rect.center());
                p.arcTo(rect, angle, -45);

                // Create a smaller inner circle
                QPainterPath p2;
                QRectF rect2 = QRectF(bx * drawInfoScale, by * drawInfoScale, bw * drawInfoScale, bh * drawInfoScale);
                p2.moveTo(rect2.center());
                p2.arcTo(rect2, angle, 360);

                // Draw their difference
                painter.drawPath(p-p2);

            } else
                painter.drawEllipse(QRectF(x * drawInfoScale, y * drawInfoScale, w * drawInfoScale, h * drawInfoScale));
        }
    } else {
        // Draw key names
        painter.setBrush(Qt::NoBrush);
        QFont font = painter.font();
        font.setBold(true);
        font.setPixelSize(5.25f * drawInfoScale);
        QFont font0 = font;
        for(const Key& key : keyMap){
            if(!key.hasScan)
                continue;

            float x = key.x + drawInfoOffset.x() - key.width / 2.f + 1.f;
            float y = key.y + drawInfoOffset.y() - key.height / 2.f;
            float w = key.width - 2.f;
            float h = key.height;

            // Print the key's friendly name (with some exceptions)
            QString keyName = KbBind::globalRemap(key.name);
            QString name = key.friendlyName(false).split(" ").last();
            name = keyNames.value(keyName, name);

            if(model == KeyMap::SABRE && keyName == "thumb1")
                name = "∙";
            else if(model == KeyMap::K65 && keyName == "volup")
                name = "🔊";
            else if(model == KeyMap::K65 && keyName == "voldn")
                name = "🔉";

            if(keyName == "mr" || keyName == "m1" || keyName == "m2" || keyName == "m3" || keyName == "up" || keyName == "down" || keyName == "left" || keyName == "right" || keyName == "lghtpgm")
                // Use a smaller size for MR, M1 - M3, and arrow keys
                font.setPixelSize(font.pixelSize() * 0.75);
            else if(keyName == "end")
                // Use a smaller size for "End" to match everything else in that area
                font.setPixelSize(font.pixelSize() * 0.65);
            else if(keyName == "light"
#ifndef Q_OS_MACOS
                    || keyName == "lwin" || keyName == "rwin"
#endif
                    )
                // Use a larger font size for Super (Linux only) and Brightness to compensate for the unicode symbols looking smaller
                font.setPixelSize(font.pixelSize() * 1.3);
            if((layout == KeyMap::EU || layout == KeyMap::EU_DVORAK) && (keyName == "hash" || keyName == "bslash_iso"))
                // Don't differentiate backslashes on the EU layout
                name = "\\";
            // Determine the appropriate size to draw the text at
            painter.setFont(font);
            QRectF rect(x * drawInfoScale, y * drawInfoScale - 1.f, w * drawInfoScale, h * drawInfoScale);
            int flags = Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextWordWrap;
            QRectF bounds = painter.boundingRect(rect, flags, name);

            while((bounds.height() >= rect.height() - 8. || bounds.width() >= rect.width() - 2.) && font.pixelSize() >= 5){
                // Scale font size down until it fits inside the key
                font.setPixelSize(font.pixelSize() - 2);
                painter.setFont(font);
                bounds = painter.boundingRect(rect, flags, name);
            }

            // Pick color based on key function
            QString bind = _bindMap.value(key.name);
            QString def = KbBind::defaultAction(key.name, model);
            if(bind.isEmpty())
                // Unbound - red
                painter.setPen(red);
            else if(KeyAction(bind).isProgram())
                // Custom program - orange
                painter.setPen(orange);
            else if(KeyAction(bind).isSpecial() && (bind == def || !KeyAction(def).isSpecial()))
                // Special function - blue (only if not mapped to a different function - if a special function is remapped, color it yellow)
                painter.setPen(blue);
            else if(KeyAction(bind).isMedia() && (bind == def || !KeyAction(def).isMedia()))
                // Media key - green
                painter.setPen(green);
            else if(bind == def)
                // Standard key - white
                painter.setPen(white);
            else
                // Remapped key - yellow
                painter.setPen(yellow);
            painter.drawText(rect, flags, name);
            font = font0;
        }

    }
    // Draw mouse highlight (if any)
    if(mouseDownMode != NONE && mouseHighlightRect.isValid()){
        painter.setPen(QPen(highlightColor, 0.5));
        painter.setBrush(bgHighlightBrush);
        painter.drawRect(mouseHighlightRect);
    }

#ifndef NDEBUG
    if(_debug){
        painter.setBrush(hitboxBrush);
        painter.setPen(red);
        for(const QRectF& hitbox : hitboxes)
            painter.drawRect(QRectF((hitbox.x() + drawInfoOffset.x()) * drawInfoScale, (hitbox.y() + drawInfoOffset.y()) * drawInfoScale, hitbox.width()*drawInfoScale, hitbox.height()*drawInfoScale));
    }
#endif

#ifdef FPS_COUNTER
    painter.setPen(QPen(green, 1.0));
    QFont fpsfont = painter.font();
    fpsfont.setBold(true);
    fpsfont.setPointSize(14);
    painter.setFont(fpsfont);
    painter.drawText(5, 18, QString::number(1.0/((double)glFpsTimer.restart()/1000.0), 'f', 2));
    painter.setPen(QPen(blue, 1.0));
    if(kbLoopElapsed > 0.0)
        painter.drawText(5, 36, QString::number(1.0/(kbLoopElapsed/1000.0), 'f', 2));
#endif
}

void KeyWidget::paintEvent(QPaintEvent* e){
    QOpenGLWidget::paintEvent(e);
}

void KeyWidget::mousePressEvent(QMouseEvent* event){
    event->accept();
    mouseDownMode = (event->modifiers() & Qt::AltModifier) ? SUBTRACT : (event->modifiers() & Qt::ShiftModifier) ? ADD : (event->modifiers() & Qt::ControlModifier) ? TOGGLE : SET;
    // See if the event hit a key
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    mouseDown = event->localPos();
#else
    mouseDown = event->position();
#endif
    QPointF mouseDownScaled = mouseDown / drawInfoScale - drawInfoOffset;
    int i = -1;
    for(const Key& key : keyMap){
        i++;
        if((_rgbMode && !key.hasLed)
                || (!_rgbMode && !key.hasScan))
            continue;
        QRectF keyRect = getKeyRect(key);
        if(keyRect.contains(mouseDownScaled)){
            // Sidelights can't have a color, but they can be toggled
            if(!strcmp(key.name, "lsidel") || !strcmp(key.name, "rsidel")){
                emit sidelightToggled(); // get the kblightwidget to record it
                update();
                break;
            }
            // TODO: Merge with the above
            if(keyMap.model() == KeyMap::M95 && !strcmp(key.name, "back")){
                emit M95LightToggled();
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

    if(mouseDownMode == NONE)
        return;

    // Find selection rectangle
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    const QPointF& mouseCurrent = event->localPos();
#else
    const QPointF& mouseCurrent = event->position();
#endif
    QPointF mouseCurrentScaled = mouseCurrent / drawInfoScale - drawInfoOffset;

    QRectF mouseHighlightRectScaled;

    // Clear new selection
    mouseHighlightRect = QRectF(mouseCurrent, mouseDown).normalized();
    mouseHighlightRectScaled = QRectF(mouseCurrentScaled, mouseDown / drawInfoScale - drawInfoOffset).normalized();

    // If the rect is not valid (mouseCurrent == mouseDown, or just a line),
    // the selection will temporarily go away due to intersect not being defined
    if(!mouseHighlightRect.isValid() || !mouseHighlightRectScaled.isValid())
        return;

    newSelection.fill(false);

    // See if the event hit any keys
    int i = -1;
    for(const Key& key: keyMap){
        i++;
        if((_rgbMode && !key.hasLed)
                || (!_rgbMode && !key.hasScan))
            continue;
        QRectF keyRect = getKeyRect(key);
        // on STRAFE Sidelights and indicators can't be assigned color the way other keys are colored
        if(((keyMap.model() == KeyMap::STRAFE || keyMap.model() == KeyMap::STRAFE_MK2) && (!strcmp(key.name, "lsidel") || !strcmp(key.name, "rsidel")))
                || (keyMap.model() == KeyMap::M95 && !strcmp(key.name, "back"))
                || _indicators.contains(key.name)) // FIX: _indicators check fails whenever _indicators is empty because "show animated" is unchecked
            continue;
        if(mouseHighlightRectScaled.intersects(keyRect))
            newSelection.setBit(i);
    }

    update();
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
    mouseHighlightRect = QRectF();
    // Emit signal with the names of the keys
    QStringList selectedNames;
    int i = 0;
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
        // Sidelights can't be selected, neither can the back LED for the M95
        if(strcmp(key.name, "lsidel") && strcmp(key.name, "rsidel") && keyMap.model() != KeyMap::M95
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
        if(!strcmp(key.toLatin1(), "lsidel") || !strcmp(key.toLatin1(), "rsidel") || keyMap.model() == KeyMap::M95)
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

void KeyWidget::drawLogo(const Key* key, QPainter* painter){
    float lx = key->x + drawInfoOffset.x() - key->width / 2.f + 2.f;
    float ly = key->y + drawInfoOffset.y() - key->height / 2.f + 2.f;
    float lw = key->width - 4.f;
    float lh = key->height - 4.f;
    QPainterPath logo;
    logo.moveTo(lx*drawInfoScale,(ly+lh)*drawInfoScale);
    logo.quadTo((lx+2.f)*drawInfoScale,(ly+lh/2.f)*drawInfoScale,lx*drawInfoScale,ly*drawInfoScale);
    logo.quadTo((lx+lw)*drawInfoScale,ly*drawInfoScale,(lx+lw)*drawInfoScale,(ly+lh)*drawInfoScale);
    logo.quadTo((lx+lw/2.f)*drawInfoScale,(ly+lh-4.f)*drawInfoScale,lx*drawInfoScale,(ly+lh)*drawInfoScale);
    painter->drawPath(logo);
}

void KeyWidget::drawBottomRightCorner(QPainter* painter, float x, float y, float w, float h, float scale){
    QPointF edgePoints[6] = {
        QPointF( x*scale,           y*scale),
        QPointF((x + w)*scale,      y*scale),
        QPointF((x + w)*scale,     (y + h)*scale),
        QPointF((x - h + w)*scale, (y + h)*scale),
        QPointF((x - h + w)*scale, (y + h - w)*scale),
        QPointF( x*scale,          (y + h - w)*scale),
    };
    painter->drawPolygon(edgePoints, 6);
}

void KeyWidget::drawBottomLeftCorner(QPainter* painter, float x, float y, float w, float h, float scale){
    QPointF edgePoints[6] = {
        QPointF( x*scale,            y*scale),
        QPointF((x + w)*scale,      y*scale),
        QPointF((x + w)*scale,     (y + h - w)*scale),
        QPointF((x + h)*scale,     (y + h - w)*scale),
        QPointF((x + h)*scale,     (y + h)*scale),
        QPointF( x*scale,          (y + h)*scale),
    };
    painter->drawPolygon(edgePoints, 6);
}

void KeyWidget::drawTopRightCorner(QPainter* painter, float x, float y, float w, float h, float scale){
    QPointF edgePoints[6] = {
        QPointF( x*scale,           y*scale),
        QPointF((x + w)*scale,      y*scale),
        QPointF((x + w)*scale,     (y + w)*scale),
        QPointF((x + w - h)*scale, (y + w)*scale),
        QPointF((x + w - h)*scale, (y + h)*scale),
        QPointF( x*scale,          (y + h)*scale),
    };
    painter->drawPolygon(edgePoints, 6);
}

void KeyWidget::drawTopLeftCorner(QPainter* painter, float x, float y, float w, float h, float scale){
    QPointF edgePoints[6] = {
        QPointF( x*scale,           y*scale),
        QPointF((x + w)*scale,      y*scale),
        QPointF((x + w)*scale,     (y + h)*scale),
        QPointF((x + h)*scale,     (y + h)*scale),
        QPointF((x + h)*scale,     (y + w)*scale),
        QPointF( x*scale,          (y + w)*scale),
    };
    painter->drawPolygon(edgePoints, 6);
}

void KeyWidget::drawStrafeSidelights(const Key* key, QPainter* painter, const QColor& kC, const QColor& c, const QColor& bgC){
    float kx = key->x + drawInfoOffset.x() - key->width / 2.f + 1.f;
    float ky = key->y + drawInfoOffset.y() - key->height / 2.f + 1.f;
    float kw = key->width - 2.f;
    float kh = key->height - 2.f;
    int wWidth = width(), wHeight = height();

    QRadialGradient gradient(QPointF(wWidth/2.f, wHeight/2.f), wWidth/2.f);
    gradient.setColorAt(0, c);
    gradient.setColorAt(0.9, c); // bring up intensity
    gradient.setColorAt(1, bgC);
    painter->setBrush(QBrush(gradient));
    painter->setPen(QPen(kC, 1.2)); //QColor(125,125,125)
    painter->drawRect(QRectF(kx * drawInfoScale, ky * drawInfoScale - 12 , kw * drawInfoScale, kh * drawInfoScale+24)); // don't really know why the 12 and 24 make it work here, but they do
    painter->setPen(QPen(QColor(0,0,0,0), 1));

}

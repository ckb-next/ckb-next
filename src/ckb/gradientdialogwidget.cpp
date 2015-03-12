#include <cmath>
#include <QPainter>
#include <QPropertyAnimation>
#include "gradientdialogwidget.h"

GradientDialogWidget::GradientDialogWidget(QWidget *parent) :
    QWidget(parent), selectedPos(-1), _current(-1)
{
    setMouseTracking(true);
}

void GradientDialogWidget::setStops(const QGradientStops& stops){
    _stops = stops;
    _colors.clear();
    foreach(const QGradientStop& stop, stops){
        double pos = stop.first;
        if(pos < 0. || pos > 1.)
            continue;
        _colors[round(pos * 100.)] = stop.second;
    }
    _current = 0;
#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
    emit currentChanged(_colors.first(), false, 0);
#else
    emit currentChanged(_colors.value(_colors.keys().first()), false, 0);
#endif
    update();
}

void GradientDialogWidget::makeStops(){
    _stops.clear();
    QMap<int, QColor> colors = selectionColors();
    // If there's only a single color, put it at the beginning and end
    if(colors.count() == 1){
#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
        QColor color = colors.first();
#else
        QColor color = colors.value(colors.keys().first());
#endif
        _stops.append(QGradientStop(0., color));
        _stops.append(QGradientStop(1., color));
        update();
        return;
    }
    // Add colors at 0 and 100 if needed
#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
    if(!colors.contains(0))
        colors[0] = colors.first();
    if(!colors.contains(100))
        colors[100] = colors.last();
#else
    if(!colors.contains(0))
        colors[0] = colors.value(colors.keys().first());
    if(!colors.contains(100))
        colors[100] = colors.value(colors.keys().last());
#endif
    QMapIterator<int, QColor> i(colors);
    while(i.hasNext()){
        i.next();
        _stops.append(QGradientStop(i.key() / 100., i.value()));
    }
    update();
}

QColor GradientDialogWidget::colorAt(int position){
    // Find which stops the position is between
    qreal pos = position / 100.;
    QVectorIterator<QGradientStop> i(_stops);
    QGradientStop previous = i.next();
    while(i.hasNext()){
        QGradientStop next = i.next();
        if(next.first >= pos){
            // Get color by linear interpolation. Premultiply the alpha value so that it returns the expected color
            // (i.e. stops with zero opacity won't contribute to color)
            QColor c1 = next.second, c2 = previous.second;
            qreal distance = next.first - previous.first;
            qreal dx = (pos - previous.first) / distance;
            qreal a1 = c1.alphaF(), a2 = c2.alphaF();
            qreal a3 = a1 * dx + a2 * (1.f - dx);
            if(a3 == 0.){
                c2.setAlpha(0);
                return c2;
            }
            qreal r = (c1.redF() * a1 * dx + c2.redF() * a2 * (1.f - dx)) / a3;
            qreal g = (c1.greenF() * a1 * dx + c2.greenF() * a2 * (1.f - dx)) / a3;
            qreal b = (c1.blueF() * a1 * dx + c2.blueF() * a2 * (1.f - dx)) / a3;
            return QColor::fromRgbF(r, g, b, a3);
        }
        previous = next;
    }
    return QColor();
}

int GradientDialogWidget::moveCurrent(int to){
    if(_current < 0 || to == _current || selected.isValid())
        return _current;
    // If there's a point in the way, skip past it by continuing in the current direction
    if(_current < to){
        while(_colors.contains(to)){
            to++;
            if(to > 100)
                return _current;
        }
    } else {
        while(_colors.contains(to)){
            to--;
            if(to < 0)
                return _current;
        }
    }
    _colors[to] = _colors.take(_current);
    _current = to;
    makeStops();
    return to;
}

void GradientDialogWidget::setCurrentColor(const QColor& color){
    if(_current < 0 || selected.isValid())
        return;
    QRgb rgb = _colors.value(_current).rgb();
    _colors[_current] = color;
    // If any points follow with the same color but different opacity, change them to match
    QMutableMapIterator<int, QColor> i(_colors);
    while(i.hasNext()){
        i.next();
        if(i.key() <= _current)
            continue;
        QColor& value = i.value();
        if(value.rgb() != rgb)
            break;
        int alpha = value.alpha();
        value = color;
        value.setAlpha(alpha);
    }
    makeStops();
}

QRect GradientDialogWidget::fillRect(){
    int x = 8, y = 1;
    int w = (width() - x * 2) / 16 * 16, h = (height() - 24) / 16 * 16;
    return QRect(x, y, w, h);
}

QRect GradientDialogWidget::stopRect(int pos){
    QRect fill = fillRect();
    int sx = round(pos / 100. * fill.width()) + fill.x() - 6;
    int sw = 12, sh = 18;
    int sy = height() - sh - 2;
    return QRect(sx, sy, sw, sh);
}

int GradientDialogWidget::selectedStop(){
    QRect rect = fillRect();
    int res = round((selectedPos - rect.x()) * 100. / rect.width());
    if(res < 0 || res > 100)
        return -1;
    return res;
}

QMap<int, QColor> GradientDialogWidget::selectionColors(){
    if(!selected.isValid())
        return _colors;
    // Determine selected stop position
    int selPos = selectedStop();
    // Re-add if in range
    QMap<int, QColor> res = _colors;
    if(selPos >= 0 && selPos <= 100)
        res[selPos] = selected;
    return res;
}

void GradientDialogWidget::paintEvent(QPaintEvent*){
    QPainter painter(this);
    QRect fill = fillRect();
    int x = fill.x(), y = fill.y(), w = fill.width(), h = fill.height();

    // Draw border around gradient
    painter.setPen(palette().color(QPalette::Dark));
    painter.drawLine(x - 1, y - 1, x + w, y - 1);
    painter.drawLine(x - 1, y - 1, x - 1, y + h);
    painter.setPen(palette().color(QPalette::Light));
    painter.drawLine(x - 1, y + h, x + w, y + h);
    painter.drawLine(x + w, y - 1, x + w, y + h);
    // Draw background grid
    for(int i = 0; i < w; i += 16){
        for(int iy = 0; iy < h; iy += 16){
            if(i % 32 != iy % 32)
                painter.fillRect(x + i, y + iy, 16, 16, QColor(192, 192, 192));
            else
                painter.fillRect(x + i, y + iy, 16, 16, QColor(255, 255, 255));
        }
    }
    // Draw gradient
    painter.setRenderHint(QPainter::Antialiasing);
    QLinearGradient gradient(x, 0, x + w, 0);
    gradient.setStops(_stops);
    painter.fillRect(fill, QBrush(gradient));

    // Draw stops
    QMapIterator<int, QColor> i(_colors);
    painter.setPen(palette().color(QPalette::Shadow));
    while(i.hasNext()){
        i.next();
        painter.setBrush(QBrush(i.value()));
        int index = i.key();
        if(index == _current){
            // Highlight currently-edited stop
            painter.setPen(QPen(palette().color(QPalette::Highlight), 2));
            painter.drawRoundedRect(stopRect(index), 4, 4);
            painter.setPen(palette().color(QPalette::Shadow));
        } else
            painter.drawRoundedRect(stopRect(index), 4, 4);
    }
    // Draw selection (if any)
    if(selected.isValid() && selectedPos >= x && selectedPos <= x + w){
        painter.setPen(QPen(palette().color(QPalette::Highlight), 2.5));
        painter.setBrush(selected);
        painter.drawRoundedRect(selectedPos - 6, height() - 18 - 2, 12, 18, 4, 4);
    }
}

void GradientDialogWidget::mousePressEvent(QMouseEvent* event){
    int x = event->x(), y = event->y();
    if(y < height() - 24)
        return;
    QMapIterator<int, QColor> i(_colors);
    while(i.hasNext()){
        i.next();
        // If a point was selected, remove it from the color map and set it as the selected point
        QRect stop = stopRect(i.key());
        if(x >= stop.left() - 1 && x <= stop.right() + 1){
            selected = i.value();
            selectedPos = stop.x() + stop.width() / 2;
            selectedOffset = selectedPos - x;
            _colors.remove(i.key());
            // On right click, delete the point
            if(event->button() == Qt::RightButton && _colors.count() > 1){
                selectedPos = -1;
                mouseReleaseEvent(event);
            } else
                update();
            return;
        }
    }
    // If nothing was selected, create a new point
    QRect fill = fillRect();
    if(x > fill.left() && x < fill.right() && event->button() != Qt::RightButton){
        selectedPos = x;
        selectedOffset = 0;
        selected = colorAt(selectedStop());
        makeStops();
    }
}

void GradientDialogWidget::mouseMoveEvent(QMouseEvent* event){
    if(!selected.isValid()){
        if(event->y() >= height() - 24)
            setCursor(QCursor(Qt::PointingHandCursor));
        else
            setCursor(QCursor(Qt::ArrowCursor));
        return;
    }
    // Move selected point (if any)
    setCursor(QCursor(Qt::ClosedHandCursor));
    selectedPos = event->x() + selectedOffset;
    // Allow the point to be deleted as long as there is at least 1 other point
    bool last = (_colors.count() == 0);
    // Hug the left/right edges
    QRect rect = fillRect();
    int left = rect.left(), right = rect.right();
    if(selectedPos < left && (last || selectedPos >= left - 30))
        selectedPos = left;
    else if(selectedPos > right && (last || selectedPos <= right + 30))
        selectedPos = right;
    // Remove point if it's too far above/below the widget
    int top = -30, bottom = height() + 30;
    if(!last && (event->y() < top || event->y() > bottom))
        selectedPos = -1;
    makeStops();
}

void GradientDialogWidget::mouseReleaseEvent(QMouseEvent* event){
    if(selected.isValid()){
        // Rejoin selection with gradient
        _current = selectedStop();
        _colors = selectionColors();
        emit currentChanged(_colors.value(_current), true, _current);
        selectedPos = -1;
        selected = QColor();
        makeStops();
    }
    if(event->y() >= height() - 24)
        setCursor(QCursor(Qt::PointingHandCursor));
    else
        setCursor(QCursor(Qt::ArrowCursor));
}

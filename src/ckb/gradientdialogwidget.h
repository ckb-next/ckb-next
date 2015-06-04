#ifndef GRADIENTDIALOGWIDGET_H
#define GRADIENTDIALOGWIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include <QPaintEvent>

class GradientDialogWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GradientDialogWidget(QWidget *parent = 0);

    void                    setStops(const QGradientStops& stops);
    inline QGradientStops   stops()                                 { return _stops; }
    inline int              stopCount()                             { return _stops.count(); }

    QColor colorAt(int position);

    inline int  current()                           { return _current; }
    int         moveCurrent(int to);
    void        setCurrentColor(const QColor& color);

signals:
    void currentChanged(QColor color, bool spontaneous, int position);

private:
    QColor  selected;
    int     selectedPos, selectedOffset;
    int     _current;
    QMap<int, QColor>   _colors;
    QGradientStops      _stops;

    QRect fillRect();
    QRect stopRect(int pos);
    int selectedStop();
    QMap<int, QColor> selectionColors();
    void makeStops();

    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
};

#endif // GRADIENTDIALOGWIDGET_H

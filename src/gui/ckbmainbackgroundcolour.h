#ifndef CKBMAINBACKGROUNDCOLOUR_H
#define CKBMAINBACKGROUNDCOLOUR_H
#include <QColor>
#include <QPixmap>
#include <QTabWidget>
#include <QDebug>

// This is awful.
// QOpenGLWidget can't be rendered transparently, so the background needs to be filled with something.
// The QPalette background doesn't always correspond to the colour the platform plugin will render with.
// We create a blank QTabWidget, take a "screenshot" of it, and then take the colour of a pixel.
// This way we are indirectly asking the platform plugin what the background colour should be, so that it can be used with the QOpenGLWidget.

// Do not call init() with a direct parent of a QOpenGLWidget as that breaks AA/multisampling for some reason...
class CkbMainBackgroundColour {
public:
    static const QColor& getColour() {
        if(c.isValid())
            return c;
        qDebug() << "Returning default colour in CkbMainBackgroundColour. This is expected for KeyWidgetDebugger.";
        return unknown;
    }
    static void init(QWidget* parent) {
        if(c.isValid())
            return;
        QTabWidget* t = new QTabWidget(parent);
        t->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        t->setFixedSize(200, 200);
        t->setAutoFillBackground(true);

        QPixmap p = t->grab();
        c = p.toImage().pixelColor(100, 180);
        delete t;
        // This can be used for debugging.
        // It pops out a window containing the "screenshot" of the QTabWidget.
        /*QLabel* lab = new QLabel;
        lab->setPixmap(p);
        lab->show();*/
    }

private:
    static QColor c;
    static const QColor unknown;
};

#endif // CKBMAINBACKGROUNDCOLOUR_H

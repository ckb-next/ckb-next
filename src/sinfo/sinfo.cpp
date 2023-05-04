#include <QGuiApplication>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOffscreenSurface>
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QGuiApplication a(argc, argv);

    QJsonObject renderer;

    QOpenGLContext ctx;
    ctx.create();

    QOffscreenSurface s;
    s.create();

    if(ctx.makeCurrent(&s)) {
        QOpenGLFunctions* gl = ctx.functions();

        QString vstr = QString::fromUtf8(reinterpret_cast<const char*>(gl->glGetString(GL_VENDOR)));
        QString rstr = QString::fromUtf8(reinterpret_cast<const char*>(gl->glGetString(GL_RENDERER)));

        renderer.insert("vendor", vstr);
        renderer.insert("renderer", rstr);
    }

    QJsonObject gpu;
    gpu.insert("gpu", renderer);

    QJsonDocument doc(gpu);

    QTextStream ts(stdout);
    ts << doc.toJson(QJsonDocument::Compact) << '\n';

    QMetaObject::invokeMethod(&a, "quit", Qt::QueuedConnection);
    return a.exec();
}

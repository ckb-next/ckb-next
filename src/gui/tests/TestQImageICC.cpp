#include "TestQImageICC.h"
#include <QImage>
#include <QFileInfo>
#include <QDir>

// Not great, but it'll do...
static QString msgs;

void checkForICCMessages(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    if(!strcmp(context.category, "qt.gui.icc"))
        msgs.append(msg);
}

void TestQImageICC::initTestCase()
{
    QLoggingCategory::setFilterRules("qt.gui.icc.warning=true");
    qInstallMessageHandler(checkForICCMessages);
}

void TestQImageICC::testResourceImages_data()
{
    QTest::addColumn<QString>("resource");

    QDir dir(":/img");
    QFileInfoList list = dir.entryInfoList();
    for(const QFileInfo& fi : list)
        QTest::newRow(fi.filePath().toLocal8Bit().constData()) << fi.absoluteFilePath();
}

void TestQImageICC::testResourceImages()
{
    QFETCH(QString, resource);

    QImage img;
    img.load(resource);

    // Early cleanup
    QString ICCFailMsg(msgs);
    msgs.clear();

    QVERIFY2(ICCFailMsg.isEmpty(), qPrintable(ICCFailMsg));
}

QTEST_APPLESS_MAIN(TestQImageICC)

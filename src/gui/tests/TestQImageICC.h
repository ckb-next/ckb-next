#include <QObject>
#include <QtTest/QtTest>

class TestQImageICC : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void testResourceImages_data();
    void testResourceImages();
};

#ifndef CKBSYSTEMQUIRKS_H
#define CKBSYSTEMQUIRKS_H
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

// Default to 8x MSAA
#define DEFAULT_MSAA 8

// This whole class is pain and suffering
// Also, none of this is thread safe
class CkbSystemQuirks
{
public:
    static void init();

    static inline int getMaxMSAA() {
        init();
        return glMSAA;
    }

    static QString getGlVendor() {
        init();
        return glVendor;
    }

    static QString getGlRenderer() {
        init();
        return glRenderer;
    }

private:
    static int glMSAA;
    static QString glVendor;
    static QString glRenderer;
    static const QString utility;
    struct intelMSAA {
        QString shortName;
        QString longName;
        int maxMSAA;
    };
    static const QList<intelMSAA> msaaForceList;
    static int checkIntelGPU(const QString& codename);
};

#endif // CKBSYSTEMQUIRKS_H

#include "ckbsystemquirks.h"
#include "ckbnextconfig.h"
#include <QDir>
#include <QFileInfo>

int CkbSystemQuirks::glMSAA = -1;
QString CkbSystemQuirks::glVendor;
QString CkbSystemQuirks::glRenderer;
const QString CkbSystemQuirks::utility = QStringLiteral("ckb-next-sinfo");

// At least with Mesa 23.0.1, 8x breaks the UI on SNB
// Assume older chips will be worse (whether it's a mesa bug or not),
// and IVB is close enough, so force 4x on that too
const QList<CkbSystemQuirks::intelMSAA> CkbSystemQuirks::msaaForceList {
    { QStringLiteral("ivb"), QStringLiteral("ivybridge"),   4 },
    { QStringLiteral("snb"), QStringLiteral("sandybridge"), 4 },
    { QStringLiteral("ilk"), QStringLiteral("ironlake"),    0 },
    { QStringLiteral("elk"), QStringLiteral("eaglelake"),   0 },
    { QStringLiteral("ctg"), QStringLiteral("cantiga"),     0 },
    { QStringLiteral("brw"), QStringLiteral("broadwater"),  0 }, // Guess
};

int CkbSystemQuirks::checkIntelGPU(const QString& codename) {
    for(const intelMSAA& iGPU : msaaForceList) {
        if(!QString::compare(iGPU.shortName, codename, Qt::CaseInsensitive) ||
                glRenderer.contains(iGPU.longName, Qt::CaseInsensitive)) {
            glMSAA = iGPU.maxMSAA;
            return 1;
        }
    }
    return 0;
}

extern const char* argv0;
void CkbSystemQuirks::init() {
    if(glMSAA != -1)
        return;

    // This is intentionally up here.
    // If we fail to run ckb-next-sinfo the first time, then don't keep trying for every request
    glMSAA = DEFAULT_MSAA;

    // Crude argv[0] parsing because QApplication is not yet available
    // If it fails, oh well...
    QFileInfo fi(argv0);
    QDir d = fi.dir();

    QProcess p;
    p.start(d.filePath(utility));
    // Note: On some UNIX operating systems, this function may return true
    // but the process may later report a QProcess::FailedToStart error.
    // ^ Applies to waitForFinished() too as it internally calls waitForStarted() in this scenario
    bool started = p.waitForFinished(5000);

    if(!started || p.error() == QProcess::FailedToStart) {
        d.setPath(CKB_NEXT_UTILITIES_PATH);
        p.start(d.filePath(utility));
        started = p.waitForFinished(5000);
    }
    if(!started || p.error() != QProcess::UnknownError || p.exitStatus() != QProcess::NormalExit ||
            p.exitCode() != 0) {
        qDebug() << "ckb-next-sinfo failed" << started << p.error() <<
                    p.exitStatus() << p.exitCode() << p.readAllStandardError();
        return;
    }

    QByteArray out = p.readAllStandardOutput();
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(out, &err);
    if(err.error != QJsonParseError::NoError) {
        qDebug() << err.errorString() << err.error;
        return;
    }

    // It's okay if these don't exist. Worst case we get empty strings
    QJsonObject gpu = doc.object().value("gpu").toObject();
    glRenderer = gpu.value("renderer").toString();
    glVendor = gpu.value("vendor").toString();

    // Sandy Bridge (and possibly earlier) have trouble with 8x MSAA
    // Force 4x instead

    // Some strings:
    // Mesa DRI Intel(R) G45/G43 (ELK)
    // Mesa DRI Mobile Intel GM45 Express (CTG)
    // Mesa Intel(R) HD Graphics 3000 (SNB GT2)
    // Mesa DRI Intel(R) Sandybridge Mobile

    // First, check for either Intel or Intel Open Source Technology Center
    if(glVendor.startsWith(QStringLiteral("intel"), Qt::CaseInsensitive)) {
        // First try the short codenames in parentheses
        QRegularExpression re(QStringLiteral("\\(([A-Za-z]+).*?\\)"));

        QRegularExpressionMatchIterator iter = re.globalMatch(glRenderer);
        while(iter.hasNext()) {
            QRegularExpressionMatch m = iter.next();
            QString codename = m.captured(1);
            if(checkIntelGPU(codename))
                break;
        }
    } else if(glRenderer.startsWith(QStringLiteral("llvmpipe"), Qt::CaseInsensitive) ||
              glRenderer.startsWith(QStringLiteral("softpipe"), Qt::CaseInsensitive)) {
        glMSAA = 0;
    }

    if(glMSAA != DEFAULT_MSAA)
        qDebug() << "Limiting MSAA to" << glMSAA <<  "because of" << glVendor << glRenderer;

}

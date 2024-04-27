#include "ckbversionnumber.h"

#include <QtGlobal>
#include <QObject>

static inline bool isAsciiDigitString(const QString& str){
    for(const QChar& c : str)
        if(c < '0' || c > '9')
            return false;
    return true;
}

static inline int FWBcdToBin(const uchar& v){
    return ((int)(v >> 4) * 10) + (v & 0xF);
}

QString CkbVersionNumber::toString() const {
    if(ver.isNull())
        return QString(QObject::tr("N/A"));
    QString vstring = ver.toString();
    // HACK: Format old version numbers the same way CUE does
    if(ver.segmentCount() == 2 && ver.minorVersion() < 10){
        int pos = vstring.indexOf(QChar('.'));
        if(pos >= 0)
            vstring.insert(pos + 1, QChar('0'));
    }
    vstring.append(suffix);
    return vstring;
}

CkbVersionNumber::CkbVersionNumber(const QByteArray& fwver) : ckbTooOld(false) {
    // FIXME: Probably not endianness aware
    if(fwver.length() == 2) {
        ver = QVersionNumber(FWBcdToBin(fwver.at(1)), FWBcdToBin(fwver.at(0)));
    } else {
        qWarning() << "Invalid fwver length in CkbVersionNumber constructor";
        ver = QVersionNumber();
    }
}

CkbVersionNumber::CkbVersionNumber(const QString& vstr) : ckbTooOld(false) {
    // FIXME: Remove this after a few versions. It's only here for backwards compat
    if(vstr.length() == 4 && isAsciiDigitString(vstr)){
        bool ok;
        // This is... not great.
        ushort uver = vstr.toUShort(&ok, 16);
        const uchar* usver = reinterpret_cast<const uchar*>(&uver);
        ver = QVersionNumber(FWBcdToBin(usver[1]), FWBcdToBin(usver[0]));
        return;
    }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    int i;
#else
    qsizetype i;
#endif

    ver = QVersionNumber::fromString(vstr, &i);
    if(!ver.isNull())
        suffix = vstr.mid(i);
}

#ifndef CKBVERSIONNUMBER_H
#define CKBVERSIONNUMBER_H

#include <QVersionNumber>
#include <QDebug>

class CkbVersionNumber {
    QVersionNumber ver;
    bool ckbTooOld;
    QString suffix;
public:
    CkbVersionNumber() : ver(QVersionNumber()), ckbTooOld(false) {}
    CkbVersionNumber(const QByteArray& fwver);
    CkbVersionNumber(const QString& vstr);

    QString toString() const;
    bool isNull() const {
        return ver.isNull();
    }
    bool CkbTooOld() const {
        return ckbTooOld;
    }
    void setCkbTooOld() {
        ckbTooOld = true;
    }

    // version suffixes are not compared using the operators
    friend inline bool operator>(const CkbVersionNumber& lhs, const CkbVersionNumber& rhs);
    friend inline bool operator>=(const CkbVersionNumber& lhs, const CkbVersionNumber& rhs);
    friend inline bool operator<(const CkbVersionNumber& lhs, const CkbVersionNumber& rhs);
    friend inline bool operator<=(const CkbVersionNumber& lhs, const CkbVersionNumber& rhs);
    friend inline bool operator==(const CkbVersionNumber& lhs, const CkbVersionNumber& rhs);
    friend inline bool operator!=(const CkbVersionNumber& lhs, const CkbVersionNumber& rhs);

    // This takes into account the version string too
    bool isEqualVerStr(const CkbVersionNumber& other) const {
        return ver == other.ver && suffix == other.suffix;
    }
};

// Comparison operators
inline bool operator>(const CkbVersionNumber& lhs, const CkbVersionNumber& rhs) {
    return lhs.ver > rhs.ver;
}

inline bool operator>=(const CkbVersionNumber& lhs, const CkbVersionNumber& rhs) {
    return lhs.ver >= rhs.ver;
}

inline bool operator<(const CkbVersionNumber& lhs, const CkbVersionNumber& rhs) {
    return lhs.ver < rhs.ver;
}

inline bool operator<=(const CkbVersionNumber& lhs, const CkbVersionNumber& rhs) {
    return lhs.ver <= rhs.ver;
}

inline bool operator==(const CkbVersionNumber& lhs, const CkbVersionNumber& rhs) {
    return lhs.ver == rhs.ver;
}

inline bool operator!=(const CkbVersionNumber& lhs, const CkbVersionNumber& rhs) {
    return lhs.ver != rhs.ver;
}

Q_DECLARE_METATYPE(CkbVersionNumber)


#endif // CKBVERSIONNUMBER_H

#ifndef MACROLINE_H
#define MACROLINE_H
#include <QString>

class MacroLine {
public:
    static const int MACRO_DELAY_DEFAULT = -1;
    MacroLine(){
        key = QString();
        usTime = usTimeMax = MACRO_DELAY_DEFAULT;
        keyDown = true;
    }
    MacroLine(const QString& _key, const qint64& _usTime, const qint64& _usTimeMax, const bool _keyDown) :
        key(_key), usTime(_usTime), usTimeMax(_usTimeMax), keyDown(_keyDown) {}
    QString key;
    qint64 usTime;
    qint64 usTimeMax; // If this is set, then this is a random delay
    bool keyDown;
    inline bool isPrintable() const{
        return key.length() == 1;
    }

    // for qDebug()
    operator QString() const { return key + (keyDown ? "d" : "u"); }
};

#endif // MACROLINE_H

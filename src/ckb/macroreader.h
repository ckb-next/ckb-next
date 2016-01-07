#ifndef MACROREADER_H
#define MACROREADER_H

//#include <QStandardPaths>
#include <QObject>
#include <QFile>
#include <QThread>
#include <QPlainTextEdit>

// Class for managing G-key macros

class MacroReaderThread : public QThread
{
    Q_OBJECT
    int macroNumber;
    QString macroPath;
    QPlainTextEdit* macroBox;
    QPlainTextEdit* macroText;

public:
    MacroReaderThread(int macNum, QString macPath, QPlainTextEdit* macBox, QPlainTextEdit* macText) {
        macroNumber = macNum;
        macroPath = macPath;
        macroBox = macBox;
        macroText = macText;
    }

    // Notification reader, launches as a separate thread and reads from file.
    // (QFile doesn't have readyRead() so there's no other way to do this asynchronously)
    void run Q_DECL_OVERRIDE ();

private slots:
    void readMacro(QString line);
};

class MacroReader : public QThread
{
    Q_OBJECT

public:
    MacroReader();
    MacroReader(int macroNumber, QString macroPath, QPlainTextEdit* macBox, QPlainTextEdit* macText);
    ~MacroReader();

    void startWorkInAThread(int macroNumber, QString macroPath, QPlainTextEdit* macBox, QPlainTextEdit* macText);

signals:

public slots:

private slots:

private:
    MacroReaderThread* macroReaderThread;
};

#endif // MACROREADER_H

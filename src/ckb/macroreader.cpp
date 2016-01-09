#include <fcntl.h>
#include <qdebug.h>
#include "macroreader.h"

MacroReader::MacroReader() {
    qDebug() << "Calling MacroReader without params is not allowed.";
}

MacroReader::MacroReader(int macroNumber, QString macroPath, QPlainTextEdit* macBox, QPlainTextEdit* macText) {
    macText->setFocus();    // we want to see the keys as they appear in the macroText Widget
    startWorkInAThread(macroNumber, macroPath, macBox, macText);
}

MacroReader::~MacroReader() {
}

void MacroReader::startWorkInAThread(int macroNumber, QString macroPath, QPlainTextEdit* macBox, QPlainTextEdit* macText) {
    macroReaderThread = new MacroReaderThread(macroNumber, macroPath, macBox, macText);
    connect(macroReaderThread, &MacroReaderThread::finished, macroReaderThread, &QObject::deleteLater);
    macroReaderThread->start();
}

//////////////////////
/// \brief MacroReaderThread::readMacro
/// \param line as QString
///
/// That method ist called wia signal from the worker thread,
/// which reads the keyboard input.
/// Just display the key code in the macroBox Widget
/// without he trailing \n
///
void MacroReaderThread::readMacro(QString line) {
    // we want to see the keys as they appear in the macroText Widget
    // Because it is possible to change the Focus via keyboard,
    // we must set the focus on each call.
    macroText->setFocus();
    QTextCursor c = macroText->textCursor();
    c.setPosition(macroText->toPlainText().length());
    macroText->setTextCursor(c);
    macroBox->appendPlainText(line.left(line.size()-1));
}

void MacroReaderThread::run() {
    qDebug() << "MacroReader::run() started with" << macroNumber << "and" << macroPath << "and" << macroBox << "and" << macroText;

    QFile macroFile(macroPath);
    // Wait a small amount of time for the node to open (100ms) (code reused from kb.cpp)
    QThread::usleep(100000);
    if(!macroFile.open(QIODevice::ReadOnly)){
        // If it's still not open, try again before giving up (1s at a time, 10s total)
        QThread::usleep(900000);
        for(int i = 1; i < 10; i++){
            if(macroFile.open(QIODevice::ReadOnly))
                break;
            QThread::sleep(1);
        }
        if(!macroFile.isOpen()) {
            qDebug() << QString("unable to open macroFile (%1)").arg(macroPath);
            return;
        }
    }
    // Read data from notification node macroPath
    QByteArray line;
    while(macroFile.isOpen() && (line = macroFile.readLine()).length() > 0){
        QString text = QString::fromUtf8(line);
        metaObject()->invokeMethod(this, "readMacro", Qt::QueuedConnection, Q_ARG(QString, text));
    }
    qDebug() << "MacroReader::run() ends.";
    macroFile.close();
    QThread::exit ();
}

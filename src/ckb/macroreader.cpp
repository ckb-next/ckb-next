#include <qdebug.h>
#include "macroreader.h"
#include <sys/time.h>

//////////
/// \class MacroReader
///

MacroReader::MacroReader() {
    qDebug() << "Calling MacroReader without params is not allowed.";
}

MacroReader::MacroReader(int macroNumber, QString macroPath, QPlainTextEdit* macBox, QPlainTextEdit* macText) {
    startWorkInAThread(macroNumber, macroPath, macBox, macText);
}
MacroReader::~MacroReader() {}

void MacroReader::startWorkInAThread(int macroNumber, QString macroPath, QPlainTextEdit* macBox, QPlainTextEdit* macText) {
    macroReaderThread = new MacroReaderThread(macroNumber, macroPath, macBox, macText);
    connect(macroReaderThread, &MacroReaderThread::finished, macroReaderThread, &QObject::deleteLater);
    macroReaderThread->start();
}

//////////
/// \class MacroReaderThread
///
void MacroReaderThread::readMacro(QString line) {
    /// \details We want to see the keys as they appear in the macroText Widget.
    ///
    /// Because it is possible to change the Focus via keyboard,
    /// we must set the focus on each call.
    macroText->setFocus();
    QTextCursor c = macroText->textCursor();
    c.setPosition(macroText->toPlainText().length());
    macroText->setTextCursor(c);
    macroBox->appendPlainText(line.left(line.size()-1));
}

//////////
/// \brief MacroReaderThread::run is the standard main function for a thread.
/// Tries to open a file \<macroPath\>\<macroNumber\> several times
/// (in this case, it should be possible the first time (the code was recycled from kb.cpp).
///
/// While the file is open, read lines an signal them via metaObject() to the main thread.
/// When the file is closed by the sender, close it as reader and terminate.
///
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
    // Count time between lines read from the interface
    QByteArray line;
    timeval t;
    gettimeofday(&t, NULL);
    double tstart = t.tv_sec+(t.tv_usec/1000000.0);
    bool firstline = true;

    while(macroFile.isOpen() && (line = macroFile.readLine()).length() > 0){
        QString text = QString::fromUtf8(line);
        gettimeofday(&t, NULL);
        double tnow = t.tv_sec+(t.tv_usec/1000000.0);

		// in the first line, there is only a delay "before start". Don't use it.
        if (!firstline) {
            text.prepend ("\n");
            text.prepend (QString::number ((tnow - tstart) * 1000000.0, 'f', 0));
            text.prepend ("=");
        } else firstline = false;
        tstart = tnow;

        metaObject()->invokeMethod(this, "readMacro", Qt::QueuedConnection, Q_ARG(QString, text));
    }
    qDebug() << "MacroReader::run() ends.";
    macroFile.close();
    QThread::exit ();
}

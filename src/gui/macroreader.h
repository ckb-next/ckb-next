#ifndef MACROREADER_H
#define MACROREADER_H

//#include <QStandardPaths>
#include <QObject>
#include <QFile>
#include <QThread>
#include <QPlainTextEdit>

//////////
/// \brief The MacroReaderThread class is responsible for reading Macro Key Values.
/// It is created as a separate thread (worker thread) for reading macro commands from an fresh opened notify channel.
/// Standard notify channel for macro definitions is number 2.
///
/// While the worker Thread gets input from the keyboard,
/// the lines are sent via signalling (metaobject) to run a member function in the context of the Qt UI manager.
///
/// When the notify channel is closed (that's normally done by pressing "Stop"-Button in the UI),
/// the worker thread closes the channelFile and leaves.
/// \sa MacroReaderThread(), ~MacroReaderThread(), readMaco(), run()
///
class MacroReaderThread : public QThread
{
    Q_OBJECT

    //////////
    /// \brief macroNumber
    /// Filenames of nofity channels have the structure <input-device-path>/ckb1/notify\<number\>
    /// First part is hold in macroPath, the number is hold in macroNumber.
    /// macroNumber may range from 0 to 9.
    int macroNumber;

    //////////
    /// \brief macroPath holds the path for the notify channel
    /// \see macroNumber
    QString macroPath;

    //////////
    /// \brief macroBox will receive the Macro Key Values sent from the keyboard while defining a new macro.
    QPlainTextEdit* macroBox;

    //////////
    /// \brief macroText is the other textpane used in the UI while typing new macros.
    /// That variable is used for setting the focus to that textpane and to set the cursor at EOT.
    QPlainTextEdit* macroText;

public:
    //////////
    /// \brief MacroReaderThread saves the four params to local vars with similar varNames.
    /// \param macNum
    /// \param macPath
    /// \param macBox
    /// \param macText
    ///
    MacroReaderThread(int macNum, QString macPath, QPlainTextEdit* macBox, QPlainTextEdit* macText) {
        macroNumber = macNum;
        macroPath = macPath;
        macroBox = macBox;
        macroText = macText;
    }

    //////////
    /// \brief run is the notification reader main loop.
    void run () Q_DECL_OVERRIDE;

private slots:
    //////////
    /// \brief readMacro is called for each line received by the worker thread.
    /// The method ist called via signal (metaObject) from the worker thread,
    /// which reads the keyboard input.
    /// Just display the key code in the macroBox Widget without he trailing newline
    /// and reposition the cursor in the macro pane.
    ///
    /// This is used, because the worker thread shouldn't get access to the UI elements
    /// (and normally has none, because the pointers macroBox and macroText remain on the stack).
    /// That mechanism guarantees, that the UI does not freeze if it happens something magic to the reading function.
    ///
    /// \param line holds the line just got from keyboard

    void readMacro(QString line);
};

//////////
/// \brief The MacroReader class creates a worker thread object.
/// It does a connect do delayed deletion of thread local variables in the case the worker thread terminates.
///
class MacroReader : public QThread
{
    Q_OBJECT

public:
    //////////
    /// \brief MacroReader Calling MacroReader without params is not allowed.
    ///
    MacroReader();
    //////////
    /// \brief MacroReader This is the only allowed constructor.
    /// It only calls startWorkInAThread() with the four params.
    /// \param macroNumber
    /// \param macroPath
    /// \param macBox
    /// \param macText
    ///
    MacroReader(int macroNumber, QString macroPath, QPlainTextEdit* macBox, QPlainTextEdit* macText);
    ~MacroReader();

    //////////
    /// \brief startWorkInAThread This member function creates the new thread object and starts it with the four params.
    /// \param macroNumber
    /// \param macroPath
    /// \param macBox
    /// \param macText
    ///
    void startWorkInAThread(int macroNumber, QString macroPath, QPlainTextEdit* macBox, QPlainTextEdit* macText);

signals:

public slots:

private slots:

private:
    MacroReaderThread* macroReaderThread;
};

#endif // MACROREADER_H

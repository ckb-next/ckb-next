#include <QElapsedTimer>
#include <QDebug>
#include "macroreader.h"
#include <QThread>
#include <QMetaEnum>
#include <QFlags>
#include "macroline.h"

qint64 MacroReader::keyStrokeTime(){
    qint64 elapsed;
    if(timer){
        elapsed = timer->nsecsElapsed() / 1000;
    } else {
        elapsed = MacroLine::MACRO_DELAY_DEFAULT;
        timer = new QElapsedTimer;
    }
    timer->start();
    return elapsed;
}

void MacroReader::macroDataReceived(QLocalSocket* f){
    while(f->bytesAvailable()){
        char macroKeyData[MACRO_ARRAY_LEN];
        qint64 dataRead = f->readLine(macroKeyData, MACRO_ARRAY_LEN);

        // Make sure there are enough characters for a basic key "key +a\n"
        if(dataRead < 7)
            return;

        // Remove the \n and terminate the string
        macroKeyData[dataRead - 1] = '\0';

        const char firstChar = macroKeyData[4];
        bool down;
        if(firstChar == '+')
            down = true;
        else if(firstChar == '-')
            down = false;
        else
            return; // If it doesn't start with + or -, don't do anything

        // Skip over the "key +"
        const char* key = macroKeyData + 5;
        QString line = QString::fromUtf8(key);
        qint64 elapsed = keyStrokeTime();
        emit macroLineRead(line, elapsed, down);
    }
}

MacroReader::MacroReader(const QStringList& macroPaths) : timer(nullptr){

    // Wait a small amount of time for the nodes to open (100ms)
    // We shouldn't block the main event loop for too long
    QThread::msleep(100);

    const int mlength = macroPaths.length();
    for(int i = 0; i < mlength; i++){
        QFile* ptr = new QFile(macroPaths.at(i));
        if(!ptr->open(QIODevice::ReadOnly)){
            qDebug() << "Unable to open" << macroPaths.at(i);
            delete ptr;
            continue;
        }
        fhandles.append(ptr);
        QLocalSocket* sock = new QLocalSocket();
        connect(sock, &QLocalSocket::readyRead, this, [=] () { macroDataReceived(sock); });
        if(!sock->setSocketDescriptor(ptr->handle(), QLocalSocket::ConnectedState, QIODevice::ReadOnly))
            qDebug() << "Error setting socket descriptor";
        fnotifiers.append(sock);
    }
}

MacroReader::~MacroReader() {
    for(int i = 0; i < fhandles.length(); i++){
        delete fnotifiers[i];
        delete fhandles[i];
    }
    delete timer;
}

// Just a single key
#define SK(keycode, string) case keycode: \
                                return string

// Key with modifier. If modifier is pressed, string2 is returned
#define SKN(keycode, string, modifier, string2) case keycode: \
                                return ((modifiers & modifier) ? string2 : string)

// Key with modifier without fallback.
#define SKM(keycode, modifier, string) SKN(keycode, nullptr, modifier, string)

static inline const char* translateQtKeyCode(int keycode, Qt::KeyboardModifiers modifiers){
    switch(keycode){

    // First row
    SK(Qt::Key_Escape, "esc");
    SK(Qt::Key_F1, "f1");
    SK(Qt::Key_F2, "f2");
    SK(Qt::Key_F3, "f3");
    SK(Qt::Key_F4, "f4");
    SK(Qt::Key_F5, "f5");
    SK(Qt::Key_F6, "f6");
    SK(Qt::Key_F7, "f7");
    SK(Qt::Key_F8, "f8");
    SK(Qt::Key_F9, "f9");
    SK(Qt::Key_F10, "f10");
    SK(Qt::Key_F11, "f11");
    SK(Qt::Key_F12, "f12");
    SK(Qt::Key_SysReq, "prtscn");
    SK(Qt::Key_Print, "prtscn");
    SK(Qt::Key_ScrollLock, "scroll");
    SK(Qt::Key_Pause, "pause");

    // Add media keys
    SK(Qt::Key_MediaPlay, "play");
    SK(Qt::Key_VolumeDown, "voldn");
    SK(Qt::Key_VolumeUp, "volup");
    SK(Qt::Key_VolumeMute, "mute");
    SK(Qt::Key_MediaStop, "stop");
    SK(Qt::Key_MediaPrevious, "prev");
    SK(Qt::Key_MediaNext, "next");

    // Second row
    SK(Qt::Key_QuoteLeft, "grave");
    SKM(Qt::Key_AsciiTilde, Qt::ShiftModifier, "grave");

    SKN(Qt::Key_1, "1", Qt::KeypadModifier, "num1");
    SKN(Qt::Key_2, "2", Qt::KeypadModifier, "num2");
    SKN(Qt::Key_3, "3", Qt::KeypadModifier, "num3");
    SKN(Qt::Key_4, "4", Qt::KeypadModifier, "num4");
    SKN(Qt::Key_5, "5", Qt::KeypadModifier, "num5");
    SK(Qt::Key_Clear, "num5");
    SKN(Qt::Key_6, "6", Qt::KeypadModifier, "num6");
    SKN(Qt::Key_7, "7", Qt::KeypadModifier, "num7");
    SKN(Qt::Key_8, "8", Qt::KeypadModifier, "num8");
    SKN(Qt::Key_9, "9", Qt::KeypadModifier, "num9");
    SKN(Qt::Key_0, "0", Qt::KeypadModifier, "num0");
    SKN(Qt::Key_Minus, "minus", Qt::KeypadModifier, "numminus");
    SKN(Qt::Key_Plus, "equal", Qt::KeypadModifier, "numplus");
    SK(Qt::Key_Equal, "equal");
    SK(Qt::Key_Backspace, "bspace");

    // Shift second row
    SKM(Qt::Key_Exclam, Qt::ShiftModifier, "1");
    SKM(Qt::Key_At, Qt::ShiftModifier, "2");
    SKM(Qt::Key_NumberSign, Qt::ShiftModifier, "3");
    SKM(Qt::Key_Dollar, Qt::ShiftModifier, "4");
    SKM(Qt::Key_Percent, Qt::ShiftModifier, "5");
    SKM(Qt::Key_AsciiCircum, Qt::ShiftModifier, "6");
    SKM(Qt::Key_Ampersand, Qt::ShiftModifier, "7");
    SKM(Qt::Key_ParenLeft, Qt::ShiftModifier, "9");
    SKM(Qt::Key_ParenRight, Qt::ShiftModifier, "0");
    SKM(Qt::Key_Underscore, Qt::ShiftModifier, "minus");

    SKN(Qt::Key_Insert, "ins", Qt::KeypadModifier, "num0");
    SKN(Qt::Key_Home, "home", Qt::KeypadModifier, "num7");
    SKN(Qt::Key_PageUp, "pgup", Qt::KeypadModifier, "num9");

    SK(Qt::Key_NumLock, "numlock");

    SKN(Qt::Key_Slash, "slash", Qt::KeypadModifier, "numslash");
    SKN(Qt::Key_Delete, "del", Qt::KeypadModifier, "numdot");
    SKN(Qt::Key_End, "end", Qt::KeypadModifier, "num1");
    SKN(Qt::Key_PageDown, "pgdn", Qt::KeypadModifier, "num3");

    case Qt::Key_Asterisk:
        if(modifiers & Qt::KeypadModifier)
            return "numstar";
        if(modifiers & Qt::ShiftModifier)
            return "8";
        return nullptr;

    // Third row
    SK(Qt::Key_Tab, "tab");
    SK(Qt::Key_Q, "q");
    SK(Qt::Key_W, "w");
    SK(Qt::Key_E, "e");
    SK(Qt::Key_R, "r");
    SK(Qt::Key_T, "t");
    SK(Qt::Key_Y, "y");
    SK(Qt::Key_U, "u");
    SK(Qt::Key_I, "i");
    SK(Qt::Key_O, "o");
    SK(Qt::Key_P, "p");
    SK(Qt::Key_BracketLeft, "lbrace");
    SK(Qt::Key_BracketRight, "rbrace");
    SK(Qt::Key_Backslash, "bslash");

    // Shift
    SKM(Qt::Key_BraceLeft, Qt::ShiftModifier, "lbrace");
    SKM(Qt::Key_BraceRight, Qt::ShiftModifier, "rbrace");
    SKM(Qt::Key_Bar, Qt::ShiftModifier, "bslash");

    // Fourth row
    SK(Qt::Key_CapsLock, "caps");
    SK(Qt::Key_A, "a");
    SK(Qt::Key_S, "s");
    SK(Qt::Key_D, "d");
    SK(Qt::Key_F, "f");
    SK(Qt::Key_G, "g");
    SK(Qt::Key_H, "h");
    SK(Qt::Key_J, "j");
    SK(Qt::Key_K, "k");
    SK(Qt::Key_L, "l");
    SK(Qt::Key_Semicolon, "colon");
    SK(Qt::Key_Apostrophe, "quote");
    SK(Qt::Key_Return, "enter");

    // Shift
    SKM(Qt::Key_Colon, Qt::ShiftModifier, "colon");
    SKM(Qt::Key_QuoteDbl, Qt::ShiftModifier, "quote");

    // Fifth row
    // We can't differentiate between left and right modifiers...
    SK(Qt::Key_Shift, "lshift");
    SK(Qt::Key_Z, "z");
    SK(Qt::Key_X, "x");
    SK(Qt::Key_C, "c");
    SK(Qt::Key_V, "v");
    SK(Qt::Key_B, "b");
    SK(Qt::Key_N, "n");
    SK(Qt::Key_M, "m");
    SKN(Qt::Key_Up, "up", Qt::KeypadModifier, "num8");
    SK(Qt::Key_Enter, "numenter");
    SK(Qt::Key_Comma, "comma");
    SKN(Qt::Key_Period, "dot", Qt::KeypadModifier, "numdot");

    // Shift
    SKM(Qt::Key_Less, Qt::ShiftModifier, "comma");
    SKM(Qt::Key_Greater, Qt::ShiftModifier, "dot");
    SKM(Qt::Key_Question, Qt::ShiftModifier, "slash");

    // Sixth row
    SK(Qt::Key_Control, "lctrl");
    SK(Qt::Key_Super_L, "lwin");
    SK(Qt::Key_Meta, "lwin"); // For some reason this is super?
    SK(Qt::Key_Alt, "lalt");
    SK(Qt::Key_Space, "space");
    SK(Qt::Key_Super_R, "rwin"); // This doesn't get called
    SK(Qt::Key_Menu, "rmenu");
    SKN(Qt::Key_Left, "left", Qt::KeypadModifier, "num4");
    SKN(Qt::Key_Down, "down", Qt::KeypadModifier, "num2");
    SKN(Qt::Key_Right, "right", Qt::KeypadModifier, "num6");
    SK(Qt::Key_Multi_key, "rmenu"); // Rmenu isn't correct for compose key, as it can be rebound
    SKM(Qt::Key_AltGr, Qt::GroupSwitchModifier, "ralt");

    default:
        return nullptr;
    }
}

void MacroReader::translateQKeyEvent(int keycode, bool down, Qt::KeyboardModifiers modifiers){
    const char* keystr = translateQtKeyCode(keycode, modifiers);
    if(!keystr){
        QMetaEnum metaEnum = QMetaEnum::fromType<QFlags<Qt::KeyboardModifier>>();
        QString modifierstr = QString(metaEnum.valueToKeys(modifiers));
        metaEnum = QMetaEnum::fromType<Qt::Key>();
        QString keycodestr = metaEnum.valueToKey(keycode);
        if(keycodestr.isEmpty())
            keycodestr = QString("keycode %1").arg(keycode);
        emit macroReadError(keycodestr, modifierstr);
        return;
    }
    QString line = QString::fromUtf8(keystr);
    qint64 elapsed = keyStrokeTime();
    emit macroLineRead(line, elapsed, down);
}
